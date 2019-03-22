#include <siderelay.hpp>

// from side chain to relay
ACTION siderelay::in( capi_name from, uint64_t id, name chain, capi_name to, asset quantity, const std::string& memo ){
   print("in ", from, " ", to, " ", quantity, "\n");
}

// from relay chain to side
ACTION siderelay::out( capi_name committer, uint64_t id, capi_name to, name chain, asset quantity, const std::string& memo ){
   print("out ", committer, " ", id, " ", to, " - ", quantity, "\n");
}

// change transfers
ACTION siderelay::chtransfers( capi_name committer, uint64_t id, const std::vector<capi_name>& transfers ){
   print("chtransfers ", committer, " ", id, "\n");
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
   in.send(from, 1, "eosforce"_n, to, quantity, memo);
}

extern "C" {
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
      if ((name(code) == "eosio"_n) && (name(action) == "transfer"_n)) {
         execute_action( name(receiver), name(action), &siderelay::ontransfer );
         return;
      }

      if( code == receiver ) {
         switch( action ) {
            EOSIO_DISPATCH_HELPER( siderelay, (out)(chtransfers) )
         }
      }
   }
}