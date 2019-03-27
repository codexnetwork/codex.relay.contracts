#include <siderelay.hpp>
#include <eosforce.hpp>

bool siderelay::outaction::commit( capi_name committer,
                                   const workersgroup& workers,
                                   const outaction_data& commit_act ) {
   const auto committer_name = name{ committer };
   auto commit_act_itr = actions.end();

   for( auto itr = actions.begin(); itr != actions.end(); ++itr ) {
      if( commit_act == *itr ) {
         // if has committed
         for( const auto& co : itr->confirmed ) {
            if( co == committer_name ) {
               return false;
            }
         }
         // find
         commit_act_itr = itr;
         break;
      }
   }

   // new commit
   if( commit_act_itr == actions.end() ) {
      actions.push_back(commit_act);
      commit_act_itr = actions.end() - 1; // last line keep actions.size() >= 1
   }

   commit_act_itr->confirmed.push_back(committer_name);
   const auto is_ok = workers.is_confirm_ok(commit_act_itr->confirmed);
   if( is_ok ) {
      if( actions.size() > 1 ) {
         actions[0] = *commit_act_itr;
         actions.resize(1);
      }
   }
   return is_ok;
}

// from side chain to relay
ACTION siderelay::in( capi_name from, name chain, capi_name to, asset quantity, const std::string& memo ) {
   print("in ", from, " ", to, " ", quantity, "\n");

   const auto itr = workergroups.get(chain.value, "chain channel no find");
}

// from relay chain to side
ACTION siderelay::out( capi_name committer, const uint64_t num, capi_name to, name chain, name contract, asset quantity, const std::string& memo ) {
   print("out ", committer, " ", chain, " ", to, " - ", quantity, "\n");

   const auto& workergroup = workergroups.get(chain.value, "chain channel no find");
   const auto account = workergroup.check_permission(name{ committer });

   auto outstates_itr = outstates.find(chain.value);
   eosio_assert(outstates_itr != outstates.end(), "chain outstates no find");
   eosio_assert(num > outstates_itr->confirmed_num, "action by num has committed");

   outaction_table outs(_self, chain.value);
   auto itr = outs.find(num);

   auto is_confirmed = false;
   const auto act_commit = outaction_data{
         to, chain, contract, quantity, memo
   };

   if( itr == outs.end() ) {
      outs.emplace(account, [&]( auto& row ) {
         row.num = num;
         is_confirmed = row.commit(committer, workergroup, act_commit);
      });
   } else {
      outs.modify(itr, account, [&]( auto& row ) {
         is_confirmed = row.commit(committer, workergroup, act_commit);
      });
   }

   if( is_confirmed ) {
      outstates.modify(outstates_itr, account, [&]( auto& row ) {
         row.confirmed_num = num;
      });
      do_out(name{to}, chain, quantity, memo);
   }
}

void siderelay::do_out( const name& to, const name& chain, const asset& quantity, const std::string& memo ) {
   print("do_out ", to, ",", chain, ",", quantity, ",", memo, "\n");
   eosforce::send_transfer_core_token(_self, to, quantity, memo );
}