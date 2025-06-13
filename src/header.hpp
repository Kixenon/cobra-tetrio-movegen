#ifndef HEADER_H
#define HEADER_H

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace Cobra {

/*----------------------------------------------------------------------------*/
// Types

using Bitboard = uint64_t;

constexpr int COL_NB = 10;
constexpr int ROW_NB = 64;

enum Piece {
    I, O, T, L, J, S, Z, PIECE_NB = 7, TSPIN = 7, NO_PIECE = 8
};

enum Rotation {
    NORTH, EAST, SOUTH, WEST, ROTATION_NB = 4
};

enum SpinType {
    NO_SPIN, MINI, FULL, SPIN_NB = 3
};

constexpr Piece allPieces[] = {
    I, O, T, L, J, S, Z
};

constexpr Rotation allRotations[] = {
    NORTH, EAST, SOUTH, WEST
};

struct Coordinates;
class Move;
class PieceCoordinates;

constexpr PieceCoordinates piece_table(Piece p, Rotation r);

/*----------------------------------------------------------------------------*/
// Debug functions

constexpr bool is_ok(Piece p);
constexpr bool is_ok(Rotation r);
constexpr bool is_ok(SpinType s);
constexpr bool is_ok_x(int x);
constexpr bool is_ok_y(int y);
constexpr bool is_ok(const Move& m);

/*----------------------------------------------------------------------------*/
// Types

struct Coordinates {
    int8_t x, y;

    Coordinates() = default;
    constexpr Coordinates(int x, int y) : x(static_cast<int8_t>(x)), y(static_cast<int8_t>(y)) {}

    constexpr Coordinates operator+(const Coordinates& c) const;
    constexpr Coordinates operator-(const Coordinates& c) const;
    constexpr Coordinates& operator+=(const Coordinates& c);
    constexpr Coordinates& operator-=(const Coordinates& c);
};

class Move {
private:
    union {
        struct {
            uint16_t y : 6;
            uint16_t x : 4;
            uint16_t piece : 3;
            uint16_t rotation : 2;
            uint16_t spin : 1;
        } bits;
        uint16_t data;
    };

    explicit constexpr Move(uint16_t data) : data(data) {}

public:
    Move() = default;
    constexpr Move(Piece p, Rotation r, int x, int y, bool fullspin = false) :
        bits{static_cast<uint16_t>(y),
             static_cast<uint16_t>(x),
             static_cast<uint16_t>(p),
             static_cast<uint16_t>(r),
             static_cast<uint16_t>(fullspin)} {
        assert(is_ok(p) || p == TSPIN);
        assert(is_ok(r));
        assert(is_ok_x(x));
        assert(is_ok_y(y));
        assert(p == TSPIN || !fullspin);
    }

    constexpr Piece piece() const { return bits.piece == TSPIN ? T : static_cast<Piece>(bits.piece); }
    constexpr Rotation rotation() const { return static_cast<Rotation>(bits.rotation); }
    constexpr SpinType spin() const { return static_cast<SpinType>((bits.piece == TSPIN) + bits.spin); }
    constexpr int x() const { return bits.x; }
    constexpr int y() const { return bits.y; }
    constexpr bool operator==(const Move& m) const { return data == m.data; }
    constexpr PieceCoordinates cells() const;
    static constexpr Move none() { return Move(0); } // Illegal move used for marking
};

class PieceCoordinates {
private:
    Coordinates coords[4];

public:
    constexpr PieceCoordinates() = default;
    constexpr PieceCoordinates(Coordinates a, Coordinates b, Coordinates c, Coordinates d)
        : coords{a, b, c, d} {}

    constexpr const Coordinates& operator[](const size_t i) const {
        assert(i >= 0 && i < 4);
        return coords[i];
    }
    constexpr PieceCoordinates& operator+=(const Coordinates& c);
};

/*----------------------------------------------------------------------------*/
// Debug functions

constexpr bool is_ok(const Piece p) {
    return p >= I && p < PIECE_NB;
}
constexpr bool is_ok(const Rotation r) {
    return r >= NORTH && r < ROTATION_NB;
}
constexpr bool is_ok(const SpinType s) {
    return s >= NO_SPIN && s < SPIN_NB;
}
constexpr bool is_ok_x(const int x) {
    return x >= 0 && x < COL_NB;
}
constexpr bool is_ok_y(const int y) {
    return y >= 0 && y < ROW_NB;
}
constexpr bool is_ok(const Move& m) {
    return is_ok(m.piece()) && is_ok(m.rotation()) &&
           is_ok(m.spin()) && is_ok_x(m.x()) && is_ok_y(m.y());
}

/*----------------------------------------------------------------------------*/
// Implementation

constexpr bool operator!(const Piece& p) { return p == NO_PIECE; }

constexpr Coordinates& Coordinates::operator+=(const Coordinates& c) {
    x += c.x;
    y += c.y;
    return *this;
}

constexpr Coordinates& Coordinates::operator-=(const Coordinates& c) {
    x -= c.x;
    y -= c.y;
    return *this;
}

constexpr Coordinates Coordinates::operator+(const Coordinates& c) const {
    return Coordinates(x + c.x, y + c.y);
}

constexpr Coordinates Coordinates::operator-(const Coordinates& c) const {
    return Coordinates(x - c.x, y - c.y);
}

constexpr PieceCoordinates Move::cells() const {
    return piece_table(piece(), rotation()) += Coordinates(x(), y());
}

constexpr PieceCoordinates& PieceCoordinates::operator+=(const Coordinates& c) {
    for (auto& i : coords)
        i += c;
    return *this;
}

constexpr PieceCoordinates piece_table(const Piece p, const Rotation r) {
    assert(is_ok(p));
    assert(is_ok(r));

    using C = Coordinates;
    constexpr auto make_piece = [](const Piece p) {
        switch(p) {
            case I: return PieceCoordinates(C(-1, 0), C( 0, 0), C( 1, 0), C( 2, 0)); // ––––
            case O: return PieceCoordinates(C( 0, 0), C( 1, 0), C( 0, 1), C( 1, 1)); // ::
            case T: return PieceCoordinates(C(-1, 0), C( 0, 0), C( 1, 0), C( 0, 1)); // _|_
            case L: return PieceCoordinates(C(-1, 0), C( 0, 0), C( 1, 0), C( 1, 1)); // __|
            case J: return PieceCoordinates(C(-1, 0), C( 0, 0), C( 1, 0), C(-1, 1)); // ––;
            case S: return PieceCoordinates(C(-1, 0), C( 0, 0), C( 0, 1), C( 1, 1)); // S
            case Z: return PieceCoordinates(C(-1, 1), C( 0, 1), C( 0, 0), C( 1, 0)); // Z
            default: __builtin_unreachable();
        }
    };

    constexpr auto rotate = [](const Rotation r, const Coordinates c) {
        switch(r) {
            case EAST:  return C(c.y, -c.x);
            case SOUTH: return C(-c.x, -c.y);
            case WEST:  return C(-c.y, c.x);
            default:    return c;
        }
    };

    const auto cells = make_piece(p);
    return PieceCoordinates(
        rotate(r, cells[0]), rotate(r, cells[1]),
        rotate(r, cells[2]), rotate(r, cells[3])
    );
}

/*----------------------------------------------------------------------------*/
// Bitboard operations

constexpr int clz(const Bitboard v) {
    return v ? __builtin_clzll(v) : 64;
}

template <typename T>
constexpr int ctz(const T v) {
    assert(v);
    return __builtin_ctzll(v);
}

constexpr int popcount(const Bitboard v) {
    return __builtin_popcountll(v);
}

constexpr int bitlen(const Bitboard v){
    return 64 - clz(v);
}

constexpr Bitboard bb(const int v){
    assert(v >= 0);
    return 1ULL << v;
}

constexpr Bitboard bb_low(const int v){
    assert(v >= 0);
    return (1ULL << v) - 1;
}

} // namespace Cobra

#endif // HEADER_H