#include "MiniGoMT.h"
#include <algorithm>
#include <random>
#include <cstring>
#include <future> // マルチスレッド用
#include <thread>
#include <iostream>

MiniGoMT::MiniGoMT(int tt_bits) {
    // メモリに合わせてサイズ調整 (例: 2^27 = 1.3億エントリ = 約2GB)
    // 搭載メモリに余裕があれば 27 や 28 に増やしてください
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
    // 並列実行前にリセット
    // memsetは高速ですが、TTEntryがPOD型(単純構造体)である必要があります
    std::memset(tt.data(), 0, tt.size() * sizeof(TTEntry));
}

uint64_t MiniGoMT::compute_hash(uint64_t my, uint64_t op) const {
    uint64_t h1 = 0;
    for (int i = 0; i < n_size; ++i) {
        if ((my >> i) & 1) h1 ^= zobrist_my[i];
        if ((op >> i) & 1) h1 ^= zobrist_op[i];
    }
    // 左右対称性の正規化
    uint64_t h2 = 0;
    for (int i = 0; i < n_size; ++i) {
        int rev_i = n_size - 1 - i;
        if ((my >> i) & 1) h2 ^= zobrist_my[rev_i];
        if ((op >> i) & 1) h2 ^= zobrist_op[rev_i];
    }
    return std::min(h1, h2);
}

// 高速化された is_captured (変更なし)
bool MiniGoMT::is_captured(uint64_t stones, uint64_t empty, uint64_t start_bit) const {
    uint64_t group = start_bit;
    while (true) {
        uint64_t expanded = group;
        expanded |= (group << 1) & stones;
        expanded |= (group >> 1) & stones;
        expanded &= full_mask;
        if (expanded == group) break;
        group = expanded;
    }
    return !(((group << 1) & empty) || ((group >> 1) & empty));
}

// solve関数全体をこれに置き換えてください
int MiniGoMT::solve(uint64_t my, uint64_t op, int alpha, int beta, int depth) {
    uint64_t key = compute_hash(my, op);
    size_t idx = key & tt_mask;

    // TT参照 (競合許容)
    TTEntry entry = tt[idx]; 
    if (entry.flag && entry.key == key) {
        return entry.score;
    }

    uint64_t empty = ~(my | op) & full_mask;
    if (empty == 0) return -1;

    // --- 【修正】Move Ordering (Center First) ---
    // ビットスキャン(ctzll)をやめ、中央から外側へ広がる順序でループする
    // N=40程度なら毎回vectorを作ってもオーバーヘッドは無視できます
    // むしろ枝刈りの効果が絶大です
    
    // 探索順序配列の生成 (毎回生成しても高速です)
    int mid = n_size / 2;
    // 予想される手の最大数は残り空きマス数以下
    // vectorの動的確保を避けるため、固定長配列的に使うかreserveする
    // ここでは可読性優先でvectorを使いますが、十分速いです
    static const int MAX_N = 64; 
    int moves[MAX_N];
    int move_count = 0;

    // 中央
    if ((empty >> mid) & 1) moves[move_count++] = mid;
    
    // 中央から左右へ広げる
    for (int dist = 1; dist <= mid + 1; ++dist) {
        int l = mid - dist;
        int r = mid + dist;
        if (l >= 0 && ((empty >> l) & 1)) moves[move_count++] = l;
        if (r < n_size && ((empty >> r) & 1)) moves[move_count++] = r;
    }

    bool can_move = false;
    int max_val = -2;

    // 生成した順序(中央優先)でループ
    for (int i = 0; i < move_count; ++i) {
        int move_idx = moves[i];
        uint64_t move_bit = 1ULL << move_idx;
        
        // --- 以下は以前と同じロジック ---

        uint64_t next_my = my | move_bit;
        bool captured = false;

        // Check capture (左)
        if ((move_idx > 0) && ((op >> (move_idx - 1)) & 1)) {
            // emptyからmove_bitを除外したマスクを渡す
            if (is_captured(op, empty & ~move_bit, 1ULL << (move_idx - 1))) captured = true;
        }
        // Check capture (右)
        if (!captured && (move_idx < n_size - 1) && ((op >> (move_idx + 1)) & 1)) {
            if (is_captured(op, empty & ~move_bit, 1ULL << (move_idx + 1))) captured = true;
        }

        if (captured) {
            tt[idx] = {key, 1, 1};
            return 1;
        }

        // Suicide check
        if (is_captured(next_my, empty & ~move_bit, move_bit)) {
            continue; 
        }

        can_move = true;
        int score = -solve(op, next_my, -beta, -alpha, depth + 1);

        if (score > max_val) {
            max_val = score;
            if (score >= beta) {
                tt[idx] = {key, (int16_t)score, 1};
                return score;
            }
            if (score > alpha) alpha = score;
        }
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
    int half_n = (n + 1) / 2; // 対称性があるので半分だけ計算

    // タスクの定義
    auto task_func = [&](int i) -> char {
        uint64_t move_bit = 1ULL << i;
        uint64_t my = move_bit;
        uint64_t op = 0;
        uint64_t empty = full_mask & ~move_bit;

        if (is_captured(my, empty, move_bit)) return 'x';

        // Solver呼び出し
        int score = -solve(op, my, -1, 1, 1);
        return (score == 1) ? 'g' : 'r';
    };

    // Futureリスト（非同期タスクの結果受け取り用）
    std::vector<std::future<char>> futures;

    // 半分だけ並列起動
    for (int i = 0; i < half_n; ++i) {
        futures.push_back(std::async(std::launch::async, task_func, i));
    }

    // 結果回収
    for (int i = 0; i < half_n; ++i) {
        result[i] = futures[i].get();
    }

    // 対称性を利用して残りを埋める
    // 例: N=5, 結果 "rrg" -> "rrgrr"
    for (int i = half_n; i < n; ++i) {
        result[i] = result[n - 1 - i];
    }

    return result;
}