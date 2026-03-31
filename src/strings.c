#include "game.h"
#include "strings.h"

const char STR_NOT_ENOUGH_RESOURCES[] = "Not enough resources!";
const char STR_PER_TURN_MAX_FMT[] = "per turn?";
const char STR_TRADER_COST[] = "A trader costs 1 lumber, 1 fabric,";
const char STR_WARSHIP_COST[] = "A warship costs 1 lumber, 1 fabric,";
const char STR_WARSHIP_COST2[] = "1 gun and 1 worker.";
const char STR_SIR_TRAIN_WORKERS1[] = "Sir, A worker costs 1 furniture";
const char STR_SIR_TRAIN_WORKERS2[] = "and 1 clothes.";
const char STR_BAR[] = "===========";


const char STR_RELATION_TERRIBLE[] = "Bad";
const char STR_RELATION_BAD[] = "Poor";
const char STR_RELATION_NEUTRAL[] = "Fair";
const char STR_RELATION_GOOD[] = "Good";
const char STR_RELATION_EXCELLENT[] = "Great";
const char STR_RELATION_ALLY[] = "Ally";
const char STR_RELATION_COLONY[] = "Colony";
static const char* const STR_DIPLOMACY[] = {
    "Foreign Office",
    "Nation",
    "Status",
    "Exports",
    "Imports",
    "Launch Trade expedition, offer",
    "Colony status, Alliance or Quit?",
    "We must build warships first!",
    "A diplomatic mission costs $1000.",
    "Nation to offer",
    "an alliance:",
    "colony status:",
    "Offer accepted!",
    "Offer rejected. Investment lost.",
    "You need Status = Great",
    "and $1000 to offer",
    "an alliance to a great power.",
    "colony status to a minor nation.",
    "Which nation to trade (1-5)?",
    "Sailing to the Sea of",
    "..."
};

const char* const STR_RESOURCE[] = {
    "Timber",
    "Wool",
    "Iron",
    "Coal",
    "Lumber",
    "Fabric",
    "Steel",
    "Furniture",
    "Clothes",
    "Tools",
    "Guns"
};

const char* get_resource_name(unsigned char resource) {
    if (resource > RESOURCE_GUNS) {
        return "";
    }

    return STR_RESOURCE[resource];
}

const char* get_diplomacy_string(unsigned char index) {
    return STR_DIPLOMACY[index];
}

const char* get_relation_name(unsigned char relation, unsigned char nation_index) {
    if (relation < RELATION_BAD) {
        return STR_RELATION_TERRIBLE;
    }

    if (relation < RELATION_NEUTRAL) {
        return STR_RELATION_BAD;
    }

    if (relation < RELATION_GOOD) {
        return STR_RELATION_NEUTRAL;
    }

    if (relation < RELATION_EXCELLENT) {
        return STR_RELATION_GOOD;
    }
    if (relation == RELATION_ALLY_COLONY) {
        if (nation_index < 2) {
            return STR_RELATION_ALLY;
        } else {
            return STR_RELATION_COLONY;
        }
    }

    return STR_RELATION_EXCELLENT;
}
