import pandas as pd

def main():
    print("=== MiniGo 1xN Simple Navigator ===")
    n = int(input("Board Size N: "))
    csv_file = f"game_map_1x{n}.csv"

    print(f"Loading {csv_file} ...")
    try:
        # 文字列として読み込む
        df = pd.read_csv(csv_file, dtype=str)
    except Exception as e:
        print("CSV not found.")
        return

    # 辞書化
    # Key: (tuple_board, player) -> Value: (hint_string, winner)
    lookup = {}
    for _, row in df.iterrows():
        raw_s = row['RawBoard'].replace('"', '') # "0,0,0"
        hint_s = row['HintBoard'].replace('"', '') # "g,r,x"
        
        b_list = list(map(int, raw_s.split(',')))
        key = (tuple(b_list), int(row['Player']))
        
        lookup[key] = (hint_s, int(row['Winner']))

    # ゲーム開始
    current_board = tuple([0] * n)
    current_player = 1 # 1:Black, -1:White

    while True:
        # 表示用文字
        p_name = "Black(●)" if current_player == 1 else "White(○)"
        
        # 1. 検索のために正規化（Canonicalization）
        #    C++側は正規化された盤面しか持っていないため
        rev_board = current_board[::-1]
        is_reversed = (rev_board < current_board)
        search_board = rev_board if is_reversed else current_board
        
        search_key = (search_board, current_player)

        # 2. CSV検索
        hints = []
        winner_prediction = 0
        
        if search_key in lookup:
            hint_str, winner_prediction = lookup[search_key]
            # 文字列 "g,r,x,1" をリスト ['g', 'r', 'x', '1'] に分解
            hints = hint_str.split(',')
            
            # ★重要: 盤面を反転して検索したなら、ヒントも左右反転して戻す必要がある
            if is_reversed:
                hints = hints[::-1]
        else:
            # 見つからない（終局など）
            hints = ["?"] * n

        # 3. 画面描画
        print("\n" + "="*40)
        display_board = []
        status_line = []
        
        for i in range(n):
            cell = current_board[i]
            hint = hints[i] if i < len(hints) else "?"

            # 盤面の石表示
            if cell == 1: display_board.append("●")
            elif cell == -1: display_board.append("○")
            else: display_board.append(".")

            # ヒートマップ表示 (g=Win, r=Lose, x=Invalid, Number=Occupied)
            if hint == 'g':
                status_line.append(" \033[92m[WIN]\033[0m ") # Green
            elif hint == 'r':
                status_line.append(" \033[91m[LOS]\033[0m ") # Red
            elif hint == 'y':
                status_line.append(" [DRW] ")
            elif hint == 'x':
                status_line.append(" [ILG] ") # Suicide/Illegal
            else:
                # すでに石がある、または不明
                status_line.append(" [ - ] ")

        print(f"Current: {' '.join(display_board)}   Turn: {p_name}")
        print("-" * 40)
        
        # もし検索できたのに「g」「r」「y」が一つもなければ、打つ手なし（負け）
        valid_moves = [h for h in hints if h in ['g', 'r', 'y']]
        if not valid_moves:
            print("No legal moves! You LOSE.")
            break

        print("Moves: " + "".join(status_line))
        print("Index: " + "".join([f"  {i}    " for i in range(n)]))

        # 4. 入力と更新
        move_str = input("\nYour move (0-N): ")
        if move_str == 'q': break
        try:
            move = int(move_str)
            if 0 <= move < n:
                hint = hints[move]
                
                # --- ゲーム進行処理 (Python側で簡易的に進める) ---
                # 正確に進めるには「ヒント」だけでなく「次の盤面」も必要だが、
                # 今回はユーザー入力に従って配列を書き換える簡易実装にする
                # ※厳密な「連の除去」などはPythonに書く必要がありますが、
                # ここでは「勝ち(g)」を選んだら「勝ちました！」で終わる簡易版にします
                
                if hint == 'g':
                    print("\n!!! You chose a WINNING move! (Assuming optimal play) !!!")
                    # ここで厳密にゲームを続けたいなら Python にも make_move が必要ですが
                    # 「必勝法を探す」目的なら、勝ち手を見つけた時点で終了でも良いかもしれません
                    # 続けるなら前のコードの MiniGoLogic を混ぜてください
                    
                elif hint == 'x' or hint not in ['g','r','y']:
                    print("Invalid move!")
                    continue
                
                # とりあえず手番だけ変えて石を置く（連処理なしの簡易更新）
                # ちゃんと遊ぶなら前のコードのLogicクラスを使ってください
                new_b = list(current_board)
                new_b[move] = current_player
                current_board = tuple(new_b)
                current_player = -current_player
                
        except:
            pass

if __name__ == "__main__":
    main()