#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>

// 置換表のエントリ
struct TTEntry {
    uint64_t key; // ハッシュキー (盤面ID)
    int16_t score; // 評価値
    uint8_t flag;  // 0:Empty, 1:Valid
};

class MiniGoBit {
public:
    MiniGoBit(int max_n_size = 64);

    // 指定されたNについて、初手の評価値を文字列で返す (例: "rgrxg...")
    std::string analyze(int n);

private:
    int n_size;
    uint64_t full_mask; // N個のビットが立ったマスク

    // --- Transposition Table ---
    std::vector<TTEntry> tt;
    uint64_t zobrist_my[64];
    uint64_t zobrist_op[64];
    uint64_t zobrist_turn; // 手番用

    void init_zobrist();
    void clear_tt();
    
    // --- Core Logic ---
    // alpha-beta探索
    // my: 手番の石, op: 相手の石
    int solve(uint64_t my, uint64_t op, int alpha, int beta, int depth);

    // グループの呼吸点が0かどうか判定する
    // stones: 対象の色の石全体, empty: 空点のビット
    // start_bit: 調べたい石の位置 (1ULL << index)
    bool is_captured(uint64_t stones, uint64_t empty, uint64_t start_bit) const;

    // 盤面のハッシュ値を計算
    uint64_t compute_hash(uint64_t my, uint64_t op) const;
};