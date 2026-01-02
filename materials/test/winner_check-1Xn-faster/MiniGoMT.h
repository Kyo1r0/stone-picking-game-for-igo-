#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <atomic>

// ロックフリーに近い簡易的なTTエントリ
struct TTEntry {
    uint64_t key;
    int16_t score;
    uint8_t flag; // 0:Empty, 1:Valid
};

class MiniGoMT {
public:
    MiniGoMT(int tt_bits = 24); // デフォルト2^24 (256MB)
    ~MiniGoMT();

    // 並列処理で解析を実行
    std::string analyze_parallel(int n);

private:
    int n_size;
    uint64_t full_mask;

    // --- Shared Transposition Table ---
    // 複数のスレッドから書き込まれるため、競合を許容する設計にします
    // (厳密な排他制御は遅くなるため、低確率の衝突は無視します)
    std::vector<TTEntry> tt;
    uint64_t tt_mask;

    // Zobrist Hash Keys
    uint64_t zobrist_my[64];
    uint64_t zobrist_op[64];

    void init_zobrist();
    void clear_tt();

    // 探索関数 (const参照でTTを読み書き)
    int solve(uint64_t my, uint64_t op, int alpha, int beta, int depth);

    // ヘルパー
    uint64_t compute_hash(uint64_t my, uint64_t op) const;
    bool is_captured(uint64_t stones, uint64_t empty, uint64_t start_bit) const;
};