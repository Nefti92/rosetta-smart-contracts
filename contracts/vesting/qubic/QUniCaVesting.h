using namespace QPI;

constexpr uint64 QUCVST_CREATE_SUCCESS = 0;
constexpr uint64 QUCVST_CREATE_INVALID_BENEFICIARY = 1;
constexpr uint64 QUCVST_CREATE_INVALID_AMOUNT = 2;
constexpr uint64 QUCVST_CREATE_INVALID_DURATION = 3;
constexpr uint64 QUCVST_CREATE_INVALID_START = 4;
constexpr uint64 QUCVST_CREATE_ALREADY_REGISTERED = 5;
constexpr uint64 QUCVST_CREATE_INVALID_REWARD = 6;

constexpr uint64 QUCVST_RELEASE_SUCCESS = 0;
constexpr uint64 QUCVST_RELEASE_NOT_REGISTERED = 1;
constexpr uint64 QUCVST_RELEASE_UNAUTHORIZED = 2;
constexpr uint64 QUCVST_RELEASE_NOTHING = 3;

constexpr uint64 QUCVST_FEE = 100;

struct QUCVST2
{
};

struct QUCVST : public ContractBase
{
    struct VestingInfo
    {
        id beneficiary;
        uint64 start;
        uint64 duration;
        uint64 amount;
        uint64 released;
        bit active;
    };

    struct StateData
    {
        VestingInfo vesting;
    };

    struct create_input
    {
        id beneficiary;
        uint64 start;
        uint64 duration;
        uint64 amount;
    };

    struct create_output
    {
        uint64 result;
        uint64 amount;
    };

    struct release_input
    {
    };

    struct release_output
    {
        uint64 result;
        uint64 amount;
    };

    struct release_locals
    {
        uint64 elapsed;
        uint64 vested;
        uint64 releasable;
        uint128 numerator;
        uint128 calculatedVested;
    };

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_PROCEDURE(create, 1);
        REGISTER_USER_PROCEDURE(release, 2);
    }

    PUBLIC_PROCEDURE(create) {

        if (state.get().vesting.active) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCVST_CREATE_ALREADY_REGISTERED;
            return;
        }

        if (input.beneficiary == NULL_ID || input.beneficiary == qpi.invocator()) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCVST_CREATE_INVALID_BENEFICIARY;
            return;
        }

        if (input.amount == 0) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCVST_CREATE_INVALID_AMOUNT;
            return;
        }

        if (input.duration == 0) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCVST_CREATE_INVALID_DURATION;
            return;
        }

        if (input.start <= qpi.tick()) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCVST_CREATE_INVALID_START;
            return;
        }

        if (qpi.invocationReward() != input.amount + QUCVST_FEE) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCVST_CREATE_INVALID_REWARD;
            return;
        }

        state.mut().vesting.beneficiary = input.beneficiary;
        state.mut().vesting.start = input.start;
        state.mut().vesting.duration = input.duration;
        state.mut().vesting.amount = input.amount;
        state.mut().vesting.released = 0;
        state.mut().vesting.active = 1;

        qpi.burn(QUCVST_FEE);

        output.amount = state.get().vesting.amount;
        output.result = QUCVST_CREATE_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(release) {

        if (!state.get().vesting.active) {
            output.result = QUCVST_RELEASE_NOT_REGISTERED;
            return;
        }

        if (qpi.invocator() != state.get().vesting.beneficiary) {
            output.result = QUCVST_RELEASE_UNAUTHORIZED;
            return;
        }

        if (qpi.tick() < state.get().vesting.start) {
            output.amount = 0;
            output.result = QUCVST_RELEASE_NOTHING;
            return;
        }

        if (qpi.tick() >= state.get().vesting.start + state.get().vesting.duration) {
            locals.vested = state.get().vesting.amount;
        }
        else {
            locals.elapsed = qpi.tick() - state.get().vesting.start;

            locals.numerator =
                uint128(state.get().vesting.amount) *
                uint128(locals.elapsed);

            locals.calculatedVested = div(
                locals.numerator,
                uint128(state.get().vesting.duration)
            );

            locals.vested = uint64(locals.calculatedVested.low);
        }

        if (locals.vested <= state.get().vesting.released) {
            output.amount = 0;
            output.result = QUCVST_RELEASE_NOTHING;
            return;
        }

        locals.releasable =
            locals.vested -
            state.get().vesting.released;

        qpi.transfer(
            state.get().vesting.beneficiary,
            locals.releasable
        );

        state.mut().vesting.released =
            state.get().vesting.released +
            locals.releasable;

        output.amount = locals.releasable;
        output.result = QUCVST_RELEASE_SUCCESS;

        if (state.get().vesting.released == state.get().vesting.amount) {
            state.mut().vesting.beneficiary = NULL_ID;
            state.mut().vesting.start = 0;
            state.mut().vesting.duration = 0;
            state.mut().vesting.amount = 0;
            state.mut().vesting.released = 0;
            state.mut().vesting.active = 0;
        }

        return;
    }
};
