#include <siderelay.hpp>
#include <chain.hpp>

int siderelay::workersgroup::get_idx_by_name( name worker ) const {
   for( int i = 0; i < requested_names.size(); i++ ) {
      if( requested_names[i] == worker ) {
         return i;
      }
   }
   return -1;
}

void siderelay::workersgroup::modify_worker( name worker, uint64_t power, const permission_level& permission ) {
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

void siderelay::workersgroup::clear_workers() {
   requested_names.clear();
   requested_powers.clear();
   requested_approvals.clear();
   power_sum = 0;
}

void siderelay::workersgroup::del_worker( name worker ) {
   const auto idx = get_idx_by_name(worker);
   check((idx >= 0) && (idx < requested_names.size()), "no found worker to delete");
   const auto last_idx = static_cast<int>(requested_names.size()) - 1;

   if( last_idx <= 0 ) {
      clear_workers();
   } else {
      power_sum -= requested_powers[idx];
      if( idx != last_idx ) {
         requested_names[idx] = requested_names[last_idx];
         requested_powers[idx] = requested_powers[last_idx];
         requested_approvals[idx] = requested_approvals[last_idx];
      }
      requested_names.pop_back();
      requested_powers.pop_back();
      requested_approvals.pop_back();
   }
}

bool siderelay::workersgroup::is_confirm_ok( const std::vector<name>& confirmed ) const {
   uint64_t confirmed_power = 0;
   for( const auto& c : confirmed ) {
      const auto idx = get_idx_by_name(c);
      if( idx >= 0 && idx <= requested_powers.size() ) {
         confirmed_power += requested_powers[idx];
      }
   }
   return (confirmed_power * 3) >= (power_sum * 2);
}

name siderelay::workersgroup::check_permission( name worker ) const {
   const auto idx = get_idx_by_name(worker);
   check((idx >= 0) && (idx < requested_names.size()), "no found worker to check");
   
   require_auth(requested_approvals[idx]);

   return requested_approvals[idx].actor;
}