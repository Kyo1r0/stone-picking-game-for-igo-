# rules.py
from board import Board, EMPTY, BLACK, WHITE

def legal_moves(board: Board, player: int):
    """Yield all legal moves (r,c). We disallow suicide (unless it captures)."""
    size = board.size
    for r in range(size):
        for c in range(size):
            if board.get(r,c) != EMPTY:
                continue
            # simulate
            b2 = board.copy()
            # place and see captures
            captured = b2.apply_move(r,c,player)
            if captured > 0:
                yield (r,c)
                continue
            # if no capture, ensure the placed stone (its group) has liberties
            group, libs = b2.group_and_liberties(r,c)
            if libs > 0:
                yield (r,c)
            # else move is suicide -> illegal

def all_moves_list(board, player):
    return list(legal_moves(board, player))
