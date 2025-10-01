# visualize.py
from board import EMPTY, BLACK, WHITE

def ascii_board(board):
    rows = []
    for r in range(board.size):
        row = []
        for c in range(board.size):
            v = board.get(r,c)
            if v == EMPTY:
                row.append('.')
            elif v == BLACK:
                row.append('X')  # black
            else:
                row.append('O')  # white
        rows.append(' '.join(row))
    return '\n'.join(rows)

def print_board(board):
    print(ascii_board(board))
