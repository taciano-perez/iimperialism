#ifndef STRINGS_H
#define STRINGS_H

extern const char STR_INVALID_ANSWER[];
extern const char STR_NOT_ENOUGH_RESOURCES[];
extern const char STR_SIR_WE_LACK_RESOURCES[];
extern const char STR_MAX_FMT[];
extern const char STR_PER_TURN_MAX_FMT[];
extern const char STR_TRADER_COST[];
extern const char STR_WARSHIP_COST[];
extern const char STR_WARSHIP_COST2[];
extern const char STR_SIR_TRAIN_WORKERS1[];
extern const char STR_SIR_TRAIN_WORKERS2[];

extern const char STR_RELATION_TERRIBLE[];
extern const char STR_RELATION_BAD[];
extern const char STR_RELATION_NEUTRAL[];
extern const char STR_RELATION_GOOD[];
extern const char STR_RELATION_EXCELLENT[];
extern const char* const STR_RESOURCE[];
const char* get_resource_name(unsigned char resource);
const char* get_relation_name(unsigned char relation);

#endif /* STRINGS_H */
