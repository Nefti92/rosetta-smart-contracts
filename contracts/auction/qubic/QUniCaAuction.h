using namespace QPI;

constexpr uint64 QUCAUC_CREATE_SUCCESS = 0;
constexpr uint64 QUCAUC_CREATE_ALREADY_REGISTERED = 1;
constexpr uint64 QUCAUC_CREATE_INVALID_STARTING_BID = 2;
constexpr uint64 QUCAUC_CREATE_INVALID_DURATION = 3;
constexpr uint64 QUCAUC_CREATE_PENDING_WITHDRAWALS = 4;

constexpr uint64 QUCAUC_START_SUCCESS = 0;
constexpr uint64 QUCAUC_START_NOT_REGISTERED = 1;
constexpr uint64 QUCAUC_START_INVALID_ID = 2;
constexpr uint64 QUCAUC_START_ALREADY_STARTED = 3;

constexpr uint64 QUCAUC_BID_SUCCESS = 0;
constexpr uint64 QUCAUC_BID_NOT_REGISTERED = 1;
constexpr uint64 QUCAUC_BID_NOT_STARTED = 2;
constexpr uint64 QUCAUC_BID_TIMEOUT = 3;
constexpr uint64 QUCAUC_BID_NOT_SUFFICIENT_AMOUNT = 4;
constexpr uint64 QUCAUC_BID_FULL = 5;

constexpr uint64 QUCAUC_WITHDRAW_SUCCESS = 0;
constexpr uint64 QUCAUC_WITHDRAW_NO_AMOUNT = 1;

constexpr uint64 QUCAUC_END_SUCCESS = 0;
constexpr uint64 QUCAUC_END_NOT_REGISTERED = 1;
constexpr uint64 QUCAUC_END_INVALID_ID = 2;
constexpr uint64 QUCAUC_END_NOT_STARTED = 3;
constexpr uint64 QUCAUC_END_NOT_EXPIRED = 4;

constexpr uint64 QUCAUC_FEE = 100;
constexpr uint64 MAX_BIDDERS = 1024;

struct QUCAUC2
{
};

struct QUCAUC : public ContractBase
{
    struct AuctionInfo
    {
        id seller;
        id highestBidder;
        uint64 startingBid;
        uint64 highestBid;
        uint64 duration;
        uint64 deadline;
        Array<uint8, 64> object;
        bit active;
        bit started;
    };

    struct StateData
    {
        AuctionInfo auction;
        HashMap<id, uint64, MAX_BIDDERS> refundableBids;
    };

    struct create_input
    {
        uint64 startingBid;
        uint64 duration;
        Array<uint8, 64> object;
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
        uint64 startingBid;
        uint64 highestBid;
        uint64 deadline;
        id highestBidder;
        uint8 active;
        uint8 started;
    };

    struct start_input
    {
    };

    struct start_output
    {
        uint64 result;
    };

    struct bid_input
    {
        uint64 amount_bid;
    };

    struct bid_output
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

    struct end_input
    {
    };

    struct end_output
    {
        uint64 result;
    };

    struct bid_locals
    {
        uint64 previousRefund;
    };

