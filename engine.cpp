#include "engine.h"

code::code(const unsigned * symbols) {
    set(symbols);
}

void code::set(const unsigned * symbols) {
    sym_code  =  1U << symbols[0];
    sym_code |= (1U << symbols[1]) << 8;
    sym_code |= (1U << symbols[2]) << 16;
    sym_code |= (1U << symbols[3]) << 24;
}

response code::compare(const code query) const {
    response result;
    const unsigned match = sym_code & query.sym_code;
    result.black = __builtin_popcount(match);

    unsigned sym_sets  = sym_code ^ match;
    sym_sets |= sym_sets << 8  | sym_sets >> 24;
    sym_sets |= sym_sets << 16 | sym_sets >> 16;
    sym_sets |= sym_sets << 24 | sym_sets >> 8;

    const unsigned unmatched_query = query.sym_code ^ match;
    result.white = __builtin_popcount(sym_sets & unmatched_query);

    return result;
}
