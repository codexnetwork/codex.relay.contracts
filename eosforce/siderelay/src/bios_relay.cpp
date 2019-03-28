#include <siderelay.hpp>

ACTION siderelay::initworker( capi_name chain, capi_name worker_typ, capi_name worker, uint64_t power, const permission_level& permission ) {
   print("initworker ", chain, " ", worker, "\n");
   require_auth(_self);

   auto itr = workergroups.find(chain);
   if( itr == workergroups.end() ) {
      workergroups.emplace(_self, [&]( auto& u ) {
         u.group_name = name{chain};
         u.requested_names.emplace_back(worker);
         u.requested_powers.emplace_back(power);
         u.requested_approvals.emplace_back(permission);
         u.power_sum = power;
      });

      // init workstate
      workstate_table workstat( _self, chain );
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

ACTION siderelay::cleanworker( capi_name chain ) {
   //print("cleanworker ", chain, "\n");
   require_auth(_self);

   auto itr = workergroups.find(chain);
   if( itr != workergroups.end() ) {
      workergroups.modify(itr, _self, [&]( auto& row ) {
         row.clear_workers();
      });
   }
}
