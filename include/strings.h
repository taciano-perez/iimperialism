#ifndef STRINGS_H
#define STRINGS_H

extern const char STR_RELATION_BAD[];
extern const char STR_RELATION_POOR[];
extern const char STR_RELATION_FAIR[];
extern const char STR_RELATION_GOOD[];
extern const char STR_RELATION_GREAT[];
extern const char* const STR_RESOURCE[];
const char* get_resource_name(unsigned char resource);
const char* get_diplomacy_string(unsigned char index);
const char* get_relation_name(unsigned char relation, unsigned char nation_index);

#endif /* STRINGS_H */
