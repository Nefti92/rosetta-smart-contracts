# Constant Product AMM — Execution Guide

## 1. Configure the test identity

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
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
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

## 3. Generate the owner identity

```bash
./qubic-cli -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -showkeys
```

### Owner

- **Seed:** `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`
- **Identity:** `BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK`

## 4. Check the balance

The balance can also be checked through the local explorer:

`http://localhost:41841/explorer#/id/`

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK
```

Expected balance:

```text
10000000000 Qu
```

## 5. Check the contract fee reserve

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -qutilqueryfeereserve 36
```

## 6. Issue token TOKA

The owner issues `10000` shares of token `TOKA`.

- **TOKA:** `1095454548`

### Issue

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 36 2 0 "{ 1095454548uint64, 10000sint64 }"
```

Expected result:

```text
result = 0
issuedNumberOfShares = 10000
```

## 7. Issue token TOKB

The owner issues `20000` shares of token `TOKB`.

- **TOKB:** `1112231764`

### Issue

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 36 2 0 "{ 1112231764uint64, 20000sint64 }"
```

Expected result:

```text
result = 0
issuedNumberOfShares = 20000
```

## 8. Create the AMM

The owner creates the AMM using `TOKA` and `TOKB`.

### Create

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 36 1 0 "{ BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXKid, 1095454548uint64, BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXKid, 1112231764uint64 }"
```

## 9. Make the first deposit

The owner deposits:

- `1000 TOKA`
- `2000 TOKB`

### Deposit

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 36 3 0 "{ 1000uint64, 2000uint64 }"
```

Expected result:

```text
result = 0
minted = 1000
```

## 10. Check the AMM state

### View

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -callcontractfunction 36 1 "{}" "{ uint64, uint64, uint64, uint8 }"
```

Expected state:

```text
r0 = 1000
r1 = 2000
supply = 1000
active = 1
```

## 11. Make a second deposit

The owner makes another deposit while maintaining the same exchange rate:

- `500 TOKA`
- `1000 TOKB`

### Deposit

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 36 3 0 "{ 500uint64, 1000uint64 }"
```

Expected result:

```text
result = 0
minted = 500
```

Check the AMM again:

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -callcontractfunction 36 1 "{}" "{ uint64, uint64, uint64, uint8 }"
```

Expected state:

```text
r0 = 1500
r1 = 3000
supply = 1500
active = 1
```

## 12. Swap TOKA for TOKB

The owner swaps `100 TOKA` for `TOKB`, with a minimum output of `180 TOKB`.

### Swap

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 36 5 0 "{ 0uint8, 100uint64, 180uint64 }"
```

Calculated output:

```text
100 * 3000 / (1500 + 100) = 187 TOKB
```

Expected result:

```text
result = 0
xOut = 187
```

Check the AMM:

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -callcontractfunction 36 1 "{}" "{ uint64, uint64, uint64, uint8 }"
```

Expected state:

```text
r0 = 1600
r1 = 2813
supply = 1500
active = 1
```

## 13. Redeem liquidity tokens

The owner redeems `500` liquidity tokens.

### Redeem

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 36 4 0 "{ 500uint64 }"
```

Calculated amounts:

```text
x0 = 500 * 1600 / 1500 = 533 TOKA
x1 = 500 * 2813 / 1500 = 937 TOKB
```

Expected result:

```text
result = 0
x0 = 533
x1 = 937
```

## 14. Check the final AMM state

### View

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -callcontractfunction 36 1 "{}" "{ uint64, uint64, uint64, uint8 }"
```

Expected state:

```text
r0 = 1067
r1 = 1876
supply = 1000
active = 1
```
