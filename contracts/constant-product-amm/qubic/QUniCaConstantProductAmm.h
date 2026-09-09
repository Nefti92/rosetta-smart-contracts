using namespace QPI;

constexpr uint64 QUCCPA_CREATE_SUCCESS = 0;
constexpr uint64 QUCCPA_CREATE_ALREADY_REGISTERED = 1;
constexpr uint64 QUCCPA_CREATE_INVALID_TOKEN = 2;
constexpr uint64 QUCCPA_CREATE_SAME_TOKEN = 3;

constexpr uint64 QUCCPA_ISSUE_SUCCESS = 0;
constexpr uint64 QUCCPA_ISSUE_INVALID_AMOUNT = 1;
constexpr uint64 QUCCPA_ISSUE_FAILED = 2;

constexpr uint64 QUCCPA_DEPOSIT_SUCCESS = 0;
constexpr uint64 QUCCPA_DEPOSIT_NOT_REGISTERED = 1;
constexpr uint64 QUCCPA_DEPOSIT_INVALID_AMOUNT = 2;
constexpr uint64 QUCCPA_DEPOSIT_INVALID_RATIO = 3;
constexpr uint64 QUCCPA_DEPOSIT_NO_LIQUIDITY = 4;
constexpr uint64 QUCCPA_DEPOSIT_FULL = 5;
constexpr uint64 QUCCPA_DEPOSIT_TRANSFER_FAILED = 6;

constexpr uint64 QUCCPA_REDEEM_SUCCESS = 0;
constexpr uint64 QUCCPA_REDEEM_NOT_REGISTERED = 1;
constexpr uint64 QUCCPA_REDEEM_INVALID_AMOUNT = 2;
constexpr uint64 QUCCPA_REDEEM_INSUFFICIENT_LIQUIDITY = 3;
constexpr uint64 QUCCPA_REDEEM_TRANSFER_FAILED = 4;

constexpr uint64 QUCCPA_SWAP_SUCCESS = 0;
constexpr uint64 QUCCPA_SWAP_NOT_REGISTERED = 1;
constexpr uint64 QUCCPA_SWAP_INVALID_TOKEN = 2;
constexpr uint64 QUCCPA_SWAP_INVALID_AMOUNT = 3;
constexpr uint64 QUCCPA_SWAP_INSUFFICIENT_OUTPUT = 4;
constexpr uint64 QUCCPA_SWAP_TRANSFER_FAILED = 5;

constexpr uint64 QUCCPA_MAX_USERS = 1024;

struct QUCCPA2
{
};

struct QUCCPA : public ContractBase
{
    struct AmmInfo
    {
        id issuer0;
        id issuer1;
        uint64 assetName0;
        uint64 assetName1;
        uint64 r0;
        uint64 r1;
        uint64 supply;
        bit everDeposited;
        bit active;
    };

    struct StateData
    {
        AmmInfo amm;
        HashMap<id, uint64, QUCCPA_MAX_USERS> minted;
    };

    struct create_input
    {
        id issuer0;
        uint64 assetName0;
        id issuer1;
        uint64 assetName1;
    };

    struct create_output
    {
        uint64 result;
    };

    struct issue_input
    {
        uint64 assetName;
        sint64 numberOfShares;
    };

    struct issue_output
    {
        uint64 result;
        sint64 issuedNumberOfShares;
    };

    struct view_input
    {
    };

    struct view_output
    {
        uint64 r0;
        uint64 r1;
        uint64 supply;
        uint8 active;
    };

    struct deposit_input
    {
        uint64 x0;
        uint64 x1;
    };

    struct deposit_output
    {
        uint64 result;
        uint64 minted;
    };

    struct redeem_input
    {
        uint64 x;
    };

    struct redeem_output
    {
        uint64 result;
        uint64 x0;
        uint64 x1;
    };

    struct swap_input
    {
        uint8 token;
        uint64 xIn;
        uint64 xOutMin;
    };

    struct swap_output
    {
        uint64 result;
        uint64 xOut;
    };

