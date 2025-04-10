MATRIX_SIZE = 8
MATRIX_POSITIONS_RATING = [
    [2.00, 0.75, 1.75, 1.75, 1.75, 1.75, 0.75, 2.00],
    [0.75, 0.50, 1.25, 1.25, 1.25, 1.25, 0.50, 0.75],
    [1.75, 1.25, 1.00, 1.00, 1.00, 1.00, 1.25, 1.75],
    [1.75, 1.25, 1.00, 1.00, 1.00, 1.00, 1.25, 1.75],
    [1.75, 1.25, 1.00, 1.00, 1.00, 1.00, 1.25, 1.75],
    [1.75, 1.25, 1.00, 1.00, 1.00, 1.00, 1.25, 1.75],
    [0.75, 0.50, 1.25, 1.25, 1.25, 1.25, 0.50, 0.75],
    [2.00, 0.75, 1.75, 1.75, 1.75, 1.75, 0.75, 2.00],
]


class ValidMove:
    """Representation of one valid move with its capture value and position rating"""

    def __init__(self, r, c):
        self.r = r
        self.c = c
        self.rating = MATRIX_POSITIONS_RATING[r][c]
        self.value = 0

    def weight(self):
        return self.value * self.rating


class Player:
    """Reversi player with heuristic moves"""

    def __init__(self, my_color, opponent_color):
        self.my_color = my_color
        self.opponent_color = opponent_color

    def map_positions(self, board):
        my_positions = []
        opponent_positions = []
        #
        for r in range(MATRIX_SIZE):
            for c in range(MATRIX_SIZE):
                if board[r][c] == self.my_color:
                    my_positions.append([r, c])
                elif board[r][c] == self.opponent_color:
                    opponent_positions.append([r, c])
        #
        return (my_positions, opponent_positions)

    def search_valid_moves(self, board, positions):
        valid_moves = []
        search_directions = [[-1, 0], [0, 1], [1, 0], [0, -1], [-1, 1], [1, 1], [1, -1], [-1, -1]]
        #
        for p in positions:
            for d in search_directions:
                counter = 0
                pointer = [p[0] + d[0], p[1] + d[1]]
                while True:
                    if pointer[0] < 0 or pointer[1] < 0 or pointer[0] >= MATRIX_SIZE or pointer[1] >= MATRIX_SIZE:
                        break  # pointer is out of board
                    elif board[pointer[0]][pointer[1]] == self.my_color:
                        break  # direction already closed
                    elif board[pointer[0]][pointer[1]] == -1 and counter == 0:
                        break  # would not capture any piece
                    elif board[pointer[0]][pointer[1]] == -1 and counter > 0:
                        move = next((m for m in valid_moves if (m.r == pointer[0] and m.c == pointer[1])), None)  # get move if already in valid_moves
                        if move == None:
                            move = ValidMove(pointer[0], pointer[1])
                            valid_moves.append(move)
                        move.value += counter
                        break  # move is valid
                    else:
                        counter += 1
                        pointer[0] += d[0]
                        pointer[1] += d[1]
        #
        return valid_moves

    def pick_weighted_move(self, moves):
        max_weight = max(m.weight() for m in moves)
        max_weight_moves = [m for m in moves if m.weight() == max_weight]
        best_move = sorted(max_weight_moves, key=lambda m: m.value, reverse=True)[0]  # most greedy of max_weight_moves
        return (best_move.r, best_move.c)

    def select_move(self, board):
        positions = self.map_positions(board)
        valid_moves = self.search_valid_moves(board, positions[0])
        #
        if len(valid_moves) == 0:
            return None
        else:
            return self.pick_weighted_move(valid_moves)
