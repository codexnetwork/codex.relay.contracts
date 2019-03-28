#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

#pragma once

using namespace eosio;

// outaction_data data for out action to commit
struct outaction_data {
public:
   capi_name         to;
   name              chain;
   name              contract;
   asset             quantity;
   std::string       memo;
   std::vector<name> confirmed;

   outaction_data() = default;
   ~outaction_data() = default;
   outaction_data( const outaction_data& ) = default;
   outaction_data( capi_name to, const name& chain,
                   const name& contract, const asset& quantity,
                   const std::string& memo )
      : to(to)
      , chain(chain)
      , contract(contract)
      , quantity(quantity)
      , memo(memo)
   {}

   friend constexpr bool operator == ( const outaction_data& a, const outaction_data& b ) {
      return std::tie( a.to, a.chain, a.contract, a.quantity, a.memo )
          == std::tie( b.to, b.chain, b.contract, b.quantity, b.memo );
   }

   EOSLIB_SERIALIZE( outaction_data, (to)(chain)(contract)(quantity)(memo)(confirmed) )
};

// now just for token map
CONTRACT siderelay : public contract {
public:
   using contract::contract;
   siderelay( name receiver, name code, datastream<const char*> ds )
      : contract(receiver, code, ds)
      , workergroups(receiver, receiver.value) {}

   // TODO Support diff token contracts

   constexpr static auto token_map_typ = "map.token"_n;

   // from side chain to relay
   ACTION in( uint64_t num,  capi_name to, const asset& quantity, const std::string& memo );

   // from relay chain to side
   ACTION out( capi_name committer, uint64_t num, capi_name to, name chain, name contract, const asset& quantity, const std::string& memo );

   // change transfers
   ACTION chworker( capi_name committer, capi_name chain, capi_name old, capi_name worker, uint64_t power, const permission_level& permission );

   // change worker in bios stage
   ACTION initworker( capi_name chain, capi_name worker_typ, capi_name worker, uint64_t power, const permission_level& permission );

   ACTION cleanworker( capi_name chain );

   TABLE workersgroup {
      public:
         name group_name;
         std::vector<name>             requested_names;
         std::vector<uint64_t>         requested_powers;
         std::vector<permission_level> requested_approvals;
         uint64_t power_sum = 0;

      private:
         int get_idx_by_name( capi_name worker ) const;

      public:
         name check_permission( capi_name worker ) const;
         void modify_worker( capi_name worker, uint64_t power, const permission_level& permission );
         void clear_workers();
         void del_worker( capi_name worker );
         bool is_confirm_ok( const std::vector<name>& confirmed ) const;

      public:
         uint64_t primary_key()const { return group_name.value; }
   };
   typedef eosio::multi_index< "workersgroup"_n, workersgroup > workersgroup_table;
   workersgroup_table workergroups;

   TABLE workstate {
         uint64_t    confirmed_num;
         name        type;

         uint64_t primary_key()const { return type.value; }
   };
   typedef eosio::multi_index< "workstates"_n, workstate > workstate_table;

   TABLE outaction {
         using action_data_typ = outaction_data;
         using action_data_vec = std::vector<action_data_typ>;

         uint64_t        num = 0;
         action_data_vec actions;

         bool commit( capi_name committer, const workersgroup& workers, const action_data_typ& act );
         uint64_t primary_key() const { return num; }
   };
   typedef eosio::multi_index< "outactions"_n, outaction > outaction_table;

public:
   void ontransfer( capi_name from, capi_name to, const asset& quantity, const std::string& memo );
   
private:
   template< typename Action_Table_T, typename Action_T >
   bool commit_work_then_check( capi_name committer, uint64_t num, const name& chain, const name& work_typ, const Action_T& act_commit );

public:
   using in_action = action_wrapper<"in"_n, &siderelay::in>;
   using out_action = action_wrapper<"out"_n, &siderelay::out>;
};

// commit_action_imp imp to commit actions
template<typename T, typename K>
bool commit_action_imp( K& actions,
                        capi_name committer,
                        const siderelay::workersgroup& workers,
                        const T& commit_act ) {
   const auto committer_name = eosio::name{ committer };
   auto commit_act_itr = actions.end();

   for( auto itr = actions.begin(); itr != actions.end(); ++itr ) {
      if( commit_act == *itr ) {
         // if has committed
         for( const auto& co : itr->confirmed ) {
            if( co == committer_name ) {
               return false;
            }
         }
         // find
         commit_act_itr = itr;
         break;
      }
   }

   // new commit
   if( commit_act_itr == actions.end() ) {
      actions.push_back(commit_act);
      commit_act_itr = actions.end() - 1; // last line keep actions.size() >= 1
   }

   commit_act_itr->confirmed.push_back(committer_name);
   const auto is_ok = workers.is_confirm_ok(commit_act_itr->confirmed);
   if( is_ok ) {
      if( actions.size() > 1 ) {
         actions[0] = *commit_act_itr;
         actions.resize(1);
      }
   }
   return is_ok;
}

template< typename Action_Table_T, typename Action_T >
bool siderelay::commit_work_then_check( capi_name committer, uint64_t num, const name& chain, const name& work_typ, const Action_T& act_commit ) {
   const auto& workergroup = workergroups.get(chain.value, "chain channel no find");
   const auto account = workergroup.check_permission( committer );

   workstate_table worker_states( _self, chain.value );
   auto states_itr = worker_states.find(work_typ.value);
   eosio_assert(states_itr != worker_states.end(), "chain work states no find");
   eosio_assert(num > states_itr->confirmed_num, "action by num has committed");

   Action_Table_T acts_table(_self, chain.value);
   auto itr = acts_table.find(num);

   auto is_confirmed = false;

   if( itr == acts_table.end() ) {
      acts_table.emplace(account, [&]( auto& row ) {
         row.num = num;
         is_confirmed = row.commit(committer, workergroup, act_commit);
      });
   } else {
      acts_table.modify(itr, account, [&]( auto& row ) {
         is_confirmed = row.commit(committer, workergroup, act_commit);
      });
   }

   if( is_confirmed ) {
      worker_states.modify(states_itr, account, [&]( auto& row ) {
         row.confirmed_num = num;
      });
   }

   return is_confirmed;
}