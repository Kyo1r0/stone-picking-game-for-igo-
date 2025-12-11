import pandas as pd
import sys

# --- ゲームルール処理 (石を取った判定用) ---
class MiniGoLogic:
    def __init__(self, size):
        self.size = size

    # 呼吸点（自由度）を数える
    def count_liberties(self, board, pos, color):
        if board[pos] != color: return 0
        left, right = pos, pos
        while left > 0 and board[left - 1] == color: left -= 1
        while right < self.size - 1 and board[right + 1] == color: right += 1
        
        liberties = 0
        if left > 0 and board[left - 1] == 0: liberties += 1
        if right < self.size - 1 and board[right + 1] == 0: liberties += 1
        return liberties

    # 石のグループを削除する
    def remove_group(self, board, pos):
        color = board[pos]
        if color == 0: return
        left, right = pos, pos
        while left > 0 and board[left - 1] == color: left -= 1
        while right < self.size - 1 and board[right + 1] == color: right += 1
        for i in range(left, right + 1): board[i] = 0

    # 一手進める（戻り値：次の盤面, 捕獲が発生したか）
    def make_move(self, board, move, player):
        new_board = list(board)
        new_board[move] = player
        captured = False
        opponent = -player
        
        # 左右の敵石を確認
        for adj in [move - 1, move + 1]:
            if 0 <= adj < self.size and new_board[adj] == opponent:
                # 敵の呼吸点が0ならグループごと削除（捕獲）
                if self.count_liberties(new_board, adj, opponent) == 0:
                    self.remove_group(new_board, adj)
                    captured = True
                    
        return tuple(new_board), captured

# --- メイン処理 ---
def main():
    print("=== MiniGo 1xN Navigator (Rule Enforced + Undo) ===")
    try:
        n_input = input("Board Size N: ")
        if not n_input: return
        n = int(n_input)
    except ValueError:
        print("Invalid number.")
        return

    csv_file = f"game_map_1x{n}.csv"
    print(f"Loading {csv_file} ...")
    
    try:
        # 文字列として読み込む
        df = pd.read_csv(csv_file, dtype=str)
    except Exception as e:
        print("CSV not found. Please run the C++ solver first.")
        return

    # 辞書化
    lookup = {}
    for _, row in df.iterrows():
        raw_s = str(row['RawBoard']).replace('"', '').strip()
        hint_s = str(row['HintBoard']).replace('"', '').strip()
        try:
            b_list = list(map(int, raw_s.split(',')))
            key = (tuple(b_list), int(row['Player']))
            lookup[key] = (hint_s, int(row['Winner']))
        except ValueError:
            continue
            
    print(f"Loaded {len(lookup)} states.")

    # ゲーム初期化
    game = MiniGoLogic(n)
    current_board = tuple([0] * n)
    current_player = 1 # 1:Black, -1:White
    
    # ★追加: 履歴保存用リスト
    history = []

    while True:
        p_name = "Black(●)" if current_player == 1 else "White(○)"
        
        # --- 1. CSV検索 (正規化対応) ---
        hints = []
        key_direct = (current_board, current_player)
        
        if key_direct in lookup:
            hint_str, _ = lookup[key_direct]
            hints = hint_str.split(',')
        else:
            # 反転して検索
            rev_board = current_board[::-1]
            key_rev = (rev_board, current_player)
            if key_rev in lookup:
                hint_str, _ = lookup[key_rev]
                hints = hint_str.split(',')[::-1] # ヒントも反転
            else:
                hints = ["?"] * n

        # --- 2. 画面描画 ---
        print("\n" + "="*40)
        display_board = []
        status_line = []
        
        for i in range(n):
            cell = current_board[i]
            hint = hints[i] if i < len(hints) else "?"

            if cell == 1: display_board.append("●")
            elif cell == -1: display_board.append("○")
            else: display_board.append(".")

            if hint == 'g': status_line.append(" \033[92m[WIN]\033[0m ")
            elif hint == 'r': status_line.append(" \033[91m[LOS]\033[0m ")
            elif hint == 'y': status_line.append(" [DRW] ")
            elif hint == 'x': status_line.append(" [ILG] ")
            else: status_line.append(" [ - ] ")

        print(f"Current: {' '.join(display_board)}   Turn: {p_name}")
        print("-" * 40)
        
        # --- 3. 終了条件チェック (A): 打つ手がない ---
        can_move = any(h in ['g', 'r', 'y'] for h in hints)
        
        if not can_move:
            print(f"No legal moves available! {p_name} LOSES.")
            # 負けた場合でも、戻りたくなるかもしれないので break せずに入力を待つ手もあるが、
            # ここではシンプルに終了とする（戻りたければ再起動）
            print("=== GAME OVER ===")
            break

        print("Moves: " + "".join(status_line))
        print("Index: " + "".join([f"  {i}    " for i in range(n)]))

        # --- 4. 入力 ---
        move_str = input("\nYour move (0-N, 'b' to back, 'q' to quit): ")
        
        # 終了
        if move_str.lower() == 'q': 
            break
            
        # ★追加: 戻る処理
        if move_str.lower() == 'b':
            if history:
                current_board, current_player = history.pop()
                print("<< Undo one step.")
                continue
            else:
                print("Cannot undo (Start of game).")
                continue
        
        try:
            move = int(move_str)
            if 0 <= move < n:
                hint = hints[move]
                
                # 自殺手(x)や埋まっている場所(-)には打てない
                if hint == 'x' or (hint not in ['g','r','y'] and hint != '?'):
                    print("Invalid move! (Suicide or Occupied)")
                    continue
                
                # --- 5. 手を打って判定 ---
                # ★追加: 手を打つ前に履歴に保存
                history.append((current_board, current_player))
                
                new_board, captured = game.make_move(current_board, move, current_player)
                
                # --- 6. 終了条件チェック (B): 石を取った ---
                if captured:
                    # 盤面更新後の表示
                    final_disp = ["●" if c==1 else "○" if c==-1 else "." for c in new_board]
                    print("\n" + "="*40)
                    print(f"Final:   {' '.join(final_disp)}")
                    print(f"!!! Stone Captured! {p_name} WINS! !!!")
                    print("=== GAME OVER ===")
                    break
                
                # ゲーム続行
                current_board = new_board
                current_player = -current_player
                
            else:
                print("Out of range.")
        except ValueError:
            print("Please enter a number.")

if __name__ == "__main__":
    main()