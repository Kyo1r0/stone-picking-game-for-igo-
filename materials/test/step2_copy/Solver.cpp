#include "Solver.h"
#include <iostream>
#include <sstream>
#include <set>

void Solver::solve(const std::vector<int>& initial_board, int initial_player) {
    MiniGo1x3 game(initial_board, initial_player);
    _find_value(game);
}

std::string Solver::make_key(const std::vector<int>& board, int player) const {
    std::ostringstream oss;
    oss << player << ":";
    for (int v : board) oss << v;
    return oss.str();
}

GameNode* Solver::_find_value(const MiniGo1x3& game) {
    std::string key = make_key(game.board, game.player);
    if (nodes.count(key)) return nodes[key].get();

    auto node = std::make_unique<GameNode>(game.board, game.player);
    GameNode* node_ptr = node.get();
    nodes[key] = std::move(node);

    auto moves = game.get_legal_moves();
    if (moves.empty()) {
        // 終局
        node_ptr->game_value = "UNKNOWN";  // 合法手なしで負け
        node_ptr->outcome_class = "P";
        node_ptr->is_optimal = false;
        node_ptr->children["Lose"] = "";
        return node_ptr;
    }


    //再帰呼び出し
    std::vector<GameNode*> children_nodes; //子ノード
    std::set<std::string> left_options_values; // 左選択肢（現プレイヤーの着手先）の値
    bool immediate_win = false; // この手番で即勝利できるか

    for (int m : moves) {
        auto [next_game, captured] = game.make_move(m);


            // 勝利条件チェック(m=1) 
        if (captured) {
            immediate_win = true; //判定するために設けた
             node_ptr->children[std::to_string(m)] = "Win-> "; // Move -> "Win"
             if (!node_ptr->is_optimal) { // まだ optimal な手が見つかっていない場合
                node_ptr->is_optimal = true; // このノードから勝利できることを示す
             }
             break;// 勝敗判定を優先する.ここでループを抜ける(本当は全探索が必要) 
        }
        GameNode* child_node = _find_value(next_game);

        node_ptr->children[make_key(next_game.board, next_game.player)] = std::to_string(m);
        children_nodes.push_back(child_node);
        left_options_values.insert(child_node->game_value); // ゲーム値計算用に収集
        node_ptr->children[std::to_string(m)] = child_node->id; // children マップを更新 (Move -> Child ID)
    }



    // Game Valueの判定
    if (immediate_win) {// 即勝利できる手があった場合
        node_ptr->outcome_class = (game.player == 1) ? "L" : "R"; 
        node_ptr->game_value = (game.player == 1) ? "1" : "-1"; 
        
    } else { //子ノードから判定
        bool can_force_win = false; // 相手をP局面にできる手があるか
        bool can_avoid_loss = false; // 相手がP局面に行けない手があるか（N局面に行けるか）
        std::string optimal_child_id_leading_to_opponent_P = "";

        for (GameNode* child : children_nodes) {
            // 相手の手番である子ノード child で、相手が負ける（＝自分が勝つ）のは？
            // 1. child が P局面 (相手が後手番で負け)
            // 2. child が R局面 (相手(White)が右番で負け) ← 現プレイヤーが黒(1)の場合
            // 3. child が L局面 (相手(Black)が左番で負け) ← 現プレイヤーが白(-1)の場合
            bool opponent_loses_from_child = (child->outcome_class == "P") ||
                                             (game.player == 1 && child->outcome_class == "R") ||
                                             (game.player == -1 && child->outcome_class == "L");

            if (opponent_loses_from_child) {
                can_force_win = true;
                 // 最初に P or 相手負け局面 に行ける子を記録
                if (optimal_child_id_leading_to_opponent_P.empty()) {
                   optimal_child_id_leading_to_opponent_P = child->id;
                }
                // break; // ★注意★ CGTの値を正確に計算するには全子を見る必要あり
            }
            // 相手の手番である子ノード child で、相手が勝つのは？
            // 1. child が N局面 (相手が先手番で勝ち)
            // 2. child が L局面 (相手(Black)が左番で勝ち) ← 現プレイヤーが黒(1)の場合
            // 3. child が R局面 (相手(White)が右番で勝ち) ← 現プレイヤーが白(-1)の場合
             bool opponent_wins_from_child = (child->outcome_class == "N") ||
                                              (game.player == 1 && child->outcome_class == "L") ||
                                              (game.player == -1 && child->outcome_class == "R");

            if (opponent_wins_from_child) {
                // 少なくとも1手、相手が勝つ局面に遷移できる手がある
                // → 自分が必ずしも負けるわけではない
            } else {
                // 相手が勝てない（自分が勝つか引き分け）局面に遷移できる手がある
                can_avoid_loss = true;
            }
        }
        
        

        // --- Outcome Class(帰結類) 判定 ---
        if (can_force_win) {
            node_ptr->outcome_class = (game.player == 1) ? "L" : "R"; // 現プレイヤーの勝ち
            node_ptr->is_optimal = true; // このノード自体が最適経路の一部になりうる
            // 最善手となる子ノードへの辺に印をつける処理が必要だが、
            // is_optimalをノードに持たせているので、ここではノード自体にマーク
            // → 可視化側で、L/RノードからP/相手負けR/相手負けLに繋がる辺を太くする方が良いかも
        } else if (can_avoid_loss) {
            // 勝ちを強制できないが、負けもしない -> 次の手番（相手）が勝つ
            node_ptr->outcome_class = "N";
            node_ptr->is_optimal = false; // 相手が勝つので最適ではない
        } else {
            // どうやっても相手が勝つ局面（P局面または相手のL/R局面）にしか行けない
            node_ptr->outcome_class = "P"; // 現プレイヤーの負け
            node_ptr->is_optimal = false;
        }

        // --- Game Value 計算 (簡易版) ここでは Outcome Class をそのまま使う
        if (node_ptr->outcome_class == "L") node_ptr->game_value = "1"; // 正の値の代表(整数)
        else if (node_ptr->outcome_class == "R") node_ptr->game_value = "-1"; // 負の値の代表(整数)
        else if (node_ptr->outcome_class == "P") node_ptr->game_value = "0"; // P局面は 0
        else node_ptr->game_value = "UNKNOWN"; // 他のはいったん保留


    }


    


    // 勝敗・最善手判定（1×3用の簡単ルール）
    // 捕獲が起こる手があれば、自分勝利（"P"）
    /* 後に実装
    bool found_win = false;
    for (GameNode* child : child_nodes) {
        if (child->outcome_class == "L") { // 相手が負ける手がある → 自分勝ち
            found_win = true;
            node_ptr->is_optimal = true; // 最善手
            break;
        }
    }
    node_ptr->game_value = found_win ? "P" : "N";
    node_ptr->outcome_class = found_win ? "P" : "N";
    */
 
    //とりあえず,UNKNOWNで対応
    /*
    node_ptr->game_value = "UNKNOWN";
    node_ptr->outcome_class = "UNKNOWN";
    node_ptr->is_optimal = false; // 最善手かどうかも不明なためfalseに設定
    */

    return node_ptr;
}


void Solver::print_all_nodes() const {
    for (const auto& [key, node] : nodes) {
        std::cout << "Node key: " << key << "\n  Board: ";
        for (int v : node->board_state) std::cout << v << " ";
        std::cout << "\n  Player: " << (node->player_to_move==1?"Black":"White")
                  << ", Value: " << node->game_value
                  << ", Outcome: " << node->outcome_class
                  << ", Optimal: " << (node->is_optimal?"Yes":"No") << "\n";
        std::cout << "  Children: ";
        for (const auto& [child_id, move_str] : node->children)
            std::cout << move_str << "(" << child_id << ") ";
        std::cout << "\n";
        // std::flushをするといいかも
        //1*3で結論をなにか出すといい
    }
}
