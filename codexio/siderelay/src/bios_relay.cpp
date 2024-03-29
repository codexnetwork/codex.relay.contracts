#include <siderelay.hpp>

ACTION siderelay::initworker( name worker_typ, name worker, uint64_t power, const permission_level& permission ) {
   require_auth(_self);

   auto itr = workergroups.find( worker_typ.value );
   if( itr == workergroups.end() ) {
      workergroups.emplace( _self, [&]( auto& u ) {
         u.group_name = worker_typ;
         u.requested_names.emplace_back(worker);
         u.requested_powers.emplace_back(power);
         u.requested_approvals.emplace_back(permission);
         u.power_sum = power;
      } );

      // init workstate
      workstate_table workstat( _self, _self.value );
      auto stat_itr = workstat.find( worker_typ.value );
      if( stat_itr == workstat.end() ) {
         workstat.emplace(_self, [&]( auto& stat ) {
            stat.type = worker_typ;
            stat.confirmed_num = 0;
         });
      } else {
         workstat.modify(stat_itr, _self, [&]( auto& stat ) {
            stat.confirmed_num = 0;
         });
      }

   } else {
      workergroups.modify(itr, _self, [&]( auto& row ) {
         row.modify_worker(worker, power, permission);
      });
   }
}

ACTION siderelay::cleanworker( name work_typ ) {
   require_auth(_self);

   auto itr = workergroups.find(work_typ.value);
   if( itr != workergroups.end() ) {
      workergroups.modify(itr, _self, [&]( auto& row ) {
         row.clear_workers();
      });
   }
}

ACTION siderelay::resetworker( name work_typ ) {
   require_auth(_self);

   auto itr = workergroups.find( work_typ.value );
   if( itr != workergroups.end() ) {
      workergroups.modify(itr, _self, [&]( auto& row ) {
         row.clear_workers();
      });
   }

   // init workstate
   workstate_table workstat( _self, _self.value );
   auto stat_itr = workstat.find( work_typ.value );
   if( stat_itr != workstat.end() ) {
      workstat.modify(stat_itr, _self, [&]( auto& row ) {
         row.confirmed_num = 0;
      });
   }
}