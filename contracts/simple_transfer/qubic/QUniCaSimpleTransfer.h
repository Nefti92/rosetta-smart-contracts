using namespace QPI;

constexpr uint64 QUCST_CREATE_SUCCESS = 0;
constexpr uint64 QUCST_CREATE_ALREADY_REGISTERED = 1;
constexpr uint64 QUCST_CREATE_INVALID_RECEIVER = 2;

constexpr uint64 QUCST_DEPOSIT_SUCCESS = 0;
constexpr uint64 QUCST_DEPOSIT_NOT_REGISTERED = 1;
constexpr uint64 QUCST_DEPOSIT_INVALID_ID = 2;
constexpr uint64 QUCST_DEPOSIT_NOT_SUFFICIENT_AMOUNT = 3;

constexpr uint64 QUCST_WITHDRAW_SUCCESS = 0;
constexpr uint64 QUCST_WITHDRAW_NOT_REGISTERED = 1;
constexpr uint64 QUCST_WITHDRAW_INVALID_ID = 2;
constexpr uint64 QUCST_WITHDRAW_NOT_SUFFICIENT_AMOUNT = 3;

constexpr uint64 QUCST_FEE = 100;

struct QUCST2
{
};

struct QUCST : public ContractBase
{
    struct TransferInfo
    {
        id owner;
        id receiver;
        uint64 amount;
        bit active;
    };

    struct StateData
    {
        TransferInfo transfer;
    };

    struct create_input
    {
        id receiver;
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
    };

    struct deposit_input
    {
        uint64 amount_deposit;
    };

    struct deposit_output
    {
        uint64 result;
    };

    struct withdraw_input
    {
        uint64 amount_receive;
    };

    struct withdraw_output
    {
        uint64 result;
    };

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_PROCEDURE(create, 1);
        REGISTER_USER_FUNCTION(view, 1);
        REGISTER_USER_PROCEDURE(deposit, 3);
        REGISTER_USER_PROCEDURE(withdraw, 4);
    }

    PUBLIC_PROCEDURE(create) {

        if (state.get().transfer.active) {
            output.result = QUCST_CREATE_ALREADY_REGISTERED;
            return;
        }

        if (input.receiver == NULL_ID || input.receiver == qpi.invocator()) {
            output.result = QUCST_CREATE_INVALID_RECEIVER;
            return;
        }

        state.mut().transfer.owner = qpi.invocator();
        state.mut().transfer.receiver = input.receiver;
        state.mut().transfer.amount = 0;
        state.mut().transfer.active = 1;

        output.result = QUCST_CREATE_SUCCESS;
        return;
    }

    PUBLIC_FUNCTION(view) {
        output.amount = state.get().transfer.amount;
    }

    PUBLIC_PROCEDURE(deposit) {

        if (!state.get().transfer.active) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCST_DEPOSIT_NOT_REGISTERED;
            return;
        }

        if (qpi.invocator() != state.get().transfer.owner) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCST_DEPOSIT_INVALID_ID;
            return;
        }

        if (qpi.invocationReward() != input.amount_deposit + QUCST_FEE) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCST_DEPOSIT_NOT_SUFFICIENT_AMOUNT;
            return;
        }

        state.mut().transfer.amount += input.amount_deposit;

        qpi.burn(QUCST_FEE);

        output.result = QUCST_DEPOSIT_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE(withdraw) {
        
        if (!state.get().transfer.active) {
            output.result = QUCST_WITHDRAW_NOT_REGISTERED;
            return;
        }

        if (qpi.invocator() != state.get().transfer.receiver) {
            output.result = QUCST_WITHDRAW_INVALID_ID;
            return;
        }

        if (input.amount_receive > state.get().transfer.amount) {
            output.result = QUCST_WITHDRAW_NOT_SUFFICIENT_AMOUNT;
            return;
        }

        qpi.transfer(qpi.invocator(), input.amount_receive);
        state.mut().transfer.amount -= input.amount_receive;

        output.result = QUCST_WITHDRAW_SUCCESS;
        return;
    }
};
