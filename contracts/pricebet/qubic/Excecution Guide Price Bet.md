# Price Bet — Execution Guide

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
"ccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
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
./qubic-cli -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -showkeys
./qubic-cli -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -showkeys
./qubic-cli -seed ccccccccccccccccccccccccccccccccccccccccccccccccccccccc -showkeys
```

### Owner

- **Seed:** `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`
- **Identity:** `BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK`

### Player

- **Seed:** `bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb`
- **Identity:** `DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL`

### Oracle Owner

- **Seed:** `ccccccccccccccccccccccccccccccccccccccccccccccccccccccc`
- **Identity:** `RXMLKVRQUUGUOGYGRAWCBYRHAMADQADDHBGXTTREXDZJVTJYTAEFERRALJYA`

## 4. Check balances

Balances can also be checked through the local explorer:

`http://localhost:41841/explorer#/id/`

### Owner

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK
```

### Player

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL
```

### Oracle Owner

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance RXMLKVRQUUGUOGYGRAWCBYRHAMADQADDHBGXTTREXDZJVTJYTAEFERRALJYA
```

Expected balance:

```text
10000000000 Qu
```

## 5. Check fee reserves

### Oracle contract

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -qutilqueryfeereserve 38
```

### Price Bet contract

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -qutilqueryfeereserve 39
```

## 6. Create the Oracle

The Oracle Owner creates the Oracle with an exchange rate of `100`.

### Create Oracle

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed ccccccccccccccccccccccccccccccccccccccccccccccccccccccc -invokecontractprocedure 38 1 0 "{ 100uint64 }"
```

## 7. Check the Oracle exchange rate

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -callcontractfunction 38 1 "{}" "{ uint64 }"
```

Expected result:

```text
rate = 100
```

## 8. Create the Price Bet

The owner creates the Price Bet with:

- **Initial pot:** `2000 Qu`
- **Bet rate:** `150`

### Create

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 39 1 2100 "{ 2000uint64, 150uint64 }"
```

`amount + fee = 2100`

## 9. Join the Price Bet

The player joins the Price Bet by depositing the same amount.

### Join

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -invokecontractprocedure 39 2 2100 "{}"
```

`amount + fee = 2100`

## 10. Update the Oracle exchange rate

The Oracle Owner changes the exchange rate to `170`.

### Set Rate

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed ccccccccccccccccccccccccccccccccccccccccccccccccccccccc -invokecontractprocedure 38 2 0 "{ 170uint64 }"
```

Check the Oracle exchange rate:

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -callcontractfunction 38 1 "{}" "{ uint64 }"
```

Expected result:

```text
rate = 170
```

## 11. Win the Price Bet

The player calls `win`. Price Bet calls the Oracle and checks whether the Oracle rate is greater than the bet rate.

### Win

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -invokecontractprocedure 39 3 0 "{}"
```

Oracle rate:

```text
170
```

Bet rate:

```text
150
```

Expected amount received by the player:

```text
4000 Qu
```

# Timeout Test

## 12. Create a new Price Bet

The owner creates another Price Bet with:

- **Initial pot:** `2000 Qu`
- **Bet rate:** `150`

### Create

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 39 1 2100 "{ 2000uint64, 150uint64 }"
```

## 13. Join the new Price Bet

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -invokecontractprocedure 39 2 2100 "{}"
```

Wait until more than `900 ticks` have passed.

## 14. Call timeout

The owner calls `timeout`.

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 39 4 0 "{}"
```

Expected amount received by the owner:

```text
4000 Qu
```
