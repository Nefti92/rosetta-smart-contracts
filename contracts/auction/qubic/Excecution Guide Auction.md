# Auction — Execution Guide

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

### Seller

- **Seed:** `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`
- **Identity:** `BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK`

### Bidder 1

- **Seed:** `bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb`
- **Identity:** `DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL`

### Bidder 2

- **Seed:** `ccccccccccccccccccccccccccccccccccccccccccccccccccccccc`
- **Identity:** `RXMLKVRQUUGUOGYGRAWCBYRHAMADQADDHBGXTTREXDZJVTJYTAEFERRALJYA`

## 4. Check balances

Balances can also be checked through the local explorer:

`http://localhost:41841/explorer#/id/`

### Seller

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK
```

### Bidder 1

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL
```

### Bidder 2

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance RXMLKVRQUUGUOGYGRAWCBYRHAMADQADDHBGXTTREXDZJVTJYTAEFERRALJYA
```

Expected balance:

```text
10000000000 Qu
```

## 5. Check the contract fee reserve

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -qutilqueryfeereserve 35
```

## 6. Create the auction

The seller creates the auction with:

- **Starting bid:** `500 Qu`
- **Duration:** `20 ticks`

### Create

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 35 1 0 "{ 500uint64, 20uint64, [64;1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8] }"
```

## 7. Start the auction

### Start

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 35 2 0 "{}"
```

## 8. Place bids

### Bidder 1

Bidder 1 bids `1000 Qu`.

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -invokecontractprocedure 35 3 1100 "{ 1000uint64 }"
```

`value + fee = 1100`

### Bidder 2

Bidder 2 bids `1500 Qu`.

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed ccccccccccccccccccccccccccccccccccccccccccccccccccccccc -invokecontractprocedure 35 3 1600 "{ 1500uint64 }"
```

`value + fee = 1600`

## 9. Check the auction

### View

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -callcontractfunction 35 1 "{}" "{ uint64, uint64, uint64, id, uint8, uint8 }"
```

## 10. Withdraw the previous bid

Bidder 1 withdraws the previous bid.

### Withdraw

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -invokecontractprocedure 35 4 0 "{}"
```

## 11. End the auction

After the deadline, the seller ends the auction and receives the highest bid.

### End

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 35 5 0 "{}"
```
