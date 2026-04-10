#ifndef STRINGS_H
#define STRINGS_H

extern const char STR_RELATION_TERRIBLE[];
extern const char STR_RELATION_BAD[];
extern const char STR_RELATION_NEUTRAL[];
extern const char STR_RELATION_GOOD[];
extern const char STR_RELATION_EXCELLENT[];
extern const char* const STR_RESOURCE[];
const char* get_resource_name(unsigned char resource);
const char* get_diplomacy_string(unsigned char index);
const char* get_relation_name(unsigned char relation, unsigned char nation_index);

#endif /* STRINGS_H */
