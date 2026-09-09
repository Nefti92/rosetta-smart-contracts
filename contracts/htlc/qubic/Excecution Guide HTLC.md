# HTLC — Execution Guide

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
./qubic-cli -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -showkeys
./qubic-cli -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -showkeys
```

### Committer

- **Seed:** `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`
- **Identity:** `BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK`

### Receiver

- **Seed:** `bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb`
- **Identity:** `DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFL`

## 4. Check balances

Balances can also be checked through the local explorer:

`http://localhost:41841/explorer#/id/`

### Committer

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -getbalance BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK
```

Expected balance:

```text
10000000000 Qu
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
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -qutilqueryfeereserve 32
```

Expected value:

```text
10000000000 Qu
```

## 6. Calculate the commitment from the secret

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -callcontractfunction 32 1 "{ { [32;1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8] } }" "{ uint64 }"
```

Set:

```text
COMMITMENT = returned_value
```

## 7. Create the HTLC

The committer creates the HTLC, deposits `2000 Qu`, and sets the receiver, delay, and commitment.

Before executing the command, replace `returned_value` with the value returned by the previous function and append `uint64`.

Example:

```text
value: 12336775920167180877
```

Use:

```text
12336775920167180877uint64
```

### Create

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 32 1 2100 "{ DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFLid, 2000uint64, 900uint64, returned_value }"
```

`value + fee = 2100`

## 8. Reveal the secret before the deadline

Before the deadline, the committer reveals the secret and receives the collateral back.

### Reveal

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 32 2 0 "{ { [32;1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8,1uint8] } }"
```

## 9. Test the timeout

Create another HTLC with a short delay.

Again, replace `returned_value` with the value returned by the commitment function and append `uint64`.

### Create

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -invokecontractprocedure 32 1 2100 "{ DJZMUACQMTYFSEJEYLDBWIGELSFCBMBLPCMBBYFXJHLTGWKHTRRJXTDEHTFLid, 2000uint64, 20uint64, returned_value }"
```

After the deadline, call `timeout`:

```bash
./qubic-cli -nodeip 127.0.0.1 -nodeport 31841 -seed bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -invokecontractprocedure 32 3 0 "{}"
```

Expected result:

```text
The 2000 Qu should be transferred to the receiver.
```
