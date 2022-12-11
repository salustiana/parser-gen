from collections import namedtuple
import sys
from typing import Union

from make_tab import SHIFT, REDUCE, ACCEPT

Token = namedtuple("Token", ["type", "val"])

tk = Token(0, 0)
tk_n = 0
stack: list[tuple[int, Union[int, str]]] = list()
start_state: int
goto_tab: dict[tuple[int, Union[int, str]], int]
state_to_sym: dict[int, Union[int, str]]
rax: float = 0
rbx: float = 0

def tk_gen():
    global tk_n
    for tk in sys.stdin.readlines():
        t, v = tk.split('\t', 1)
        t = int(t)
        v = v.rstrip()
        try:
            v = int(v)
        except ValueError:
            pass
        tk_n += 1
        yield (t, v)

tokens = tk_gen()
def next_token():
    global tk
    try:
        tk = Token(*tokens.__next__())
    except StopIteration:
        tk = Token(ord('$'), "")

def shift_action(tt):
    global rax, rbx
    if tt not in [ord('('), ord(')')]:
        rbx = rax

def reduce_action(reduction):
    global rax, rbx

    to, frm = reduction
    if to == "fact":
        if frm == (ord('('), "expr", ord(')')):
            pass
        elif type(frm[0]) == int and len(frm) == 1:
            rax = int(stack[-1][1])
        else:
            raise Exception("invalid production for fact")
    if to == "term":
        if frm == ("term", ord('*'), "fact"):
            rax *= rbx
        elif frm == ("term", ord('/'), "fact"):
            rax = rbx / rax
        elif frm[0] == "fact" and len(frm) == 1:
            pass
        else:
            raise Exception("invalid production for term")
    if to == "expr":
        if frm == ("expr", ord('+'), "term"):
            rax += rbx
        elif frm == ("expr", ord('-'), "term"):
            rax = rbx - rax
        elif frm[0] == "term" and len(frm) == 1:
            pass
        else:
            raise Exception("invalid production for expr")


def parse():
    stack.append((start_state, 0))

    next_token()
    while True:
        s = stack[-1][0]
        act = action_tab[s, tk.type]
        if act[0] == SHIFT:
            shift_action(tk.type)
            stack.append((act[1], tk.val))
            next_token()
        elif act[0] == REDUCE:
            reduce_action(act[1])
            for _ in range(len(act[1][1])):
                stack.pop()
            stack.append((goto_tab[stack[-1][0], act[1][0]], "unset"))
            #print(act[1][0], "->", [repr_sym(s) for s in act[1][1]])
        elif act[0] == ACCEPT:
            print(rax)
            break
        else:
            print(stack)
            print(act)
            raise Exception("Can not handle token", tk.type)


if __name__ == "__main__":
    import pickle
    with open("lalr-tab", "rb") as f:
        start_state, action_tab, goto_tab, state_to_sym = pickle.load(f)

    parse()
