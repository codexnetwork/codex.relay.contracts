#include <side.relay.hpp>
ACTION relay::hi( name nm ) {
   /* fill in action body */
   print_f("Name : %\n",nm);
}

EOSIO_DISPATCH( relay, (hi) )