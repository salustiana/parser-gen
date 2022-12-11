import sys
from collections import namedtuple

from make_tab import SHIFT, REDUCE, ACCEPT

Token = namedtuple("Token", ["type", "str_val"])

tk = Token(0, 0)
tk_n = 0
stack = list()

def tk_gen():
    global tk_n
    for tk in sys.stdin.readlines():
        t, v = tk.split('\t', 1)
        tk_n += 1
        yield (int(t), v.rstrip())

tokens = tk_gen()
def next_token():
    global tk
    try:
        tk = Token(*tokens.__next__())
    except StopIteration:
        tk = Token(ord('$'), "")


def parse():
    stack.append(start_state)

    next_token()
    while True:
        s = stack[-1]
        act = action_tab[s, tk.type]
        if act[0] == SHIFT:
            stack.append(act[1])
            next_token()
        elif act[0] == REDUCE:
            for _ in range(len(act[1][1])):
                stack.pop()
            stack.append(goto_tab[stack[-1], act[1][0]])
            print(act[1][0], "->", act[1][1])
        elif act[0] == ACCEPT:
            break
        else:
            print(stack)
            print(act)
            raise Exception("Can not handle token", tk.type)


if __name__ == "__main__":
    import pickle
    with open("lalr-tab", "rb") as f:
        start_state, action_tab, goto_tab, sym_states = pickle.load(f)

    parse()
