#include "../include/escrow.hpp"

ACTION escrow::openescrow(name account)
{
   deposits_table deposits(get_self(), account.value);

   uint64_t id = deposits.available_primary_key();
   // TODO: consider who should pay for ram here
   deposits.emplace(account, [&](auto& deposit) {
      deposit.id     = id;
      deposit.amount = asset(0, TLOS_SYMBOL);
   });
}

ACTION escrow::claimescrow(name account)
{
   deposits_table deposits(get_self(), account.value);
   asset          claim_amount = asset(0, TLOS_SYMBOL);
   for (auto itr = deposits.begin(); itr != deposits.end(); itr++) {
      auto deposit = *itr;
      if (current_time_point().sec_since_epoch() > deposit.vesting) {
         claim_amount += deposit.amount;
         deposits.modify(itr, get_self(), [&](auto& deposit) {
            deposit.vesting = 0;
            deposit.amount  = asset(0, TLOS_SYMBOL);
         });
      }
   }

   check(claim_amount.amount > 0, "No claimable deposits");
   action(permission_level{_self, name("active")}, name("eosio.token"), name("transfer"),
          make_tuple(get_self(), account, claim_amount, std::string("Escrow payout")))
      .send();
}

ACTION escrow::clearescrow(name account, uint64_t id)
{
   deposits_table deposits(get_self(), account.value);
   auto&          deposit = deposits.get(id, "Unable to find escrow deposit");
   check(has_auth(account), "Only the beneficiary can clear an escrow deposit");
   check(deposit.vesting == 0 && deposit.amount.amount == 0,
         "Escrow deposit has not been claimed or is not yet vested");
   deposits.erase(deposit);
}

void escrow::on_transfer(name from, name to, asset quantity, string memo)
{
   // get initial receiver contract
   name rec = get_first_receiver();

   // validate
   if (rec == TOKEN_ACCOUNT && to == get_self() && from != get_self() && quantity.symbol == TLOS_SYMBOL) {
      auto splitIndex           = memo.find(":");
      auto accountString        = memo.substr(0, splitIndex);
      auto account              = name(accountString);
      auto now                  = current_time_point().sec_since_epoch();
      auto vesting_duration_str = memo.substr(splitIndex + 1, memo.length());
      auto vesting_duration     = std::atoi(vesting_duration_str.c_str());
      check(vesting_duration > 0, "The vesting duration must be greater than zero, this means the memo was not "
                                  "properly formatted or the vesting period was actually set to 0");
      check(is_account(account), "Beneficiary account does not exist");
      deposits_table deposits(get_self(), account.value);
      bool deposit_updated = false;
      for (auto itr = deposits.begin(); itr != deposits.end(); itr++) {
         auto deposit = *itr;
         if (deposit.vesting == 0 && deposit.amount.amount == 0) {
            deposits.modify(itr, same_payer, [&](auto& deposit) {
               deposit.vesting = now + vesting_duration;
               deposit.amount  = quantity;
            });
            deposit_updated = true;
         }
      }
      check(deposit_updated, "Unable to find empty deposit slot, please first call \"openescrow\"");
   }
}