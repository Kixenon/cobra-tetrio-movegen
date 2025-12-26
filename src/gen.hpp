#ifndef GEN_H
#define GEN_H

#include "board.hpp"
#include "header.hpp"

#include <array>
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
consteval int canonical_size() {
    if constexpr (p == O)
        return 1;
    if constexpr (p == I || p == S || p == Z)
        return 2;
    return 4; // L, J, T
}

template<Piece p>
constexpr Rotation canonical_r(const Rotation r){
    // if constexpr (p == O)
    //     return NORTH;
    if constexpr (p == I || p == S || p == Z)
        return static_cast<Rotation>(r & 1);
    return r; // L, J, T
}

template<Piece p>
constexpr Coordinates canonical_offset(const Rotation r) {
    if constexpr (p == I) {
        if (r == SOUTH)
            return {1, 0};
        if (r == WEST)
            return {0, 1}; 
    }
    if constexpr (p == S || p == Z) {
        if (r == WEST)
            return {1, 0};
        if (r == SOUTH)
            return {0, 1};
    }
    return {0, 0};
}

template<Piece p>
class CollisionMap {
private:
    static constexpr int canonicalSize = canonical_size<p>();
    Bitboard board[COL_NB][canonicalSize];

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
            }(std::make_index_sequence<canonicalSize>{});
        }(std::make_index_sequence<COL_NB>{});
    }

    const Bitboard* operator[](const int x) const { return board[x]; }

    Bitboard operator()(const int x, const Rotation r) const { return board[x][canonical_r<p>(r)]; }
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
using Offsets = std::array<Coordinates, N>;

template<size_t N>
using OffsetsRot = std::array<Offsets<N>, ROTATION_NB>;


#define e Coordinates
constexpr OffsetsRot<5> kicks[2][Direction_NB] = {
    { // LJSZT
        { // CW
            e( 0,  0), e(-1,  0), e(-1,  1), e( 0, -2), e(-1, -2),
            e( 0,  0), e( 1,  0), e( 1, -1), e( 0,  2), e( 1,  2),
            e( 0,  0), e( 1,  0), e( 1,  1), e( 0, -2), e( 1, -2),
            e( 0,  0), e(-1,  0), e(-1, -1), e( 0,  2), e(-1,  2)
        },
        { // CCW
            e( 0,  0), e( 1,  0), e( 1,  1), e( 0, -2), e( 1, -2),
            e( 0,  0), e( 1,  0), e( 1, -1), e( 0,  2), e( 1,  2),
            e( 0,  0), e(-1,  0), e(-1,  1), e( 0, -2), e(-1, -2),
            e( 0,  0), e(-1,  0), e(-1, -1), e( 0,  2), e(-1,  2)
        }
    },
    { // I SRS+
        { // CW
            e( 1,  0), e( 2,  0), e(-1,  0), e(-1, -1), e( 2,  2),
            e( 0, -1), e(-1, -1), e( 2, -1), e(-1,  1), e( 2, -2),
            e(-1,  0), e( 1,  0), e(-2,  0), e( 1,  1), e(-2, -2),
            e( 0,  1), e( 1,  1), e(-2,  1), e( 1, -1), e(-2,  2)
        },
        { // CCW
            e( 0, -1), e(-1, -1), e( 2, -1), e( 2, -2), e(-1,  1),
            e(-1,  0), e(-2,  0), e( 1,  0), e(-2, -2), e( 1,  1),
            e( 0,  1), e(-2,  1), e( 1,  1), e(-2,  2), e( 1, -1),
            e( 1,  0), e( 2,  0), e(-1,  0), e( 2,  2), e(-1, -1)
        }
    }
};

constexpr OffsetsRot<6> kicks180[2] = {
    { // LJSZT
        e( 0,  0), e( 0,  1), e( 1,  1), e(-1,  1), e( 1,  0), e(-1,  0),
        e( 0,  0), e( 1,  0), e( 1,  2), e( 1,  1), e( 0,  2), e( 0,  1),
        e( 0,  0), e( 0, -1), e(-1, -1), e( 1, -1), e(-1,  0), e( 1,  0),
        e( 0,  0), e(-1,  0), e(-1,  2), e(-1,  1), e( 0,  2), e( 0,  1)
    },
    { // I
        e( 1, -1), e( 1,  0), e( 2,  0), e( 0,  0), e( 2, -1), e( 0, -1),
        e(-1, -1), e( 0, -1), e( 0,  1), e( 0,  0), e(-1,  1), e(-1,  0),
        e(-1,  1), e(-1,  0), e(-2,  0), e( 0,  0), e(-2,  1), e( 0,  1),
        e( 1,  1), e( 0,  1), e( 0,  3), e( 0,  2), e( 1,  3), e( 1,  2)
    }
};
#undef e

} // namespace Gen

} // namespace Cobra

#endif