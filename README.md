# Telos Escrow
An escrow contract that can be customized for various purposes.  The default configuration is for REX lockup.

Changing the token and sender can be done in the escrow.hpp file.

### Workflow
A beneficiary account must first call `openescrow` to emplace a row and pay for it's ram.  This row is then populated when tokens are received.

When sending tokens to this escrow contract, the token contract account and symbol must match the configuration of this contract,
additionally the sender of the transfer must match the configured sender to prevent abuse by a bad actor sending dust to fill a row created by a beneficiary.

The memo must match the pattern `<beneficiary account>:<lockupseconds>` e.g. `eosio:60` would update an existing deposit row for the `eosio` account with a lockup of `60` seconds 

The beneficiary account must have an empty row already or this transfer will be rejected.

NOTE: The c++ atoi function is used to parse the lockup seconds from the memo, so the seconds must be less than max integer (`2147483647`)