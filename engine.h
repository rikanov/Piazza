#pragma once
#include <initializer_list>

struct response {
    unsigned black = 0;
    unsigned white = 0;
};

struct code {
    unsigned sym_code = 0;

    code() = default;
    code(const unsigned * symbols);
    void set(const unsigned * symbols);

    response compare(const code query) const;
};

struct HistoryEntry {
    unsigned guess[4];
    response res;
};

inline
response row_value( const code secret, const int row, const unsigned * table ) {

    return secret.compare( code(table + 4 * row ));
}

inline
response col_value( const code secret, const int col, const unsigned * table ) {

    unsigned column[4] = {};
    for( int i: { 0, 1, 2, 3 } ) {
        column[i] = *( table  + col + 4 * i);
    }
    return secret.compare( code( column ));
};

inline
int is_solved( const code secret, const unsigned * table) {

    for( int i: {0, 1, 2, 3 } ) {
        if( 4 == row_value(secret, i, table).black ) {
            return 1 + i; // 1-4: Sor győzelem
        }
        else if (4 == col_value(secret, i, table).black ) {
            return 5 + i; // 5-8: Oszlop győzelem
        }
    }
    return 0; // 0: Nincs megoldva
}
