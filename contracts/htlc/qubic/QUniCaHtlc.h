using namespace QPI;

constexpr uint64 QUCHTLC_CREATE_SUCCESS = 0;
constexpr uint64 QUCHTLC_CREATE_ALREADY_REGISTERED = 1;
constexpr uint64 QUCHTLC_CREATE_INVALID_RECEIVER = 2;
constexpr uint64 QUCHTLC_CREATE_INVALID_AMOUNT = 3;
constexpr uint64 QUCHTLC_CREATE_INVALID_DELAY = 4;
constexpr uint64 QUCHTLC_CREATE_INVALID_REWARD = 5;

constexpr uint64 QUCHTLC_REVEAL_SUCCESS = 0;
constexpr uint64 QUCHTLC_REVEAL_NOT_REGISTERED = 1;
constexpr uint64 QUCHTLC_REVEAL_INVALID_ID = 2;
constexpr uint64 QUCHTLC_REVEAL_TIMEOUT = 3;
constexpr uint64 QUCHTLC_REVEAL_INVALID_SECRET = 4;

constexpr uint64 QUCHTLC_TIMEOUT_SUCCESS = 0;
constexpr uint64 QUCHTLC_TIMEOUT_NOT_REGISTERED = 1;
constexpr uint64 QUCHTLC_TIMEOUT_NOT_EXPIRED = 2;

constexpr uint64 QUCHTLC_FEE = 100;

struct QUCHTLC2
{
};

struct QUCHTLC : public ContractBase
{
    struct Secret
    {
        Array<uint8, 32> data;
    };

    struct HTLCInfo
    {
        id committer;
        id receiver;
        uint64 amount;
        uint64 deadline;
        uint64 commitment;
        bit active;
    };

    struct StateData
    {
        HTLCInfo htlc;
    };

    struct create_input
    {
        id receiver;
        uint64 amount;
        uint64 delay;
        uint64 commitment;
    };

    struct create_output
    {
        uint64 result;
    };

    struct reveal_input
    {
        Secret secret;
    };

    struct reveal_output
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

    struct reveal_locals
    {
        uint64 secretHash;
    };

    struct hash_input
    {
        Secret secret;
    };

    struct hash_output
    {
        uint64 commitment;
    };

    struct hash_locals
    {
        uint64 secretHash;
    };

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_PROCEDURE(create, 1);
        REGISTER_USER_PROCEDURE(reveal, 2);
        REGISTER_USER_PROCEDURE(timeout, 3);
        REGISTER_USER_FUNCTION(hash, 1);
    }

    PUBLIC_FUNCTION_WITH_LOCALS(hash) {
        locals.secretHash = HashFunction<Secret>::hash(input.secret);
        output.commitment = locals.secretHash;
    }

    PUBLIC_PROCEDURE(create) {   
        if (state.get().htlc.active) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCHTLC_CREATE_ALREADY_REGISTERED;
            return;
        }

        if (input.receiver == NULL_ID || input.receiver == qpi.invocator()) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCHTLC_CREATE_INVALID_RECEIVER;
            return;
        }

        if (input.amount == 0) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCHTLC_CREATE_INVALID_AMOUNT;
            return;
        }

        if (input.delay == 0) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCHTLC_CREATE_INVALID_DELAY;
            return;
        }

        if (qpi.invocationReward() != input.amount + QUCHTLC_FEE) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCHTLC_CREATE_INVALID_REWARD;
            return;
        }

        state.mut().htlc.committer = qpi.invocator();
        state.mut().htlc.receiver = input.receiver;
        state.mut().htlc.amount = input.amount;
        state.mut().htlc.deadline = qpi.tick() + input.delay;
        state.mut().htlc.commitment = input.commitment;
        state.mut().htlc.active = 1;

        qpi.burn(QUCHTLC_FEE);

        output.result = QUCHTLC_CREATE_SUCCESS;
        return;     
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(reveal) {
        if (!state.get().htlc.active) {
            output.result = QUCHTLC_REVEAL_NOT_REGISTERED;
            return;
        }

        if (qpi.invocator() != state.get().htlc.committer) {
            output.result = QUCHTLC_REVEAL_INVALID_ID;
            return;
        }

        if (qpi.tick() > state.get().htlc.deadline) {
            output.result = QUCHTLC_REVEAL_TIMEOUT;
            return;
        }

        locals.secretHash = HashFunction<Secret>::hash(input.secret);

        if (locals.secretHash != state.get().htlc.commitment) {
            output.result = QUCHTLC_REVEAL_INVALID_SECRET;
            return;
        }

        qpi.transfer(state.get().htlc.committer, state.get().htlc.amount);

        state.mut().htlc.committer = NULL_ID;
        state.mut().htlc.receiver = NULL_ID;
        state.mut().htlc.amount = 0;
        state.mut().htlc.deadline = 0;
        state.mut().htlc.commitment = 0;
        state.mut().htlc.active = 0;

        output.result = QUCHTLC_REVEAL_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE(timeout) {
        if (!state.get().htlc.active) {
            output.result = QUCHTLC_TIMEOUT_NOT_REGISTERED;
            return;
        }

        if (qpi.tick() <= state.get().htlc.deadline) {
            output.result = QUCHTLC_TIMEOUT_NOT_EXPIRED;
            return;
        }

        qpi.transfer(state.get().htlc.receiver, state.get().htlc.amount);

        state.mut().htlc.committer = NULL_ID;
        state.mut().htlc.receiver = NULL_ID;
        state.mut().htlc.amount = 0;
        state.mut().htlc.deadline = 0;
        state.mut().htlc.commitment = 0;
        state.mut().htlc.active = 0;

        output.result = QUCHTLC_TIMEOUT_SUCCESS;
        return;
    }
};
