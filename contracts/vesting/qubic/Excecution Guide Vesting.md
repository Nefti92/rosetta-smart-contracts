# Vesting — Execution Guide

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
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
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
```

### Owner

- **Seed:** `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`
- **Identity:** `BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK`

### Beneficiary

- **Seed:** `bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb`
- **Identity:** `DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL`

## 4. Check balances

Balances can also be checked through the local explorer:

`http://localhost:41841/explorer#/id/`

### Owner

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK
```

### Beneficiary

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL
```

Expected balance:

```text
10000000000 Qu
```

## 5. Check the contract fee reserve

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -qutilqueryfeereserve 40
```

## 6. Check the current tick

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getcurrenttickinfo
```

Choose a start tick greater than the current tick.

For example, if the current tick is:

```text
1000
```

use:

```text
start = 1020
```

## 7. Create the vesting

The owner creates the vesting for the beneficiary with:

- **Start:** `1020`
- **Duration:** `100`
- **Amount:** `10000 Qu`

### Create

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 40 1 10100 "{ DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFLid, 1020uint64, 100uint64, 10000uint64 }"
```

`amount + fee = 10100`

## 8. Test release before the start

Before the vesting starts, the beneficiary calls `release`.

### Release

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -invokecontractprocedure 40 2 0 "{}"
```

Expected result:

```text
result = 3
amount = 0
```

## 9. Test partial vesting

Wait until approximately half of the vesting duration has passed.

Example:

```text
start = 1020
duration = 100
```

At tick:

```text
1070
```

approximately `50%` of the amount should be vested.

The beneficiary calls `release`:

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -invokecontractprocedure 40 2 0 "{}"
```

Expected released amount:

```text
Approximately 5000 Qu
```

## 10. Test complete vesting

Wait until:

```text
start + duration = 1120
```

The beneficiary calls `release` again:

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -invokecontractprocedure 40 2 0 "{}"
```

The remaining vested amount should be released.

Expected total amount received by the beneficiary:

```text
10000 Qu
```
