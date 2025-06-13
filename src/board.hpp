#ifndef BOARD_H
#define BOARD_H

#include "header.hpp"

#include <cassert>
#include <string>

namespace Cobra {

class Board {
private:
    Bitboard col[COL_NB];

public:
    bool occupied(const int x, const int y) const { return col[x] & bb(y); }
    bool occupied(const Coordinates& c) const { return occupied(c.x, c.y); }
    bool obstructed(const int x, const int y) const { return !is_ok_x(x) || !is_ok_y(y) || occupied(x, y); }
    bool obstructed(const Coordinates& c) const { return obstructed(c.x, c.y); }
    bool obstructed(const Move& move) const;

    constexpr Bitboard& operator[](const int x) const {
        assert(is_ok_x(x));
        return const_cast<Bitboard&>(col[x]);
    }

    bool empty() const;
    Bitboard line_clears() const;

    void clear();
    void clear_lines(Bitboard l);
    void place(const Move& move);

    std::string to_string() const;
    std::string to_string(const Move& move) const;
};

struct MoveInfo {
    Piece piece;
    SpinType spin;
    int clear;
    int b2b;
    int combo;
    bool pc;

    int lines_sent(double multiplier = 1.0) const;
};

struct State {
    Board board;
    Piece hold;
    int b2b;
    int combo;

    void init();
    MoveInfo do_move(const Move& move);
};

} // namespace Cobra

#endif // BOARD_H