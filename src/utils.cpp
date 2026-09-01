#include <cstring>

#include "utils.h"

J dict_find_idx(K dict, const char * key) {
    if (dict == nullptr || dict->t != XD) { return -1; }
    K keys = kK(dict)[0];
    // kS is only valid for a symbol vector; on any other type it would
    // reinterpret non-pointer data as char*.
    if (keys->t != KS) { return -1; }
    for (J i = 0; i < keys->n; ++i) {
        if (0 == std::strcmp(key, kS(keys)[i])) { return i; }
    }
    return -1;
}

DictLookup dict_find_str(K dict, const char * key, std::string & out) {
    const J i = dict_find_idx(dict, key);
    if (i < 0) { return DictLookup::Absent; }

    K vals = kK(dict)[1];
    if (vals->t == KS) {  // typed value list, e.g. `loglevel`region!`INFO`eu-west-1
        out.assign(kS(vals)[i]);
        return DictLookup::Found;
    }
    if (vals->t != 0) { return DictLookup::WrongType; }

    K val = kK(vals)[i];  // mixed value list, one element per key
    if (val->t == KC) {
        out.assign((S)kC(val), val->n);
        return DictLookup::Found;
    }
    if (val->t == -KS) {
        out.assign(val->s);
        return DictLookup::Found;
    }
    return DictLookup::WrongType;
}
