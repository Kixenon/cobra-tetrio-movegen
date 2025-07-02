#include "board.hpp"
#include "gen.hpp"
#include "header.hpp"
#include "movegen.hpp"

#include <cassert>
#include <cstddef>
#include <utility>

namespace Cobra {

const Bitboard spinMapDummy[COL_NB][1 + ROTATION_NB] = {};

template<Piece p1>
Move* generate(const Board& b, Move* moves, const bool force, const Gen::CollisionMap<p1 == TSPIN ? T : p1>& cm, [[maybe_unused]] const Bitboard (&spinMap)[COL_NB][1 + ROTATION_NB] = spinMapDummy) {
    constexpr Piece p = p1 == TSPIN ? T : p1;
    constexpr bool checkSpin = p1 == TSPIN;
    constexpr int moveSetSize = p == O ? 1 : (p == I || p == S || p == Z) ? 2 : 4;
    constexpr int searchSize = p == O ? 1 : ROTATION_NB;
    static_assert(is_ok(p));

    int total = 0;
    Bitboard remaining = 0;
    Bitboard toSearch[COL_NB][searchSize] = {};
    Bitboard searched[COL_NB][searchSize];
    Bitboard moveSet[COL_NB][moveSetSize] = {};
    Bitboard spinSet[COL_NB][ROTATION_NB][checkSpin ? SPIN_NB : 0] = {};

    auto remaining_index = [](int x, Rotation r) { return bb(x * ROTATION_NB + r); };

    for (int x = 0; x < COL_NB; ++x)
        for (int r = 0; r < searchSize; ++r)
            searched[x][r] = cm[x][static_cast<Rotation>(r)];

    if ([&]{
            Bitboard m = b[0];
            for (int i = 1; i < COL_NB; ++i)
                m |= b[i];
            return bitlen(m) > Gen::SPAWN_ROW - 3;
        }()) {
        const Bitboard spawn = [&]{
            if (force) {
                const Bitboard s = ~cm[Gen::SPAWN_COL][NORTH] & (~0ULL << Gen::SPAWN_ROW);
                return s & -s;
            }
            return ~cm[Gen::SPAWN_COL][NORTH] & bb(Gen::SPAWN_ROW);
        }();
        if (!spawn)
            return moves;

        toSearch[Gen::SPAWN_COL][NORTH] = spawn;
        remaining |= remaining_index(Gen::SPAWN_COL, NORTH);
        if constexpr (checkSpin)
            spinSet[Gen::SPAWN_COL][NORTH][NO_SPIN] = spawn;
    } else {
        auto init = [&]<int x>{
            auto process = [&]<Rotation r>{
                if constexpr (!Gen::in_bounds<p, r>(x))
                    return;

                assert(cm[x][r] != ~0ULL);
                const int y = bitlen(cm[x][r]);
                const Bitboard surface = bb_low(Gen::SPAWN_ROW) & ~bb_low(y);

                searched[x][r] |= toSearch[x][r] = surface;
                remaining |= remaining_index(x, r);
                if constexpr (checkSpin)
                    spinSet[x][r][NO_SPIN] = surface;
                else if constexpr (r < moveSetSize) {
                    *moves++ = Move(p, r, x, y);
                    total += popcount(~cm[x][r] & ((cm[x][r] << 1) | 1)) - 1;
                }
            };

            [&]<size_t... rs>(std::index_sequence<rs...>) {
                (process.template operator()<static_cast<Rotation>(rs)>(), ...);
            }(std::make_index_sequence<searchSize>());
        };

        [&]<size_t... xs>(std::index_sequence<xs...>) {
            (init.template operator()<xs>(), ...);
        }(std::make_index_sequence<COL_NB>());

        if constexpr (!checkSpin)
            if (!total)
                return moves;
    }

    while (remaining) {
        const int index = ctz(remaining);
        const int x = index >> 2;
        const Rotation r = static_cast<Rotation>(index & 3);

        assert(is_ok_x(x));
        assert(is_ok(r));
        assert(toSearch[x][r]);
        assert((toSearch[x][r] & ~cm[x][r]) == toSearch[x][r]);

        // Softdrops
        {
            if constexpr (checkSpin) {
                Bitboard m = (toSearch[x][r] >> 1) & ~cm[x][r];
                while ((m & toSearch[x][r]) != m) {
                    toSearch[x][r] |= m;
                    m |= (m >> 1) & ~cm[x][r];
                }
                spinSet[x][r][NO_SPIN] |= m;
            } else {
                Bitboard m = (toSearch[x][r] >> 1) & ~toSearch[x][r] & ~searched[x][r];
                // if (m) {
                //     const Bitboard m1 = __builtin_bitreverse64(m);
                //     const Bitboard f = __builtin_bitreverse64(searched[x][r]);
                //     toSearch[x][r] |= __builtin_bitreverse64(((f & (~f + m1)) - m1));
                // }
                // Alternative if no/slow bit reverse function (x86 arch):
                while (m) {
                    toSearch[x][r] |= m;
                    m = (m >> 1) & ~searched[x][r];
                }
            }
        }

        // Harddrops
        {
            if constexpr (checkSpin)
                moveSet[x][r] |= toSearch[x][r] & ((cm[x][r] << 1) | 1);
            else {
                Bitboard m = toSearch[x][r] & ((cm[x][r] << 1) | 1) & ~searched[x][r];

                if (m) {
                    const Rotation r1 = [&]{
                        if constexpr (p == O)
                            return NORTH;
                        if constexpr (p == I || p == S || p == Z)
                            return static_cast<Rotation>(r & 1);
                        return r; // L/J/T
                    }();

                    const int x1 = x - [&]() -> int {
                        if constexpr (p == O)
                            return r == WEST || r == SOUTH;
                        if constexpr (p == I)
                            return r == SOUTH;
                        if constexpr (p == S || p == Z)
                            return r == WEST;
                        return 0; // L/J/T
                    }();

                    if constexpr (p != L && p != J && p != T) {
                        m >>= (p == O && (r == EAST || r == SOUTH));
                        m >>= ((p == S || p == Z) && (r == SOUTH));
                        m <<= (p == I && r == WEST);
                    }

                    assert(is_ok_x(x1));
                    assert(is_ok(r1));
                    assert(!(m & cm[x1][r1]));
                    assert(((m >> 1) & cm[x1][r1]) == (m >> 1));

                    if (m &= ~moveSet[x1][r1]) {
                        moveSet[x1][r1] |= m;
                        total -= popcount(m);
                        while (m) {
                            *moves++ = Move(p, r1, x1, ctz(m));
                            m &= m - 1;
                        }
                        if (!total)
                            return moves;
                    }
                }
            }
        }

        // Shift
        {
            auto shift = [&](int x1) {
                const Bitboard m = toSearch[x][r] & ~searched[x1][r];
                if (m) {
                    toSearch[x1][r] |= m;
                    remaining |= remaining_index(x1, r);
                    if constexpr (checkSpin)
                        spinSet[x1][r][NO_SPIN] |= m;
                }
            };
            if (x > 0)
                shift(x - 1);
            if (x < 9)
                shift(x + 1);
        }

        // Rotate
        if constexpr (p != O) {
            auto process = [&]<size_t N>(const Gen::Offsets<N>& kicks, Rotation r1) {
                Bitboard current = toSearch[x][r];
                for (size_t i = 0; i < N && current; ++i) {
                    const int x1 = x + kicks[i].x;
                    if (!is_ok_x(x1))
                        continue;

                    constexpr int threshold = 3;
                    const int y1 = threshold + kicks[i].y;
                    assert(y1 >= 0);

                    Bitboard m = ((current << y1) >> threshold) & ~cm[x1][r1];
                    current ^= (m << threshold) >> y1;

                    if constexpr (checkSpin) {
                        const Bitboard spins = m & spinMap[x1][0];

                        spinSet[x1][r1][NO_SPIN] |= m ^ spins;

                        if (spins) {
                            if (i >= 4)
                                spinSet[x1][r1][FULL] |= spins;
                            else {
                                spinSet[x1][r1][MINI] |= spins & ~spinMap[x1][1 + r1];
                                spinSet[x1][r1][FULL] |= spins & spinMap[x1][1 + r1];
                            }
                        }
                    }

                    if ((m &= ~searched[x1][r1])) {
                        toSearch[x1][r1] |= m;
                        remaining |= remaining_index(x1, r1);
                    }
                }
            };

            process.template operator()<5>(Gen::kicks[p == I][Gen::Direction::CW][r], Gen::rotate<Gen::Direction::CW>(r));
            process.template operator()<5>(Gen::kicks[p == I][Gen::Direction::CCW][r], Gen::rotate<Gen::Direction::CCW>(r));
            process.template operator()<6>(Gen::kicks180[p == I][r], Gen::rotate<Gen::Direction::FLIP>(r));
        }

        searched[x][r] |= toSearch[x][r];
        toSearch[x][r] = 0;
        remaining ^= bb(index);
    }

    if constexpr (checkSpin)
        for (int x = 0; x < COL_NB; ++x)
            for (const Rotation r : allRotations) {
                if (!moveSet[x][r])
                    continue;

                for (const auto s : {NO_SPIN, MINI, FULL}) {
                    Bitboard current = moveSet[x][r] & spinSet[x][r][s];
                    while (current) {
                        *moves++ = Move(s == NO_SPIN ? T : TSPIN, r, x, ctz(current), s == FULL);
                        current &= current - 1;
                    }
                }
            }

    return moves;
}

Move* generate(const Board& b, Move* moves, const Piece p, const bool force) {
    switch(p) {
        case I: return generate<I>(b, moves, force, Gen::CollisionMap<I>(b));
        case O: return generate<O>(b, moves, force, Gen::CollisionMap<O>(b));
        case T:
            {
                const Gen::CollisionMap<T> cm(b);
                bool checkSpin = false;
                Bitboard spinMap[COL_NB][1 + ROTATION_NB] = {};
                auto init = [&]<int x>{
                    const Bitboard corners[] = {
                        x > 0 ? b[x - 1] >> 1 : ~0ULL,
                        x < 9 ? b[x + 1] >> 1 : ~0ULL,
                        x < 9 ? (b[x + 1] << 1 | 1) : ~0ULL,
                        x > 0 ? (b[x - 1] << 1 | 1) : ~0ULL
                    };

                    const Bitboard spins = (
                        (corners[0] & corners[1] & (corners[2] | corners[3])) |
                        (corners[2] & corners[3] & (corners[0] | corners[1]))
                    );

                    spinMap[x][0] = spins;
                    if (spins) {
                        auto process = [&]<Rotation r>{
                            if (Gen::in_bounds<T, r>(x)) {
                                spinMap[x][1 + r] = spins & corners[r] & corners[Gen::rotate<Gen::Direction::CW>(r)];
                                checkSpin |= spins & ~cm[x][r] & ((cm[x][r] << 1) | 1);
                            }
                        };

                        [&]<size_t... rs>(std::index_sequence<rs...>) {
                            (process.template operator()<static_cast<Rotation>(rs)>(), ...);
                        }(std::make_index_sequence<ROTATION_NB>());
                    }
                };

                [&]<size_t... xs>(std::index_sequence<xs...>) {
                    (init.template operator()<xs>(), ...);
                }(std::make_index_sequence<COL_NB>());

                if (checkSpin)
                    return generate<TSPIN>(b, moves, force, cm, spinMap);
                return generate<T>(b, moves, force, cm);
            }
        case L: return generate<L>(b, moves, force, Gen::CollisionMap<L>(b));
        case J: return generate<J>(b, moves, force, Gen::CollisionMap<J>(b));
        case S: return generate<S>(b, moves, force, Gen::CollisionMap<S>(b));
        case Z: return generate<Z>(b, moves, force, Gen::CollisionMap<Z>(b));
        default: __builtin_unreachable();
    }
}

} // namespace Cobra