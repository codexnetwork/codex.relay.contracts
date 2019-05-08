#include <siderelay.hpp>

// worker manager

// change transfers
ACTION siderelay::chworker( name committer, uint64_t num, name work_typ, name old, name worker, uint64_t power, const permission_level& permission ) {
   //print("chworker ", committer, " ", worker, " from ", old, "\n");

   if( !WORK_CHECK( chworker, committer, 
            name{work_typ}, name{old}, name{worker}, power, permission ) ){
      return;
   }

   auto itr = workergroups.find(work_typ.value);
   check(itr != workergroups.end(), "no found chain channel");
   workergroups.modify(itr, _self, [&]( auto& row ) {
      if( old != worker ) {
         row.del_worker(old);
      }
      row.modify_worker(worker, power, permission);
   });
}