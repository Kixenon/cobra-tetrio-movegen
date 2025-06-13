#include "bench.hpp"
#include "board.hpp"
#include "header.hpp"
#include "movegen.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>

namespace Cobra {

uint64_t perft(State& state, const Piece* next, unsigned depth) {
    if (depth == 1)
        return static_cast<uint64_t>(MoveList(state.board, *next).size());

    uint64_t nodes = 0;
    for (const Move& move : MoveList(state.board, *next)) {
        State nextState = state;
        nextState.do_move(move);
        nodes += perft(nextState, next + 1, depth - 1);
    }

    return nodes;
}

void bench_perft() {
    // Depth should be <= the queue size, but that is left to the user
    constexpr unsigned depth = 6;
    const Piece queue[] = {I, O, T, L, J, S, Z};
    State state;
    state.init();

    const auto start = std::chrono::high_resolution_clock::now();

    const uint64_t nodes = perft(state, queue, depth);

    const auto end = std::chrono::high_resolution_clock::now();
    const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Depth: " << depth
              << " Nodes: " << nodes
              << " Time: " << dt << "ms"
              << " NPS: " << (nodes * 1000) / static_cast<uint64_t>(dt + 1) << std::endl;
}

} // namespace Cobra