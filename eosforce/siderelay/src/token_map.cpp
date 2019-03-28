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

// from relay chain to side
ACTION siderelay::out( capi_name committer, uint64_t num, capi_name to, name chain, name contract, const asset& quantity, const std::string& memo ) {
   print("out ", committer, " ", chain, " ", to, " - ", quantity, "\n");

   const auto& workergroup = workergroups.get(chain.value, "chain channel no find");
   const auto account = workergroup.check_permission( committer );

   workstate_table worker_states( _self, chain.value );
   auto states_itr = worker_states.find(token_map_typ.value);
   eosio_assert(states_itr != worker_states.end(), "chain work states no find");
   eosio_assert(num > states_itr->confirmed_num, "action by num has committed");

   outaction_table outs(_self, chain.value);
   auto itr = outs.find(num);

   auto is_confirmed = false;
   const auto act_commit = outaction_data{
         to, chain, contract, quantity, memo
   };

   if( itr == outs.end() ) {
      outs.emplace(account, [&]( auto& row ) {
         row.num = num;
         is_confirmed = row.commit(committer, workergroup, act_commit);
      });
   } else {
      outs.modify(itr, account, [&]( auto& row ) {
         is_confirmed = row.commit(committer, workergroup, act_commit);
      });
   }

   if( is_confirmed ) {
      worker_states.modify(states_itr, account, [&]( auto& row ) {
         row.confirmed_num = num;
      });
      do_out(name{to}, chain, quantity, memo);
   }
}

void siderelay::do_out( const name& to, const name& chain, const asset& quantity, const std::string& memo ) {
   print("do_out ", to, ",", chain, ",", quantity, ",", memo, "\n");
   eosforce::send_transfer_core_token(_self, to, quantity, memo );
}