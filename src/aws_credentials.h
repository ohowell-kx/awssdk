#pragma once

// This header pulls in k.h, whose short macros (R, O, Z, P, U, ...) collide with
// AWS SDK identifiers, e.g. `template<class R, class E> class Outcome`. Include
// it *after* any AWS header in a translation unit, never before.
#include "k.h"


namespace awssdk {
    // Takes an ignored argument because dl() cannot register a rank-0 function.
    K getCredentials(K unused);
}