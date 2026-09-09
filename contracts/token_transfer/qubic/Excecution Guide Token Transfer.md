# Token Transfer — Execution Guide

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

### Receiver

- **Seed:** `bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb`
- **Identity:** `DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL`

## 4. Check balances

Balances can also be checked through the local explorer:

`http://localhost:41841/explorer#/id/`

### Owner

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK
```

### Receiver

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL
```

Expected balance:

```text
10000000000 Qu
```

## 5. Check the contract fee reserve

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -qutilqueryfeereserve 33
```

## 6. Issue TEST tokens

The owner creates `10000 TEST` tokens.

### Issue

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 33 2 100 "{ 1414743380uint64, 10000sint64 }"
```

## 7. Check the owner's assets

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getasset BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK
```

## 8. Create the transfer

The owner creates the transfer and specifies the receiver and the token.

### Create

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 33 1 0 "{ DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFLid, BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXKid, 1414743380uint64 }"
```

## 9. Deposit TEST tokens

The owner deposits `2000 TEST`.

### Deposit

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 33 3 100 "{ 2000sint64 }"
```

## 10. Check the deposited amount

### View

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -callcontractfunction 33 1 "{}" "{ sint64 }"
```

Expected result:

```text
2000
```

## 11. Withdraw TEST tokens

The receiver withdraws `400 TEST`.

### Withdraw

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -invokecontractprocedure 33 4 0 "{ 400sint64 }"
```

## 12. Check the remaining deposited amount

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -callcontractfunction 33 1 "{}" "{ sint64 }"
```

Expected result:

```text
1600
```

## 13. Check the receiver's assets

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getasset DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL
```
