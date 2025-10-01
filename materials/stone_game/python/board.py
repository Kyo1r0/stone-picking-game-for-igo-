# board.py
from collections import deque
from copy import deepcopy

EMPTY = 0
BLACK = 1
WHITE = -1

class Board:
    def __init__(self, size=3, grid=None):
        self.size = size
        if grid is None:
            self.grid = [[EMPTY]*size for _ in range(size)]
        else:
            self.grid = deepcopy(grid)

    def copy(self):
        return Board(self.size, self.grid)

    def inside(self, r, c):
        return 0 <= r < self.size and 0 <= c < self.size

    def get(self, r, c):
        return self.grid[r][c]

    def set(self, r, c, color):
        self.grid[r][c] = color

    def neighbors(self, r, c):
        for dr, dc in ((1,0),(-1,0),(0,1),(0,-1)):
            nr, nc = r+dr, c+dc
            if self.inside(nr,nc):
                yield nr, nc

    def group_and_liberties(self, r, c):
        """Return (set of coords in group, number of liberties)."""
        color = self.get(r,c)
        if color == EMPTY:
            return set(), 0
        visited = set()
        q = deque()
        q.append((r,c))
        visited.add((r,c))
        liberties = set()
        while q:
            x,y = q.popleft()
            for nx, ny in self.neighbors(x,y):
                val = self.get(nx,ny)
                if val == EMPTY:
                    liberties.add((nx,ny))
                elif val == color and (nx,ny) not in visited:
                    visited.add((nx,ny))
                    q.append((nx,ny))
        return visited, len(liberties)

    def remove_group(self, coords):
        for r,c in coords:
            self.set(r,c, EMPTY)

    def apply_move(self, r, c, color):
        """
        Place stone of color at (r,c) assuming the move is legal (callers should check)
        Returns number of opponent stones captured by this move.
        """
        self.set(r,c,color)
        captured = 0
        opp = -color
        # Check neighbor opponent groups for capture
        to_remove = []
        for nr, nc in self.neighbors(r,c):
            if self.get(nr,nc) == opp:
                group, libs = self.group_and_liberties(nr,nc)
                if libs == 0:
                    to_remove.append(group)
        for grp in to_remove:
            captured += len(grp)
            self.remove_group(grp)
        # Note: suicide is prevented in rules (so no need to remove own group)
        return captured

    def zobrist_hashable(self):
        # simple immutable representation for memoization
        return tuple(tuple(row) for row in self.grid)
