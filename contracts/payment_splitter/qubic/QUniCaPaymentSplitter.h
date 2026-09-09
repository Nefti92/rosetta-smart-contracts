using namespace QPI;

constexpr uint64 QUCPSP_CREATE_SUCCESS = 0;
constexpr uint64 QUCPSP_CREATE_ALREADY_REGISTERED = 1;
constexpr uint64 QUCPSP_CREATE_INVALID_COUNT = 2;
constexpr uint64 QUCPSP_CREATE_INVALID_SHAREHOLDER = 3;
constexpr uint64 QUCPSP_CREATE_INVALID_SHARES = 4;
constexpr uint64 QUCPSP_CREATE_DUPLICATE_SHAREHOLDER = 5;

constexpr uint64 QUCPSP_RECEIVE_SUCCESS = 0;
constexpr uint64 QUCPSP_RECEIVE_NOT_REGISTERED = 1;
constexpr uint64 QUCPSP_RECEIVE_INVALID_AMOUNT = 2;

constexpr uint64 QUCPSP_RELEASE_SUCCESS = 0;
constexpr uint64 QUCPSP_RELEASE_NOT_REGISTERED = 1;
constexpr uint64 QUCPSP_RELEASE_INVALID_SHAREHOLDER = 2;
constexpr uint64 QUCPSP_RELEASE_NO_PAYMENT = 3;

constexpr uint64 QUCPSP_FEE = 100;
constexpr uint64 QUCPSP_MAX_SHAREHOLDERS = 16;

struct QUCPSP2
{
};

struct QUCPSP : public ContractBase
{
    struct PaymentSplitterInfo
    {
        uint64 shareholderCount;
        uint64 totalShares;
        uint64 totalReceived;
        bit active;
    };

    struct StateData
    {
        PaymentSplitterInfo splitter;

        Array<id, QUCPSP_MAX_SHAREHOLDERS> shareholders;

        HashMap<id, uint64, QUCPSP_MAX_SHAREHOLDERS> shares;
        HashMap<id, uint64, QUCPSP_MAX_SHAREHOLDERS> released;
    };

    struct create_input
    {
        uint64 shareholderCount;
        Array<id, QUCPSP_MAX_SHAREHOLDERS> shareholders;
        Array<uint64, QUCPSP_MAX_SHAREHOLDERS> shares;
    };

    struct create_output
    {
        uint64 result;
    };

    struct view_input
    {
    };

    struct view_output
    {
        uint64 shareholderCount;
        uint64 totalShares;
        uint64 totalReceived;
        uint8 active;
    };

    struct receive_input
    {
        uint64 amount;
    };

    struct receive_output
    {
        uint64 result;
    };

    struct release_input
    {
        id shareholder;
    };

    struct release_output
    {
        uint64 result;
    };

    struct create_locals
    {
        uint64 i;
        uint64 j;
        uint64 totalShares;
    };

    struct release_locals
    {
        uint64 shareholderShares;
        uint64 alreadyReleased;
        uint64 payment;
        uint128 numerator;
        uint128 calculatedPayment;
    };
    
    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_PROCEDURE(create, 1);
        REGISTER_USER_FUNCTION(view, 1);
        REGISTER_USER_PROCEDURE(receive, 2);
        REGISTER_USER_PROCEDURE(release, 3);
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(create) {
        if (state.get().splitter.active) {
            output.result = QUCPSP_CREATE_ALREADY_REGISTERED;
            return;
        }

        if (input.shareholderCount == 0 ||
            input.shareholderCount > QUCPSP_MAX_SHAREHOLDERS) {

            output.result = QUCPSP_CREATE_INVALID_COUNT;
            return;
        }

        locals.totalShares = 0;

        for (locals.i = 0; locals.i < QUCPSP_MAX_SHAREHOLDERS; locals.i++) {
            if (locals.i < input.shareholderCount) {

                if (input.shareholders.get(locals.i) == NULL_ID) {
                    output.result = QUCPSP_CREATE_INVALID_SHAREHOLDER;
                    return;
                }

                if (input.shares.get(locals.i) == 0) {
                    output.result = QUCPSP_CREATE_INVALID_SHARES;
                    return;
                }

                for (locals.j = 0; locals.j < QUCPSP_MAX_SHAREHOLDERS; locals.j++) {
                    if (locals.j < locals.i) {
                        if (input.shareholders.get(locals.i) ==
                            input.shareholders.get(locals.j)) {

                            output.result = QUCPSP_CREATE_DUPLICATE_SHAREHOLDER;
                            return;
                        }
                    }
                }

                locals.totalShares += input.shares.get(locals.i);
            }
        }

        state.mut().shares.reset();
        state.mut().released.reset();

        for (locals.i = 0; locals.i < QUCPSP_MAX_SHAREHOLDERS; locals.i++) {
            if (locals.i < input.shareholderCount) {

                state.mut().shareholders.set(
                    locals.i,
                    input.shareholders.get(locals.i)
                );

                state.mut().shares.set(
                    input.shareholders.get(locals.i),
                    input.shares.get(locals.i)
                );

                state.mut().released.set(
                    input.shareholders.get(locals.i),
                    0
                );
            }
        }

        state.mut().splitter.shareholderCount = input.shareholderCount;
        state.mut().splitter.totalShares = locals.totalShares;
        state.mut().splitter.totalReceived = 0;
        state.mut().splitter.active = 1;

        output.result = QUCPSP_CREATE_SUCCESS;
        return;
    }

    PUBLIC_FUNCTION(view) {
        output.shareholderCount = state.get().splitter.shareholderCount;
        output.totalShares = state.get().splitter.totalShares;
        output.totalReceived = state.get().splitter.totalReceived;
        output.active = state.get().splitter.active;
    }

    PUBLIC_PROCEDURE(receive) {
        if (!state.get().splitter.active) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCPSP_RECEIVE_NOT_REGISTERED;
            return;
        }

        if (input.amount == 0 ||
            qpi.invocationReward() != input.amount + QUCPSP_FEE) {

            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCPSP_RECEIVE_INVALID_AMOUNT;
            return;
        }

        state.mut().splitter.totalReceived += input.amount;

        qpi.burn(QUCPSP_FEE);

        output.result = QUCPSP_RECEIVE_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(release) {
        if (!state.get().splitter.active) {
            output.result = QUCPSP_RELEASE_NOT_REGISTERED;
            return;
        }

        locals.shareholderShares = 0;

        if (!state.get().shares.get(
            input.shareholder,
            locals.shareholderShares
        )) {
            output.result = QUCPSP_RELEASE_INVALID_SHAREHOLDER;
            return;
        }

        locals.alreadyReleased = 0;

        state.get().released.get(
            input.shareholder,
            locals.alreadyReleased
        );

        locals.numerator =
            uint128(state.get().splitter.totalReceived) *
            uint128(locals.shareholderShares);

        locals.calculatedPayment = div(
            locals.numerator,
            uint128(state.get().splitter.totalShares)
        );

        locals.payment =
            uint64(locals.calculatedPayment.low) -
            locals.alreadyReleased;

        if (locals.payment == 0) {
            output.result = QUCPSP_RELEASE_NO_PAYMENT;
            return;
        }

        qpi.transfer(input.shareholder, locals.payment);

        state.mut().released.set(
            input.shareholder,
            locals.alreadyReleased + locals.payment
        );

        output.result = QUCPSP_RELEASE_SUCCESS;
        return;
    }
};
