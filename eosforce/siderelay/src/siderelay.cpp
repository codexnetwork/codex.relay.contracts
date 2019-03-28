#include <siderelay.hpp>

// worker manager

// change transfers
ACTION siderelay::chworker( capi_name committer, capi_name chain, capi_name old, capi_name worker, uint64_t power, const permission_level& permission ) {
   print("chworker ", committer, " ", worker, " from ", old, "\n");

   // TODO first commit then exec

   // TODO just exec in first version
   auto itr = workergroups.find(chain);
   eosio_assert(itr != workergroups.end(), "no found chain channel");
   workergroups.modify(itr, _self, [&]( auto& row ) {
      if( old != worker ) {
         row.del_worker(old);
      }
      row.modify_worker(worker, power, permission);
   });
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