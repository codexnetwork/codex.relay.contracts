#include <siderelay.hpp>

// change transfers
ACTION siderelay::chworker( capi_name committer, const name& chain, const name& old, const name& worker, const uint64_t power, const permission_level& permission ) {
   print("chworker ", committer, " ", worker, " from ", old, "\n");

   // TODO first commit then exec

   // TODO just exec in first version
   auto itr = workergroups.find(chain.value);
   eosio_assert(itr != workergroups.end(), "no found chain channel");
   workergroups.modify(itr, _self, [&]( auto& row ) {
      if( old != worker ) {
         row.del_worker(old);
      }
      row.modify_worker(worker, power, permission);
   });
}

void siderelay::ontransfer( capi_name from, capi_name to, asset quantity, std::string memo ) {
   if( name(from) == _self || name(to) != _self ) {
      return;
   }
   if( "NoProcessMemo" == memo ) {
      return;
   }

   print("on transfer ", name(from), " -> ", name(to), " ", quantity, " by ", memo, "\n");

   siderelay::in_action in(_self, { _self, "active"_n });
   in.send(from, "eosforce"_n, to, quantity, memo);
}

extern "C" {
void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
   if( (name(code) == "eosio"_n) && (name(action) == "transfer"_n) ) {
      execute_action(name(receiver), name(action), &siderelay::ontransfer);
      return;
   }

   if( code == receiver ) {
      switch( action ) {
         EOSIO_DISPATCH_HELPER(siderelay, (out)(chworker)(initworker)(cleanworker))
      }
   }
}
}