#include <siderelay.hpp>

// from side chain to relay
ACTION siderelay::in( capi_name from, name chain, capi_name to, asset quantity, const std::string& memo ){
   print("in ", from, " ", to, " ", quantity, "\n");

   const auto itr = workergroups.get(chain.value, "chain channel no find");
}

// from relay chain to side
ACTION siderelay::out( capi_name committer, const uint64_t id, capi_name to, name chain, asset quantity, const std::string& memo ){
   print("out ", committer, " ", chain, " ", to, " - ", quantity, "\n");

   const auto itr = workergroups.get(chain.value, "chain channel no find");
}

// change transfers
ACTION siderelay::chworker( capi_name committer, const name& chain, const name& old, const name& worker, const uint64_t power, const permission_level& permission ){
   print("chworker ", committer, " ", worker, " from ", old, "\n");

   // TODO first commit then exec

   // TODO just exec in first version
   auto itr = workergroups.find(chain.value);
   eosio_assert(itr != workergroups.end(), "no found chain channel");
   workergroups.modify( itr, _self, [&]( auto& row ) {
      if( old != worker ) {
         row.del_worker(old);
      }
      row.modify_worker(worker, power, permission);
   });
}

ACTION siderelay::initworker( const name& chain, const name& worker, const uint64_t power, const permission_level& permission ){
   print("initworker ", chain, " ", worker, "\n");
   require_auth(_self);

   auto itr = workergroups.find(chain.value);
   if(itr == workergroups.end()) {
      workergroups.emplace( _self, [&]( auto& u ) {
            u.group_name = chain;
            u.requested_names.emplace_back(worker);
            u.requested_powers.emplace_back(power);
            u.requested_approvals.emplace_back(permission);
            u.power_sum = power;
      });
   } else {
      workergroups.modify( itr, _self, [&]( auto& row ) {
         row.modify_worker(worker, power, permission);
      });
   }
}

ACTION siderelay::cleanworker( const name& chain ){
   print("cleanworker ", chain, "\n");
   require_auth(_self);

   auto itr = workergroups.find(chain.value);
   if(itr != workergroups.end()) {
      workergroups.modify( itr, _self, [&]( auto& row ) {
         row.clear_workers();
      });
   }
}

void siderelay::ontransfer( capi_name from, capi_name to, asset quantity, std::string memo ){
   if (name(from) == _self || name(to) != _self) {
      return;
   }
   if ("NoProcessMemo" == memo) {
      return;
   }

   print("on transfer ", name(from), " -> ", name(to), " ", quantity, " by ", memo, "\n");

   siderelay::in_action in(_self, {_self, "active"_n});
   in.send(from, "eosforce"_n, to, quantity, memo);
}

extern "C" {
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
      if ((name(code) == "eosio"_n) && (name(action) == "transfer"_n)) {
         execute_action( name(receiver), name(action), &siderelay::ontransfer );
         return;
      }

      if( code == receiver ) {
         switch( action ) {
            EOSIO_DISPATCH_HELPER( siderelay, (out)(chworker)(initworker)(cleanworker) )
         }
      }
   }
}