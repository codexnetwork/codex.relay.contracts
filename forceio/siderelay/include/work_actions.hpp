#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

#pragma once

using namespace eosio;

// outaction_data data for out action to commit
struct out_action_data {
public:
   capi_name         to;
   capi_name         chain;
   capi_name         contract;
   capi_name         action;
   asset             quantity;
   std::string       memo;
   std::vector<name> confirmed;

   friend constexpr bool operator == ( const out_action_data& a, const out_action_data& b ) {
      return std::tie( a.to, a.chain, a.contract, a.action, a.quantity, a.memo )
          == std::tie( b.to, b.chain, b.contract, b.action, b.quantity, b.memo );
   }

   EOSLIB_SERIALIZE( out_action_data, (to)(chain)(contract)(action)(quantity)(memo)(confirmed) )
};

// chworkeraction_data data for chworker action to commit
struct chworker_action_data {
public:
   name              chain;
   name              old;
   name              worker;
   uint64_t          power;
   permission_level  permission;

   std::vector<name> confirmed;

   friend constexpr bool operator == ( const chworker_action_data& a, const chworker_action_data& b ) {
      return std::tie( a.chain, a.old, a.worker, a.power, a.permission )
          == std::tie( b.chain, b.old, b.worker, b.power, b.permission );
   }

   EOSLIB_SERIALIZE( chworker_action_data, (chain)(old)(worker)(power)(permission)(confirmed) )
};