import sys
from collections import namedtuple

# These are defined in "lexer.h"
TK_STR = 149
TK_ID = 150
EOI = ord('$')
EMPTY_STR = ord('@')
NG = ord('`') # a symbol not present in the grammar

SHIFT = 0
REDUCE = 1
ACCEPT = 2
ERROR = -3

Token = namedtuple("Token", ["type", "str_val"])
LR0Item = namedtuple("Item", ["head", "body", "dot"])
Item = namedtuple("Item", ["head", "body", "dot", "look"])

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
canon_kerns = list()
look_tab = dict()
lalr_set = list()
lalr_set_n = 0
goto_tab = dict()
action_tab = dict()
canon_n = 0
start_lr0_item = None
start_state = 0

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
        rep = str(sym)
    return rep

def print_grammar():
    print("GRAMMAR")
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
    print("FIRST_TAB")
    for sym in symbols:
        if type(sym) == int:
            continue
        print("FIRST(", repr_sym(sym), ") = { ", sep="", end="")
        for s in first_tab[sym]:
            print(repr_sym(s), end=", ")
        print("}")

def print_follow_tab():
    print("FOLLOW_TAB")
    for sym in symbols:
        if type(sym) == int:
            continue
        print("FOLLOW(", repr_sym(sym), ") = { ", sep="", end="")
        for s in follow_tab[sym]:
            print(repr_sym(s), end=", ")
        print("}")

def print_lr0_item(it):
    print("[", it.head, "-> ", end="")
    for i, s in enumerate(it.body):
        if i == it.dot:
            print(".", sep="", end="")
        print(repr_sym(s), end=" ")
    if it.dot == len(it.body):
        print(".", end="")
    print("]", end="")

def print_item(it):
    print("[", it.head, "-> ", end="")
    for i, s in enumerate(it.body):
        if i == it.dot:
            print(".", sep="", end="")
        print(repr_sym(s), end=" ")
    if it.dot == len(it.body):
        print(".", end="")
    print(",", chr(it.look), "]", end="")


def print_canon():
    print("CANON")
    for i, items in enumerate(canon):
        print(f"I{i}")
        for it in items:
            print("\t", end="")
            print_lr0_item(it)
            print()
        print()

def print_canon_kerns():
    print("CANON_KERNS")
    for i, items in enumerate(canon_kerns):
        print(f"I{i}")
        for it in items:
            print("\t", end="")
            print_lr0_item(it)
            print()
        print()

def print_look_tab():
    print("LOOK_TAB")
    for k, kern in enumerate(canon_kerns):
        print(k)
        for i in range(len(kern)):
            print("\t", end="")
            print_lr0_item(canon_kerns[k][i])
            print([chr(lk) for lk in look_tab[k, i]])

def print_lalr_set():
    print("LALR_SET")
    for i, items in enumerate(lalr_set):
        print(f"I{i}")
        for it in items:
            print("\t", end="")
            print_item(it)
            print()
        print()

def print_goto_tab():
    print("GOTO")
    for i in range(canon_n):
        print("\t", i)
        for nt in symbols:
            if type(nt) != str or nt == start_sym:
                continue
            print("\t\t", nt, "\t", goto_tab[i, nt])

def print_action_tab():
    print("ACTION")
    for i in range(canon_n):
        print("\t", i)
        for t in symbols:
            if type(t) != int:
                continue
            print("\t\t", repr_sym(t), "\t", action_tab[i, t])

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
    try:
        tk = Token(*tokens.__next__())
        return True
    except StopIteration:
        return False

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

    more_input = True
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
            more_input = next_token()
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
                more_input = next_token()
            else:
                add_sym(tk.type)
                next_token()
                if tk.type != ord('`'):
                    expected("`")
                more_input = next_token()
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

def lr0_closure(lr0_items):
    clos = lr0_items.copy()

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
                        LR0Item(head=it.body[it.dot], body=prod, dot=0)
                    )
                added_nts.append(it.body[it.dot])
                added_to_clos = True
    return clos

