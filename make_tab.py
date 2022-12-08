import sys
from collections import namedtuple

# These are defined in "lexer.h"
TK_STR = 149
TK_ID = 150
EOI = ord('$')
EMPTY_STR = ord('@')

SHIFT = 0
REDUCE = 1
ACCEPT = 2
ERROR = -3

Token = namedtuple("Token", ["type", "str_val"])
Item = namedtuple("Item", ["head", "body", "dot"])

productions = dict()
symbols = list()
start_sym = ""
tk = Token(0, 0)
tk_n = 0
curr_head = ""
curr_prod = list()
first_tab = dict()
follow_tab = dict()
canon = list()
goto_tab = dict()
action_tab = dict()
canon_n = 0

def repr_sym(sym):
    if sym == TK_ID:
        return "TK_ID"
    if sym == TK_STR:
        return "TK_STR"
    if sym == EOI:
        return "$"
    if sym == EMPTY_STR:
        return "EMPTY_STR"
    try:
        rep = chr(sym)
    except (TypeError, ValueError):
        rep = "<" + str(sym) + ">"
    return rep

def print_grammar():
    for h, ps in productions.items():
        print("<", h, "> ::= ", sep="", end="")
        first = 1
        for p in ps:
            if first:
                first = 0
            else:
                print("\t|  ", end="")
            for s in p:
                print(repr_sym(s), end=" ")
            print()
        print()

def print_symbols():
    print("symbols: {", end=" ")
    for s in symbols:
        print(repr_sym(s), end=", ")
    print("}")

def print_first_tab():
    for sym in symbols:
        if type(sym) == int:
            continue
        print("FIRST(", repr_sym(sym), ") = { ", sep="", end="")
        for s in first_tab[sym]:
            print(repr_sym(s), end=", ")
        print("}")

def print_follow_tab():
    for sym in symbols:
        if type(sym) == int:
            continue
        print("FOLLOW(", repr_sym(sym), ") = { ", sep="", end="")
        for s in follow_tab[sym]:
            print(repr_sym(s), end=", ")
        print("}")

def print_item(it):
    print(it.head, "-> ", end="")
    for i, s in enumerate(it.body):
        if i == it.dot:
            print(".", sep="", end="")
        print(repr_sym(s), end=" ")
    if it.dot == len(it.body):
        print(".", end="")


def print_canon():
    for i, items in enumerate(canon):
        print(f"I{i}")
        for it in items:
            print("\t", end="")
            print_item(it)
            print()
        print()

def expected(exp):
    raise Exception(
        f"on token {tk_n} \"{chr(tk.type)}\": "
        f"expected {exp}"
    )

def tk_gen():
    global tk_n
    for tk in sys.stdin.readlines():
        t, v = tk.split('\t', 1)
        tk_n += 1
        yield (int(t), v.rstrip())

tokens = tk_gen()
def next_token():
    global tk
    tk = Token(*tokens.__next__())

def skip_tks(chars):
    for char in chars:
        if tk.type != ord(char):
            expected(chars)
        next_token()

def add_sym(sym):
    curr_prod.append(sym)
    if sym not in symbols:
        symbols.append(sym)

def add_prod():
    global curr_prod

    productions[curr_head].append(curr_prod.copy())
    curr_prod = list()

def parse_prods():
    global curr_head

    more_input = 1
    while more_input:
        if tk.type == ord('|'):
            add_prod()
            next_token()
        elif tk.type == ord('<'):
            next_token()
            if tk.type != TK_ID:
                expected("nonterm")
            nt = tk.str_val
            next_token()
            if tk.type != ord('>'):
                expected(">")
            try:
                next_token()
            except StopIteration:
                more_input = 0
            if tk.type == ord(':'):
                skip_tks("::=")
                add_prod()
                curr_head = nt
                if not productions.get(curr_head):
                    productions[curr_head] = list()
            else:
                add_sym(nt)
        elif tk.type == ord('`'):
            next_token()
            if tk.type == ord('`'):
                add_sym(EMPTY_STR)
                try:
                    next_token()
                except StopIteration:
                    more_input = 0
            else:
                add_sym(tk.type)
                next_token()
                if tk.type != ord('`'):
                    expected("`")
                try:
                    next_token()
                except StopIteration:
                    more_input = 0
        else:
            expected("a valid token")

        if not more_input:
            add_prod()

def augment_grammar():
    global curr_prod, curr_head, start_sym
    curr_head = start_sym + "_s"
    if curr_head in symbols:
        raise Exception(f"{curr_head} already exists")
    symbols.append(curr_head)
    productions[curr_head] = list()
    curr_prod = list()
    add_sym(start_sym)
    add_prod()
    start_sym = curr_head

def first(sym):
    if first_tab.get(sym):
        return first_tab[sym]
    if type(sym) == int:
        first_tab[sym] = [sym]
        return first_tab[sym]

    first_tab[sym] = list()
    for prod in productions[sym]:
        if prod[0] == sym:
            if EMPTY_STR in first_tab[sym]:
                prod = prod[1:]
            else:
                continue
        added_es = True
        for sm in prod:
            if not added_es:
                break
            added_es = False
            for s in first(sm):
                if s == EMPTY_STR:
                    added_es = True
                elif s not in first_tab[sym]:
                    first_tab[sym].append(s)
        if added_es and EMPTY_STR not in first_tab[sym]:
            first_tab[sym].append(EMPTY_STR)

    return first_tab[sym]

