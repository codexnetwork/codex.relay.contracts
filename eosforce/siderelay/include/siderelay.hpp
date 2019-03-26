#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;

// now just for token map
CONTRACT siderelay : public contract {
   public:
   using contract::contract;

   // TODO Support diff token contracts

   // from side chain to relay
   ACTION in( capi_name from, name chain, capi_name to, asset quantity, const std::string& memo );

   // from relay chain to side
   ACTION out( capi_name committer, capi_name to, name chain, asset quantity, const std::string& memo );

   // change transfers
   ACTION chworker( capi_name committer, const name& chain, const name& worker, const uint64_t power, const permission_level& permission );

   ACTION initworker( const name& chain, const name& worker, const uint64_t power, const permission_level& permission );

   TABLE worksgroup {
      public:
         name group_name;
         std::vector<name>             requested_names;
         std::vector<uint64_t>         requested_powers;
         std::vector<permission_level> requested_approvals;
         uint64_t power_sum = 0;

      public:
         uint64_t check_permission( const name& worker );
         void modify_worker( const name& worker, const uint64_t power, const permission_level& permission );

      public:
         uint64_t primary_key()const { return group_name.value; }
   };
   typedef eosio::multi_index< "worksgroup"_n, worksgroup > worksgroup_table;

   void ontransfer( capi_name from, capi_name to, asset quantity, std::string memo );

   using in_action = action_wrapper<"in"_n, &siderelay::in>;
   using out_action = action_wrapper<"out"_n, &siderelay::out>;
   using chworker_action = action_wrapper<"chworker"_n, &siderelay::chworker>;
   using initworker_action = action_wrapper<"initworker"_n, &siderelay::initworker>;
};