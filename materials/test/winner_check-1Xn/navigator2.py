import pygame
import pandas as pd
import sys

# --- 設定 ---
CELL_SIZE = 100       # 1マスの大きさ
MARGIN = 20           # 画面端の余白
BOARD_COLOR = (220, 179, 92)  # 碁盤の色
LINE_COLOR = (0, 0, 0)
BLACK_COLOR = (20, 20, 20)
WHITE_COLOR = (240, 240, 240)
HINT_WIN_COLOR = (0, 255, 0, 100)  # 緑 (半透明)
HINT_LOSE_COLOR = (255, 0, 0, 100) # 赤 (半透明)
TEXT_COLOR = (255, 255, 255)
BG_COLOR = (50, 50, 50)

# --- ゲームロジック (石を取る判定用) ---
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
        
        # 左右確認
        for adj in [move - 1, move + 1]:
            if 0 <= adj < self.size and new_board[adj] == opponent:
                if self.count_liberties(new_board, adj, opponent) == 0:
                    self.remove_group(new_board, adj)
                    captured = True
        return tuple(new_board), captured

# --- メインアプリ ---
class MiniGoApp:
    def __init__(self):
        pygame.init()
        self.font = pygame.font.SysFont(None, 40)
        self.small_font = pygame.font.SysFont(None, 24)
        
        # 設定入力 (コンソール)
        print("=== MiniGo 1xN Pygame Navigator ===")
        try:
            self.n = int(input("Board Size N: "))
        except ValueError:
            self.n = 5 # default
            
        self.csv_file = f"game_map_1x{self.n}.csv"
        self.game = MiniGoLogic(self.n)
        
        # 画面設定
        width = self.n * CELL_SIZE + MARGIN * 2
        height = 300
        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption(f"MiniGo 1x{self.n} Solver")

        # データ読み込み
        self.load_csv()

        # ゲーム状態
        self.current_board = tuple([0] * self.n)
        self.current_player = 1 # 1:Black, -1:White
        self.history = [] # 履歴 (Undo用)
        self.message = "Left Click: Move / Right Click: Undo"
        self.game_over = False

    def load_csv(self):
        print(f"Loading {self.csv_file} ...")
        try:
            df = pd.read_csv(self.csv_file, dtype=str)
            self.lookup = {}
            for _, row in df.iterrows():
                # 文字列整形
                raw_s = str(row['RawBoard']).replace('"', '')
                hint_s = str(row['HintBoard']).replace('"', '')
                
                b_list = list(map(int, raw_s.split(',')))
                key = (tuple(b_list), int(row['Player']))
                
                # Winnerも取得しておく
                self.lookup[key] = (hint_s, int(row['Winner']))
            print("CSV Loaded.")
        except Exception as e:
            print(f"Error loading CSV: {e}")
            self.lookup = {}

    def get_hints(self):
        # 盤面の正規化 (左右対称チェック)
        rev_board = self.current_board[::-1]
        is_reversed = (rev_board < self.current_board)
        search_board = rev_board if is_reversed else self.current_board
        
        key = (search_board, self.current_player)
        
        hints = ["?"] * self.n
        
        if key in self.lookup:
            hint_str, _ = self.lookup[key]
            raw_hints = hint_str.split(',')
            # 反転していたらヒントも反転して戻す
            if is_reversed:
                hints = raw_hints[::-1]
            else:
                hints = raw_hints
        
        return hints

    def handle_click(self, pos):
        if self.game_over:
            return

        x, y = pos
        # 盤面エリア内か判定
        board_y_start = (300 - CELL_SIZE) // 2
        if board_y_start <= y <= board_y_start + CELL_SIZE:
            idx = (x - MARGIN) // CELL_SIZE
            if 0 <= idx < self.n:
                self.try_move(idx)

    def try_move(self, idx):
        # すでに石があるかチェック
        if self.current_board[idx] != 0:
            return

        # ヒントを確認して自殺手なら弾く（CSVベースの判定）
        hints = self.get_hints()
        if idx < len(hints):
            if hints[idx] == 'x': # Illegal
                self.message = "Illegal Move (Suicide)!"
                return

        # 履歴に保存 (Undo用)
        self.history.append((self.current_board, self.current_player))

        # 手を打つ
        new_board, captured = self.game.make_move(self.current_board, idx, self.current_player)
        
        if captured:
            self.message = f"{'Black' if self.current_player==1 else 'White'} Wins! (Stones Captured)"
            self.game_over = True
        
        self.current_board = new_board
        self.current_player = -self.current_player

        # 詰み判定（打つ手がない）
        if not self.game_over:
            next_hints = self.get_hints() # 次の手番でのヒント
            valid_moves = [h for h in next_hints if h in ['g', 'r', 'y']]
            if not valid_moves:
                self.message = f"No moves! {'Black' if self.current_player==1 else 'White'} Loses."
                self.game_over = True

    def undo(self):
        if self.history:
            self.current_board, self.current_player = self.history.pop()
            self.game_over = False
            self.message = "Undo."
        else:
            self.message = "Cannot undo."

    def draw(self):
        self.screen.fill(BG_COLOR)

        # 盤面の基本位置
        start_x = MARGIN
        start_y = (300 - CELL_SIZE) // 2
        
        # ヒント取得
        hints = self.get_hints()

        # 1. 盤面描画
        pygame.draw.rect(self.screen, BOARD_COLOR, (start_x, start_y, self.n * CELL_SIZE, CELL_SIZE))
        pygame.draw.rect(self.screen, LINE_COLOR, (start_x, start_y, self.n * CELL_SIZE, CELL_SIZE), 2)
        
        for i in range(self.n):
            cx = start_x + i * CELL_SIZE
            cy = start_y
            center = (cx + CELL_SIZE // 2, cy + CELL_SIZE // 2)
            
            # 格子線
            pygame.draw.rect(self.screen, LINE_COLOR, (cx, cy, CELL_SIZE, CELL_SIZE), 1)

            # ヒートマップ (石がない場所のみ)
            if self.current_board[i] == 0 and not self.game_over:
                hint = hints[i] if i < len(hints) else "?"
                
                # 半透明のサーフェスを作成
                s = pygame.Surface((CELL_SIZE, CELL_SIZE), pygame.SRCALPHA)
                if hint == 'g': # Win
                    s.fill(HINT_WIN_COLOR) # Green
                    self.screen.blit(s, (cx, cy))
                    # テキストでも表示
                    text = self.small_font.render("WIN", True, (0, 100, 0))
                    self.screen.blit(text, (cx + 35, cy + 5))
                    
                elif hint == 'r': # Lose
                    s.fill(HINT_LOSE_COLOR) # Red
                    self.screen.blit(s, (cx, cy))
                    
                elif hint == 'x': # Illegal
                    # X印
                    pygame.draw.line(self.screen, (100, 0, 0), (cx+20, cy+20), (cx+80, cy+80), 3)
                    pygame.draw.line(self.screen, (100, 0, 0), (cx+80, cy+20), (cx+20, cy+80), 3)

            # 石の描画
            stone = self.current_board[i]
            if stone == 1: # Black
                pygame.draw.circle(self.screen, BLACK_COLOR, center, CELL_SIZE // 2 - 10)
            elif stone == -1: # White
                pygame.draw.circle(self.screen, WHITE_COLOR, center, CELL_SIZE // 2 - 10)
                pygame.draw.circle(self.screen, BLACK_COLOR, center, CELL_SIZE // 2 - 10, 1) # 縁取り

        # 2. 情報表示
        turn_text = f"Turn: {'Black (●)' if self.current_player == 1 else 'White (○)'}"
        if self.game_over:
            turn_text = "GAME OVER"
            
        t_surf = self.font.render(turn_text, True, TEXT_COLOR)
        self.screen.blit(t_surf, (20, 20))

        m_surf = self.small_font.render(self.message, True, (200, 200, 200))
        self.screen.blit(m_surf, (20, 60))
        
        pygame.display.flip()

    def run(self):
        clock = pygame.time.Clock()
        while True:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()
                elif event.type == pygame.MOUSEBUTTONDOWN:
                    if event.button == 1: # Left Click
                        self.handle_click(event.pos)
                    elif event.button == 3: # Right Click (Undo)
                        self.undo()
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_z or event.key == pygame.K_LEFT:
                        self.undo()

            self.draw()
            clock.tick(30)

if __name__ == "__main__":
    app = MiniGoApp()
    app.run()