def first_of_string(sym_str):
    fst = list()
    added_es = True
    for sym in sym_str:
        if not added_es:
            break
        added_es = False
        for s in first(sym):
            if s == EMPTY_STR:
                added_es = True
            elif s not in fst:
                fst.append(s)
    if added_es:
        fst.append(EMPTY_STR)
    return fst

def compute_first_tab():
    for sym in symbols:
        first(sym)

def compute_follow_tab():
    for sym in symbols:
        if type(sym) == int:
            continue
        follow_tab[sym] = list()

    follow_tab[start_sym] = [EOI]
    added_to_follow = True
    while added_to_follow:
        added_to_follow = False
        for head, prods in productions.items():
            for prod in prods:
                for i, sym in enumerate(prod):
                    # if A -> xBy add {FIRST(y) - EMPTY_STR} to FOLLOW(B)
                    if type(sym) == int:
                        continue
                    for s in first_of_string(prod[i+1:]):
                        if s == EMPTY_STR:
                            continue
                        if s not in follow_tab[sym]:
                            follow_tab[sym].append(s)
                            added_to_follow = True
                    # if A -> xB or (A -> xBy and EMPTY_STR in FIRST(y))
                    # add FOLLOW(A) to FOLLOW(B)
                    if not prod[i:] or EMPTY_STR in first_of_string(prod[i+1:]):
                        for s in follow_tab[head]:
                            if s not in follow_tab[sym]:
                                follow_tab[sym].append(s)
                                added_to_follow = True


def closure(items):
    clos = items.copy()

    added_nts = list()
    added_to_clos = True
    while added_to_clos:
        added_to_clos = False
        for it in clos:
            # if A -> x.By is in clos, add B -> .z to clos
            # for every production B -> z
            if (
                it.dot < len(it.body)
                and type(it.body[it.dot]) == str
                and it.body[it.dot] not in added_nts
            ):
                for prod in productions[it.body[it.dot]]:
                    clos.append(
                        Item(head=it.body[it.dot], body=prod, dot=0)
                    )
                added_nts.append(it.body[it.dot])
                added_to_clos = True
    return clos

def goto(items, sym):
    go = list()
    for it in items:
        if it.dot < len(it.body) and it.body[it.dot] == sym:
            go.append(
                Item(head=it.head, body=it.body, dot=it.dot+1)
            )
    return closure(go)

def compute_canon_and_goto_tab():
    global canon_n

    if canon:
        raise Exception("canon is not empty")
    start_prods = productions[start_sym]
    if len(start_prods) != 1:
        raise Exception("start_sym has more than one prod")
    canon.append(
        closure([Item(head=start_sym, body=start_prods[0], dot=0)])
    )

    added_to_canon = True
    while added_to_canon:
        added_to_canon = False
        for i, items in enumerate(canon):
            for sym in symbols:
                gt = goto(items, sym)
                if not gt:
                    continue
                try:
                    gti = canon.index(gt)
                    goto_tab[i, sym] = gti
                except ValueError:
                    goto_tab[i, sym] = canon_n
                    canon.append(gt)
                    added_to_canon = True
    canon_n = len(canon)

    for i in range(canon_n):
        for nt in symbols:
            if type(nt) != str:
                continue
            if not goto_tab.get((i, nt)):
                goto_tab[i, nt] = ERROR

def compute_action_tab():
    for i, items in enumerate(canon):
        for it in items:
            if it.dot > len(it.body):
                raise Exception("Item dot is beyond bounds")
            # if [A -> x.ty] is in CANON[i] and GOTO[i, t] = j, then
            # set ACTION[i, t] to (SHIFT, j). t is a terminal.
            if it.dot < len(it.body):
                t = it.body[it.dot]
                if type(t) != int:
                    continue
                act = (SHIFT, goto_tab[i, t])
                # check for conflicts
                if (curr := action_tab.get((i, t))) and curr != act:
                    raise Exception(f"Conflict for action_tab[{i}, {t}]")
                action_tab[i, t] = act
                continue
            # if [S' -> S.] is in CANON[i], then set ACTION[i, $] to ACCEPT
            if it.head == start_sym and it.dot == 1:
                if (curr := action_tab.get((i, ord('$')))) and curr!=(ACCEPT, ):
                    raise Exception(f"Conflict for action_tab[{i}, $]")
                action_tab[i, ord('$')] = (ACCEPT, )
                continue
            # if [A -> x.] is in CANON[i], then set ACTION[i, t] to
            # (REDUCE, A -> x) for all t in FOLLOW(A).
            act = (REDUCE, (it.head, it.body))
            for t in follow_tab[it.head]:
                if (curr := action_tab.get((i, t))) and curr != act:
                    raise Exception(f"Conflict for action_tab[{i}, {t}]")
                action_tab[i, t] = act

    for i in range(canon_n):
        for t in symbols:
            if type(t) != int:
                continue
            if not action_tab.get((i, t)):
                action_tab[i, t] = (ERROR, )

def parse_bn():
    global start_sym, curr_head
    next_token()
    skip_tks("<")
    if tk.type != TK_ID:
        expected("starting nonterm")
    start_sym = tk.str_val
    next_token()
    skip_tks(">::=")
    curr_head = start_sym
    symbols.append(curr_head)
    productions[curr_head] = list()
    parse_prods()

if __name__ == "__main__":
    parse_bn()
    augment_grammar()
    compute_first_tab()
    compute_follow_tab()
    compute_canon_and_goto_tab()
    compute_action_tab()

    import pickle
    with open("slr-tab", "wb") as f:
        pickle.dump((symbols, start_sym, canon_n, action_tab, goto_tab), f)
