using namespace QPI;

constexpr uint64 QUCTT_CREATE_SUCCESS = 0;
constexpr uint64 QUCTT_CREATE_ALREADY_REGISTERED = 1;
constexpr uint64 QUCTT_CREATE_INVALID_RECEIVER = 2;

constexpr uint64 QUCTT_ISSUE_SUCCESS = 0;
constexpr uint64 QUCTT_ISSUE_INVALID_AMOUNT = 1;
constexpr uint64 QUCTT_ISSUE_INVALID_REWARD = 2;
constexpr uint64 QUCTT_ISSUE_FAILED = 3;

constexpr uint64 QUCTT_DEPOSIT_SUCCESS = 0;
constexpr uint64 QUCTT_DEPOSIT_NOT_REGISTERED = 1;
constexpr uint64 QUCTT_DEPOSIT_INVALID_ID = 2;
constexpr uint64 QUCTT_DEPOSIT_NOT_SUFFICIENT_AMOUNT = 3;

constexpr uint64 QUCTT_WITHDRAW_SUCCESS = 0;
constexpr uint64 QUCTT_WITHDRAW_NOT_REGISTERED = 1;
constexpr uint64 QUCTT_WITHDRAW_INVALID_ID = 2;
constexpr uint64 QUCTT_WITHDRAW_NOT_SUFFICIENT_AMOUNT = 3;

constexpr uint64 QUCTT_ISSUE_FEE = 100;
constexpr uint64 QUCTT_FEE = 100;

struct QUCTT2
{
};

struct QUCTT : public ContractBase
{
    struct TransferInfo
    {
        id owner;
        id receiver;
        id issuer;
        uint64 assetName;
        sint64 amount;
        bit active;
    };

    struct StateData
    {
        TransferInfo transfer;
    };

    struct create_input
    {
        id receiver;
        id issuer;
        uint64 assetName;
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
        sint64 amount;
    };

    struct issue_input
    {
        uint64 assetName;
        sint64 numberOfShares;
    };

    struct issue_output
    {
        uint64 result;
    };

    struct deposit_input
    {
        sint64 amount_deposit;
    };

    struct deposit_output
    {
        uint64 result;
    };

    struct withdraw_input
    {
        sint64 amount_receive;
    };

    struct withdraw_output
    {
        uint64 result;
    };

    struct issue_locals
    {
        sint64 issuedShares;
    };

    struct deposit_locals
    {
        sint64 transferredAmount;
    };

    struct withdraw_locals
    {
        sint64 transferredAmount;
    };

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_PROCEDURE(create, 1);
        REGISTER_USER_FUNCTION(view, 1);
        REGISTER_USER_PROCEDURE(issue, 2);
        REGISTER_USER_PROCEDURE(deposit, 3);
        REGISTER_USER_PROCEDURE(withdraw, 4);
    }

    PUBLIC_PROCEDURE(create) {
        if (state.get().transfer.active) {
            output.result = QUCTT_CREATE_ALREADY_REGISTERED;
            return;
        }

        if (input.receiver == NULL_ID || input.receiver == qpi.invocator()) {
            output.result = QUCTT_CREATE_INVALID_RECEIVER;
            return;
        }

        state.mut().transfer.owner = qpi.invocator();
        state.mut().transfer.receiver = input.receiver;
        state.mut().transfer.issuer = input.issuer;
        state.mut().transfer.assetName = input.assetName;
        state.mut().transfer.amount = 0;
        state.mut().transfer.active = 1;

        output.result = QUCTT_CREATE_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(issue) {
        if (input.numberOfShares <= 0) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCTT_ISSUE_INVALID_AMOUNT;
            return;
        }

        if (qpi.invocationReward() != QUCTT_ISSUE_FEE) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCTT_ISSUE_INVALID_REWARD;
            return;
        }

        locals.issuedShares = qpi.issueAsset(input.assetName, qpi.invocator(), 0, input.numberOfShares, 0);

        if (locals.issuedShares <= 0) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCTT_ISSUE_FAILED;
            return;
        }

        qpi.burn(QUCTT_ISSUE_FEE);

        output.result = QUCTT_ISSUE_SUCCESS;
    }

    PUBLIC_FUNCTION(view) {
        output.amount = state.get().transfer.amount;
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(deposit) {
        if (!state.get().transfer.active) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCTT_DEPOSIT_NOT_REGISTERED;
            return;
        }

        if (qpi.invocator() != state.get().transfer.owner) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCTT_DEPOSIT_INVALID_ID;
            return;
        }

        if (input.amount_deposit <= 0 || qpi.invocationReward() != QUCTT_FEE) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCTT_DEPOSIT_NOT_SUFFICIENT_AMOUNT;
            return;
        }

        locals.transferredAmount = qpi.transferShareOwnershipAndPossession(
            state.get().transfer.assetName,
            state.get().transfer.issuer,
            state.get().transfer.owner,
            state.get().transfer.owner,
            input.amount_deposit,
            SELF
        );

        if (locals.transferredAmount < 0) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCTT_DEPOSIT_NOT_SUFFICIENT_AMOUNT;
            return;
        }

        state.mut().transfer.amount += input.amount_deposit;

        qpi.burn(QUCTT_FEE);

        output.result = QUCTT_DEPOSIT_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(withdraw) {
        if (!state.get().transfer.active) {
            output.result = QUCTT_WITHDRAW_NOT_REGISTERED;
            return;
        }

        if (qpi.invocator() != state.get().transfer.receiver) {
            output.result = QUCTT_WITHDRAW_INVALID_ID;
            return;
        }

        if (input.amount_receive <= 0 || input.amount_receive > state.get().transfer.amount) {
            output.result = QUCTT_WITHDRAW_NOT_SUFFICIENT_AMOUNT;
            return;
        }

        locals.transferredAmount = qpi.transferShareOwnershipAndPossession(
            state.get().transfer.assetName,
            state.get().transfer.issuer,
            SELF,
            SELF,
            input.amount_receive,
            qpi.invocator()
        );

        if (locals.transferredAmount < 0) {
            output.result = QUCTT_WITHDRAW_NOT_SUFFICIENT_AMOUNT;
            return;
        }

        state.mut().transfer.amount -= input.amount_receive;

        output.result = QUCTT_WITHDRAW_SUCCESS;
        return;
    }
};
