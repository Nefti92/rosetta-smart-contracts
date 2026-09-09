# Crowdfund — Execution Guide

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

### Creator

- **Seed:** `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`
- **Identity:** `BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK`

### Recipient

- **Seed:** `bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb`
- **Identity:** `DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL`

### Donor

- **Seed:** `ccccccccccccccccccccccccccccccccccccccccccccccccccccccc`
- **Identity:** `RXMLKVRQUUGUOGYGRAWCBYRHAMADQADDHBGXTTREXDZJVTJYTAEFERRALJYA`

## 4. Check balances

Balances can also be checked through the local explorer:

`http://localhost:41841/explorer#/id/`

### Creator

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK
```

### Recipient

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL
```

### Donor

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance RXMLKVRQUUGUOGYGRAWCBYRHAMADQADDHBGXTTREXDZJVTJYTAEFERRALJYA
```

Expected balance:

```text
10000000000 Qu
```

## 5. Check the contract fee reserve

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -qutilqueryfeereserve 34
```

## 6. Create the crowdfund

The creator creates the crowdfund and specifies:

- **Recipient**
- **Goal:** `2000 Qu`
- **Deadline:** `900 ticks`

### Create

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 34 1 0 "{ DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFLid, 2000uint64, 900uint64 }"
```

## 7. Deposit below the goal

The donor deposits `1000 Qu`.

### Deposit

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed ccccccccccccccccccccccccccccccccccccccccccccccccccccccc -invokecontractprocedure 34 2 1100 "{ 1000uint64 }"
```

`value + fee = 1100`

## 8. Check the crowdfund state

### View

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -callcontractfunction 34 1 "{}" "{ uint64, uint64, uint64, uint8 }"
```

Expected state:

```text
goal = 2000
totalDonated = 1000
active = 1
```

## 9. Reclaim the donation

After the deadline, since the goal has not been reached, the donor reclaims the donated amount.

### Reclaim

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed ccccccccccccccccccccccccccccccccccccccccccccccccccccccc -invokecontractprocedure 34 4 0 "{}"
```

## 10. Create another crowdfund

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 34 1 0 "{ DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFLid, 2000uint64, 900uint64 }"
```

## 11. Reach the goal

The donor deposits `2000 Qu`.

### Deposit

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed ccccccccccccccccccccccccccccccccccccccccccccccccccccccc -invokecontractprocedure 34 2 2100 "{ 2000uint64 }"
```

`value + fee = 2100`

## 12. Withdraw the funds

After the deadline, the recipient withdraws the funds.

### Withdraw

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -invokecontractprocedure 34 3 0 "{}"
```
