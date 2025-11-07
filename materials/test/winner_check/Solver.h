#pragma once
#include "MiniGo1x3.h"
#include <string>
#include <map>
#include <memory>
#include <vector>

struct GameNode {
    std::string id;                     // ノードID
    std::vector<int> board_state;       // 盤面
    int player_to_move;                 // 手番
    std::map<std::string, std::string> children; // child_id -> move_str
    std::string game_value;             // ゲーム値 ("P","N","L","R" など)
    std::string outcome_class;          // 勝敗分類
    bool is_optimal;                    // 最善手フラグ
    int winner = 0;                     // 勝利判定0=勝敗未定, 1=黒勝ち, -1=白勝ち

    GameNode(const std::vector<int>& board, int player)
        : board_state(board), player_to_move(player), game_value(""), outcome_class(""), is_optimal(false) {}
};

class Solver {
public:
    Solver() {}

    // 初期盤面・初期手番から解析を開始
    void solve(const std::vector<int>& initial_board, int initial_player);

    // メモ化マップに格納されたノード数を取得
    size_t num_nodes() const { return nodes.size(); }

    // デバッグ用：全ノード出力e
    void print_all_nodes() const;

    // デバッグ用:Minimaxの要約出力
    void print_minimax_summary() const;

    // デバック用:最善手を出力する
    void print_optinal_sort() const;

    //勝者を判定する(Max,min法)
    int get_initial_winner() const;

    
    
private:
    std::map<std::string, std::unique_ptr<GameNode>> nodes; //ゲーム木をマップで管理


    // 盤面と手番からノードキーを生成
    std::string make_key(const std::vector<int>& board, int player) const;


    // (メイン) 再帰探索の「振り分け」を行う
    GameNode* _find_value(const MiniGo1x3& game);

    // (ヘルパー1) 新しいノードを作成し、マップに登録する
    GameNode* _create_new_node(const std::string& key, const MiniGo1x3& game);

    // (ヘルパー2) ノードを終局として設定する
    void _setup_terminal_node(GameNode* node, int winner, const std::string& reason);

    // (ヘルパー3) 子ノードを探索し、結果を評価する
    void _explore_children_and_evaluate(GameNode* node_ptr, const MiniGo1x3& game, const std::vector<int>& moves);

    // (ヘルパー4) 子ノードリストから親の勝敗を決定する (前回作成)
    void search_winner_Minimax(GameNode* node_ptr, const std::vector<GameNode*>& child_nodes, int current_player);
};
