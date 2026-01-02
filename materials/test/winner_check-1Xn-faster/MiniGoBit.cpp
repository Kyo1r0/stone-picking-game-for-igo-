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

int MiniGoBit::solve(uint64_t my, uint64_t op, int alpha, int beta, int depth) {
    // 1. 置換表 (Transposition Table) 参照
    uint64_t key = compute_hash(my, op);
    size_t idx = key & (tt.size() - 1);
    
    if (tt[idx].flag && tt[idx].key == key) {
        return tt[idx].score;
    }

    // 2. 合法手の生成と探索
    // 空点かつ、打てる場所を探す
    uint64_t empty = ~(my | op) & full_mask;
    
    // 合法手が一つもない場合 -> 負け
    if (empty == 0) {
        return -1;
    }

    bool can_move = false;
    int max_val = -2; // 負け(-1)より小さい値で初期化

    // ビットスキャンで空きマスを走査
    // temp_emptyをコピーして操作する
    uint64_t temp_empty = empty;
    while (temp_empty) {
        // 最下位ビットのインデックスを取得 (GCC/Clang built-in)
        // MSVCなら _BitScanForward64 を使う
        int move_idx = __builtin_ctzll(temp_empty);
        uint64_t move_bit = 1ULL << move_idx;
        
        // 調べたビットを消す
        temp_empty &= ~move_bit;

        // --- 着手 ---
        uint64_t next_my = my | move_bit;
        
        // --- 勝利判定 (捕獲) ---
        // 自分の石を置いたことで、隣接する相手の石が死んだか？
        bool captured = false;
        
        // 左隣のチェック
        if ((move_idx > 0) && ((op >> (move_idx - 1)) & 1)) {
            // 左隣に相手がいる。その相手グループは死んでいるか？
            // 自分の石が増えたので、現在の empty から move_bit が減った状態を渡す必要はない
            // is_captured関数は stones と empty のマスクのみで判定する
            // ここでの empty は「現在の盤面の空」なので、move_bit は既に埋まっているとみなす必要がある
            // つまり is_captured の empty 引数は (empty & ~move_bit)
            if (is_captured(op, empty & ~move_bit, 1ULL << (move_idx - 1))) {
                captured = true;
            }
        }
        // 右隣のチェック (capturedが決まっていない場合のみ)
        if (!captured && (move_idx < n_size - 1) && ((op >> (move_idx + 1)) & 1)) {
            if (is_captured(op, empty & ~move_bit, 1ULL << (move_idx + 1))) {
                captured = true;
            }
        }

        if (captured) {
            // 相手の石を取ったら即勝ちのルール
            tt[idx] = {key, 1, 1};
            return 1;
        }

        // --- 自殺手のチェック ---
        // 相手を取れなかった場合、自分が死んでしまう手は打てない
        if (is_captured(next_my, empty & ~move_bit, move_bit)) {
            continue; // Illegal move (Suicide)
        }

        // --- 再帰探索 ---
        can_move = true;
        // 相手の手番へ (op と next_my を入れ替える)
        int score = -solve(op, next_my, -beta, -alpha, depth + 1);

        if (score > max_val) {
            max_val = score;
            // Beta Cutoff
            if (score >= beta) {
                tt[idx] = {key, (int16_t)score, 1};
                return score;
            }
            // Alpha update
            if (score > alpha) {
                alpha = score;
            }
        }
    }

    if (!can_move) {
        // 打てる場所がすべて自殺手だった場合 -> 負け
        tt[idx] = {key, -1, 1};
        return -1;
    }

    tt[idx] = {key, (int16_t)max_val, 1};
    return max_val;
}

std::string MiniGoBit::analyze(int n) {
    n_size = n;
    full_mask = (1ULL << n) - 1;
    clear_tt(); // 新しいNのためにテーブルをクリア

    std::string result = "";
    
    // 初手 i のループ
    for (int i = 0; i < n; ++i) {
        uint64_t move_bit = 1ULL << i;
        uint64_t my = move_bit;
        uint64_t op = 0;
        uint64_t empty = full_mask & ~move_bit;

        // 自殺手チェック (初手で自殺手はありえないが、念のため)
        if (is_captured(my, empty, move_bit)) {
            result += "x"; // Illegal
            continue;
        }

        // 初手で捕獲は発生しない (相手がいないため)
        
        // 相手の手番として探索開始
        int score = -solve(op, my, -1, 1, 1);
        
        if (score == 1) result += "g"; // Win (勝ち)
        else result += "r";            // Lose (負け)
    }
    return result;
}