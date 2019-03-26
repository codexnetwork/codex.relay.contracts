#include <siderelay.hpp>

// from side chain to relay
ACTION siderelay::in( capi_name from, name chain, capi_name to, asset quantity, const std::string& memo ){
   print("in ", from, " ", to, " ", quantity, "\n");
}

// from relay chain to side
ACTION siderelay::out( capi_name committer, capi_name to, name chain, asset quantity, const std::string& memo ){
   print("out ", committer, " ", chain, " ", to, " - ", quantity, "\n");
}

// change transfers
ACTION siderelay::chworker( capi_name committer, const name& chain, const name& worker, const uint64_t power, const permission_level& permission ){
   print("chworker ", committer, " ", worker, "\n");
}

ACTION siderelay::initworker( const name& chain, const name& worker, const uint64_t power, const permission_level& permission ){
   print("initworker ", chain, " ", worker, "\n");
   require_auth(_self);
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
            EOSIO_DISPATCH_HELPER( siderelay, (out)(chworker)(initworker) )
         }
      }
   }
}