using namespace QPI;

constexpr uint64 QUCPRBO_CREATE_SUCCESS = 0;
constexpr uint64 QUCPRBO_CREATE_ALREADY_REGISTERED = 1;
constexpr uint64 QUCPRBO_CREATE_INVALID_RATE = 2;

constexpr uint64 QUCPRBO_SET_RATE_SUCCESS = 0;
constexpr uint64 QUCPRBO_SET_RATE_NOT_REGISTERED = 1;
constexpr uint64 QUCPRBO_SET_RATE_INVALID_ID = 2;
constexpr uint64 QUCPRBO_SET_RATE_INVALID_RATE = 3;

struct QUCPRBO2
{
};

struct QUCPRBO : public ContractBase
{
    struct OracleInfo
    {
        id owner;
        uint64 rate;
        bit active;
    };

    struct StateData
    {
        OracleInfo oracle;
    };

    struct create_input
    {
        uint64 rate;
    };

    struct create_output
    {
        uint64 result;
    };

    struct getRate_input
    {
    };

    struct getRate_output
    {
        uint64 rate;
    };

    struct setRate_input
    {
        uint64 rate;
    };

    struct setRate_output
    {
        uint64 result;
    };

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_PROCEDURE(create, 1);
        REGISTER_USER_FUNCTION(getRate, 1);
        REGISTER_USER_PROCEDURE(setRate, 2);
    }

    PUBLIC_PROCEDURE(create) {

        if (state.get().oracle.active) {
            output.result = QUCPRBO_CREATE_ALREADY_REGISTERED;
            return;
        }

        if (input.rate == 0) {
            output.result = QUCPRBO_CREATE_INVALID_RATE;
            return;
        }

        state.mut().oracle.owner = qpi.invocator();
        state.mut().oracle.rate = input.rate;
        state.mut().oracle.active = 1;

        output.result = QUCPRBO_CREATE_SUCCESS;
        return;
    }

    PUBLIC_FUNCTION(getRate) {

        output.rate = state.get().oracle.rate;
    }

    PUBLIC_PROCEDURE(setRate) {

        if (!state.get().oracle.active) {
            output.result = QUCPRBO_SET_RATE_NOT_REGISTERED;
            return;
        }

        if (qpi.invocator() != state.get().oracle.owner) {
            output.result = QUCPRBO_SET_RATE_INVALID_ID;
            return;
        }

        if (input.rate == 0) {
            output.result = QUCPRBO_SET_RATE_INVALID_RATE;
            return;
        }

        state.mut().oracle.rate = input.rate;

        output.result = QUCPRBO_SET_RATE_SUCCESS;
        return;
    }
};