def lr0_goto(lr0_items, sym):
    go = list()
    for it in lr0_items:
        if it.dot < len(it.body) and it.body[it.dot] == sym:
            go.append(
                LR0Item(head=it.head, body=it.body, dot=it.dot+1)
            )
    return lr0_closure(go)

def closure(items):
    clos = items.copy()

    added_to_clos = True
    while added_to_clos:
        added_to_clos = False
        for it in clos:
            if it.dot > len(it.body):
                raise Exception("Item dot is beyond bounds")
            if it.dot == len(it.body) or type(it.body[it.dot]) != str:
                continue
            for prod in productions[it.body[it.dot]]:
                for t in first_of_string(it.body[it.dot+1:] + [it.look]):
                    if type(t) != int:
                        continue
                    nit = Item(head=it.body[it.dot], body=prod, dot=0, look=t)
                    if nit in clos:
                        continue
                    clos.append(nit)
                    added_to_clos = True
    return clos

def goto(items, sym):
    go = list()
    for it in items:
        if it.dot > len(it.body):
            raise Exception("Item dot is beyond bounds")
        if it.dot == len(it.body) or it.body[it.dot] != sym:
            continue
        go.append(
            Item(head=it.head, body=it.body, dot=it.dot+1, look=it.look)
        )
    return closure(go)

def compute_canon():
    global canon_n, start_state, start_lr0_item

    if canon:
        raise Exception("canon is not empty")
    start_prods = productions[start_sym]
    if len(start_prods) != 1:
        raise Exception("start_sym has more than one prod")
    start_lr0_item = LR0Item(head=start_sym, body=start_prods[0], dot=0)
    canon.append(lr0_closure([start_lr0_item]))
    start_state = 0

    added_to_canon = True
    while added_to_canon:
        added_to_canon = False
        for items in canon:
            for sym in symbols:
                gt = lr0_goto(items, sym)
                if not gt or gt in canon:
                    continue
                canon.append(gt)
                added_to_canon = True
    canon_n = len(canon)

def compute_canon_kerns():
    if canon_kerns:
        raise Exception("canon_kerns is not empty")
    for items in canon:
        canon_kerns.append([
            it for it in items
            if not (it.dot == 0 and it.head != start_sym)
        ])

def determine_lookaheads(kernel, sym):
    spont_gen = dict()
    propagate = dict()
    for i, it in enumerate(kernel):
        j = closure([Item(head=it.head, body=it.body, dot=it.dot, look=NG)])
        for im in j:
            if im.dot == len(im.body) or im.body[im.dot] != sym:
                continue
            if im.look == NG:
                # conclude that lookaheads propagate from it in kernel to im in GOTO(I, sym)
                if not propagate.get(i):
                    propagate[i] = list()
                propagate[i].append(im)
                continue
            # conclude that lookahead im.look is generated spontaneously for item im in GOTO(I, sym)
            if not spont_gen.get(im.look):
                spont_gen[im.look] = list()
            spont_gen[im.look].append(im)
    return spont_gen, propagate

def get_ck_index(item, from_k, from_sym):
    gt = lr0_goto(canon[from_k], from_sym)
    k = -1
    for i, items in enumerate(canon):
        in_canon = True
        for it in gt:
            if it not in items:
                in_canon = False
                break
        if in_canon:
            if k >= 0:
                raise Exception("goto found more than once in canon")
            k = i
            break
    if k < 0:
        raise Exception("goto not found in canon")

    item = LR0Item(head=item.head, body=item.body, dot=item.dot+1)
    i = -1
    for cki, it in enumerate(canon_kerns[k]):
        if item == it:
            if i >= 0:
                raise Exception(f"item found more than once in canon_kerns")
            i = cki
            continue
    if i < 0:
        raise Exception(f"item not found in canon_kerns")

    return (k, i)

