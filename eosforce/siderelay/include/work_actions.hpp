#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

#pragma once

using namespace eosio;

// outaction_data data for out action to commit
struct outaction_data {
public:
   capi_name         to;
   name              chain;
   name              contract;
   asset             quantity;
   std::string       memo;
   std::vector<name> confirmed;

   outaction_data() = default;
   ~outaction_data() = default;
   outaction_data( const outaction_data& ) = default;
   outaction_data( capi_name to, const name& chain,
                   const name& contract, const asset& quantity,
                   const std::string& memo )
      : to(to)
      , chain(chain)
      , contract(contract)
      , quantity(quantity)
      , memo(memo)
   {}

   friend constexpr bool operator == ( const outaction_data& a, const outaction_data& b ) {
      return std::tie( a.to, a.chain, a.contract, a.quantity, a.memo )
          == std::tie( b.to, b.chain, b.contract, b.quantity, b.memo );
   }

   EOSLIB_SERIALIZE( outaction_data, (to)(chain)(contract)(quantity)(memo)(confirmed) )
};