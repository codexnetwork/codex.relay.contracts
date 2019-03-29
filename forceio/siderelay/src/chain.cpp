#include <chain.hpp>

void eosforce::send_transfer_core_token( const eosio::name& from,
                                         const eosio::name& to,
                                         const eosio::asset& quantity,
                                         const std::string& memo ) {
   eosio::action{
      std::vector<eosio::permission_level>{{from, "active"_n}},
      "force.token"_n,
      "transfer"_n,
      eosforce::transfer_args{
         from, to, quantity, memo
      }
   }.send();
}