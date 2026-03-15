#include "game.h"
#include "strings.h"

const char STR_INVALID_ANSWER[] = "Invalid answer!";
const char STR_NOT_ENOUGH_RESOURCES[] = "Not enough resources!";
const char STR_MAX_FMT[] = "(max %u)";
const char STR_PER_TURN_MAX_FMT[] = "per turn?";
const char STR_TRADER_COST[] = "A trader costs 1 lumber, 1 fabric,";
const char STR_WARSHIP_COST[] = "A warship costs 1 lumber, 1 fabric,";
const char STR_WARSHIP_COST2[] = "1 gun and 1 worker.";
const char STR_SIR_TRAIN_WORKERS1[] = "Sir, A worker costs 1 furniture";
const char STR_SIR_TRAIN_WORKERS2[] = "and 1 clothes.";

const char STR_RELATION_TERRIBLE[] = "Terrible";
const char STR_RELATION_BAD[] = "Bad";
const char STR_RELATION_NEUTRAL[] = "Neutral";
const char STR_RELATION_GOOD[] = "Good";
const char STR_RELATION_EXCELLENT[] = "Excellent";

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

const char* get_resource_name(unsigned int resource) {
    if (resource > RESOURCE_GUNS) {
        return "";
    }

    return STR_RESOURCE[resource];
}

const char* get_relation_name(unsigned int relation) {
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

    return STR_RELATION_EXCELLENT;
}
