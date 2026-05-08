from blockworld import BlockWorldEnv
import random


class QLearning:
    def __init__(self, env: BlockWorldEnv):
        self.env = env
        self.Q = {}
        self.alpha = 0.1
        self.gamma = 0.9
        self.epsilon = 0.05

    def train(self):
        while True:
            s, _ = self.env.reset()
            done = False

            while not done:
                self.init_q(s)
                a = self.act(s)

                s_next, r, done, _, _ = self.env.step(a)
                self.init_q(s_next)

                max_next = max(self.Q[s_next].values()) if self.Q[s_next] else 0
                self.Q[s][a] += self.alpha * (r + self.gamma * max_next - self.Q[s][a])

                s = s_next

    def act(self, s):
        self.init_q(s)
        if random.random() < self.epsilon:
            return random.choice(list(self.Q[s].keys()))
        else:
            return max(self.Q[s], key=self.Q[s].get)

    def init_q(self, s):
        if s not in self.Q:
            self.Q[s] = {a: 0.0 for a in s[0].get_actions()}


if __name__ == "__main__":
    N = 4

    env = BlockWorldEnv(N)
    qlearning = QLearning(env)

    qlearning.train()

    test_env = BlockWorldEnv(N)

    test_problems = 10
    solved = 0
    avg_steps = []

    for test_id in range(test_problems):
        s, _ = test_env.reset()
        done = False

        print(f"\nProblem {test_id}:")
        print(f"{s[0]} -> {s[1]}")

        for step in range(50):  # max 50 steps per problem
            a = qlearning.act(s)
            s_, r, done, truncated, _ = test_env.step(a)

            print(f"{a}: {s[0]}")

            s = s_

            if done:
                solved += 1
                avg_steps.append(step + 1)
                break

    avg_steps = sum(avg_steps) / len(avg_steps)
    print(f"Solved {solved}/{test_problems} problems, with average number of steps {avg_steps}.")
