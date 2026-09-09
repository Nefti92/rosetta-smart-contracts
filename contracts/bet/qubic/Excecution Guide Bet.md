# Bet — Execution Guide

## 1. Configure the test identities

Open `private_settings.h`.

Replace:

```cpp
static unsigned char customSeeds[][55 + 1] = {
    "qubicorelitebyfeiyuivqubicqubicqubicqubicqubicquicqubic"
};
```

with:

```cpp
static unsigned char customSeeds[][55 + 1] = {
    "qubicorelitebyfeiyuivqubicqubicqubicqubicqubicquicqubic",
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
    "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
    "ccccccccccccccccccccccccccccccccccccccccccccccccccccccc",
    "ddddddddddddddddddddddddddddddddddddddddddddddddddddddd",
    "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee",
    "fffffffffffffffffffffffffffffffffffffffffffffffffffffff",
    "ggggggggggggggggggggggggggggggggggggggggggggggggggggggg",
    "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh",
    "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",
    "jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj",
    "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk",
    "lllllllllllllllllllllllllllllllllllllllllllllllllllllll",
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm",
    "nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn",
    "ooooooooooooooooooooooooooooooooooooooooooooooooooooooo",
    "ppppppppppppppppppppppppppppppppppppppppppppppppppppppp",
    "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq",
    "rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr",
    "sssssssssssssssssssssssssssssssssssssssssssssssssssssss",
    "ttttttttttttttttttttttttttttttttttttttttttttttttttttttt",
    "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu",
    "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv",
    "wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww",
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
    "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy",
    "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
};
```

## 2. Build and start the node

```bash
cd /mnt/c/Qubic_Project/core-lite/build && cmake --build . --target Qubic -j$(nproc)
```

In another terminal:

```bash
cd /mnt/c/Qubic_Project/qubic-cli/build
```

## 3. Generate the identities

```bash
/.qubic-cli -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -showkeys
/.qubic-cli -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -showkeys
/.qubic-cli -seed ccccccccccccccccccccccccccccccccccccccccccccccccccccccc -showkeys
```

### Player 1

- **Seed:** `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`
- **Identity:** `BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK`

### Player 2

- **Seed:** `bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb`
- **Identity:** `DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL`

### Oracle

- **Seed:** `ccccccccccccccccccccccccccccccccccccccccccccccccccccccc`
- **Identity:** `RXMLKVRQUUGUOGYGRAWCBYRHAMADQADDHBGXTTREXDZJVTJYTAEFERRALJYA`

## 4. Check balances

Balances can also be checked through the local explorer:

`http://localhost:41841/explorer#/id/`

### Player 1

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 \
-getbalance BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK
```

Expected balance:

```text
10000000000 Qu
```

### Player 2

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 \
-getbalance DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL
```

Expected balance:

```text
10000000000 Qu
```

## 5. Check the contract fee reserve

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 \
-qutilqueryfeereserve 29
```

Expected value:

```text
10000000000 Qu
```

## 6. Create the bet

Player 1 creates the bet and specifies the oracle and bet amount.

### Create

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 29 1 0 "{ RXMLKVRQUUGUOGYGRAWCBYRHAMADQADDHBGXTTREXDZJVTJYTAEFERRALJYAid, 1000uint64 }"
```

Check the tick for which the transaction has been scheduled.

## 7. Player 1 approves and sends the amount

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 29 2 1100 "{}"
```

## 8. Player 2 joins the bet

Player 2 does not know beforehand that it is `player2`, so it also sends the amount and `oracleId`.

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -invokecontractprocedure 29 1 0 "{ RXMLKVRQUUGUOGYGRAWCBYRHAMADQADDHBGXTTREXDZJVTJYTAEFERRALJYAid, 1000uint64 }"
```

Player 2 approves and sends the amount:

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -invokecontractprocedure 29 2 1100 "{}"
```

## 9. Oracle declares the winner

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed ccccccccccccccccccccccccccccccccccccccccccccccccccccccc -invokecontractprocedure 29 3 0 "{ BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXKid }"
```

## 10. Deadline scenario

The bet ends after `900 ticks`.

Check the final balances:

### Player 1

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK
```

### Player 2

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL
```
