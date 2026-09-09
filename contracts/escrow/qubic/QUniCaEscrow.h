using namespace QPI;

constexpr uint64 QUCESCR_CREATE_SUCCESS = 0;
constexpr uint64 QUCESCR_CREATE_ALREADY_REGISTERED = 1;
constexpr uint64 QUCESCR_CREATE_INVALID_BUYER = 2;
constexpr uint64 QUCESCR_CREATE_INVALID_AMOUNT = 3;

constexpr uint64 QUCESCR_DEPOSIT_SUCCESS = 0;
constexpr uint64 QUCESCR_DEPOSIT_NOT_REGISTERED = 1;
constexpr uint64 QUCESCR_DEPOSIT_ALREADY_SET = 2;
constexpr uint64 QUCESCR_DEPOSIT_INVALID_ID = 3;
constexpr uint64 QUCESCR_DEPOSIT_NOT_SUFFICIENT_AMOUNT = 4;

constexpr uint64 QUCESCR_PAY_SUCCESS = 0;
constexpr uint64 QUCESCR_PAY_NOT_REGISTERED = 1;
constexpr uint64 QUCESCR_PAY_NOT_SET = 2;
constexpr uint64 QUCESCR_PAY_INVALID_ID = 3;

constexpr uint64 QUCESCR_REFUND_SUCCESS = 0;
constexpr uint64 QUCESCR_REFUND_NOT_REGISTERED = 1;
constexpr uint64 QUCESCR_REFUND_NOT_SET = 2;
constexpr uint64 QUCESCR_REFUND_INVALID_ID = 3;

constexpr uint64 QUCESCR_FEE = 100;

struct QUCESCR2
{
};

struct QUCESCR : public ContractBase
{
    struct EscrowInfo
    {
        id seller;
        id buyer;
        uint64 amount;
        bit active;
        uint8 deposit;
    };

    struct StateData
    {
        EscrowInfo escrow;
    };

    struct create_input
    {
        id buyer;
        uint64 amount;
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
        uint64 amount;
        uint8 deposit;
    };

    struct deposit_input
    {
    };

    struct deposit_output
    {
        uint64 result;
    };

    struct pay_input
    {
    };

    struct pay_output
    {
        uint64 result;
    };

    struct refund_input
    {
    };

    struct refund_output
    {
        uint64 result;
    };

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_PROCEDURE(create, 1);
        REGISTER_USER_FUNCTION(view, 1);
        REGISTER_USER_PROCEDURE(deposit, 2);
        REGISTER_USER_PROCEDURE(pay, 3);
        REGISTER_USER_PROCEDURE(refund, 4);
    }

    PUBLIC_PROCEDURE(create) {

        if (state.get().escrow.active) {
            output.result = QUCESCR_CREATE_ALREADY_REGISTERED;
            return;
        }

        if (input.buyer == NULL_ID || input.buyer == qpi.invocator()) {
            output.result = QUCESCR_CREATE_INVALID_BUYER;
            return;
        }

        if (input.amount == 0) {
            output.result = QUCESCR_CREATE_INVALID_AMOUNT;
            return;
        }

        state.mut().escrow.seller = qpi.invocator();
        state.mut().escrow.buyer = input.buyer;
        state.mut().escrow.amount = input.amount;
        state.mut().escrow.active = 1;
        state.mut().escrow.deposit = 0;

        output.result = QUCESCR_CREATE_SUCCESS;
        return;
    }

    PUBLIC_FUNCTION(view) {
        output.amount = state.get().escrow.amount;
        output.deposit = state.get().escrow.deposit;
    }

    PUBLIC_PROCEDURE(deposit) {

        if (!state.get().escrow.active) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCESCR_DEPOSIT_NOT_REGISTERED;
            return;
        }

        if (state.get().escrow.deposit) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCESCR_DEPOSIT_ALREADY_SET;
            return;
        }

        if (qpi.invocator() != state.get().escrow.buyer) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCESCR_DEPOSIT_INVALID_ID;
            return;
        }

        if (qpi.invocationReward() != state.get().escrow.amount + QUCESCR_FEE) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCESCR_DEPOSIT_NOT_SUFFICIENT_AMOUNT;
            return;
        }

        state.mut().escrow.deposit = 1;

        qpi.burn(QUCESCR_FEE);

        output.result = QUCESCR_DEPOSIT_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE(pay) {

        if (!state.get().escrow.active) {
            output.result = QUCESCR_PAY_NOT_REGISTERED;
            return;
        }

        if (!state.get().escrow.deposit) {
            output.result = QUCESCR_PAY_NOT_SET;
            return;
        }

        if (qpi.invocator() != state.get().escrow.buyer) {
            output.result = QUCESCR_PAY_INVALID_ID;
            return;
        }

        qpi.transfer(state.get().escrow.seller, state.get().escrow.amount);

        state.mut().escrow.seller = NULL_ID;
        state.mut().escrow.buyer = NULL_ID;
        state.mut().escrow.amount = 0;
        state.mut().escrow.active = 0;
        state.mut().escrow.deposit = 0;

        output.result = QUCESCR_PAY_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE(refund) {

        if (!state.get().escrow.active) {
            output.result = QUCESCR_REFUND_NOT_REGISTERED;
            return;
        }

        if (!state.get().escrow.deposit) {
            output.result = QUCESCR_REFUND_NOT_SET;
            return;
        }

        if (qpi.invocator() != state.get().escrow.seller) {
            output.result = QUCESCR_REFUND_INVALID_ID;
            return;
        }

        qpi.transfer(state.get().escrow.buyer, state.get().escrow.amount);

        state.mut().escrow.seller = NULL_ID;
        state.mut().escrow.buyer = NULL_ID;
        state.mut().escrow.amount = 0;
        state.mut().escrow.active = 0;
        state.mut().escrow.deposit = 0;

        output.result = QUCESCR_REFUND_SUCCESS;
        return;
    }
};
