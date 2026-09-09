using namespace QPI;

constexpr uint64 QUCCRF_CREATE_SUCCESS = 0;
constexpr uint64 QUCCRF_CREATE_ALREADY_REGISTERED = 1;
constexpr uint64 QUCCRF_CREATE_INVALID_RECIPIENT = 2;
constexpr uint64 QUCCRF_CREATE_INVALID_GOAL = 3;
constexpr uint64 QUCCRF_CREATE_INVALID_DELAY = 4;

constexpr uint64 QUCCRF_DONATE_SUCCESS = 0;
constexpr uint64 QUCCRF_DONATE_NOT_REGISTERED = 1;
constexpr uint64 QUCCRF_DONATE_TIMEOUT = 2;
constexpr uint64 QUCCRF_DONATE_INVALID_AMOUNT = 3;
constexpr uint64 QUCCRF_DONATE_FULL = 4;

constexpr uint64 QUCCRF_WITHDRAW_SUCCESS = 0;
constexpr uint64 QUCCRF_WITHDRAW_NOT_REGISTERED = 1;
constexpr uint64 QUCCRF_WITHDRAW_NOT_EXPIRED = 2;
constexpr uint64 QUCCRF_WITHDRAW_INVALID_ID = 3;
constexpr uint64 QUCCRF_WITHDRAW_GOAL_NOT_REACHED = 4;

constexpr uint64 QUCCRF_RECLAIM_SUCCESS = 0;
constexpr uint64 QUCCRF_RECLAIM_NOT_REGISTERED = 1;
constexpr uint64 QUCCRF_RECLAIM_NOT_EXPIRED = 2;
constexpr uint64 QUCCRF_RECLAIM_GOAL_REACHED = 3;
constexpr uint64 QUCCRF_RECLAIM_NO_DONATION = 4;

constexpr uint64 QUCCRF_FEE = 100;
constexpr uint64 MAX_DONORS = 1024;

struct QUCCRF2
{
};

struct QUCCRF : public ContractBase
{
    struct CrowdfundInfo
    {
        id recipient;
        uint64 goal;
        uint64 deadline;
        uint64 totalDonated;
        bit active;
    };

    struct StateData
    {
        CrowdfundInfo crowdfund;
        HashMap<id, uint64, MAX_DONORS> donations;
    };

    struct create_input
    {
        id recipient;
        uint64 goal;
        uint64 delay;
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
        uint64 goal;
        uint64 deadline;
        uint64 totalDonated;
        uint8 active;
    };

    struct donate_input
    {
        uint64 amount_donation;
    };

    struct donate_output
    {
        uint64 result;
    };

    struct withdraw_input
    {
    };

    struct withdraw_output
    {
        uint64 result;
    };

    struct reclaim_input
    {
    };

    struct reclaim_output
    {
        uint64 result;
    };

    struct donate_locals
    {
        uint64 previousDonation;
    };

