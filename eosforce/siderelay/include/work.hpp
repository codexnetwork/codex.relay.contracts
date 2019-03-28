#include <eosiolib/name.hpp>

#pragma once

#define WORK_TYPE_TABLE_DEFINE(name) \
   constexpr static auto work_typ_##name = BOOST_PP_CAT(BOOST_PP_STRINGIZE(name),_n);\
   TABLE name##action { \
         uint64_t num = 0; \
         std::vector<name##action_data> actions; \
         bool commit( capi_name committer, const workersgroup& workers, const name##action_data& act ); \
         uint64_t primary_key() const { return num; } \
   }; \
   typedef eosio::multi_index< BOOST_PP_CAT(BOOST_PP_STRINGIZE(name##actions),_n), name##action > name##action_table;


#define WORK_TYPE_IMPS(name) \
bool siderelay::name##action::commit( capi_name committer, const workersgroup& workers, const name##action_data& commit_act ) { \
   return ::commit_action_imp(actions, committer, workers, commit_act); \
}