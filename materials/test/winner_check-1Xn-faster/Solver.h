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
    
    // 特定の盤面・手番での勝敗を返す (1:勝ち, -1:負け)
    int solve(const std::vector<int>& board, int player);

    // 指定サイズNの「初手」の全パターンの勝敗を解析して文字列で返す
    // 例: "r,r,g,g,g,r,r"
    std::string analyze_initial_moves(int n);

    size_t get_node_count() const { return node_count; }

private:
    // メモ化テーブル (盤面ハッシュ -> 勝敗 1 or -1)
    std::unordered_map<HashKey, int> transposition_table;
    
    // Zobrist Hashing
    std::vector<std::vector<HashKey>> zobrist_table;
    HashKey zobrist_player[2];
    void init_zobrist(int max_n);
    HashKey compute_hash(const std::vector<int>& board, int player) const;
    std::vector<int> canonicalize(const std::vector<int>& board) const;

    // Alpha-Beta探索の実体
    // alpha: 自分が確保済みの最低スコア
    // beta:  相手が許容する最高スコア
    int alpha_beta(const MiniGo1xN& game, int alpha, int beta);

    size_t node_count = 0;
};