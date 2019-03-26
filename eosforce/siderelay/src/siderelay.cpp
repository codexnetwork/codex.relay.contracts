#include <siderelay.hpp>

// from side chain to relay
ACTION siderelay::in( capi_name from, name chain, capi_name to, asset quantity, const std::string& memo ){
   print("in ", from, " ", to, " ", quantity, "\n");

   const auto itr = workergroups.get(chain.value, "chain channel no find");
}

// from relay chain to side
ACTION siderelay::out( capi_name committer, const uint64_t num, capi_name to, name chain, name contract, asset quantity, const std::string& memo ){
   print("out ", committer, " ", chain, " ", to, " - ", quantity, "\n");

   const auto& workergroup = workergroups.get(chain.value, "chain channel no find");
   const auto account = workergroup.check_permission(name{committer});

   auto outstates_itr = outstates.find(chain.value);
   eosio_assert(outstates_itr != outstates.end(), "chain outstates no find");
   eosio_assert(num > outstates_itr->confirmed_num, "action by num has committed");

   outaction_table outs(_self, chain.value);
   auto itr = outs.find(num);

   auto is_confirmed = false;
   const auto act_commit = outaction_data{
      to, chain, contract, quantity, memo // TODO can change by map
   };

   if( itr == outs.end() ) {
      outs.emplace( account, [&]( auto& row ) {
            row.num = num;
            is_confirmed = row.commit(committer, workergroup, act_commit);
      });
   } else {
      outs.modify( itr, account, [&]( auto& row ) {
         is_confirmed = row.commit(committer, workergroup, act_commit);
      });
   }

   if( is_confirmed ){
      do_out(to, chain, quantity, memo);
   }
}

void siderelay::do_out( capi_name to, name chain, const asset& quantity, const std::string& memo ){
   print("do_out ", to, ",", chain, ",", quantity, ",", memo, "\n");
}

name siderelay::workersgroup::check_permission( const name& worker ) const {
   const auto idx = get_idx_by_name(worker);
   eosio_assert((idx >= 0) && (idx < requested_names.size()), 
      "no found worker to check");
   require_auth(requested_approvals[idx]);

   return requested_approvals[idx].actor;
}

bool siderelay::outaction::commit( capi_name committer, 
                                   const workersgroup& workers, 
                                   const outaction_data& commit_act ) {
   for( auto& act : actions ){
      if( commit_act == act ){
         act.confirmed.push_back( name{ committer } );
         return workers.is_confirm_ok(act.confirmed);
      }
   }

   auto act = commit_act;
   act.confirmed.push_back( name{ committer } );
   actions.push_back( act );
   return workers.is_confirm_ok(act.confirmed);
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
      outstates.emplace( _self, [&]( auto& u ) {
            u.chain = chain;
            u.confirmed_num = 0;
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