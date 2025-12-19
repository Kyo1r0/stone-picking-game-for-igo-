#pragma once
#include "MiniGo1xN.h"
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <random>
#include <string>

using HashKey = std::uint64_t;

class Solver {
public:
    Solver();

    // 盤面 board・手番 player (1 or -1) から見た勝敗
    // 返り値: 1 = 勝ち, -1 = 負け
    int solve(const std::vector<int>& board, int player);
      std::string analyze_initial_moves(int n);

    size_t get_node_count() const { return node_count; }

private:
    // --- Transposition Table ---
    std::unordered_map<HashKey, int> table;

    // --- Zobrist Hash ---
    std::vector<std::vector<HashKey>> zobrist;
    HashKey zobrist_player[2];

    void init_zobrist(int max_n);
    HashKey compute_hash(const std::vector<int>& board, int player) const;
    std::vector<int> canonicalize(const std::vector<int>& board) const;

    // --- Alpha-Beta ---
    int alpha_beta(const MiniGo1xN& game, int alpha, int beta);

    size_t node_count = 0;
};
