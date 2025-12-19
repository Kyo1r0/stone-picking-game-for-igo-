#include "Solver.h"
#include <iostream>
#include <algorithm>

Solver::Solver() {
    init_zobrist(100); // N=100まで対応可能にしておく
}

void Solver::init_zobrist(int max_n) {
    zobrist_table.assign(max_n, std::vector<HashKey>(3));
    std::mt19937_64 rng(12345); // 固定シードで再現性確保
    for (int i = 0; i < max_n; ++i) {
        for (int j = 0; j < 3; ++j) zobrist_table[i][j] = rng();
    }
    zobrist_player[0] = rng();
    zobrist_player[1] = rng();
}

std::vector<int> Solver::canonicalize(const std::vector<int>& board) const {
    std::vector<int> rev(board.rbegin(), board.rend());
    return (rev < board) ? rev : board;
}

HashKey Solver::compute_hash(const std::vector<int>& original, int player) const {
    auto board = canonicalize(original);
    HashKey h = 0;
    for (size_t i = 0; i < board.size(); ++i) {
        int idx = (board[i] == 0 ? 0 : (board[i] == 1 ? 1 : 2));
        h ^= zobrist_table[i][idx];
    }
    h ^= zobrist_player[player == 1 ? 0 : 1];
    return h;
}

int Solver::solve(const std::vector<int>& board, int player) {
    node_count = 0;
    transposition_table.clear();
    MiniGo1xN game(board, player);
    // 初期値: -infinity to +infinity
    return alpha_beta(game, -2, 2); 
}

// Alpha-Beta法
// 戻り値: 1(勝ち), -1(負け)
int Solver::alpha_beta(const MiniGo1xN& game, int alpha, int beta) {
    node_count++;

    // 1. メモ化チェック
    HashKey key = compute_hash(game.board, game.player);
    if (transposition_table.count(key)) {
        return transposition_table[key];
    }

    // 2. 合法手の生成
    auto moves = game.get_legal_moves();
    
    // 敗北条件: 打つ手がない
    if (moves.empty()) {
        return -1; // 負け
    }

    // 3. 手の並べ替え (Move Ordering)
    // 「石を取れる手」を優先的に調べることで枝刈り効率を上げる
    // ※ make_move は少し重いので、本来は軽量なチェックが良いが、
    //    このゲームは「取ったら勝ち」なので make_move して勝てたら即リターンで良い。

    int best_score = -2; // 負け(-1)より小さい値で初期化

    for (int m : moves) {
        auto [next_game, captured] = game.make_move(m);

        // ★勝利条件: 石を取ったら即勝ち
        if (captured) {
            transposition_table[key] = 1;
            return 1;
        }

        // 再帰探索 (相手の手番なので、alpha/beta を反転して渡す)
        // 戻り値が -1(相手負け) なら、自分は +1(勝ち)
        int score = -alpha_beta(next_game, -beta, -alpha);

        if (score > best_score) {
            best_score = score;
        }

        // Alpha-Beta Pruning (枝刈り)
        // 自分が勝てる手が見つかったら(score=1)、それ以上探索する必要はない
        if (score >= beta) {
            // betaカット (相手はこの局面にさせてくれない)
            // このゲームは勝敗のみ(1 or -1)なので、勝ちが見えたら即終了でOK
            transposition_table[key] = score;
            return score;
        }
        
        // alpha値の更新 (自分が見つけた最善手)
        if (score > alpha) {
            alpha = score;
        }
    }

    transposition_table[key] = best_score;
    return best_score;
}

// 初期盤面の各手の評価を一覧化する便利関数
std::string Solver::analyze_initial_moves(int n) {
    std::vector<int> board(n, 0);
    MiniGo1xN game(board, 1);
    std::string result = "";

    // ★ ここで1回だけ初期化
    transposition_table.clear();
    node_count = 0;

    for (int i = 0; i < n; ++i) {
        if (i > 0) result += ",";

        if (game.would_be_suicide(i)) {
            result += "x";
            continue;
        }

        auto [next_game, captured] = game.make_move(i);

        if (captured) {
            result += "g";
        } else {
            // ★ solve() を使わない
            int res = eval(next_game);

            if (res == -1) result += "g";
            else result += "r";
        }
    }
    return result;
}


// solve() の下あたりに追加
int Solver::eval(const MiniGo1xN& game) {
    return alpha_beta(game, -2, 2);
}