    struct withdraw_locals
    {
        uint64 amount;
    };
    
    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_PROCEDURE(create, 1);
        REGISTER_USER_FUNCTION(view, 1);
        REGISTER_USER_PROCEDURE(start, 2);
        REGISTER_USER_PROCEDURE(bid, 3);
        REGISTER_USER_PROCEDURE(withdraw, 4);
        REGISTER_USER_PROCEDURE(end, 5);
    }

    PUBLIC_PROCEDURE(create) {
        if (state.get().auction.active) {
            output.result = QUCAUC_CREATE_ALREADY_REGISTERED;
            return;
        }

        if (state.get().refundableBids.population() > 0) {
            output.result = QUCAUC_CREATE_PENDING_WITHDRAWALS;
            return;
        }

        if (input.startingBid == 0) {
            output.result = QUCAUC_CREATE_INVALID_STARTING_BID;
            return;
        }

        if (input.duration == 0) {
            output.result = QUCAUC_CREATE_INVALID_DURATION;
            return;
        }

        state.mut().auction.seller = qpi.invocator();
        state.mut().auction.highestBidder = NULL_ID;
        state.mut().auction.startingBid = input.startingBid;
        state.mut().auction.highestBid = 0;
        state.mut().auction.duration = input.duration;
        state.mut().auction.deadline = 0;
        state.mut().auction.object = input.object;
        state.mut().auction.active = 1;
        state.mut().auction.started = 0;

        state.mut().refundableBids.reset();

        output.result = QUCAUC_CREATE_SUCCESS;
        return;
    }

    PUBLIC_FUNCTION(view) {

        output.startingBid = state.get().auction.startingBid;
        output.highestBid = state.get().auction.highestBid;
        output.deadline = state.get().auction.deadline;
        output.highestBidder = state.get().auction.highestBidder;
        output.active = state.get().auction.active;
        output.started = state.get().auction.started;
    }

    PUBLIC_PROCEDURE(start) {
        if (!state.get().auction.active) {
            output.result = QUCAUC_START_NOT_REGISTERED;
            return;
        }

        if (qpi.invocator() != state.get().auction.seller) {
            output.result = QUCAUC_START_INVALID_ID;
            return;
        }

        if (state.get().auction.started) {
            output.result = QUCAUC_START_ALREADY_STARTED;
            return;
        }

        state.mut().auction.deadline = qpi.tick() + state.get().auction.duration;
        state.mut().auction.started = 1;

        output.result = QUCAUC_START_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(bid) {
        if (!state.get().auction.active) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCAUC_BID_NOT_REGISTERED;
            return;
        }

        if (!state.get().auction.started) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCAUC_BID_NOT_STARTED;
            return;
        }

        if (qpi.tick() > state.get().auction.deadline) {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCAUC_BID_TIMEOUT;
            return;
        }

        if (qpi.invocationReward() != input.amount_bid + QUCAUC_FEE ||
            input.amount_bid < state.get().auction.startingBid ||
            input.amount_bid <= state.get().auction.highestBid) {

            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            output.result = QUCAUC_BID_NOT_SUFFICIENT_AMOUNT;
            return;
        }

        if (state.get().auction.highestBidder != NULL_ID) {
            locals.previousRefund = 0;

            if (!state.get().refundableBids.get(
                state.get().auction.highestBidder,
                locals.previousRefund
            )) {
                if (state.get().refundableBids.population() >= state.get().refundableBids.capacity()) {
                    qpi.transfer(qpi.invocator(), qpi.invocationReward());
                    output.result = QUCAUC_BID_FULL;
                    return;
                }
            }

            state.mut().refundableBids.set(
                state.get().auction.highestBidder,
                locals.previousRefund + state.get().auction.highestBid
            );
        }

        state.mut().auction.highestBidder = qpi.invocator();
        state.mut().auction.highestBid = input.amount_bid;

        qpi.burn(QUCAUC_FEE);

        output.result = QUCAUC_BID_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE_WITH_LOCALS(withdraw) {

        locals.amount = 0;

        if (!state.get().refundableBids.get(qpi.invocator(), locals.amount) ||
            locals.amount == 0) {

            output.result = QUCAUC_WITHDRAW_NO_AMOUNT;
            return;
        }

        qpi.transfer(qpi.invocator(), locals.amount);

        state.mut().refundableBids.removeByKey(qpi.invocator());

        output.result = QUCAUC_WITHDRAW_SUCCESS;
        return;
    }

    PUBLIC_PROCEDURE(end) {
        if (!state.get().auction.active) {
            output.result = QUCAUC_END_NOT_REGISTERED;
            return;
        }

        if (qpi.invocator() != state.get().auction.seller) {
            output.result = QUCAUC_END_INVALID_ID;
            return;
        }

        if (!state.get().auction.started) {
            output.result = QUCAUC_END_NOT_STARTED;
            return;
        }

        if (qpi.tick() <= state.get().auction.deadline) {
            output.result = QUCAUC_END_NOT_EXPIRED;
            return;
        }

        if (state.get().auction.highestBid > 0) {
            qpi.transfer(
                state.get().auction.seller,
                state.get().auction.highestBid
            );
        }

        state.mut().auction.seller = NULL_ID;
        state.mut().auction.highestBidder = NULL_ID;
        state.mut().auction.startingBid = 0;
        state.mut().auction.highestBid = 0;
        state.mut().auction.duration = 0;
        state.mut().auction.deadline = 0;
        state.mut().auction.active = 0;
        state.mut().auction.started = 0;

        output.result = QUCAUC_END_SUCCESS;
        return;
    }
};
