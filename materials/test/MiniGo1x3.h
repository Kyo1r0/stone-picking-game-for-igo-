#ifndef MINIGO1X3_H
#define MINIGO1X3_H

#include <vector>
#include <string>
#include <utility>

class MiniGo1x3 {
public:
    std::vector<int> board;
    int player_to_move;

    MiniGo1x3(const std::vector<int>& board_, int player_);

    // 合法手を返す（自殺手禁止）
    std::vector<int> get_legal_moves() const;

    // 指定の手を打って新しい盤面を返す。捕獲発生のboolも返す
    std::pair<MiniGo1x3, bool> make_move(int move) const;

    // 盤面キーを作成（メモ化用）
    std::string make_key(int player) const;

private:
    bool will_capture(int pos) const;
};

#endif
