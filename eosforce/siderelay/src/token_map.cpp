#include <siderelay.hpp>
#include <chain.hpp>

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