#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

#include <work.hpp>
#include <work_actions.hpp>

#pragma once

using namespace eosio;

CONTRACT siderelay : public contract {
public:
   using contract::contract;
   siderelay( name receiver, name code, datastream<const char*> ds )
      : contract(receiver, code, ds)
      , workergroups(receiver, receiver.value) {}

   // TODO Support diff token contracts

   // from side chain to relay
   ACTION in( uint64_t num, capi_name to, const asset& quantity, const std::string& memo );

   // from relay chain to side
   ACTION out( capi_name committer, 
               uint64_t num, 
               capi_name to, 
               capi_name chain, 
               capi_name contract, 
               capi_name action, 
               const asset& quantity, 
               const std::string& memo );

   // change transfers
   ACTION chworker( capi_name committer, 
                    uint64_t num, 
                    capi_name work_typ, 
                    capi_name old, 
                    capi_name worker, 
                    uint64_t power, 
                    const permission_level& permission );

   // change worker in bios stage
   ACTION initworker( capi_name worker_typ, 
                      capi_name worker, 
                      uint64_t power, 
                      const permission_level& permission );

   ACTION cleanworker( capi_name work_typ );

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



public:
   void ontransfer( capi_name from, capi_name to, const asset& quantity, const std::string& memo );
   
private:
   template< typename Action_Table_T, typename Action_T >
   bool commit_work_then_check( capi_name committer, uint64_t num, const name& work_typ, const Action_T& act_commit );

   template<typename T, typename K>
   static bool commit_action_imp( K& actions,
                           capi_name committer,
                           const workersgroup& workers,
                           const T& commit_act );

public:
   WORK_TYPE_TABLE_DEFINE(out)
   WORK_TYPE_TABLE_DEFINE(chworker)

public:
   using in_action = action_wrapper<"in"_n, &siderelay::in>;
   using out_action = action_wrapper<"out"_n, &siderelay::out>;
};

// commit_action_imp imp to commit actions
template<typename T, typename K>
bool siderelay::commit_action_imp( K& actions,
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
bool siderelay::commit_work_then_check( capi_name committer, uint64_t num, const name& work_typ, const Action_T& act_commit ) {
   const auto& workergroup = workergroups.get(work_typ.value, "work_typ channel no find");
   const auto account = workergroup.check_permission( committer );

   workstate_table worker_states( _self, _self.value );
   auto states_itr = worker_states.find(work_typ.value);
   eosio_assert(states_itr != worker_states.end(), "work_typ work states no find");
   eosio_assert(num > states_itr->confirmed_num, "action by num has committed");

   Action_Table_T acts_table(_self, work_typ.value);
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