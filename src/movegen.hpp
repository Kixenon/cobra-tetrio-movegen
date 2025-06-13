#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.hpp"
#include "header.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>

namespace Cobra {

constexpr int MAX_MOVES = 256;

Move* generate(const Board& b, Move* moves, Piece p, bool force);

class MoveList {
private:
    Move moves[MAX_MOVES];
    const Move* const last;

    bool no_duplicates() const {
        for (const Move* m = begin(); m < end() - 1; ++m)
            for (const Move* n = m + 1; n < end(); ++n)
                if (*m == *n)
                    return false;
        return true;
    }

    bool all_valid(const Board& b) const {
        for (const Move* m = begin(); m != end(); ++m) {
            if (!is_ok(*m))
                return false;

            PieceCoordinates pc = m->cells();
            pc += Coordinates(0, -1);
            if (!b.obstructed(pc[0]) &&
                !b.obstructed(pc[1]) &&
                !b.obstructed(pc[2]) &&
                !b.obstructed(pc[3]))
                return false;
        }
        return true;
    }

public:
    MoveList(const Board& b, Piece p) : last(generate(b, moves, p, false)) {
        assert(size() < MAX_MOVES);
        assert(no_duplicates());
        assert(all_valid(b));
    }

    MoveList(const Board& b, Piece p, Piece hold, bool force = false) :
        last([&]{
            Move* l = generate(b, moves, p, force);
            return (l != moves && p != hold) ? generate(b, l, hold, force) : l;
        }()) {
        assert(size() < MAX_MOVES);
        assert(no_duplicates());
        assert(all_valid(b));
    }

    size_t size() const { return static_cast<size_t>(last - moves); }
    bool empty() const { return last == moves; }
    bool contains(const Move& m) const { return std::any_of(begin(), end(), [m](const Move& move) { return move == m; }); }

    const Move* begin() const { return moves; }
    const Move* end() const { return last; }
};

} // namespace Cobra

#endif