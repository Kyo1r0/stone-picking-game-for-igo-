# main.py
from game_tree import solve_for_first_capture
from board import BLACK, WHITE
from game_tree import solve

def pretty_moves(moves):
    return [f"({r},{c})" for r,c in moves]

def run():
    size = 3
    print(f"Solving 3x3, first-to-capture-1 rule.")
    can_win_black, moves_black = solve_for_first_capture(size, BLACK)
    print("Black to move (first player):")
    print("  Can force capture-first? ->", can_win_black)
    if can_win_black:
        print("  Winning moves:", pretty_moves(moves_black))

    # also check White to move from empty
    can_win_white, moves_white = solve_for_first_capture(size, WHITE)
    print("\nWhite to move (second player if we start empty with White):")
    print("  Can force capture-first? ->", can_win_white)
    if can_win_white:
        print("  Winning moves:", pretty_moves(moves_white))

if __name__ == "__main__":
    run()
