#include <chain.hpp>

void chainspec::send_token( const eosio::name& contract,
                            const eosio::name& action_name,
                            const eosio::name& from,
                            const eosio::name& to,
                            const eosio::asset& quantity,
                            const std::string& memo ) {
   eosio::action{
      std::vector<eosio::permission_level>{{from, "active"_n}},
      contract,
      action_name,
      chainspec::transfer_args{
         from, to, quantity, memo
      }
   }.send();
}