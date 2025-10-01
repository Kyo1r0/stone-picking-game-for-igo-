# game_tree.py
from board import Board, BLACK, WHITE
from rules import legal_moves
from functools import lru_cache

def state_key(board: Board, player: int):
    return (board.zobrist_hashable(), player)

def solve_for_first_capture(size=3, starting_player=BLACK):
    """
    Build empty board and solve whether starting_player can force to be
    the first to capture >=1 stone on a size x size board.
    Returns a tuple (can_force_win, winning_moves)
    """
    board = Board(size)
    return solve(board, starting_player)

def solve(board: Board, player: int):
    """
    Returns (can_force_win_bool, winning_moves_list)
    Use memoization on board state and current player.
    A 'win' is defined as current player can force a move sequence
    that results in that player capturing >=1 stone first.
    """
    @lru_cache(maxsize=None)
    def dfs(key):
        grid_key, cur = key
        # Reconstruct a Board from grid if needed (cheap since grid small)
        b = Board(len(grid_key))
        for i in range(len(grid_key)):
            b.grid[i] = list(grid_key[i])

        # Generate legal moves
        moves = list(legal_moves(b, cur))
        if not moves:
            return (False, ())  # no legal move -> cannot capture

        # Try each move: if it captures immediately -> win
        winning = []
        for r,c in moves:
            b2 = b.copy()
            captured = b2.apply_move(r,c,cur)
            if captured > 0:
                # immediate capture -> current player wins
                winning.append((r,c))
                continue
            # else recursive check: if opponent cannot force win from next state,
            # then current player can force win by playing this move.
            next_key = (b2.zobrist_hashable(), -cur)
            opp_can_win, _ = dfs(next_key)
            if not opp_can_win:
                winning.append((r,c))

        if winning:
            return (True, tuple(winning))
        else:
            return (False, ())

    init_key = (board.zobrist_hashable(), player)
    return dfs(init_key)
