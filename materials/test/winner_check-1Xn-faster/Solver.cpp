#include "Solver.h"
#include <algorithm>

Solver::Solver() {
    init_zobrist(100); // 最大 N=100 まで対応
}

void Solver::init_zobrist(int max_n) {
    zobrist.assign(max_n, std::vector<HashKey>(3));
    std::mt19937_64 rng(123456);
    for (int i = 0; i < max_n; ++i)
        for (int j = 0; j < 3; ++j)
            zobrist[i][j] = rng();

    zobrist_player[0] = rng(); // 黒
    zobrist_player[1] = rng(); // 白
}

std::vector<int> Solver::canonicalize(const std::vector<int>& board) const {
    std::vector<int> rev(board.rbegin(), board.rend());
    return std::min(board, rev);
}

HashKey Solver::compute_hash(const std::vector<int>& board, int player) const {
    auto b = canonicalize(board);
    HashKey h = 0;
    for (int i = 0; i < (int)b.size(); ++i) {
        int idx = (b[i] == 0 ? 0 : (b[i] == 1 ? 1 : 2));
        h ^= zobrist[i][idx];
    }
    h ^= zobrist_player[player == 1 ? 0 : 1];
    return h;
}

int Solver::solve(const std::vector<int>& board, int player) {
    node_count = 0;
    table.clear();
    MiniGo1xN game(board, player);
    return alpha_beta(game, -1, 1);
}

int Solver::alpha_beta(const MiniGo1xN& game, int alpha, int beta) {
    node_count++;

    HashKey key = compute_hash(game.board, game.player);
    if (table.count(key)) return table[key];

    auto moves = game.get_legal_moves();
    if (moves.empty()) {
        return table[key] = -1; // 手なし負け
    }

    for (int m : moves) {
        auto [next, captured] = game.make_move(m);

        // 即勝利
        if (captured) {
            return table[key] = 1;
        }

        int score = -alpha_beta(next, -beta, -alpha);

        if (score >= beta) {
            return table[key] = score; // βカット
        }
        alpha = std::max(alpha, score);
    }

    return table[key] = alpha;
}


std::string Solver::analyze_initial_moves(int n) {
    std::vector<int> board(n, 0);
    MiniGo1xN game(board, 1);
    std::string res;

    for (int i = 0; i < n; ++i) {
        // 合法手でない
        auto moves = game.get_legal_moves();
        if (std::find(moves.begin(), moves.end(), i) == moves.end()) {
            res += "x";
            continue;
        }

        auto [next, captured] = game.make_move(i);

        if (captured) {
            res += "g";
            continue;
        }

        int r = solve(next.board, next.player);
        res += (r == -1 ? "g" : "r");
    }
    return res; // ← "rgrrg" のような文字列
}

