#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <atomic>

struct TTEntry {
    uint64_t key;
    int16_t score;
    uint8_t flag; 
};

class MiniGoMT {
public:
    MiniGoMT(int tt_bits = 24);
    ~MiniGoMT();

    std::string analyze_parallel(int n);

private:
    int n_size;
    uint64_t full_mask;

    // Transposition Table
    std::vector<TTEntry> tt;
    uint64_t tt_mask;

    uint64_t zobrist_my[64];
    uint64_t zobrist_op[64];

    void init_zobrist();
    void clear_tt();

    int solve(uint64_t my, uint64_t op, int alpha, int beta, int depth);

    uint64_t compute_hash(uint64_t my, uint64_t op) const;
    
    // O(1) に高速化された判定関数
    bool is_captured(uint64_t stones, uint64_t empty, uint64_t start_bit) const;
};