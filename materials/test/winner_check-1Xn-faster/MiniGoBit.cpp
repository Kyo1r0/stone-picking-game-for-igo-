#include "MiniGoBit.h"
#include <algorithm>
#include <random>
#include <cstring>

// コンストラクタ: TTとZobristの初期化
MiniGoBit::MiniGoBit(int max_n_size) {
    // 置換表のサイズ: 2^24 = 約1677万エントリ (約256MB)
    // Nが大きくなると衝突が増えるため、メモリが許す限り大きくする
    tt.resize(1 << 27); 
    init_zobrist();
}

void MiniGoBit::init_zobrist() {
    std::mt19937_64 rng(12345);
    for (int i = 0; i < 64; ++i) {
        zobrist_my[i] = rng();
        zobrist_op[i] = rng();
    }
    zobrist_turn = rng();
}

void MiniGoBit::clear_tt() {
    // 高速化のため、全て0クリアではなく世代管理等をするのが一般的だが
    // ここではシンプルにmemsetする (数ms程度)
    std::memset(tt.data(), 0, tt.size() * sizeof(TTEntry));
}

// 盤面の正規化（左右反転の対称性除去）をしてハッシュ化
uint64_t MiniGoBit::compute_hash(uint64_t my, uint64_t op) const {
    // 現在の盤面
    uint64_t h1 = 0;
    for (int i = 0; i < n_size; ++i) {
        if ((my >> i) & 1) h1 ^= zobrist_my[i];
        if ((op >> i) & 1) h1 ^= zobrist_op[i];
    }

    // 左右反転した盤面
    uint64_t h2 = 0;
    for (int i = 0; i < n_size; ++i) {
        int rev_i = n_size - 1 - i;
        if ((my >> i) & 1) h2 ^= zobrist_my[rev_i];
        if ((op >> i) & 1) h2 ^= zobrist_op[rev_i];
    }

    // 小さい方を採用することで対称形を同一視
    return std::min(h1, h2);
}

// 高速なグループ判定ロジック
// 指定したビット(start_bit)を含む連結成分が、呼吸点(empty)を持つか判定
bool MiniGoBit::is_captured(uint64_t stones, uint64_t empty, uint64_t start_bit) const {
    // 連結成分（グループ）をビットマスクとして抽出する
    // アルゴリズム: 変化しなくなるまで左右に広げる
    uint64_t group = start_bit;
    while (true) {
        uint64_t expanded = group;
        // 左に連結している石を追加
        expanded |= (group << 1) & stones;
        // 右に連結している石を追加
        expanded |= (group >> 1) & stones;
        
        // Nの範囲外にはみ出したビットをカット (full_mask依存)
        expanded &= full_mask;

        if (expanded == group) break;
        group = expanded;
    }

    // グループの隣（左シフトと右シフト）に空点があるか？
    // (group << 1) & empty : 左側の呼吸点
    // (group >> 1) & empty : 右側の呼吸点
    bool has_liberty = (((group << 1) & empty) || ((group >> 1) & empty));
    
    return !has_liberty;
}


// MiniGoBit.cpp

// ---------------------------------------------------------
// analyze 関数を修正 (探索順序の生成を追加)
// ---------------------------------------------------------
std::string MiniGoBit::analyze(int n) {
    n_size = n;
    full_mask = (1ULL << n) - 1;
    clear_tt();

    // ★追加: 中央から外側に向かう探索順序を生成
    move_order.clear();
    move_order.reserve(n);
    int mid = n / 2;
    move_order.push_back(mid);
    for (int dist = 1; dist < n; ++dist) {
        if (mid - dist >= 0) move_order.push_back(mid - dist);
        if (mid + dist < n)  move_order.push_back(mid + dist);
    }
    // ------------------------------------------------

    std::string result = "";
    
    // ここも move_order 順に調べたほうが「解析結果」は早く出始めますが、
    // 文字列の並び順(0〜N-1)を維持したいので、あえて普通のループのままにします
    for (int i = 0; i < n; ++i) {
        // ... (既存のコードと同じ)
        uint64_t move_bit = 1ULL << i;
        uint64_t my = move_bit;
        uint64_t op = 0;
        uint64_t empty = full_mask & ~move_bit;

        if (is_captured(my, empty, move_bit)) {
            result += "x"; 
            continue;
        }

        // 探索呼び出し
        int score = -solve(op, my, -1, 1, 1);
        
        if (score == 1) result += "g"; 
        else result += "r";            
    }
    return result;
}

// ---------------------------------------------------------
// solve 関数を修正 (ビットスキャンをやめて move_order ループへ)
// ---------------------------------------------------------
int MiniGoBit::solve(uint64_t my, uint64_t op, int alpha, int beta, int depth) {
    // 1. 置換表参照 (変更なし)
    uint64_t key = compute_hash(my, op);
    size_t idx = key & (tt.size() - 1);
    
    if (tt[idx].flag && tt[idx].key == key) {
        return tt[idx].score;
    }

    uint64_t empty = ~(my | op) & full_mask;
    if (empty == 0) return -1;

    bool can_move = false;
    int max_val = -2; 

    // ★修正: while(temp_empty) をやめて、move_order でループする
    // これにより「中央付近」から優先的に探索される
    for (int move_idx : move_order) {
        uint64_t move_bit = 1ULL << move_idx;

        // 空きマスでなければスキップ
        if (!((empty >> move_idx) & 1)) {
            continue;
        }

        // --- 以下、以前のロジックと同じ ---
        
        uint64_t next_my = my | move_bit;
        bool captured = false;
        
        // 左隣
        if ((move_idx > 0) && ((op >> (move_idx - 1)) & 1)) {
            if (is_captured(op, empty & ~move_bit, 1ULL << (move_idx - 1))) {
                captured = true;
            }
        }
        // 右隣
        if (!captured && (move_idx < n_size - 1) && ((op >> (move_idx + 1)) & 1)) {
            if (is_captured(op, empty & ~move_bit, 1ULL << (move_idx + 1))) {
                captured = true;
            }
        }

        if (captured) {
            tt[idx] = {key, 1, 1};
            return 1;
        }

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
            if (score > alpha) {
                alpha = score;
            }
        }
    }
    // ループ終了

    if (!can_move) {
        tt[idx] = {key, -1, 1};
        return -1;
    }

    tt[idx] = {key, (int16_t)max_val, 1};
    return max_val;
}