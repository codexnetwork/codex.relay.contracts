#include <siderelay.hpp>

ACTION siderelay::initworker( capi_name worker_typ, capi_name worker, uint64_t power, const permission_level& permission ) {
   require_auth(_self);

   auto itr = workergroups.find(worker_typ);
   if( itr == workergroups.end() ) {
      workergroups.emplace(_self, [&]( auto& u ) {
         u.group_name = name{worker_typ};
         u.requested_names.emplace_back(worker);
         u.requested_powers.emplace_back(power);
         u.requested_approvals.emplace_back(permission);
         u.power_sum = power;
      });

      // init workstate
      workstate_table workstat( _self, _self.value );
      workstat.emplace(_self, [&]( auto& u ) {
         u.type = name{worker_typ};
         u.confirmed_num = 0;
      });

   } else {
      workergroups.modify(itr, _self, [&]( auto& row ) {
         row.modify_worker(worker, power, permission);
      });
   }
}

ACTION siderelay::cleanworker( capi_name work_typ ) {
   require_auth(_self);

   auto itr = workergroups.find(work_typ);
   if( itr != workergroups.end() ) {
      workergroups.modify(itr, _self, [&]( auto& row ) {
         row.clear_workers();
      });
   }
}