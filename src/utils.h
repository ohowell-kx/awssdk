#pragma once

#include <string>

#include "k.h"

// Outcome of reading an option out of a q dictionary.
enum class DictLookup {
  Absent,    // the key is not in the dictionary
  Found,     // the value was read into the out parameter
  WrongType  // the key is present, but its value has an unusable type
};

// Index of `key` in a q dictionary with symbol keys, or -1 if `dict` is not
// such a dictionary or the key is absent.
J dict_find_idx(K dict, const char * key);

// Reads `key` from `dict` as a string, accepting char vectors ("INFO") and
// symbols (`INFO) from both mixed and typed value lists.
DictLookup dict_find_str(K dict, const char * key, std::string & out);
