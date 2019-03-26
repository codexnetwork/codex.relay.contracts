#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;

// now just for token map
CONTRACT siderelay : public contract {
   public:
   using contract::contract;
   siderelay( name receiver, name code, datastream<const char*> ds )
      : contract(receiver, code, ds), workergroups(receiver, receiver.value) {}

   // TODO Support diff token contracts

   // from side chain to relay
   ACTION in( capi_name from, name chain, capi_name to, asset quantity, const std::string& memo );

   // from relay chain to side
   ACTION out( capi_name committer, const uint64_t id, capi_name to, name chain, asset quantity, const std::string& memo );

   // change transfers
   ACTION chworker( capi_name committer, const name& chain, const name& old, const name& worker, const uint64_t power, const permission_level& permission );

   // change worker in bios stage
   ACTION initworker( const name& chain, const name& worker, const uint64_t power, const permission_level& permission );

   ACTION cleanworker( const name& chain );

   TABLE workersgroup {
      public:
         name group_name;
         std::vector<name>             requested_names;
         std::vector<uint64_t>         requested_powers;
         std::vector<permission_level> requested_approvals;
         uint64_t power_sum = 0;

      private:
         inline int get_idx_by_name( const name& worker ) const {
            for(int i = 0; i < requested_names.size(); i++){
               if( requested_names[i] == worker){
                  return i;
               }
            }
            return -1;
         }

      public:
         uint64_t check_permission( const name& worker );
         void modify_worker( const name& worker, const uint64_t power, const permission_level& permission ){
            const auto idx = get_idx_by_name(worker);
            if( idx < 0 ) {
               requested_names.emplace_back(worker);
               requested_powers.emplace_back(power);
               requested_approvals.emplace_back(permission);
               power_sum += power;
            } else {
               const auto old_power = requested_powers[idx];
               power_sum += (power - old_power);
               requested_powers[idx] = power;
               requested_approvals[idx] = permission;
            }
         }

         inline void clear_workers(){
            requested_names.clear();
            requested_powers.clear();
            requested_approvals.clear();
            power_sum = 0;
         }

         void del_worker( const name& worker ) {
            const auto idx = get_idx_by_name(worker);
            eosio_assert((idx >= 0) && (idx < requested_names.size()), 
               "no found worker to delete");
            const auto last_idx = static_cast<int>(requested_names.size()) - 1;

            if( last_idx <= 0 ){
               clear_workers();
            } else {
               power_sum -= requested_powers[idx];
               if( idx != last_idx ){
                  requested_names[idx] = requested_names[last_idx];
                  requested_powers[idx] = requested_powers[last_idx];
                  requested_approvals[idx] = requested_approvals[last_idx];
               }
               requested_names.pop_back();
               requested_powers.pop_back();
               requested_approvals.pop_back();
            }
         }

      public:
         uint64_t primary_key()const { return group_name.value; }
   };
   typedef eosio::multi_index< "workersgroup"_n, workersgroup > workersgroup_table;
   workersgroup_table workergroups;

   void ontransfer( capi_name from, capi_name to, asset quantity, std::string memo );

   using in_action = action_wrapper<"in"_n, &siderelay::in>;
   using out_action = action_wrapper<"out"_n, &siderelay::out>;
   using chworker_action = action_wrapper<"chworker"_n, &siderelay::chworker>;
   using initworker_action = action_wrapper<"initworker"_n, &siderelay::initworker>;
};