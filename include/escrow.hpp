#include <eosio/action.hpp>
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/symbol.hpp>

using namespace eosio;
using namespace std;

CONTRACT escrow : public contract
{
public:
   static constexpr name   SENDING_ACCOUNT = "jesse.tf"_n;
   static constexpr name   TOKEN_ACCOUNT   = "eosio.token"_n;
   static constexpr symbol TLOS_SYMBOL     = symbol{"TLOS", 4};

   escrow(name self, name code, datastream<const char*> ds) : contract(self, code, ds){};
   ~escrow(){};

   ACTION openescrow(name account);

   ACTION claimescrow(name account);

   ACTION clearescrow(name account, uint64_t id);

   [[eosio::on_notify("eosio.token::transfer")]] void on_transfer(name from, name to, asset quantity, string memo);

   TABLE deposit
   {
      uint64_t id;
      name     account;
      asset amount;
      uint64_t vesting;

      uint64_t primary_key() const { return id; }
      EOSLIB_SERIALIZE(deposit, (id)(account)(amount)(vesting))
   };
   typedef multi_index<name("deposits"), deposit> deposits_table;
};