using namespace QPI;

constexpr uint64 QUCPRB_CREATE_SUCCESS = 0;
constexpr uint64 QUCPRB_CREATE_INVALID_AMOUNT = 1;
constexpr uint64 QUCPRB_CREATE_INVALID_RATE = 2;
constexpr uint64 QUCPRB_CREATE_ALREADY_REGISTERED = 3;
constexpr uint64 QUCPRB_CREATE_INVALID_REWARD = 4;

constexpr uint64 QUCPRB_JOIN_SUCCESS = 0;
constexpr uint64 QUCPRB_JOIN_TIMEOUT = 1;
constexpr uint64 QUCPRB_JOIN_INVALID_AMOUNT = 2;
constexpr uint64 QUCPRB_JOIN_ALREADY_JOINED = 3;
constexpr uint64 QUCPRB_JOIN_UNAUTHORIZED = 4;

constexpr uint64 QUCPRB_WIN_SUCCESS = 0;
constexpr uint64 QUCPRB_WIN_UNAUTHORIZED = 1;
constexpr uint64 QUCPRB_WIN_PLAYER_NOT_READY = 2;
constexpr uint64 QUCPRB_WIN_TIMEOUT = 3;
constexpr uint64 QUCPRB_WIN_ORACLE_ERROR = 4;
constexpr uint64 QUCPRB_WIN_RATE_NOT_REACHED = 5;

constexpr uint64 QUCPRB_TIMEOUT_SUCCESS = 0;
constexpr uint64 QUCPRB_TIMEOUT_UNAUTHORIZED = 1;
constexpr uint64 QUCPRB_TIMEOUT_NOT_STARTED = 2;
constexpr uint64 QUCPRB_TIMEOUT_NOT_EXPIRED = 3;

//One tick lasts from 0.2 seconds to 5 seconds
//900 ticks range from 3 minutes to 75 minutes
constexpr uint64 QUCPRB_TIMEOUT = 900;
constexpr uint64 QUCPRB_FEE = 100;

struct QUCPRB2
{
};

struct QUCPRB : public ContractBase
{
    struct BetInfo
    {
        id owner;
        id player;
        uint64 deadline;
        uint64 amountBet;
        uint64 betRate;
        bit playerApproved;
        bit start;
    };

    struct StateData
    {
        BetInfo bet;
    };

    struct create_input
    {
        uint64 amountBet;
        uint64 betRate;
    };

    struct create_output
    {
        uint64 result;
        uint64 amountBet;
    };

    struct join_input
    {
    };

    struct join_output
    {
        uint64 result;
    };

    struct win_input
    {
    };

    struct win_output
    {
        uint64 result;
    };

    struct timeout_input
    {
    };

    struct timeout_output
    {
        uint64 result;
    };

    struct win_locals
    {
        QUCPRBO::getRate_input oracleInput;
        QUCPRBO::getRate_output oracleOutput;
    };

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_PROCEDURE(create, 1);
        REGISTER_USER_PROCEDURE(join, 2);
        REGISTER_USER_PROCEDURE(win, 3);
        REGISTER_USER_PROCEDURE(timeout, 4);
    }

    PUBLIC_PROCEDURE(create) {

        if (state.get().bet.start) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCPRB_CREATE_ALREADY_REGISTERED;
            return;
        }

        if (input.amountBet == 0) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCPRB_CREATE_INVALID_AMOUNT;
            return;
        }

        if (input.betRate == 0) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCPRB_CREATE_INVALID_RATE;
            return;
        }

        if (qpi.invocationReward() != input.amountBet + QUCPRB_FEE) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCPRB_CREATE_INVALID_REWARD;
            return;
        }

        state.mut().bet.deadline = qpi.tick() + QUCPRB_TIMEOUT;
        state.mut().bet.start = 1;
        state.mut().bet.owner = qpi.invocator();
        state.mut().bet.player = NULL_ID;
        state.mut().bet.amountBet = input.amountBet;
        state.mut().bet.betRate = input.betRate;
        state.mut().bet.playerApproved = 0;

        qpi.burn(QUCPRB_FEE);

        output.amountBet = state.get().bet.amountBet;
        output.result = QUCPRB_CREATE_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE(join) {

        if (!state.get().bet.start) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCPRB_JOIN_UNAUTHORIZED;
            return;
        }

        if (qpi.tick() > state.get().bet.deadline) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCPRB_JOIN_TIMEOUT;
            return;
        }

        if (qpi.invocator() == state.get().bet.owner) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCPRB_JOIN_UNAUTHORIZED;
            return;
        }

        if (state.get().bet.playerApproved) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCPRB_JOIN_ALREADY_JOINED;
            return;
        }

        if (qpi.invocationReward() != state.get().bet.amountBet + QUCPRB_FEE) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCPRB_JOIN_INVALID_AMOUNT;
            return;
        }

        qpi.burn(QUCPRB_FEE);

        state.mut().bet.player = qpi.invocator();
        state.mut().bet.playerApproved = 1;

        output.result = QUCPRB_JOIN_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(win) {

        if (!state.get().bet.start || !state.get().bet.playerApproved) {
            output.result = QUCPRB_WIN_PLAYER_NOT_READY;
            return;
        }

        if (qpi.tick() > state.get().bet.deadline) {
            output.result = QUCPRB_WIN_TIMEOUT;
            return;
        }

        if (qpi.invocator() != state.get().bet.player) {
            output.result = QUCPRB_WIN_UNAUTHORIZED;
            return;
        }

        CALL_OTHER_CONTRACT_FUNCTION(
            QUCPRBO,
            getRate,
            locals.oracleInput,
            locals.oracleOutput
        );

        if (interContractCallError != NoCallError) {
            output.result = QUCPRB_WIN_ORACLE_ERROR;
            return;
        }

        if (locals.oracleOutput.rate <= state.get().bet.betRate) {
            output.result = QUCPRB_WIN_RATE_NOT_REACHED;
            return;
        }

        qpi.transfer(
            state.get().bet.player,
            state.get().bet.amountBet * 2
        );

        state.mut().bet.owner = NULL_ID;
        state.mut().bet.player = NULL_ID;
        state.mut().bet.deadline = 0;
        state.mut().bet.amountBet = 0;
        state.mut().bet.betRate = 0;
        state.mut().bet.playerApproved = 0;
        state.mut().bet.start = 0;

        output.result = QUCPRB_WIN_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE(timeout) {

        if (!state.get().bet.start) {
            output.result = QUCPRB_TIMEOUT_NOT_STARTED;
            return;
        }

        if (qpi.invocator() != state.get().bet.owner) {
            output.result = QUCPRB_TIMEOUT_UNAUTHORIZED;
            return;
        }

        if (qpi.tick() <= state.get().bet.deadline) {
            output.result = QUCPRB_TIMEOUT_NOT_EXPIRED;
            return;
        }

        if (state.get().bet.playerApproved) {
            qpi.transfer(
                state.get().bet.owner,
                state.get().bet.amountBet * 2
            );
        }
        else {
            qpi.transfer(
                state.get().bet.owner,
                state.get().bet.amountBet
            );
        }

        state.mut().bet.owner = NULL_ID;
        state.mut().bet.player = NULL_ID;
        state.mut().bet.deadline = 0;
        state.mut().bet.amountBet = 0;
        state.mut().bet.betRate = 0;
        state.mut().bet.playerApproved = 0;
        state.mut().bet.start = 0;

        output.result = QUCPRB_TIMEOUT_SUCCESS;
        return;
    }
};