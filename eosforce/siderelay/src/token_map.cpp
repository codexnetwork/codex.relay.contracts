#include <siderelay.hpp>
#include <eosforce.hpp>

bool siderelay::outaction::commit( capi_name committer,
                                   const workersgroup& workers,
                                   const outaction_data& commit_act ) {
   return ::commit_action_imp(actions, committer, workers, commit_act);
}

// from side chain to relay
ACTION siderelay::in( uint64_t num,  capi_name to, const asset& quantity, const std::string& memo ) {
   // print("in ", from, " ", to, " ", quantity, "\n");
   // TODO By FanYang check num if ok by relay chain
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

// from relay chain to side
ACTION siderelay::out( capi_name committer, uint64_t num, capi_name to, name chain, name contract, const asset& quantity, const std::string& memo ) {
   //print("out ", committer, " ", chain, " ", to, " - ", quantity, "\n");

   const auto is_confirmed = 
      commit_work_then_check<outaction_table, outaction_data>(
         committer, num, chain, token_map_typ, outaction_data{
            to, chain, contract, quantity, memo
         });

   if( is_confirmed ) {
      print("do_out ", name{to}, ",", chain, ",", quantity, ",", memo, "\n");
      eosforce::send_transfer_core_token(_self, name{to}, quantity, memo );
   }
}