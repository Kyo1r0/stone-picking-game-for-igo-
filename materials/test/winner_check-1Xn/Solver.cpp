#include "Solver.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

void Solver::solve(const std::vector<int>& initial_board, int initial_player) {
    //テーブルを初期化
    init_zobrist(static_cast<int>(initial_board.size()));
    MiniGo1xN game(initial_board, initial_player);
    _find_value(game);
}


// Zobrist 初期化
void Solver::init_zobrist(int board_size) {
    zobrist_table.assign(board_size, std::vector<HashKey>(3)); // 0:空,1:黒,2:白

    std::random_device rd;
    std::mt19937_64 rng(rd());
    std::uniform_int_distribution<HashKey> dist;

    for (int i = 0; i < board_size; ++i) {
        for (int j = 0; j < 3; ++j) {
            zobrist_table[i][j] = dist(rng);
        }
    }

    zobrist_player[0] = dist(rng); // 黒番
    zobrist_player[1] = dist(rng); // 白番
}

// 石の値 → インデックスに変換
static int piece_index(int v) {
    if (v == 0) return 0;   // 空
    if (v == 1) return 1;   // 黒
    return 2;               // 白(-1)
}

std::vector<int> Solver::canonicalize_board(const std::vector<int>& board) const {
    std::vector<int> rev(board.rbegin(), board.rend());
    // 辞書順比較（vector<int> の < 演算子は辞書順）
    if (rev < board) {
        return rev;  // 反転した方が小さい → これを採用
    }
    return board;     // そのままの方が小さい
}



// ハッシュ計算
HashKey Solver::compute_hash(const std::vector<int>& original, int player) const {

    std::vector<int> board = canonicalize_board(original);

    HashKey h = 0;
    int n = static_cast<int>(board.size()); //盤面のサイズを安全に取得
    for (int i = 0; i < n; ++i) {
        int idx = piece_index(board[i]);
        h ^= zobrist_table[i][idx];
    }
    int pidx = (player == 1 ? 0 : 1);
    h ^= zobrist_player[pidx];
    return h;
}



std::string Solver::make_key(const std::vector<int>& original, int player) const {
    auto board = canonicalize_board(original);
    std::string s;
    s.reserve(board.size() + 3);
    s += (player == 1 ? '1' : '2');  // 手番を表す
    s += ':';
    for (int x : board) {
        if (x == 1) s += '1';
        else if (x == -1) s += '-';
        else s += '0';
    }
    return s;
}






// (ヘルパー1) ノードの作成
GameNode* Solver::_create_new_node(HashKey key, const MiniGo1xN& game) {
    auto node = std::make_unique<GameNode>(game.board, game.player);

    // 表示用IDは従来の string のまま
    node->id = make_key(game.board, game.player);

    GameNode* node_ptr = node.get();
    nodes[key] = std::move(node);
    return node_ptr;
}



// (ヘルパー2) 終局ノードの設定 
void Solver::_setup_terminal_node(GameNode* node, int winner, const std::string& reason) {
    node->winner = winner;
    node->game_value = reason;
    node->outcome_class = "UNKNOWN"; 
    node->is_optimal = false;       // 終局なので
}

// (ヘルパー3) 探索と評価
void Solver::_explore_children_and_evaluate(GameNode* node_ptr,
                                            const MiniGo1xN& game,
                                            const std::vector<int>& moves) {
    std::vector<GameNode*> child_nodes;

    for (int m : moves) {
        auto [next_game, captured] = game.make_move(m);

        // 表示用ID
        std::string child_id_str = make_key(next_game.board, next_game.player);
        node_ptr->children[child_id_str] = std::to_string(m);

        GameNode* child_node = nullptr;

        if (captured) {
            // 捕獲 = 終局ノード
            HashKey ckey = compute_hash(next_game.board, next_game.player);
            if (!nodes.count(ckey)) {
                child_node = _create_new_node(ckey, next_game);
                _setup_terminal_node(child_node, game.player, "Lost (Captured)");
            } else {
                child_node = nodes.at(ckey).get();
            }
        } else {
            // 再帰
            child_node = _find_value(next_game);
        }
        child_nodes.push_back(child_node);
    }

    search_winner_Minimax(node_ptr, child_nodes, game.player);
}


