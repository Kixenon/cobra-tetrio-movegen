#ifndef GEN_H
#define GEN_H

#include "board.hpp"
#include "header.hpp"

#include <cassert>
#include <cstddef>
#include <utility>

namespace Cobra {

namespace Gen {

constexpr int SPAWN_COL = 4;
constexpr int SPAWN_ROW = 21;

template<Piece p, Rotation r>
consteval bool in_bounds(const int x) {
    static_assert(is_ok(p));
    static_assert(is_ok(r));
    constexpr PieceCoordinates pc = piece_table(p, r);
    return is_ok_x(pc[0].x + x) && is_ok_x(pc[1].x + x) && is_ok_x(pc[2].x + x) && is_ok_x(pc[3].x + x);
}

template<Piece p>
class CollisionMap {
private:
    static constexpr int searchSize = p == O ? 1 : ROTATION_NB;
    Bitboard board[COL_NB][searchSize];

public:
    explicit CollisionMap(const Board& b) {
        auto init = [&]<int x, Rotation r>{
            if constexpr (!in_bounds<p, r>(x))
                return ~0ULL;
            constexpr PieceCoordinates pc = piece_table(p, r);
            Bitboard result = 0;
            for (size_t i = 0; i < 4; ++i)
                result |= (pc[i].y < 0) ? ~(~b[x + pc[i].x] << -pc[i].y) : (b[x + pc[i].x] >> pc[i].y);
            return result;
        };

        [&]<size_t... xs>(std::index_sequence<xs...>) {
            auto init1 = [&]<Rotation r>{
                ((board[xs][r] = init.template operator()<xs, r>()), ...);
            };

            [&]<size_t... rs>(std::index_sequence<rs...>) {
                (init1.template operator()<static_cast<Rotation>(rs)>(), ...);
            }(std::make_index_sequence<searchSize>{});
        }(std::make_index_sequence<COL_NB>{});
    }

    const Bitboard* operator[](const int x) const { return board[x]; }
};

enum Direction {
    CW, CCW, FLIP, Direction_NB = 2
};

template<Direction d>
constexpr Rotation rotate(const Rotation r) {
    switch(d) {
        case CW: return static_cast<Rotation>((static_cast<int>(r) + 1) & 3);
        case CCW: return static_cast<Rotation>((static_cast<int>(r) + 3) & 3);
        case FLIP: return static_cast<Rotation>((static_cast<int>(r) + 2) & 3);
        default: __builtin_unreachable();
    }
}

template<size_t N>
class Offsets {
private:
    Coordinates coords[N];

public:
    template<typename... Ts>
    explicit constexpr Offsets(Ts... cs) : coords{cs...} {
        static_assert(sizeof...(cs) == N);
    }

    constexpr Coordinates operator[](const size_t i) const {
        assert(i < N);
        return coords[i];
    }
};

#define e Coordinates
constexpr Offsets<5> kicks[2][Direction_NB][ROTATION_NB] = {
    { // LJSZT
        { // CW
            Offsets<5>(e( 0,  0), e(-1,  0), e(-1,  1), e( 0, -2), e(-1, -2)),
            Offsets<5>(e( 0,  0), e( 1,  0), e( 1, -1), e( 0,  2), e( 1,  2)),
            Offsets<5>(e( 0,  0), e( 1,  0), e( 1,  1), e( 0, -2), e( 1, -2)),
            Offsets<5>(e( 0,  0), e(-1,  0), e(-1, -1), e( 0,  2), e(-1,  2))
        },
        { // CCW
            Offsets<5>(e( 0,  0), e( 1,  0), e( 1,  1), e( 0, -2), e( 1, -2)),
            Offsets<5>(e( 0,  0), e( 1,  0), e( 1, -1), e( 0,  2), e( 1,  2)),
            Offsets<5>(e( 0,  0), e(-1,  0), e(-1,  1), e( 0, -2), e(-1, -2)),
            Offsets<5>(e( 0,  0), e(-1,  0), e(-1, -1), e( 0,  2), e(-1,  2))
        }
    },
    { // I SRS+
        { // CW
            Offsets<5>(e( 1,  0), e( 2,  0), e(-1,  0), e(-1, -1), e( 2,  2)),
            Offsets<5>(e( 0, -1), e(-1, -1), e( 2, -1), e(-1,  1), e( 2, -2)),
            Offsets<5>(e(-1,  0), e( 1,  0), e(-2,  0), e( 1,  1), e(-2, -2)),
            Offsets<5>(e( 0,  1), e( 1,  1), e(-2,  1), e( 1, -1), e(-2,  2))
        },
        { // CCW
            Offsets<5>(e( 0, -1), e(-1, -1), e( 2, -1), e( 2, -2), e(-1,  1)),
            Offsets<5>(e(-1,  0), e(-2,  0), e( 1,  0), e(-2, -2), e( 1,  1)),
            Offsets<5>(e( 0,  1), e(-2,  1), e( 1,  1), e(-2,  2), e( 1, -1)),
            Offsets<5>(e( 1,  0), e( 2,  0), e(-1,  0), e( 2,  2), e(-1, -1))
        }
    }
};

constexpr Offsets<6> kicks180[2][ROTATION_NB] = {
    { // LJSZT
        Offsets<6>(e( 0,  0), e( 0,  1), e( 1,  1), e(-1,  1), e( 1,  0), e(-1,  0)),
        Offsets<6>(e( 0,  0), e( 1,  0), e( 1,  2), e( 1,  1), e( 0,  2), e( 0,  1)),
        Offsets<6>(e( 0,  0), e( 0, -1), e(-1, -1), e( 1, -1), e(-1,  0), e( 1,  0)),
        Offsets<6>(e( 0,  0), e(-1,  0), e(-1,  2), e(-1,  1), e( 0,  2), e( 0,  1))
    },
    { // I
        Offsets<6>(e( 1, -1), e( 1,  0), e( 2,  0), e( 0,  0), e( 2, -1), e( 0, -1)),
        Offsets<6>(e(-1, -1), e( 0, -1), e( 0,  1), e( 0,  0), e(-1,  1), e(-1,  0)),
        Offsets<6>(e(-1,  1), e(-1,  0), e(-2,  0), e( 0,  0), e(-2,  1), e( 0,  1)),
        Offsets<6>(e( 1,  1), e( 0,  1), e( 0,  3), e( 0,  2), e( 1,  3), e( 1,  2))
    }
};
#undef e

} // namespace Gen

} // namespace Cobra

#endif