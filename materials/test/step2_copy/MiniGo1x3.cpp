#include "MiniGo1x3.h"
#include <iostream>

// 初期盤面用コンストラクタ
MiniGo1x3::MiniGo1x3(const std::vector<int>& b, int p)
    : board(b), player(p) {
    // previous_board は自動的に空のベクターで初期化される
}

// --- ▼▼▼ 変更点 3: 新しいコンストラクタの実装を追加 ▼▼▼ ---
// 一手前の盤面も引き継ぐコンストラクタ
MiniGo1x3::MiniGo1x3(const std::vector<int>& b, int p, const std::vector<int>& prev_b)
    : board(b), player(p), previous_board(prev_b) {}

// 指定マスの呼吸点を数える (変更なし)
int MiniGo1x3::count_liberties(int pos, int color, const std::vector<int>& b) const {
    int liberties = 0;
    if (pos > 0 && b[pos - 1] == 0) liberties++;
    if (pos < 2 && b[pos + 1] == 0) liberties++;
    return liberties;
}

// 捕獲判定（隣の相手石の呼吸点が0になるか） (変更なし)
bool MiniGo1x3::is_capture(int pos, int current_player) const {
    bool captured = false;
    int opponent = -current_player;
    std::vector<int> tmp = board;

    tmp[pos] = current_player;
    for (int adj : {pos - 1, pos + 1}) {
        if (adj >= 0 && adj <= 2 && tmp[adj] == opponent) {
            if (count_liberties(adj, opponent, tmp) == 0) {
                captured = true;
            }
        }
    }
    return captured;
}

// 自分の石が置かれた結果呼吸点0になるか (変更なし)
bool MiniGo1x3::would_be_suicide(int pos) const {
    std::vector<int> tmp = board;
    tmp[pos] = player;
    if (is_capture(pos, player)) return false;
    return count_liberties(pos, player, tmp) == 0;
}


// --- ▼▼▼ 変更点 4: get_legal_moves にコウ判定を追加 ▼▼▼ ---
// 合法手を返す
std::vector<int> MiniGo1x3::get_legal_moves() const {
    std::vector<int> moves;
    for (int i = 0; i < 3; i++) {
        // 1. その場所が空きマスか？
        if (board[i] != 0) {
            continue;
        }

        // 2. 自殺手ではないか？
        if (would_be_suicide(i)) {
            continue;
        }

        // 3. コウのルールを破らないか？
        // まず、その手を打った後の盤面を仮に生成する
        auto [next_game_state, captured] = make_move(i);
        
        // 生成された盤面が、一手前の盤面と同じ場合はコウなので違法手
        if (!previous_board.empty() && next_game_state.board == previous_board) {
            continue;
        }

        // 全てのチェックを通過したら合法手
        moves.push_back(i);
    }
    return moves;
}

// --- ▼▼▼ 変更点 5: make_move で一手前の盤面を引き継ぐ ▼▼▼ ---
// 着手と捕獲処理
std::pair<MiniGo1x3, bool> MiniGo1x3::make_move(int move) const {
    std::vector<int> new_board = board;
    new_board[move] = player;
    bool captured = false;

    int opponent = -player;
    for (int adj : {move - 1, move + 1}) {
        if (adj >= 0 && adj <= 2 && new_board[adj] == opponent) {
            if (count_liberties(adj, opponent, new_board) == 0) {
                new_board[adj] = 0;
                captured = true;
            }
        }
    }

    // 新しい状態を生成する際に、現在の盤面(this->board)を「一手前の盤面」として渡す
    MiniGo1x3 next(new_board, -player, this->board);
    return {next, captured};
}