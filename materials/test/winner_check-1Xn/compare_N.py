import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import glob
import os
import re

def main():
    print("=== MiniGo 1xN Comparison Tool ===")
    
    # 1. CSVファイルの検索
    csv_files = glob.glob("game_map_1x*.csv")
    if not csv_files:
        print("CSV file not found. Please run the C++ solver first.")
        return

    data_list = []      # ヒートマップ用データ
    winner_data = []    # 勝者一覧用データ
    
    print(f"Found {len(csv_files)} files.")

    for file in csv_files:
        # ファイル名から N を抽出 (game_map_1x5.csv -> 5)
        match = re.search(r'1x(\d+)', file)
        if not match:
            continue
        n = int(match.group(1))
        
        try:
            # CSV読み込み
            df = pd.read_csv(file, dtype=str)
            
            # 初期盤面 (0,0,...,0) を探す
            # RawBoardのカラムにある "0,0,0..." を探す（ダブルクォート有無に対応）
            target_raw = ",".join(["0"] * n)
            
            # 検索
            found = False
            for _, row in df.iterrows():
                raw = str(row['RawBoard']).replace('"', '').strip()
                if raw == target_raw:
                    # 初期盤面が見つかった
                    hint_s = str(row['HintBoard']).replace('"', '').strip()
                    winner = int(row['Winner'])
                    
                    hints = hint_s.split(',')
                    
                    # ヒートマップ用データに追加
                    for i, h in enumerate(hints):
                        val = 0
                        if h == 'g': val = 1   # Win
                        elif h == 'r': val = -1 # Lose
                        elif h == 'x': val = -2 # Illegal/Suicide
                        else: val = 0 # Draw/Unknown
                        
                        data_list.append({
                            'N': n,
                            'Position': i,
                            'Value': val,
                            'Type': h
                        })
                    
                    # 勝者データに追加
                    winner_data.append({'N': n, 'Winner': winner})
                    found = True
                    break
            
            if not found:
                print(f"Warning: Initial board not found in {file}")

        except Exception as e:
            print(f"Error reading {file}: {e}")

    if not data_list:
        print("No valid data found.")
        return

    # --- 可視化 1: 積み上げヒートマップ ---
    df_heat = pd.DataFrame(data_list)
    
    # ピボットテーブル作成 (行: N, 列: Position, 値: Value)
    pivot_table = df_heat.pivot(index='N', columns='Position', values='Value')
    
    # Nの順序をソート
    pivot_table = pivot_table.sort_index(ascending=False) # 大きいNを上に

    plt.figure(figsize=(10, len(pivot_table) * 0.8 + 2))
    
    # カラーマップ作成
    # -2(Suicide): Gray, -1(Lose): Red, 0(Draw): Yellow, 1(Win): Green
    from matplotlib.colors import ListedColormap
    # 値の範囲に合わせて色を定義 (-2, -1, 0, 1)
    # Seabornのheatmapは値を色にマッピングする
    # ここでは離散的なのでカスタム関数で塗るほうが綺麗だが、簡易的にcmapで対応
    
    # カスタム注釈（文字）を作成
    annot_df = df_heat.pivot(index='N', columns='Position', values='Type').sort_index(ascending=False)
    annot_df = annot_df.fillna("")
    
    # 表示用に文字変換 (g->Win, etc)
    annot_text = annot_df.replace({'g': 'W', 'r': 'L', 'x': 'x', 'y': 'D'})

    # ヒートマップ描画
    # 値が -2, -1, 0, 1 なので、それに合うカラーバー設定は少し難しいが、
    # 視覚的にわかりやすい色を指定する
    # vmin=-2, vmax=1
    cmap = ListedColormap(['#555555', '#ff9999', '#ffff99', '#99ff99']) 
    # gray, light red, light yellow, light green
    
    ax = sns.heatmap(pivot_table, annot=annot_text, fmt="", 
                     cmap=cmap, linewidths=1, linecolor='black',
                     cbar=False, vmin=-2, vmax=1)
    
    ax.set_title("First Move Analysis by Board Size (N)", fontsize=15)
    plt.xlabel("Position Index")
    plt.ylabel("Board Size N")
    plt.tight_layout()
    plt.savefig("comparison_heatmap.png")
    print("Saved 'comparison_heatmap.png'")
    
    # --- 可視化 2: 勝者一覧 ---
    df_win = pd.DataFrame(winner_data).sort_values('N')
    
    plt.figure(figsize=(8, 4))
    # 勝ち=1, 負け=-1
    colors = ['black' if w==1 else 'white' for w in df_win['Winner']]
    edgecolors = ['black'] * len(df_win)
    
    plt.bar(df_win['N'].astype(str), df_win['Winner'], color=colors, edgecolor=edgecolors)
    plt.axhline(0, color='black', linewidth=0.5)
    plt.yticks([-1, 1], ['White Wins', 'Black Wins'])
    plt.xlabel("Board Size N")
    plt.title("Initial Winner (Perfect Play)")
    plt.grid(axis='y', linestyle='--')
    
    # ラベル追加
    for i, row in df_win.iterrows():
        txt = "Black" if row['Winner'] == 1 else "White"
        plt.text(str(row['N']), row['Winner'] * 0.5, txt, 
                 ha='center', va='center', 
                 color='white' if row['Winner']==1 else 'black',
                 fontweight='bold')

    plt.tight_layout()
    plt.savefig("comparison_winner.png")
    print("Saved 'comparison_winner.png'")
    
    # 表示（環境によってはplt.show()が必要）
    # plt.show()

if __name__ == "__main__":
    main()