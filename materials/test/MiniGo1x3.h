#pragma once
#include <vector>
#include <string>
#include <utility>

class MiniGo1x3 {
public:
    std::vector<int> board; // 盤面: 1=黒, -1=白, 0=空
    int player;             // 手番: 1=黒, -1=白

    MiniGo1x3(const std::vector<int>& b, int p);

    // 合法手を取得
    std::vector<int> get_legal_moves() const;

    // 指定手を打って新盤面を返す。捕獲が発生したかも返す
    std::pair<MiniGo1x3, bool> make_move(int move) const;

    // 盤面＋手番からキー文字列を生成
    std::string make_key(int player) const;

private:
    // 内部補助: 指定マスの呼吸点を計算
    int get_liberty(int index, int color) const;
};
