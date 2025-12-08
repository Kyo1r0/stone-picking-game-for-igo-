import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import seaborn as sns
import sys

# 再帰制限を少し増やしておく（念のため）
sys.setrecursionlimit(5000)

class MiniGo1xN:
    def __init__(self, size):
        self.size = size
        # 0: Empty, 1: Black, -1: White
        self.board = tuple([0] * size) 
        self.memo = {}
        # 現在探索中のパスを記録するセット（ループ検出用）
        self.recursion_stack = set()

    def get_canonical(self, board):
        # 左右対称形の同一視
        return min(board, board[::-1])

    def count_liberties(self, board, pos, color):
        if board[pos] != color:
            return 0
        
        # グループの範囲を探索
        left = pos
        while left > 0 and board[left - 1] == color:
            left -= 1
        
        right = pos
        while right < self.size - 1 and board[right + 1] == color:
            right += 1
            
        liberties = 0
        if left > 0 and board[left - 1] == 0:
            liberties += 1
        if right < self.size - 1 and board[right + 1] == 0:
            liberties += 1
            
        return liberties

    def remove_group(self, board_list, pos):
        color = board_list[pos]
        if color == 0: return
        
        left = pos
        while left > 0 and board_list[left - 1] == color:
            left -= 1
        
        right = pos
        while right < self.size - 1 and board_list[right + 1] == color:
            right += 1
            
        for i in range(left, right + 1):
            board_list[i] = 0

    def make_move(self, board, move, player):
        new_board = list(board)
        new_board[move] = player
        captured = False
        opponent = -player
        
        # 左右の隣接石の呼吸点を確認
        for adj in [move - 1, move + 1]:
            if 0 <= adj < self.size and new_board[adj] == opponent:
                if self.count_liberties(new_board, adj, opponent) == 0:
                    self.remove_group(new_board, adj)
                    captured = True
                    
        return tuple(new_board), captured

    def is_suicide(self, board, move, player):
        # 仮に置いてみる
        tmp_board, captured = self.make_move(board, move, player)
        # 相手石を取れるなら自殺手ではない
        if captured:
            return False
        # 置いた石の呼吸点が0なら自殺手
        if self.count_liberties(tmp_board, move, player) == 0:
            return True
        return False

    def solve(self, board, player):
        # 局面のキー生成（盤面の正規化 + 手番）
        canonical_board = self.get_canonical(board)
        key = (canonical_board, player)

        # 1. メモ化済みならそれを返す
        if key in self.memo:
            return self.memo[key]

        # 2. ループ検出（現在探索中のパスにこの局面が含まれていればループ）
        if key in self.recursion_stack:
            return 0 # ループは引き分け扱い

        self.recursion_stack.add(key) # スタックに積む

        # 3. 合法手の生成
        legal_moves = []
        for i in range(self.size):
            if board[i] == 0:
                if not self.is_suicide(board, i, player):
                    legal_moves.append(i)
        
        res = 0
        if not legal_moves:
            res = -player # 打てる手がない場合は負け
        else:
            # Minimax探索
            # 自分の手番で、勝てる手が一つでもあるか探す
            can_win = False
            can_draw = False
            
            for m in legal_moves:
                next_board, _ = self.make_move(board, m, player)
                # 相手番の結果を取得
                winner = self.solve(next_board, -player)
                
                if winner == player:
                    can_win = True
                    break # 勝ち確定
                elif winner == 0:
                    can_draw = True
            
            if can_win:
                res = player
            elif can_draw:
                res = 0 # 最善を尽くしても引き分け（ループ）
            else:
                res = -player # どう打っても負け

        # 4. 結果の保存とスタックからの削除
        self.recursion_stack.remove(key) # スタックから降ろす
        self.memo[key] = res
        return res

# --- Generate Heatmap Data ---

def analyze_first_move(size):
    game = MiniGo1xN(size)
    initial_board = tuple([0] * size)
    player = 1 # Black First
    
    results = []
    
    print(f"Analyzing 1x{size}...")
    
    for i in range(size):
        # 自殺手チェック
        if game.is_suicide(initial_board, i, player):
            results.append({"Move": i, "Outcome": "Illegal", "Value": 0})
            continue
            
        # 1手進める
        next_board, _ = game.make_move(initial_board, i, player)
        
        # 解析（ループ検出付き）
        winner = game.solve(next_board, -player)
        
        if winner == 1:
            outcome_str = "Win"
        elif winner == -1:
            outcome_str = "Loss"
        else:
            outcome_str = "Draw" # Loop
            
        results.append({"Move": i, "Outcome": outcome_str, "Value": winner})
        
    return pd.DataFrame(results)

# --- Visualization ---

def plot_heatmap(df, size, filename):
    # ヒートマップ用のデータ変形 (1行N列)
    grid = np.zeros((1, size))
    labels = np.empty((1, size), dtype=object)
    
    for _, row in df.iterrows():
        idx = int(row['Move'])
        val = row['Value']
        grid[0, idx] = val
        
        if val == 1:
            labels[0, idx] = "WIN"
        elif val == -1:
            labels[0, idx] = "LOSE"
        else:
            labels[0, idx] = "DRAW"

    plt.figure(figsize=(size * 1.5, 3))
    
    # -1(赤:負け) -> 0(灰:引き分け) -> 1(緑:勝ち)
    from matplotlib.colors import ListedColormap
    # 値に応じて色を割り当て: -1, 0, 1
    cmap = ListedColormap(['#ff6b6b', '#d3d3d3', '#6bff96']) 
    
    ax = sns.heatmap(grid, annot=labels, fmt="", cmap=cmap, 
                     cbar=False, linewidths=1, linecolor='black',
                     vmin=-1, vmax=1, square=True)
    
    ax.set_title(f"1x{size} First Move Analysis (Black's Turn)")
    ax.set_xticks(np.arange(size) + 0.5)
    ax.set_xticklabels(np.arange(size))
    ax.set_yticks([])
    plt.xlabel("Board Position Index")
    
    plt.savefig(filename)
    plt.close()
    print(f"Saved {filename}")

# --- Main Execution ---

if __name__ == "__main__":
    # Analyze 1x5
    df_1x5 = analyze_first_move(5)
    plot_heatmap(df_1x5, 5, "heatmap_1x5.png")

    # Analyze 1x6
    df_1x6 = analyze_first_move(6)
    plot_heatmap(df_1x6, 6, "heatmap_1x6.png")

    # CSV Output
    df_1x5['BoardSize'] = '1x5'
    df_1x6['BoardSize'] = '1x6'
    df_combined = pd.concat([df_1x5, df_1x6])
    df_combined.to_csv("minimax_analysis_fixed.csv", index=False)
    print("All done.")