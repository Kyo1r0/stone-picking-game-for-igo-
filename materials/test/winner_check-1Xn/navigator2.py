import pandas as pd
import sys

# --- ゲームルール処理クラス ---
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
        
        # 左右の敵石を確認
        for adj in [move - 1, move + 1]:
            if 0 <= adj < self.size and new_board[adj] == opponent:
                # 敵の呼吸点が0なら捕獲
                if self.count_liberties(new_board, adj, opponent) == 0:
                    self.remove_group(new_board, adj)
                    captured = True
                    
        return tuple(new_board), captured

    def is_suicide(self, board, move, player):
        tmp_board, captured = self.make_move(board, move, player)
        if captured: return False # 取れるなら自殺手ではない
        return self.count_liberties(tmp_board, move, player) == 0

    # C++側と同じ「正規化（左右反転して小さい方を採用）」
    def canonical(self, board_tuple):
        rev = board_tuple[::-1]
        # タプル同士の比較 (Pythonのタプル比較は辞書順でC++のvector比較と同じ挙動)
        return rev if rev < board_tuple else board_tuple

# --- メインツール ---

def main():
    print("=== MiniGo 1xN Navigator (Result Logic) ===")
    try:
        n_input = input("Board Size N (e.g. 5): ")
        n = int(n_input)
        csv_file = f"game_map_1x{n}.csv"
        
        print(f"Loading {csv_file} ...")
        # CSV読み込み（全て文字列として読み込んでから処理する安全策）
        df = pd.read_csv(csv_file, dtype=str)
        
    except Exception as e:
        print(f"Error: {e}")
        print("CSVが見つかりません。C++で game_map_1xN.csv を作成してください。")
        return

    # --- CSVデータの辞書化 ---
    lookup = {}
    print("Parsing CSV data...")
    for _, row in df.iterrows():
        # 盤面文字列のクリーニング（ダブルクォートや余計な空白を除去）
        raw_s = str(row['BoardStr']).replace('"', '').strip()
        
        try:
            # "0,0,1" -> [0, 0, 1]
            b_list = list(map(int, raw_s.split(',')))
            
            # キー: (盤面タプル, 手番)
            key = (tuple(b_list), int(row['Player']))
            
            # 値: 勝者
            winner = int(row['Winner'])
            lookup[key] = winner
            
        except ValueError:
            continue

    game = MiniGoLogic(n)
    current_board = tuple([0] * n)
    current_player = 1 # 1:Black, -1:White

    while True:
        p_str = "Black(●)" if current_player == 1 else "White(○)"
        
        # 画面表示
        print("\n" + "="*50)
        display = []
        for cell in current_board:
            if cell == 1: display.append("●")
            elif cell == -1: display.append("○")
            else: display.append(".")
        print(f"Current: {' '.join(display)}   Turn: {p_str}")

        # --- 終了判定: 打てる手があるか？ ---
        legal_moves = [i for i in range(n) if current_board[i] == 0 and not game.is_suicide(current_board, i, current_player)]
        if not legal_moves:
            print(f"\nResult: No legal moves for {p_str}. You LOSE.")
            break
        print("-" * 50)

        # --- ★ヒートマップ解析（提案されたロジック） ---
        print("Analysis (Next Move Prediction):")
        status_line = []
        
        for i in range(n):
            # すでに石がある場所
            if current_board[i] != 0:
                status_line.append(" [ x ] ")
                continue
            
            # 自殺手のチェック
            if game.is_suicide(current_board, i, current_player):
                status_line.append(" [ILG] ")
                continue

            # -------------------------------------------------
            # ★判定ロジック: 子ノードを見て勝敗を決める
            # -------------------------------------------------
            
            # 1. 試しに打ってみる (Next Board)
            next_b_raw, is_capture = game.make_move(current_board, i, current_player)
            
            # 石が取れるならその時点で「勝ち」確定
            if is_capture:
                 status_line.append(" \033[92m[WIN!]\033[0m") # 緑色
                 continue

            # 2. 次の局面の検索キーを作成
            #    - 盤面: 正規化する
            #    - 手番: 相手の手番になる
            next_b_canon = game.canonical(next_b_raw)
            next_p = -current_player
            key = (next_b_canon, next_p)
            
            # 3. CSVから勝者を取得して比較
            if key in lookup:
                winner = lookup[key]
                
                # ★ここが提案の核心部分
                if winner == current_player:
                    # その局面の勝者が自分なら、この手は「勝ち手」
                    status_line.append(" \033[92m[WIN]\033[0m ") # 緑
                elif winner == -current_player:
                    # その局面の勝者が相手なら、この手は「負け手」
                    status_line.append(" \033[91m[LOS]\033[0m ") # 赤
                else:
                    # 引き分け (0)
                    status_line.append(" [DRW] ")
            else:
                # CSVに載っていない（通常ありえないが、終局図など）
                status_line.append(" [UNK] ")

            # -------------------------------------------------

        # 結果表示
        print("Moves: " + "".join(status_line))
        print("Index: " + "".join([f"  {i}    " for i in range(n)]))

        # --- 入力処理 ---
        move_str = input("\nYour move (0-N, 'q' to quit): ")
        if move_str.lower() == 'q': break
        
        try:
            move = int(move_str)
            if move in legal_moves:
                new_board, captured = game.make_move(current_board, move, current_player)
                
                # 石を取ったら勝ち
                if captured:
                    display = ["●" if c==1 else "○" if c==-1 else "." for c in new_board]
                    print("\n" + "="*50)
                    print(f"Final:   {' '.join(display)}")
                    print(f"\nResult: Captured stones! {p_str} WIN!")
                    break

                current_board = new_board
                current_player = -current_player
            else:
                print("!!! Invalid or Illegal move !!!")
        except ValueError:
            pass

if __name__ == "__main__":
    main()