    struct deposit_locals
    {
        uint64 currentMinted;
        uint64 toMint;
        sint64 transferResult0;
        sint64 transferResult1;
        uint128 ratio0;
        uint128 ratio1;
        uint128 numerator;
        uint128 calculatedMint;
    };

    struct redeem_locals
    {
        uint64 currentMinted;
        uint64 x0;
        uint64 x1;
        sint64 transferResult0;
        sint64 transferResult1;
        uint128 numerator0;
        uint128 numerator1;
        uint128 calculatedX0;
        uint128 calculatedX1;
    };

    struct swap_locals
    {
        uint64 rIn;
        uint64 rOut;
        uint64 xOut;
        sint64 transferResultIn;
        sint64 transferResultOut;
        uint128 numerator;
        uint128 denominator;
        uint128 calculatedOutput;
    };

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_PROCEDURE(create, 1);
        REGISTER_USER_FUNCTION(view, 1);
        REGISTER_USER_PROCEDURE(issue, 2);
        REGISTER_USER_PROCEDURE(deposit, 3);
        REGISTER_USER_PROCEDURE(redeem, 4);
        REGISTER_USER_PROCEDURE(swap, 5);
    }

    PUBLIC_PROCEDURE(create) {

        if (state.get().amm.active) {
            output.result = QUCCPA_CREATE_ALREADY_REGISTERED;
            return;
        }

        if (!qpi.isAssetIssued(input.issuer0, input.assetName0) ||
            !qpi.isAssetIssued(input.issuer1, input.assetName1)) {
            output.result = QUCCPA_CREATE_INVALID_TOKEN;
            return;
        }

        if (input.issuer0 == input.issuer1 &&
            input.assetName0 == input.assetName1) {
            output.result = QUCCPA_CREATE_SAME_TOKEN;
            return;
        }

        state.mut().amm.issuer0 = input.issuer0;
        state.mut().amm.issuer1 = input.issuer1;
        state.mut().amm.assetName0 = input.assetName0;
        state.mut().amm.assetName1 = input.assetName1;
        state.mut().amm.r0 = 0;
        state.mut().amm.r1 = 0;
        state.mut().amm.supply = 0;
        state.mut().amm.everDeposited = 0;
        state.mut().amm.active = 1;

        state.mut().minted.reset();

        output.result = QUCCPA_CREATE_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE(issue) {

        output.issuedNumberOfShares = 0;

        if (input.numberOfShares <= 0) {
            output.result = QUCCPA_ISSUE_INVALID_AMOUNT;
            return;
        }

        output.issuedNumberOfShares = qpi.issueAsset(
            input.assetName,
            qpi.invocator(),
            0,
            input.numberOfShares,
            0
        );

        if (output.issuedNumberOfShares <= 0) {
            output.result = QUCCPA_ISSUE_FAILED;
            return;
        }

        output.result = QUCCPA_ISSUE_SUCCESS;
        return;
    }

    PUBLIC_FUNCTION(view) {

        output.r0 = state.get().amm.r0;
        output.r1 = state.get().amm.r1;
        output.supply = state.get().amm.supply;
        output.active = state.get().amm.active;
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(deposit) {

        if (!state.get().amm.active) {
            output.result = QUCCPA_DEPOSIT_NOT_REGISTERED;
            return;
        }

        if (input.x0 == 0 || input.x1 == 0) {
            output.result = QUCCPA_DEPOSIT_INVALID_AMOUNT;
            return;
        }

        if (state.get().amm.everDeposited) {

            locals.ratio0 =
                uint128(state.get().amm.r0) *
                uint128(input.x1);

            locals.ratio1 =
                uint128(state.get().amm.r1) *
                uint128(input.x0);

            if (locals.ratio0 != locals.ratio1) {
                output.result = QUCCPA_DEPOSIT_INVALID_RATIO;
                return;
            }

            locals.numerator =
                uint128(input.x0) *
                uint128(state.get().amm.supply);

            locals.calculatedMint = div(
                locals.numerator,
                uint128(state.get().amm.r0)
            );

            locals.toMint = uint64(locals.calculatedMint.low);
        }
        else {
            locals.toMint = input.x0;
        }

        if (locals.toMint == 0) {
            output.result = QUCCPA_DEPOSIT_NO_LIQUIDITY;
            return;
        }

        if (!state.get().minted.get(qpi.invocator(), locals.currentMinted)) {
            locals.currentMinted = 0;

            if (state.get().minted.population() >= state.get().minted.capacity()) {
                output.result = QUCCPA_DEPOSIT_FULL;
                return;
            }
        }

        locals.transferResult0 = qpi.transferShareOwnershipAndPossession(
            state.get().amm.assetName0,
            state.get().amm.issuer0,
            qpi.invocator(),
            qpi.invocator(),
            sint64(input.x0),
            SELF
        );

        if (locals.transferResult0 < 0) {
            output.result = QUCCPA_DEPOSIT_TRANSFER_FAILED;
            return;
        }

        locals.transferResult1 = qpi.transferShareOwnershipAndPossession(
            state.get().amm.assetName1,
            state.get().amm.issuer1,
            qpi.invocator(),
            qpi.invocator(),
            sint64(input.x1),
            SELF
        );

        if (locals.transferResult1 < 0) {

            qpi.transferShareOwnershipAndPossession(
                state.get().amm.assetName0,
                state.get().amm.issuer0,
                SELF,
                SELF,
                sint64(input.x0),
                qpi.invocator()
            );

            output.result = QUCCPA_DEPOSIT_TRANSFER_FAILED;
            return;
        }

        state.mut().minted.set(
            qpi.invocator(),
            locals.currentMinted + locals.toMint
        );

        state.mut().amm.supply =
            state.get().amm.supply +
            locals.toMint;

        state.mut().amm.r0 =
            state.get().amm.r0 +
            input.x0;

        state.mut().amm.r1 =
            state.get().amm.r1 +
            input.x1;

        state.mut().amm.everDeposited = 1;

        output.minted = locals.toMint;
        output.result = QUCCPA_DEPOSIT_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(redeem) {

        if (!state.get().amm.active) {
            output.result = QUCCPA_REDEEM_NOT_REGISTERED;
            return;
        }

        if (input.x == 0 || input.x >= state.get().amm.supply) {
            output.result = QUCCPA_REDEEM_INVALID_AMOUNT;
            return;
        }

        if (!state.get().minted.get(qpi.invocator(), locals.currentMinted) ||
            locals.currentMinted < input.x) {
            output.result = QUCCPA_REDEEM_INSUFFICIENT_LIQUIDITY;
            return;
        }

        locals.numerator0 =
            uint128(input.x) *
            uint128(state.get().amm.r0);

        locals.calculatedX0 = div(
            locals.numerator0,
            uint128(state.get().amm.supply)
        );

        locals.x0 = uint64(locals.calculatedX0.low);

        locals.numerator1 =
            uint128(input.x) *
            uint128(state.get().amm.r1);

        locals.calculatedX1 = div(
            locals.numerator1,
            uint128(state.get().amm.supply)
        );

        locals.x1 = uint64(locals.calculatedX1.low);

        locals.transferResult0 = qpi.transferShareOwnershipAndPossession(
            state.get().amm.assetName0,
            state.get().amm.issuer0,
            SELF,
            SELF,
            sint64(locals.x0),
            qpi.invocator()
        );

        if (locals.transferResult0 < 0) {
            output.result = QUCCPA_REDEEM_TRANSFER_FAILED;
            return;
        }

        locals.transferResult1 = qpi.transferShareOwnershipAndPossession(
            state.get().amm.assetName1,
            state.get().amm.issuer1,
            SELF,
            SELF,
            sint64(locals.x1),
            qpi.invocator()
        );

        if (locals.transferResult1 < 0) {

            qpi.transferShareOwnershipAndPossession(
                state.get().amm.assetName0,
                state.get().amm.issuer0,
                qpi.invocator(),
                qpi.invocator(),
                sint64(locals.x0),
                SELF
            );

            output.result = QUCCPA_REDEEM_TRANSFER_FAILED;
            return;
        }

        state.mut().amm.r0 =
            state.get().amm.r0 -
            locals.x0;

        state.mut().amm.r1 =
            state.get().amm.r1 -
            locals.x1;

        state.mut().amm.supply =
            state.get().amm.supply -
            input.x;

        state.mut().minted.set(
            qpi.invocator(),
            locals.currentMinted - input.x
        );

        output.x0 = locals.x0;
        output.x1 = locals.x1;
        output.result = QUCCPA_REDEEM_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(swap) {

        if (!state.get().amm.active ||
            !state.get().amm.everDeposited) {
            output.result = QUCCPA_SWAP_NOT_REGISTERED;
            return;
        }

        if (input.token > 1) {
            output.result = QUCCPA_SWAP_INVALID_TOKEN;
            return;
        }

        if (input.xIn == 0) {
            output.result = QUCCPA_SWAP_INVALID_AMOUNT;
            return;
        }

        if (input.token == 0) {
            locals.rIn = state.get().amm.r0;
            locals.rOut = state.get().amm.r1;
        }
        else {
            locals.rIn = state.get().amm.r1;
            locals.rOut = state.get().amm.r0;
        }

        locals.numerator =
            uint128(input.xIn) *
            uint128(locals.rOut);

        locals.denominator =
            uint128(locals.rIn) +
            uint128(input.xIn);

        locals.calculatedOutput = div(
            locals.numerator,
            locals.denominator
        );

        locals.xOut =
            uint64(locals.calculatedOutput.low);

        if (locals.xOut == 0) {
            output.result = QUCCPA_SWAP_INVALID_AMOUNT;
            return;
        }

        if (locals.xOut < input.xOutMin) {
            output.result = QUCCPA_SWAP_INSUFFICIENT_OUTPUT;
            return;
        }

        if (input.token == 0) {

            locals.transferResultIn = qpi.transferShareOwnershipAndPossession(
                state.get().amm.assetName0,
                state.get().amm.issuer0,
                qpi.invocator(),
                qpi.invocator(),
                sint64(input.xIn),
                SELF
            );

            if (locals.transferResultIn < 0) {
                output.result = QUCCPA_SWAP_TRANSFER_FAILED;
                return;
            }

            locals.transferResultOut = qpi.transferShareOwnershipAndPossession(
                state.get().amm.assetName1,
                state.get().amm.issuer1,
                SELF,
                SELF,
                sint64(locals.xOut),
                qpi.invocator()
            );

            if (locals.transferResultOut < 0) {

                qpi.transferShareOwnershipAndPossession(
                    state.get().amm.assetName0,
                    state.get().amm.issuer0,
                    SELF,
                    SELF,
                    sint64(input.xIn),
                    qpi.invocator()
                );

                output.result = QUCCPA_SWAP_TRANSFER_FAILED;
                return;
            }

            state.mut().amm.r0 = state.get().amm.r0 + input.xIn;

            state.mut().amm.r1 = state.get().amm.r1 - locals.xOut;
        }
        else {

            locals.transferResultIn = qpi.transferShareOwnershipAndPossession(
                state.get().amm.assetName1,
                state.get().amm.issuer1,
                qpi.invocator(),
                qpi.invocator(),
                sint64(input.xIn),
                SELF
            );

            if (locals.transferResultIn < 0) {
                output.result = QUCCPA_SWAP_TRANSFER_FAILED;
                return;
            }

            locals.transferResultOut = qpi.transferShareOwnershipAndPossession(
                state.get().amm.assetName0,
                state.get().amm.issuer0,
                SELF,
                SELF,
                sint64(locals.xOut),
                qpi.invocator()
            );

            if (locals.transferResultOut < 0) {

                qpi.transferShareOwnershipAndPossession(
                    state.get().amm.assetName1,
                    state.get().amm.issuer1,
                    SELF,
                    SELF,
                    sint64(input.xIn),
                    qpi.invocator()
                );

                output.result = QUCCPA_SWAP_TRANSFER_FAILED;
                return;
            }

            state.mut().amm.r1 = state.get().amm.r1 + input.xIn;

            state.mut().amm.r0 = state.get().amm.r0 - locals.xOut;
        }

        output.xOut = locals.xOut;
        output.result = QUCCPA_SWAP_SUCCESS;
        return;
    }
};