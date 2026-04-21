#include "game.h"

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
    "...",
    "Buy, Sell or Quit?",
    "Commodity to buy?",
    "Commodity to sell?",
    "How many units (Max:    )?",
    "Warship cost: 1 lumber, 1 fabric,",
    "  gun(s) & 1 worker."
};
static const char* const STR_FINAL_VICTORY[] = {
    "Final Report To The Crown",
    "Net Treasury:",
    "Sea Power:",
    "Merchant Fleet:",
    "Foreign Relations:",
    "You triumphed in ",
    "turns",
    "friendly provinces",
    "firepower",
    "ships,",
    "capacity",
    "Queen Victoria",
    "Otto von Bismarck",
    "Napoleon III",
    "Charles X",
    "Ferdinand VII",
    "50,000 and over",
    "35,000 to 49,999",
    "20,000 to 34,999",
    " 8,000 to 19,999",
    " less than 8,000",
    "You founded an Empire",
    "where the sun never sets.",
    "Your skill forged an Empire",
    "never to be forgotten.",
    "You reigned resolutely,",
    "though not always wisely.",
    "Your court looked grander",
    "than your results.",
    "Your creditors remember you",
    "more vividly than your subjects."
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

const char* get_final_victory_string(unsigned char index) {
    return STR_FINAL_VICTORY[index];
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
