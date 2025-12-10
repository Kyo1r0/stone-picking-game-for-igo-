# minigo.py
class MiniGo1xN:
    def __init__(self, board, player):
        self.board = board[:]     # 0 = empty, 1 = black, -1 = white
        self.player = player      # 1 or -1

    def count_liberties(self, pos, color, b):
        if b[pos] != color:
            return 0
        n = len(b)

        left = pos
        while left > 0 and b[left-1] == color:
            left -= 1

        right = pos
        while right < n-1 and b[right+1] == color:
            right += 1

        liberties = 0
        if left > 0 and b[left-1] == 0:
            liberties += 1
        if right < n-1 and b[right+1] == 0:
            liberties += 1
        return liberties

    def remove_group(self, pos, b):
        color = b[pos]
        n = len(b)
        if color == 0:
            return

        left = pos
        while left > 0 and b[left-1] == color:
            left -= 1
        right = pos
        while right < n-1 and b[right+1] == color:
            right += 1

        for i in range(left, right + 1):
            b[i] = 0

    def is_capture(self, pos, player):
        b = self.board[:]
        b[pos] = player
        opp = -player
        n = len(b)

        for adj in [pos-1, pos+1]:
            if 0 <= adj < n and b[adj] == opp:
                if self.count_liberties(adj, opp, b) == 0:
                    return True
        return False

    def would_be_suicide(self, pos):
        b = self.board[:]
        b[pos] = self.player

        if self.is_capture(pos, self.player):
            return False

        return self.count_liberties(pos, self.player, b) == 0

    def get_legal_moves(self):
        return [i for i in range(len(self.board))
                if self.board[i] == 0 and not self.would_be_suicide(i)]

    def make_move(self, pos):
        b = self.board[:]
        b[pos] = self.player
        n = len(b)
        opp = -self.player
        captured = False

        for adj in [pos-1, pos+1]:
            if 0 <= adj < n and b[adj] == opp:
                if self.count_liberties(adj, opp, b) == 0:
                    self.remove_group(adj, b)
                    captured = True

        return MiniGo1xN(b, -self.player), captured
