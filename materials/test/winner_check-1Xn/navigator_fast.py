import pandas as pd
import sys
import pickle
import os

# --- ゲームルール処理 ---
class MiniGoLogic:
    def __init__(self, size):
        self.size = size

    def count_liberties(self, board, pos, color):
        if board[pos] != color: return 0
        left, right = pos, pos
        while left > 0 and board[left - 1] == color: left -= 1
        while right < self.size - 1 and board[right + 1] == color: right += 1
        
        liberties = 0
        if left > 0 and board[left - 1] == 0: liberties += 1
        if right < self.size - 1 and board[right + 1] == 0: liberties += 1
        return liberties

    def remove_group(self, board, pos):
        color = board[pos]
        if color == 0: return
        left, right = pos, pos
        while left > 0 and board[left - 1] == color: left -= 1
        while right < self.size - 1 and board[right + 1] == color: right += 1
        for i in range(left, right + 1): board[i] = 0

    def make_move(self, board, move, player):
        new_board = list(board)
        new_board[move] = player
        captured = False
        opponent = -player
        for adj in [move - 1, move + 1]:
            if 0 <= adj < self.size and new_board[adj] == opponent:
                if self.count_liberties(new_board, adj, opponent) == 0:
                    self.remove_group(new_board, adj)
                    captured = True
        return tuple(new_board), captured

# --- メイン処理 ---
def main():
    print("=== MiniGo 1xN Navigator (Fast Load) ===")
    try:
        n_input = input("Board Size N: ")
        if not n_input: return
        n = int(n_input)
    except ValueError:
        print("Invalid number.")
        return

    csv_file = f"game_map_1x{n}.csv"
    # キャッシュファイル名 (例: game_map_1x5.pkl)
    cache_file = f"game_map_1x{n}.pkl"
    
    lookup = {}

    # --- 1. 高速読み込みロジック ---
    # キャッシュが存在し、かつCSVより新しい（更新されていない）場合はキャッシュを使う
    use_cache = False
    if os.path.exists(cache_file):
        # CSVがない、またはCSVの更新日時よりキャッシュの方が新しい場合
        if not os.path.exists(csv_file) or os.path.getmtime(cache_file) > os.path.getmtime(csv_file):
            use_cache = True
    
    if use_cache:
        print(f"Loading from cache: {cache_file} ... (Fast!)")
        try:
            with open(cache_file, 'rb') as f:
                lookup = pickle.load(f)
            print(f"Loaded {len(lookup)} states from cache.")
        except Exception as e:
            print(f"Cache load failed ({e}). Fallback to CSV.")
            use_cache = False

    # キャッシュが使えない場合はCSVから読み込む
    if not use_cache:
        print(f"Loading from CSV: {csv_file} ... (This may take time)")
        try:
            df = pd.read_csv(csv_file, dtype=str)
        except Exception as e:
            print("CSV not found. Please run the C++ solver first.")
            return

        print("Parsing and building cache...")
        for _, row in df.iterrows():
            raw_s = str(row['RawBoard']).replace('"', '').strip()
            hint_s = str(row['HintBoard']).replace('"', '').strip()
            try:
                b_list = list(map(int, raw_s.split(',')))
                key = (tuple(b_list), int(row['Player']))
                lookup[key] = (hint_s, int(row['Winner']))
            except ValueError:
                continue
        
        # 次回のために保存 (Pickle)
        try:
            with open(cache_file, 'wb') as f:
                pickle.dump(lookup, f)
            print(f"Cache saved to {cache_file}")
        except Exception as e:
            print(f"Warning: Could not save cache: {e}")
            
        print(f"Loaded {len(lookup)} states.")

    # --- 以下、ゲームループ (前回と同じ) ---
    game = MiniGoLogic(n)
    current_board = tuple([0] * n)
    current_player = 1 
    history = []

    while True:
        p_name = "Black(●)" if current_player == 1 else "White(○)"
        
        # 検索
        hints = []
        key_direct = (current_board, current_player)
        if key_direct in lookup:
            hint_str, _ = lookup[key_direct]
            hints = hint_str.split(',')
        else:
            rev_board = current_board[::-1]
            key_rev = (rev_board, current_player)
            if key_rev in lookup:
                hint_str, _ = lookup[key_rev]
                hints = hint_str.split(',')[::-1]
            else:
                hints = ["?"] * n

        # 描画
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
        
        # 終了判定A
        can_move = any(h in ['g', 'r', 'y'] for h in hints)
        if not can_move:
            print(f"No legal moves! {p_name} LOSES.")
            print("=== GAME OVER ===")
            break

        print("Moves: " + "".join(status_line))
        print("Index: " + "".join([f"  {i}    " for i in range(n)]))

        # 入力
        move_str = input("\nYour move (0-N, 'b' to back, 'q' to quit): ")
        if move_str.lower() == 'q': break
        
        if move_str.lower() == 'b':
            if history:
                current_board, current_player = history.pop()
                print("<< Undo.")
                continue
            else:
                print("Cannot undo.")
                continue
        
        try:
            move = int(move_str)
            if 0 <= move < n:
                hint = hints[move]
                if hint == 'x' or (hint not in ['g','r','y'] and hint != '?'):
                    print("Invalid move!")
                    continue
                
                history.append((current_board, current_player))
                new_board, captured = game.make_move(current_board, move, current_player)
                
                if captured:
                    final_disp = ["●" if c==1 else "○" if c==-1 else "." for c in new_board]
                    print("\n" + "="*40)
                    print(f"Final:   {' '.join(final_disp)}")
                    print(f"!!! Stone Captured! {p_name} WINS! !!!")
                    print("=== GAME OVER ===")
                    break
                
                current_board = new_board
                current_player = -current_player
            else:
                print("Out of range.")
        except ValueError:
            print("Please enter a number.")

if __name__ == "__main__":
    main()