def compute_look_tab():
    compute_canon_kerns()

    propagate = dict()
    for k, kern in enumerate(canon_kerns):
        for i in range(len(kern)):
            look_tab[k, i] = list()
            propagate[k, i] = list()

    si_i = -1
    for i, item in enumerate(canon_kerns[start_state]):
        if start_lr0_item == item:
            if si_i >= 0:
                raise Exception(
                    "start_lr0_item found more than once " \
                    "in canon_kerns[start_state]"
                )
            si_i = i
    if si_i < 0:
        raise Exception("start_lr0_item not found in canon_kerns[start_state]")
    look_tab[start_state, si_i].append(EOI)

    for k, kern in enumerate(canon_kerns):
        for sym in symbols:
            sp_gen, prop = determine_lookaheads(kern, sym)
            for lk, items in sp_gen.items():
                for it in items:
                    look_tab[get_ck_index(it, k, sym)].append(lk)
            for from_i, tos in prop.items():
                propagate[k, from_i] += [get_ck_index(to, k, sym) for to in tos]

    added_to_look = True
    while added_to_look:
        added_to_look = False
        for k, kern in enumerate(canon_kerns):
            for i in range(len(kern)):
                for to_i in propagate[k, i]:
                    for lk in look_tab[k, i]:
                        if lk not in look_tab[to_i]:
                            look_tab[to_i].append(lk)
                            added_to_look = True

def compute_lalr_set():
    global lalr_set_n

    if lalr_set:
        raise Exception("lalr_set is not empty")
    lalr_set_n = 0
    for k, kern in enumerate(canon_kerns):
        if lalr_set_n != k:
            raise Exception("index mismatch while computing lalr_set")
        lalr_kern = list()
        for i, it in enumerate(kern):
            for lk in look_tab[k, i]:
                lalr_kern.append(
                    Item(head=it.head, body=it.body, dot=it.dot, look=lk)
                )
        lalr_set.append(closure(lalr_kern))
        lalr_set_n += 1

def compute_goto_tab():
    if goto_tab:
        raise Exception("goto_tab is not empty")
    for i, items in enumerate(lalr_set):
        for sym in symbols:
            gt = goto(items, sym)
            if not gt:
                goto_tab[i, sym] = ERROR
                continue
            already_in_lalr = False
            for _i, _items in enumerate(lalr_set):
                in_lalr = True
                for it in gt:
                    if it not in _items:
                        in_lalr = False
                        break
                if in_lalr:
                    if already_in_lalr:
                        raise Exception(
                            "goto found more than once in lalr_set"
                        )
                    goto_tab[i, sym] = _i
                    already_in_lalr = True
                    break
            if already_in_lalr:
                continue
            goto_tab[i, sym] = ERROR

def compute_action_tab():
    for i, items in enumerate(lalr_set):
        for it in items:
            if it.dot > len(it.body):
                raise Exception("Item dot is beyond bounds")
            # if [A -> x.ty, b] is in LALR_SET[i] and GOTO[i, t] = j, then
            # set ACTION[i, t] to (SHIFT, j). t and b are terminals.
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
            # if [S' -> S., $] is in LALR_SET[i], then set ACTION[i, $] to ACCEPT
            if it.head == start_sym and it.look == EOI:
                if (curr := action_tab.get((i, EOI))) and curr!=(ACCEPT, ):
                    raise Exception(f"Conflict for action_tab[{i}, $]")
                action_tab[i, EOI] = (ACCEPT, )
                continue
            # if [A -> x., t] is in LALR_SET[i], then set ACTION[i, t] to
            # (REDUCE, A -> x).
            act = (REDUCE, (it.head, it.body))
            if (curr := action_tab.get((i, it.look))) and curr != act:
                raise Exception(f"Conflict for action_tab[{i}, {it.look}]")
            action_tab[i, it.look] = act

    for i in range(lalr_set_n):
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
    compute_canon()
    compute_look_tab()
    compute_lalr_set()
    compute_goto_tab()
    compute_action_tab()

    print_action_tab()
    print_goto_tab()

    import pickle
    with open("lalr-tab", "wb") as f:
        pickle.dump((start_state, action_tab, goto_tab), f)
