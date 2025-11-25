#include "board.hpp"
#include "header.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <string>

namespace Cobra {

bool Board::obstructed(const Move& move) const {
    const PieceCoordinates pc = move.cells();
    return obstructed(pc[0])
        || obstructed(pc[1])
        || obstructed(pc[2])
        || obstructed(pc[3]);
}

bool Board::empty() const {
    return std::all_of(std::begin(col), std::end(col), [](Bitboard b) { return b == 0; });
}

Bitboard Board::line_clears() const {
    Bitboard result = col[0];
    for (int x = 1; x < COL_NB && result; ++x)
        result &= col[x];
    return result;
}

void Board::clear() {
    __builtin_memset(col, 0, sizeof(col));
}

void Board::clear_lines(Bitboard l) {
    assert(l);
    do {
        const Bitboard mask = ~((l & -l) - 1);
        for (auto& c : col)
            c = c ^ ((c ^ (c >> 1)) & mask);
    } while ((l = (l & (l - 1)) >> 1));
}

void Board::place(const Move& move) {
    const PieceCoordinates pc = move.cells();
    for (size_t i = 0; i < 4; ++i)
        col[pc[i].x] |= bb(pc[i].y);
}

std::string Board::to_string() const {
    constexpr int lines = 20;
    std::string output;
    output.reserve((lines + 1) * 86 + 44);
    output += "\n +---+---+---+---+---+---+---+---+---+---+\n";
    for (int y = lines; y >= 0; --y) {
        for (const auto& c : col) {
            output += " | ";
            output += (c & bb(y) ? '#' : ' ');
        }
        output += " |\n +---+---+---+---+---+---+---+---+---+---+\n";
    }
    return output;
}

std::string Board::to_string(const Move& move) const {
    std::string output = to_string();
    if (!obstructed(move)) {
        constexpr int lines = 20;
        const PieceCoordinates pc = move.cells();
        for (size_t i = 0; i < 4; ++i) {
            const int inverseY = lines - pc[i].y;
            if (inverseY < 0) // Moves above printable area
                continue;
            output[static_cast<size_t>(inverseY * 86 + pc[i].x * 4 + 47)] = '.';
        }
    }
    return output;
}

int MoveInfo::lines_sent(const double multiplier) const {
    if (!clear)
        return 0;
    assert(clear > 0 && combo > 0);

    constexpr int AttackTableBase[SPIN_NB][4] = {
        {0, 1, 2, 4},
        {0, 1},
        {2, 4, 6},
    };

    double lines = AttackTableBase[spin][clear - 1];

    if (b2b > 1) {
        const double v = std::log1p((b2b - 1) * 0.8);
        lines += static_cast<int>(1 + v) + (b2b == 2 ? 0 : (1 + v - static_cast<int>(v)) / 3);
    }

    lines *= 1 + 0.25 * (combo - 1);

    if (combo > 2)
        lines = std::max(std::log1p((combo - 1) * 1.25), lines);

    return static_cast<int>(lines * multiplier) + static_cast<int>(pc * 10 * multiplier);
}

void State::init() {
    board.clear();
    hold = NO_PIECE;
    b2b = combo = 0;
}

MoveInfo State::do_move(const Move& move) {
    assert(is_ok(move));
    assert(!board.obstructed(move));

    board.place(move);
    const Bitboard clears = board.line_clears();
    if (!clears)
        return MoveInfo{move.piece(), NO_SPIN, 0, 0, combo = 0, false};

    board.clear_lines(clears);
    const int clearCount = popcount(clears);
    const SpinType spin = move.spin();

    return MoveInfo{
        move.piece(),
        spin,
        clearCount,
        b2b = (spin || clearCount == 4) ? b2b + 1 : 0,
        ++combo,
        board.empty()
    };
}

} // namespace Cobra