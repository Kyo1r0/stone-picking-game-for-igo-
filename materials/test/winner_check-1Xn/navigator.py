import pandas as pd
import sys

# --- ゲームルール定義 ---
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
        opponent = -player
        captured = False
        
        # 左右を確認
        for adj in [move - 1, move + 1]:
            if 0 <= adj < self.size and new_board[adj] == opponent:
                # 敵グループの呼吸点が0ならグループごと削除
                if self.count_liberties(new_board, adj, opponent) == 0:
                    self.remove_group(new_board, adj)
                    captured = True
                    
        return tuple(new_board), captured

    def is_suicide(self, board, move, player):
        tmp_board, captured = self.make_move(board, move, player)
        if captured: return False
        return self.count_liberties(tmp_board, move, player) == 0

    def canonical(self, board_tuple):
        rev = board_tuple[::-1]
        return board_tuple if board_tuple <= rev else rev

# --- メインツール ---

def main():
    print("=== MiniGo 1xN Navigator (Fixed Rules) ===")
    try:
        n_input = input("Board Size N (e.g. 5): ")
        n = int(n_input)
        csv_file = f"game_map_1x{n}.csv"
        # CSV読み込み (文字列として読み込む設定を追加)
        print(f"Loading {csv_file} ...")
        df = pd.read_csv(csv_file, dtype={'BoardStr': str})
    except Exception as e:
        print(f"Error: {e}")
        print("CSVが見つかりません。先にC++を実行して game_map_1xN.csv を作成してください。")
        return

    # データを辞書化
    lookup = {}
    for _, row in df.iterrows():
        # ダブルクォートが除去されている前提、または除去処理を入れる
        s = row['BoardStr'].replace('"', '') 
        b_list = list(map(int, s.split(',')))
        key = (tuple(b_list), int(row['Player']))
        lookup[key] = int(row['Winner'])

    game = MiniGoLogic(n)
    current_board = tuple([0] * n)
    current_player = 1 # 1:Black, -1:White

    while True:
        p_name = "Black(●)" if current_player == 1 else "White(○)"
        print("\n" + "="*50)
        
        # 1. 盤面表示
        display = []
        for cell in current_board:
            if cell == 1: display.append("●")
            elif cell == -1: display.append("○")
            else: display.append(".")
        print(f"Current: {' '.join(display)}   Turn: {p_name}")

        # --- 終了判定チェック1: 合法手があるか？ ---
        legal_moves = []
        for i in range(n):
            if current_board[i] == 0 and not game.is_suicide(current_board, i, current_player):
                legal_moves.append(i)
        
        if not legal_moves:
            print(f"\nResult: {p_name} has NO moves! You LOSE.")
            print("GAME OVER")
            break

        print("-" * 50)

        # 2. 次の一手の解析 (ヒートマップ)
        print("Analysis:")
        status_line = []
        
        for i in range(n):
            if current_board[i] != 0:
                status_line.append(" [ x ] ")
                continue
            
            if game.is_suicide(current_board, i, current_player):
                status_line.append(" [ILG] ")
                continue

            # 仮に打ってみる
            next_b, is_cap = game.make_move(current_board, i, current_player)
            
            # もし打って「捕獲」が発生するなら、その手は「勝ち確定」
            if is_cap:
                status_line.append(" \033[92m[WIN!]\033[0m") # 即勝ち
                continue

            # そうでなければCSV検索
            next_p = -current_player
            canon_b = game.canonical(next_b)
            key = (canon_b, next_p)
            
            if key in lookup:
                winner = lookup[key]
                if winner == current_player:
                    status_line.append(" \033[92m[WIN]\033[0m ")
                elif winner == -current_player:
                    status_line.append(" \033[91m[LOS]\033[0m ")
                else:
                    status_line.append(" [DRAW] ")
            else:
                status_line.append(" [UNK] ")

        print("Moves: " + "".join(status_line))
        print("Index: " + "".join([f"  {i}    " for i in range(n)]))

        # 3. 入力処理
        move_str = input("\nYour move (0-N, 'q' to quit): ")
        if move_str.lower() == 'q': break
        
        try:
            move = int(move_str)
            if move in legal_moves:
                # 手を反映
                new_board, captured = game.make_move(current_board, move, current_player)
                
                # --- 終了判定チェック2: 石を取ったか？ ---
                if captured:
                    # 盤面表示（最終状態）
                    display = []
                    for cell in new_board:
                        if cell == 1: display.append("●")
                        elif cell == -1: display.append("○")
                        else: display.append(".")
                    print("\n" + "="*50)
                    print(f"Final:   {' '.join(display)}")
                    print(f"\nResult: {p_name} captured stones! You WIN!")
                    print("GAME OVER")
                    break
                
                # ゲーム続行なら交代
                current_board = new_board
                current_player = -current_player
            else:
                if 0 <= move < n and current_board[move] != 0:
                    print("!!! Occupied !!!")
                elif 0 <= move < n:
                    print("!!! Illegal move (Suicide) !!!")
                else:
                    print("!!! Out of range !!!")
        except ValueError:
            pass

if __name__ == "__main__":
    main()