#include "MiniGoMT.h"
#include <algorithm>
#include <random>
#include <cstring>
#include <future>
#include <thread>
#include <iostream>

#if defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)
#endif

// ビットスキャン関数のラッパー
inline int bit_scan_forward(uint64_t b) {
#if defined(_MSC_VER)
    unsigned long index;
    _BitScanForward64(&index, b);
    return (int)index;
#else
    return __builtin_ctzll(b);
#endif
}

inline int bit_scan_reverse(uint64_t b) {
#if defined(_MSC_VER)
    unsigned long index;
    _BitScanReverse64(&index, b);
    return (int)index;
#else
    return 63 - __builtin_clzll(b);
#endif
}

MiniGoMT::MiniGoMT(int tt_bits) {
    size_t size = 1ULL << tt_bits;
    tt.resize(size);
    tt_mask = size - 1;
    init_zobrist();
}

MiniGoMT::~MiniGoMT() {}

void MiniGoMT::init_zobrist() {
    std::mt19937_64 rng(12345);
    for (int i = 0; i < 64; ++i) {
        zobrist_my[i] = rng();
        zobrist_op[i] = rng();
    }
}

void MiniGoMT::clear_tt() {
    std::memset(tt.data(), 0, tt.size() * sizeof(TTEntry));
}

uint64_t MiniGoMT::compute_hash(uint64_t my, uint64_t op) const {
    uint64_t h1 = 0;
    for (int i = 0; i < n_size; ++i) {
        if ((my >> i) & 1) h1 ^= zobrist_my[i];
        if ((op >> i) & 1) h1 ^= zobrist_op[i];
    }
    uint64_t h2 = 0;
    for (int i = 0; i < n_size; ++i) {
        int rev_i = n_size - 1 - i;
        if ((my >> i) & 1) h2 ^= zobrist_my[rev_i];
        if ((op >> i) & 1) h2 ^= zobrist_op[rev_i];
    }
    return std::min(h1, h2);
}

// ★改良: ループなしで O(1) で判定
bool MiniGoMT::is_captured(uint64_t stones, uint64_t empty, uint64_t start_bit) const {
    // stones: チェック対象の色の石
    // 境界(壁または相手の石または空)を探す
    // boundaries = 「自分の石ではない場所」のビットマスク
    uint64_t boundaries = (~stones) & full_mask;
    
    int idx = bit_scan_forward(start_bit);

    // 左側の境界を探す (idxより小さいビットで、boundariesが立っている最大の場所)
    // mask: idxより下位のビットのみ1
    uint64_t left_mask = (1ULL << idx) - 1;
    uint64_t left_bounds = boundaries & left_mask;
    int l_boundary_idx = (left_bounds == 0) ? -1 : bit_scan_reverse(left_bounds);

    // 右側の境界を探す (idxより大きいビットで、boundariesが立っている最小の場所)
    // mask: idx+1より上位のビットのみ1
    uint64_t right_mask = ~((1ULL << (idx + 1)) - 1);
    uint64_t right_bounds = boundaries & right_mask;
    int r_boundary_idx = (right_bounds == 0) ? n_size : bit_scan_forward(right_bounds);

    // 境界が「空点」であれば呼吸点あり
    bool lib_left = (l_boundary_idx != -1) && ((empty >> l_boundary_idx) & 1);
    bool lib_right = (r_boundary_idx != n_size) && ((empty >> r_boundary_idx) & 1);

    return !(lib_left || lib_right);
}

int MiniGoMT::solve(uint64_t my, uint64_t op, int alpha, int beta, int depth) {
    uint64_t key = compute_hash(my, op);
    size_t idx = key & tt_mask;

    TTEntry entry = tt[idx]; 
    if (entry.flag && entry.key == key) {
        return entry.score;
    }

    uint64_t empty = ~(my | op) & full_mask;
    if (empty == 0) return -1;

    // ★改良: 動的な Neighbor Priority
    // 1. 相手の石の隣 (攻撃・防御の急所)
    uint64_t op_adj = ((op << 1) | (op >> 1)) & empty;
    // 2. 自分の石の隣 (連結・眼作り)
    uint64_t my_adj = ((my << 1) | (my >> 1)) & empty & ~op_adj;
    // 3. その他 (飛び石)
    uint64_t rest = empty & ~(op_adj | my_adj);

    bool can_move = false;
    int max_val = -2;

    // ラムダ式で探索ロジックを共通化 (インライン展開される)
    auto process_moves = [&](uint64_t moves_mask) -> int {
        while (moves_mask) {
            int move_idx = bit_scan_forward(moves_mask);
            uint64_t move_bit = 1ULL << move_idx;
            moves_mask &= ~move_bit;

            uint64_t next_my = my | move_bit;
            bool captured = false;

            // 捕獲チェック (move_idxの隣だけ見れば良い)
            if ((move_idx > 0) && ((op >> (move_idx - 1)) & 1)) {
                if (is_captured(op, empty & ~move_bit, 1ULL << (move_idx - 1))) captured = true;
            }
            if (!captured && (move_idx < n_size - 1) && ((op >> (move_idx + 1)) & 1)) {
                if (is_captured(op, empty & ~move_bit, 1ULL << (move_idx + 1))) captured = true;
            }

            if (captured) {
                return 1; // 勝ち確定シグナル
            }

            // 自殺手チェック
            if (is_captured(next_my, empty & ~move_bit, move_bit)) {
                continue;
            }

            can_move = true;
            int score = -solve(op, next_my, -beta, -alpha, depth + 1);

            if (score > max_val) {
                max_val = score;
                if (score >= beta) return 2; // Beta Cutoffシグナル
                if (score > alpha) alpha = score;
            }
        }
        return 0; // 続行
    };

    // 優先順位に従って実行
    if (int res = process_moves(op_adj)) {
        if (res == 1) { tt[idx] = {key, 1, 1}; return 1; }
        if (res == 2) { tt[idx] = {key, (int16_t)max_val, 1}; return max_val; }
    }
    if (int res = process_moves(my_adj)) {
        if (res == 1) { tt[idx] = {key, 1, 1}; return 1; }
        if (res == 2) { tt[idx] = {key, (int16_t)max_val, 1}; return max_val; }
    }
    if (int res = process_moves(rest)) {
        if (res == 1) { tt[idx] = {key, 1, 1}; return 1; }
        if (res == 2) { tt[idx] = {key, (int16_t)max_val, 1}; return max_val; }
    }

    if (!can_move) {
        tt[idx] = {key, -1, 1};
        return -1;
    }

    tt[idx] = {key, (int16_t)max_val, 1};
    return max_val;
}

std::string MiniGoMT::analyze_parallel(int n) {
    n_size = n;
    full_mask = (1ULL << n) - 1;
    clear_tt();

    std::string result(n, ' ');
    int half_n = (n + 1) / 2;

    auto task_func = [&](int i) -> char {
        uint64_t move_bit = 1ULL << i;
        uint64_t my = move_bit;
        uint64_t op = 0;
        uint64_t empty = full_mask & ~move_bit;

        if (is_captured(my, empty, move_bit)) return 'x';

        int score = -solve(op, my, -1, 1, 1);
        return (score == 1) ? 'g' : 'r';
    };

    std::vector<std::future<char>> futures;
    for (int i = 0; i < half_n; ++i) {
        futures.push_back(std::async(std::launch::async, task_func, i));
    }

    for (int i = 0; i < half_n; ++i) {
        result[i] = futures[i].get();
    }
    for (int i = half_n; i < n; ++i) {
        result[i] = result[n - 1 - i];
    }
    return result;
}