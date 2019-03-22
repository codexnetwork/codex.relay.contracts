#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;

CONTRACT siderelay : public contract {
   public:
   using contract::contract;

   // TODO Support diff token contracts

   // from side chain to relay
   ACTION in( capi_name from, uint64_t id, name chain, capi_name to, asset quantity, const std::string& memo );

   // from relay chain to side
   ACTION out( capi_name committer, uint64_t id, capi_name to, name chain, asset quantity, const std::string& memo );

   // change transfers
   ACTION chtransfers( capi_name committer, uint64_t id, const std::vector<capi_name>& transfers );

   void ontransfer( capi_name from, capi_name to, asset quantity, std::string memo );

   using in_action = action_wrapper<"in"_n, &siderelay::in>;
   using out_action = action_wrapper<"out"_n, &siderelay::out>;
   using chtransfers_action = action_wrapper<"chtransfers"_n, &siderelay::chtransfers>;
};