#include <eosiolib/name.hpp>

#pragma once

#define WORK_TYPE_TABLE_DEFINE(name) \
   constexpr static auto work_typ_##name = BOOST_PP_CAT(BOOST_PP_STRINGIZE(name),_n);\
   TABLE name##_actions { \
         uint64_t num = 0; \
         std::vector<name##_action_data> actions; \
         bool commit( capi_name committer, const workersgroup& workers, const name##_action_data& commit_act ) { \
            return siderelay::commit_action_imp(actions, committer, workers, commit_act); } \
         uint64_t primary_key() const { return num; } \
   }; \
   typedef eosio::multi_index< BOOST_PP_CAT(BOOST_PP_STRINGIZE(name##acts),_n), name##_actions > name##_action_table;

#define WORK_CHECK(name, committer, args...) \
      (commit_work_then_check< name##_action_table, name##_action_data >( \
         committer, num, work_typ_##name, name##_action_data { args }))