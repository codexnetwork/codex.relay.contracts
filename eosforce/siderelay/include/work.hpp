#include <eosio/name.hpp>

#pragma once

#define WORK_TYPE_TABLE_DEFINE(work_name) \
   constexpr static auto work_typ_##work_name = BOOST_PP_CAT(BOOST_PP_STRINGIZE(work_name),_n);\
   TABLE work_name##_actions { \
         uint64_t num = 0; \
         std::vector<work_name##_action_data> actions; \
         bool commit( name committer, const workersgroup& workers, const work_name##_action_data& commit_act ) { \
            return siderelay::commit_action_imp(actions, committer, workers, commit_act); } \
         uint64_t primary_key() const { return num; } \
   }; \
   typedef eosio::multi_index< BOOST_PP_CAT(BOOST_PP_STRINGIZE(work_name##acts),_n), work_name##_actions > work_name##_action_table;

#define WORK_CHECK(work_name, committer, args...) \
      (commit_work_then_check< work_name##_action_table, work_name##_action_data >( \
         committer, num, work_typ_##work_name, work_name##_action_data { args }))