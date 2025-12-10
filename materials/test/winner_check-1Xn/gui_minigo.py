import pygame
import csv
from minigo import MiniGo1xN

# ==============================
# 設定
# ==============================
CELL = 80
PADDING = 20
HEIGHT = 150
FPS = 60

# CSV から色を読み込む
def load_heatmap(filename, n):
    colors = ["gray"] * n
    try:
        with open(filename, newline="") as f:
            reader = csv.DictReader(f)
            for row in reader:
                i = int(row["move"])
                colors[i] = row["color"]
    except:
        print("heatmap CSV 読み込み失敗")
    return colors


def color_to_rgb(name):
    return {
        "green": (100, 255, 100),
        "yellow": (255, 255, 100),
        "red": (255, 120, 120),
        "gray": (200, 200, 200),
        "unknown": (180, 180, 180)
    }.get(name, (200,200,200))


# ==============================
# GUI メイン
# ==============================
def run_game(n):
    pygame.init()
    width = n * CELL + PADDING*2
    screen = pygame.display.set_mode((width, HEIGHT))
    pygame.display.set_caption("MiniGo 1xN Heatmap GUI")

    clock = pygame.time.Clock()

    # 初期盤面
    game = MiniGo1xN([0]*n, 1)

    # CSV からヒートマップ読み込み
    heatmap_colors = load_heatmap(f"heatmap_1x{n}.csv", n)

    running = True
    while running:
        clock.tick(FPS)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            elif event.type == pygame.MOUSEBUTTONDOWN:
                x, y = event.pos
                i = (x - PADDING) // CELL

                if 0 <= i < n:
                    legal = game.get_legal_moves()
                    if i in legal:
                        game, cap = game.make_move(i)

        # ==============================
        # 描画
        # ==============================
        screen.fill((240,240,240))

        for i in range(n):
            x = PADDING + i * CELL

            # ヒートマップ色
            color = color_to_rgb(heatmap_colors[i])
            pygame.draw.rect(screen, color, (x, 30, CELL-5, CELL-5))

            # 石を描く
            if game.board[i] == 1:
                pygame.draw.circle(screen, (0,0,0), (x+CELL//2, 30+CELL//2), 25)
            elif game.board[i] == -1:
                pygame.draw.circle(screen, (255,255,255), (x+CELL//2, 30+CELL//2), 25)
                pygame.draw.circle(screen, (0,0,0), (x+CELL//2, 30+CELL//2), 25, 2)

        pygame.display.flip()

    pygame.quit()


if __name__ == "__main__":
    run_game(6)  # 好きなサイズに変更
