#include <siderelay.hpp>
ACTION siderelay::hi( name nm ) {
   /* fill in action body */
   print_f("Name : %\n",nm);
}

EOSIO_DISPATCH( siderelay, (hi) )