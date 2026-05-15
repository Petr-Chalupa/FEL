import ox
import os
import torch
import torch.nn as nn
import random, time
import math

WIN_MASKS: list = []


def init_win_masks(size=8, k=5):
    masks = []
    # Horizontal
    for i in range(size):
        for j in range(size - k + 1):
            mask = 0
            for t in range(k):
                mask |= 1 << (i * size + (j + t))
            masks.append(mask)
    # Vertical
    for i in range(size - k + 1):
        for j in range(size):
            mask = 0
            for t in range(k):
                mask |= 1 << ((i + t) * size + j)
            masks.append(mask)
    # Diagonal \
    for i in range(size - k + 1):
        for j in range(size - k + 1):
            mask = 0
            for t in range(k):
                mask |= 1 << ((i + t) * size + (j + t))
            masks.append(mask)
    # Diagonal /
    for i in range(size - k + 1):
        for j in range(k - 1, size):
            mask = 0
            for t in range(k):
                mask |= 1 << ((i + t) * size + (j - t))
            masks.append(mask)
    return masks


class MCTSState:
    def __init__(self):
        self.x_bits = 0
        self.o_bits = 0
        self.player = 0

    @classmethod
    def from_board(cls, board: ox.Board):
        s = cls()
        s.player = board.player

        for action in board.set_x:
            s.x_bits |= 1 << action

        for action in board.set_o:
            s.o_bits |= 1 << action

        return s

    def copy(self):
        s = MCTSState()
        s.x_bits = self.x_bits
        s.o_bits = self.o_bits
        s.player = self.player
        return s

    def legal_moves(self):
        occupied = self.x_bits | self.o_bits
        empty = ~occupied & ((1 << 64) - 1)

        if occupied == 0:
            return [27]  # Explicitly return middle of board when empty

        candidates = set()

        for i in range(64):
            if occupied & (1 << i):
                row, col = divmod(i, 8)
                for drow, dcol in ((0, 1), (0, -1), (1, 0), (-1, 0), (1, 1), (-1, -1), (1, -1), (-1, 1)):
                    nr, nc = row + drow, col + dcol
                    if 0 <= nr < 8 and 0 <= nc < 8:
                        candidates.add(nr * 8 + nc)

        return [m for m in candidates if empty & (1 << m)]

    def make_move(self, move):
        new = self.copy()
        bit = 1 << move

        if self.player == 0:
            new.x_bits |= bit
        else:
            new.o_bits |= bit

        new.player = 1 - self.player

        return new

    def check_win(self, bits):
        for mask in WIN_MASKS:
            if (bits & mask) == mask:
                return True
        return False

    def is_terminal(self):
        return self.check_win(self.x_bits) or self.check_win(self.o_bits) or (self.x_bits | self.o_bits) == (1 << 64) - 1


class MCTSNode:
    def __init__(self, state: MCTSState, parent=None, move=None):
        self.state = state
        self.parent = parent
        self.move = move
        self.children = []
        self.visits = 0
        self.value = 0
        self.untried_moves = list(state.legal_moves())

    def best_child(self, c):
        best_score = float("-inf")
        best_child = None

        for child in self.children:
            uct = float("inf") if child.visits == 0 else (child.value / child.visits) + c * math.sqrt(math.log(self.visits) / child.visits)
            if uct > best_score:
                best_score = uct
                best_child = child

        return best_child

    def update(self, result):
        self.visits += 1
        self.value += result

    def expand(self):
        move = self.untried_moves.pop()
        new_state = self.state.make_move(move)
        child = MCTSNode(state=new_state, parent=self, move=move)
        self.children.append(child)

        return child

    def is_fully_expanded(self):
        return len(self.untried_moves) == 0


class MCTSBot:
    def __init__(self, play_as: int, time_limit: float):
        self.play_as = play_as
        self.time_limit = time_limit * 0.9

    def play_action(self, board: ox.Board):
        start_time = time.time()
        root_state = MCTSState.from_board(board)
        root = MCTSNode(root_state)

        while (time.time() - start_time) < self.time_limit:
            node = self.selection(root)
            if node.untried_moves:
                node = node.expand()

            result = self.simulation(node.state)
            self.backpropagation(node, result)

        best_child = max(root.children, key=lambda n: n.visits)
        return best_child.move

    def selection(self, node: MCTSNode):
        while node.is_fully_expanded() and node.children:
            node = node.best_child(c=1.41)
        return node

    def simulation(self, state: MCTSState):
        current = state.copy()

        while not current.is_terminal():
            moves = current.legal_moves()
            move = random.choice(moves)
            current = current.make_move(move)

        x_win = current.check_win(current.x_bits)
        o_win = current.check_win(current.o_bits)
        if x_win:
            return 1 if self.play_as == 0 else -1
        if o_win:
            return 1 if self.play_as == 1 else -1

        return 0

    def backpropagation(self, node: MCTSNode, result):
        while node is not None:
            node.update(result)
            node = node.parent


class ValueNetwork(nn.Module):
    def __init__(self):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(64, 256),
            nn.ReLU(),
            nn.Linear(256, 256),
            nn.ReLU(),
            nn.Linear(256, 128),
            nn.ReLU(),
            nn.Linear(128, 1),
            nn.Tanh(),
        )

    def forward(self, x):
        return self.net(x)


class NeuralMCTSBot(MCTSBot):
    def __init__(self, play_as: int, time_limit: float):
        super().__init__(play_as, time_limit)

        self.net = ValueNetwork()

        try:
            path = os.path.join(os.path.dirname(__file__), "nn_mcts.pth")
            if os.path.exists(path):
                self.net.load_state_dict(torch.load(path, map_location="cpu"))
            else:
                print(f"NN model file not found: {path}")
        except Exception as e:
            message = str(e).strip()
            if message:
                print(f"NN model load error: {message}")

        self.net = self.net.to("cpu")
        self.net.eval()

    def encode_state(self, state: MCTSState):
        vec = torch.zeros(64, dtype=torch.float32)

        for i in range(64):
            bit = 1 << i
            if state.x_bits & bit:
                vec[i] = 1
            elif state.o_bits & bit:
                vec[i] = -1

        return vec

    def simulation(self, state: MCTSState):
        x = self.encode_state(state).unsqueeze(0)

        with torch.no_grad():
            return float(torch.clamp(self.net(x), -1, 1).item())


WIN_MASKS = init_win_masks(8, 5)  # 8x8, 5 in a row


if __name__ == "__main__":
    board = ox.Board(8)  # 8x8
    bots = [MCTSBot(0, 0.1), MCTSBot(1, 0.1)]

    while not board.is_terminal():
        current_player = board.current_player()
        current_player_mark = ox.MARKS_AS_CHAR[ox.PLAYER_TO_MARK[current_player]]

        current_bot = bots[current_player]
        a = current_bot.play_action(board)
        board.apply_action(a)

        print(f"{current_player_mark}: {a} -> \n{board}\n")
