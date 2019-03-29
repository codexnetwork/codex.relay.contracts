#include <siderelay.hpp>

// worker manager

// change transfers
ACTION siderelay::chworker( capi_name committer, uint64_t num, capi_name work_typ, capi_name old, capi_name worker, uint64_t power, const permission_level& permission ) {
   //print("chworker ", committer, " ", worker, " from ", old, "\n");

   if( !WORK_CHECK( chworker, committer, 
            name{work_typ}, name{old}, name{worker}, power, permission ) ){
      return;
   }

   auto itr = workergroups.find(work_typ);
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
   // TODO FORCEIO allow change main symbol name, so it is need support config this
   if( (name(code) == "force.token"_n) && (name(action) == "transfer"_n) ) {
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