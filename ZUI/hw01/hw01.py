from blockworld import BlockWorld
import heapq


class BlockWorldHeuristic(BlockWorld):
    def __init__(self, num_blocks=5, state=None):
        BlockWorld.__init__(self, num_blocks, state)

    def heuristic(self, goal):
        self_state = self.get_state()
        goal_state = goal.get_state()

        h = 0.0
        for goal_stack in goal_state:
            bottom_block = goal_stack[-1]
            curr_stack = self.get_stack(bottom_block, self_state)
            pref = self.correct_prefix(goal_stack, curr_stack)
            h += len(goal_stack) - len(pref)

        return h

    def get_stack(self, block, state):
        for stack in state:
            if block in stack:
                return stack

        return None

    def correct_prefix(self, goal_stack, curr_stack):
        pref = []

        for correct, real in zip(reversed(goal_stack), reversed(curr_stack)):
            if correct == real:
                pref.append(correct)
            else:
                break

        return pref


class AStar:
    def search(self, start: BlockWorldHeuristic, goal: BlockWorldHeuristic):
        opened: list[tuple[float, BlockWorldHeuristic]] = []
        heapq.heappush(opened, (0, start))

        cost = {start: 0}
        parent = {start: None}
        action_taken = {start: None}

        while opened:
            f, curr = heapq.heappop(opened)

            if f > cost[curr] + curr.heuristic(goal):
                continue

            if curr == goal:
                break

            for action, neighbor in curr.get_neighbors():
                new_cost = cost[curr] + 1
                if new_cost < cost.get(neighbor, float("inf")):
                    cost[neighbor] = new_cost
                    parent[neighbor] = curr
                    action_taken[neighbor] = action
                    f = new_cost + neighbor.heuristic(goal)
                    heapq.heappush(opened, (f, neighbor))

        path = []
        curr = goal
        while parent[curr] is not None:
            path.append(action_taken[curr])
            curr = parent[curr]
        path.reverse()

        return path


if __name__ == "__main__":
    N = 5

    start = BlockWorldHeuristic(N)
    goal = BlockWorldHeuristic(N)

    print("Searching for a path:")
    print(f"{start} -> {goal}")
    print()

    astar = AStar()
    path = astar.search(start, goal)

    if path is not None:
        print("Found a path:")
        print(path)

        print("\nHere's how it goes:")

        s = start.clone()
        print(s)

        for a in path:
            s.apply(a)
            print(s)

    else:
        print("No path exists.")

    print("Total expanded nodes:", BlockWorld.expanded)
