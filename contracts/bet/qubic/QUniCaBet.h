using namespace QPI;

constexpr uint64 QUCBET_CREATE_SUCCESS = 0;
constexpr uint64 QUCBET_CREATE_INVALID_ORACLE = 1;
constexpr uint64 QUCBET_CREATE_INVALID_AMOUNT = 2;
constexpr uint64 QUCBET_CREATE_ALREADY_REGISTERED = 3;
constexpr uint64 QUCBET_CREATE_BET_FULL = 4;

constexpr uint64 QUCBET_JOIN_SUCCESS = 0;
constexpr uint64 QUCBET_JOIN_TIMEOUT = 1;
constexpr uint64 QUCBET_JOIN_INVALID_AMOUNT = 2;
constexpr uint64 QUCBET_JOIN_UNAUTHORIZED = 3;
constexpr uint64 QUCBET_JOIN_ALREADY_JOINED = 4;

constexpr uint64 QUCBET_WIN_SUCCESS = 0;
constexpr uint64 QUCBET_WIN_UNAUTHORIZED = 1;
constexpr uint64 QUCBET_WIN_PLAYERS_NOT_READY = 2;
constexpr uint64 QUCBET_WIN_TIMEOUT = 3;
constexpr uint64 QUCBET_WIN_INVALID_WINNER = 4;
//One tick lasts from 0.2 seconds to 5 seconds
//900 ticks range from 3 minutes to 75 minutes
constexpr uint64 TIMEOUT = 900;
constexpr uint64 QUCBET_FEE = 100;

struct QUCBET2
{
};

struct QUCBET : public ContractBase
{
    struct BetInfo
    {
        id player1;
        id player2;
        id oracle;
        uint64 deadline;
        uint64 amountBet;
        bit player1Approved;
        bit player2Approved;
        bit start;
    };

    struct StateData
    {
        BetInfo bet;
    };

    struct create_input
    {
        id oracle;
        uint64 amountBet;
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
        id winner;
    };

    struct win_output
    {
        uint64 result;
    };

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_PROCEDURE(create, 1);
        REGISTER_USER_PROCEDURE(join, 2);
        REGISTER_USER_PROCEDURE(win, 3);
    }

    PUBLIC_PROCEDURE(create) {

        if (qpi.invocator() == state.get().bet.player1 || qpi.invocator() == state.get().bet.player2) {
            output.result = QUCBET_CREATE_ALREADY_REGISTERED;
            return;
        }

        if (state.get().bet.player1 == NULL_ID) {

            if (input.oracle == NULL_ID || input.oracle == qpi.invocator()) {
                output.result = QUCBET_CREATE_INVALID_ORACLE;
                return;
            }

            if (input.amountBet == 0) {
                output.result = QUCBET_CREATE_INVALID_AMOUNT;
                return;
            }

            state.mut().bet.deadline = qpi.tick() + TIMEOUT;
            state.mut().bet.start = 1;
            state.mut().bet.player1 = qpi.invocator();
            state.mut().bet.oracle = input.oracle;
            state.mut().bet.amountBet = input.amountBet;
            state.mut().bet.player2 = NULL_ID;
            state.mut().bet.player1Approved = 0;
            state.mut().bet.player2Approved = 0;

            output.amountBet = state.get().bet.amountBet;
            output.result = QUCBET_CREATE_SUCCESS;
            return;
        }

        if (state.get().bet.player1 != NULL_ID && state.get().bet.player2 == NULL_ID) {

            if (qpi.invocator() == state.get().bet.oracle) {
                output.result = QUCBET_CREATE_INVALID_ORACLE;
                return;
            }

            state.mut().bet.player2 = qpi.invocator();

            output.amountBet = state.get().bet.amountBet;
            output.result = QUCBET_CREATE_SUCCESS;
            return;
        }
        output.result = QUCBET_CREATE_BET_FULL;
    }

    PUBLIC_PROCEDURE(join) {

        if (state.get().bet.start && qpi.tick() > state.get().bet.deadline) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCBET_JOIN_TIMEOUT;
            return;
        }

        if (qpi.invocationReward() != state.get().bet.amountBet + QUCBET_FEE) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCBET_JOIN_INVALID_AMOUNT;
            return;
        }

        if (qpi.invocator() != state.get().bet.player1 && qpi.invocator() != state.get().bet.player2) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCBET_JOIN_UNAUTHORIZED;
            return;
        }

        if (qpi.invocator() == state.get().bet.player1)
        {
            if (state.get().bet.player1Approved)
            {
                qpi.transfer(qpi.invocator(), qpi.invocationReward());
                output.result = QUCBET_JOIN_ALREADY_JOINED;
                return;
            }

            qpi.burn(QUCBET_FEE);

            state.mut().bet.player1Approved = 1;
            output.result = QUCBET_JOIN_SUCCESS;
            return;
        }

        if (qpi.invocator() == state.get().bet.player2)
        {
            if (state.get().bet.player2Approved)
            {
                qpi.transfer(qpi.invocator(), qpi.invocationReward());
                output.result = QUCBET_JOIN_ALREADY_JOINED;
                return;
            }

            qpi.burn(QUCBET_FEE);
            
            state.mut().bet.player2Approved = 1;
            output.result = QUCBET_JOIN_SUCCESS;
            return;
        }
    }

    PUBLIC_PROCEDURE(win) {

        if (!state.get().bet.start) {
            output.result = QUCBET_WIN_PLAYERS_NOT_READY;
            return;
        }

        if (qpi.tick() > state.get().bet.deadline) {
            output.result = QUCBET_WIN_TIMEOUT;
            return;
        }

        if (qpi.invocator() != state.get().bet.oracle) {
            output.result = QUCBET_WIN_UNAUTHORIZED;
            return;
        }

        if (!state.get().bet.player1Approved || !state.get().bet.player2Approved) {
            output.result = QUCBET_WIN_PLAYERS_NOT_READY;
            return;
        }

        if (input.winner != state.get().bet.player1 && input.winner != state.get().bet.player2) {
            output.result = QUCBET_WIN_INVALID_WINNER;
            return;
        }

        qpi.transfer(input.winner, state.get().bet.amountBet * 2);

        state.mut().bet.player1 = NULL_ID;
        state.mut().bet.player2 = NULL_ID;
        state.mut().bet.oracle = NULL_ID;
        state.mut().bet.deadline = 0;
        state.mut().bet.amountBet = 0;
        state.mut().bet.player1Approved = 0;
        state.mut().bet.player2Approved = 0;
        state.mut().bet.start = 0;

        output.result = QUCBET_WIN_SUCCESS;
        return;
    }

    END_TICK() {

        if (!state.get().bet.start) {
            return;
        }

        if (qpi.tick() <= state.get().bet.deadline) {
            return;
        }

        if (state.get().bet.player1Approved) {
            qpi.transfer(state.get().bet.player1, state.get().bet.amountBet);
        }

        if (state.get().bet.player2Approved) {
            qpi.transfer(state.get().bet.player2, state.get().bet.amountBet);
        }

        state.mut().bet.player1 = NULL_ID;
        state.mut().bet.player2 = NULL_ID;
        state.mut().bet.oracle = NULL_ID;
        state.mut().bet.deadline = 0;
        state.mut().bet.amountBet = 0;
        state.mut().bet.player1Approved = 0;
        state.mut().bet.player2Approved = 0;
        state.mut().bet.start = 0;
    }
};