    struct reclaim_locals
    {
        uint64 donation;
    };

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_PROCEDURE(create, 1);
        REGISTER_USER_FUNCTION(view, 1);
        REGISTER_USER_PROCEDURE(donate, 2);
        REGISTER_USER_PROCEDURE(withdraw, 3);
        REGISTER_USER_PROCEDURE(reclaim, 3);
    }

    PUBLIC_PROCEDURE(create) {
        if (state.get().crowdfund.active) {
            output.result = QUCCRF_CREATE_ALREADY_REGISTERED;
            return;
        }

        if (input.recipient == NULL_ID) {
            output.result = QUCCRF_CREATE_INVALID_RECIPIENT;
            return;
        }

        if (input.goal == 0) {
            output.result = QUCCRF_CREATE_INVALID_GOAL;
            return;
        }

        if (input.delay == 0) {
            output.result = QUCCRF_CREATE_INVALID_DELAY;
            return;
        }

        state.mut().crowdfund.recipient = input.recipient;
        state.mut().crowdfund.goal = input.goal;
        state.mut().crowdfund.deadline = qpi.tick() + input.delay;
        state.mut().crowdfund.totalDonated = 0;
        state.mut().crowdfund.active = 1;

        state.mut().donations.reset();

        output.result = QUCCRF_CREATE_SUCCESS;
        return;
    }

    PUBLIC_FUNCTION(view) {
        output.goal = state.get().crowdfund.goal;
        output.deadline = state.get().crowdfund.deadline;
        output.totalDonated = state.get().crowdfund.totalDonated;
        output.active = state.get().crowdfund.active;
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(donate) {
        if (!state.get().crowdfund.active) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCCRF_DONATE_NOT_REGISTERED;
            return;
        }

        if (qpi.tick() > state.get().crowdfund.deadline) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCCRF_DONATE_TIMEOUT;
            return;
        }

        if (input.amount_donation == 0 || qpi.invocationReward() != input.amount_donation + QUCCRF_FEE) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCCRF_DONATE_INVALID_AMOUNT;
            return;
        }

        locals.previousDonation = 0;

        if (state.get().donations.get(qpi.invocator(), locals.previousDonation)) {
            state.mut().donations.set(
                qpi.invocator(),
                locals.previousDonation + input.amount_donation
            );
        }
        else {
            if (state.get().donations.population() >= state.get().donations.capacity()) {
                qpi.transfer(qpi.invocator(), qpi.invocationReward());
                output.result = QUCCRF_DONATE_FULL;
                return;
            }

            state.mut().donations.set(qpi.invocator(), input.amount_donation);
        }

        state.mut().crowdfund.totalDonated += input.amount_donation;

        qpi.burn(QUCCRF_FEE);

        output.result = QUCCRF_DONATE_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE(withdraw) {
        if (!state.get().crowdfund.active) {
            output.result = QUCCRF_WITHDRAW_NOT_REGISTERED;
            return;
        }

        if (qpi.tick() <= state.get().crowdfund.deadline) {
            output.result = QUCCRF_WITHDRAW_NOT_EXPIRED;
            return;
        }

        if (qpi.invocator() != state.get().crowdfund.recipient) {
            output.result = QUCCRF_WITHDRAW_INVALID_ID;
            return;
        }

        if (state.get().crowdfund.totalDonated < state.get().crowdfund.goal) {
            output.result = QUCCRF_WITHDRAW_GOAL_NOT_REACHED;
            return;
        }

        qpi.transfer(state.get().crowdfund.recipient, state.get().crowdfund.totalDonated);

        state.mut().crowdfund.recipient = NULL_ID;
        state.mut().crowdfund.goal = 0;
        state.mut().crowdfund.deadline = 0;
        state.mut().crowdfund.totalDonated = 0;
        state.mut().crowdfund.active = 0;

        state.mut().donations.reset();

        output.result = QUCCRF_WITHDRAW_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(reclaim) {
        if (!state.get().crowdfund.active) {
            output.result = QUCCRF_RECLAIM_NOT_REGISTERED;
            return;
        }

        if (qpi.tick() <= state.get().crowdfund.deadline) {
            output.result = QUCCRF_RECLAIM_NOT_EXPIRED;
            return;
        }

        if (state.get().crowdfund.totalDonated >= state.get().crowdfund.goal) {
            output.result = QUCCRF_RECLAIM_GOAL_REACHED;
            return;
        }

        locals.donation = 0;

        if (!state.get().donations.get(qpi.invocator(), locals.donation)) {
            output.result = QUCCRF_RECLAIM_NO_DONATION;
            return;
        }

        qpi.transfer(qpi.invocator(), locals.donation);

        state.mut().crowdfund.totalDonated -= locals.donation;
        state.mut().donations.removeByKey(qpi.invocator());

        if (state.get().crowdfund.totalDonated == 0) {
            state.mut().crowdfund.recipient = NULL_ID;
            state.mut().crowdfund.goal = 0;
            state.mut().crowdfund.deadline = 0;
            state.mut().crowdfund.totalDonated = 0;
            state.mut().crowdfund.active = 0;

            state.mut().donations.reset();
        }

        output.result = QUCCRF_RECLAIM_SUCCESS;
        return;
    }
};