// (ヘルパー4)勝敗判定 (Minimax法) すべての子ノードの結果を見て現在のノードの勝敗を決定する
void Solver::search_winner_Minimax(GameNode* node_ptr, const std::vector<GameNode*>& child_nodes, int current_player){ 
    bool found_win = false;
    for (GameNode* child : child_nodes) {
        if (child->winner == current_player) { 
            found_win = true;
        }
    }

    if (found_win) {
        node_ptr->winner = current_player;
        node_ptr->outcome_class = "Win"; 
        node_ptr->game_value = "UNKNOWN";
    } else {
        node_ptr->winner = -current_player;
    }

    // 細かいところは"UNKNOWN"にする 
    node_ptr->game_value = "UNKNOWN";
    node_ptr->outcome_class = "UNKNOWN";
    node_ptr->is_optimal = false;
}


//メインで指揮をとっているだけ
GameNode* Solver::_find_value(const MiniGo1xN& game) {
    // 1. Zobrist ハッシュでメモ化チェック
    HashKey key = compute_hash(game.board, game.player);
    if (nodes.count(key)) {
        return nodes.at(key).get();
    }

    // 2. ノードの作成
    GameNode* node_ptr = _create_new_node(key, game);

    // 3. 終局かどうか
    auto moves = game.get_legal_moves();
    if (moves.empty()) {
        _setup_terminal_node(node_ptr, -(game.player), "Loss (No Moves)");
    } else {
        _explore_children_and_evaluate(node_ptr, game, moves);
    }
    return node_ptr;
}


void Solver::print_all_nodes() const {
    for (const auto& [key, node] : nodes) { //nodesがsolver.hで定義済み
        std::cout << "Node key: " << key << "\n  Board: ";
        for (int v : node->board_state) std::cout << v << " ";
        std::cout << "\n  Player: " << (node->player_to_move==1?"Black":"White")
                  << ", Value: " << node->game_value
                  << ", Outcome: " << node->outcome_class
                  << ", Optimal: " << (node->is_optimal?"Yes":"No") << "\n";
        std::cout << "  Children: ";
        for (const auto& [child_id, move_str] : node->children)
            std::cout << move_str << "(" << child_id << ") ";

        if (node->children.empty()) {
            std::cout << "\n  Winner: " << node->winner;
        }
        std::cout << "\n";
    }
}


// 引数に合わせて修正。固定の {0,0,0} ではなく、引数からキーを作る
int Solver::get_initial_winner(const std::vector<int>& board, int player) const {
    HashKey root_key = compute_hash(board, player);
    auto it = nodes.find(root_key);
    if (it != nodes.end()) {
        return it->second->winner;
    }
    return 0;
}



void Solver::print_minimax_summary() const {
    std::cout << "---  Minimax Summary ---" << "\n";
    std::cout << "(Key: [Player] -> Winner: W, Optimal: O, Children: [...])\n\n";

    for (const auto& [key, node] : nodes) {
        std::cout << key << ": "
                  << "[" << (node->player_to_move == 1 ? "Black" : "White") << "] -> ";

        std::cout << "W: " << node->winner
                  << ", O: " << (node->is_optimal ? "Yes" : "No") << "\n";

        std::cout << "    Children: ";
        if (node->children.empty()) {
            std::cout << "(Terminal)";
        } else {
            for (const auto& [child_id, move_str] : node->children) {
                
                std::cout << child_id << " "; 
            }
        }
        std::cout << "\n----------------\n";
    }
}
//void Solver::print_optinal_sort() const {}


void Solver::export_heatmap_csv(const std::vector<int>& board, int player, 
                                const std::string& filename)const {
    MiniGo1xN game(board, player);
    auto moves = game.get_legal_moves();

    std::ofstream ofs(filename);
    ofs << "move,color,result_list\n";

    for (int mv : moves) {
        auto [next_game, captured] = game.make_move(mv);
        HashKey child_hash = compute_hash(next_game.board, next_game.player);

        std::vector<int> results;

        // 子ノード探索
        for (auto& [key, node_ptr] : nodes) {
            if (key == child_hash) {
                // 子ノードを探索
                for (auto& child : node_ptr->children) {
                    HashKey ck = std::stoull(child.first);
                    auto it = nodes.find(ck);
                    if (it != nodes.end()) {
                        results.push_back(it->second->winner);
                    }
                }
                break;
            }
        }

        // 分類
        std::string color;
        if (!results.empty()) {
            bool all_win  = std::all_of(results.begin(), results.end(),
                                        [&](int w){ return w == player; });
            bool all_lose = std::all_of(results.begin(), results.end(),
                                        [&](int w){ return w == -player; });

            if (all_win)      color = "green";
            else if (all_lose) color = "red";
            else               color = "yellow";
        } else {
            color = "unknown";
        }

        ofs << mv << "," << color << ",[";
        for (size_t i = 0; i < results.size(); ++i) {
            ofs << results[i];
            if (i + 1 < results.size()) ofs << " ";
        }
        ofs << "]\n";
    }
}