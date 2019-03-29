#include <eosiolib/eosio.hpp>
using namespace eosio;

CONTRACT siderelay : public contract {
   public:
      using contract::contract;

      ACTION hi( name nm );

      using hi_action = action_wrapper<"hi"_n, &siderelay::hi>;
};