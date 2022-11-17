import sys
from collections import namedtuple

# XXX: these are defined in "lexer.h"
TK_STR = 149
TK_ID = 150
BN_ES = -8
TK_EOF = -1

grammar = dict()
symbols = list()
start_sym = ""
Token = namedtuple("Token", ["type", "str_val"])
Item = namedtuple("Item", ["head", "body", "dot"])
tk = Token(0, 0)
curr_nt = ""
curr_prod = list()
tk_n = 0


def print_token(tk):
    print(tk.type, end='\t')
    if tk.type == TK_ID or tk.type == TK_STR:
        print(tk.str_val)
        return
    print(chr(tk.type))


def repr_sym(sym):
    if sym == TK_ID:
        return "TK_ID"
    if sym == TK_STR:
        return "TK_STR"
    if sym == BN_ES or sym == "":
        return "``"
    if sym == TK_EOF:
        return "$"
    try:
        rep = "`" + str(chr(sym)) + "`"
    except (TypeError, ValueError):
        rep = "<" + str(sym) + ">"
    return rep


def repr_item(item):
    rep = "[" + item.head + " ->"
    if item.dot == len(item.body):
        for sym in item.body:
            rep += " " + repr_sym(sym)
        rep += " o]"
        return rep

    for i, sym in enumerate(item.body):
        if item.dot == i:
            rep += " o"
        rep += " " + repr_sym(sym)
    rep += "]"
    return rep


def tk_gen():
    global tk_n
    for tk in sys.stdin.readlines():
        t, v = tk.split('\t', 1)
        tk_n += 1
        yield (int(t), v.rstrip())

tokens = tk_gen()


def expected(exp):
    raise Exception(
        f"on token {tk_n} \"{chr(tk.type)}\": "
        f"expected {exp}"
    )


def next_token():
    global tk
    tk = Token(*tokens.__next__())


def skip_tks(chars):
    for char in chars:
        if tk.type != ord(char):
            expected(chars)
        next_token()


def add_prod():
    global grammar
    global curr_nt
    global curr_prod

    grammar[curr_nt].append(curr_prod.copy())
    curr_prod = list()


def create_entry(key, table):
    table[key] = list()


def print_grammar():
    for nt, prods in grammar.items():
        print("<", nt, "> ::= ", sep="", end="")
        first = 1
        for prod in prods:
            if first:
                first = 0
            else:
                print("\t| ", end="")
            for sym in prod:
                print(repr_sym(sym), "", end="")
            print()
        print()


def print_first(sym):
    global first_tab
    print("FIRST(", sym, "): ", sep="")
    print("\t", [repr_sym(s) for s in first_tab[sym]])


def print_follow(nt):
    global follow_tab
    print("FOLLOW(", nt, "): ", sep="")
    print("\t", [repr_sym(s) for s in follow_tab[nt]])


def parse_prods():
    global tk
    global symbols
    global curr_nt
    global curr_prod

    more_input = 1
    while more_input:
        if tk.type == ord('|'): # new prod for curr_nt
            add_prod()
            next_token()
            continue

        elif tk.type == ord('<'): # parse nonterm
            next_token()
            if tk.type != TK_ID:
                expected("nonterm")
            nt = tk.str_val
            next_token()
            if tk.type != ord('>'):
                expected("'>'")
            try: # BN could end here
                next_token()
            except StopIteration:
                more_input = 0
            if tk.type == ord(':'): # we are in a new def
                skip_tks("::=")
                add_prod()
                curr_nt = nt # start prod for new def
                if grammar.get(curr_nt) == None:
                    create_entry(curr_nt, grammar)
                continue
            curr_prod.append(nt) # add the nonterm to curr_prod
            if nt not in symbols:
                symbols.append(nt)

        elif tk.type == ord('`'): # parse a terminal
            next_token()
            if tk.type == ord('`'): # empty string (``)
                curr_prod.append(BN_ES)
                # XXX empty string is not added to symbol list
                try: # BN could end here
                    next_token()
                except StopIteration:
                    more_input = 0
                continue
            curr_prod.append(tk.type)
            if tk.type not in symbols:
                symbols.append(tk.type)
            next_token()
            if tk.type != ord('`'):
                expected("'`'")
            try: # BN could end here
                next_token()
            except StopIteration:
                more_input = 0

        else:
            print_token(tk)
            expected("a token supported by BN syntax")

        if not more_input:
            add_prod()


def parse_bn():
    global tk
    global symbols
    global curr_nt
    global curr_prod
    global start_sym

    next_token()
    if tk.type != ord('<'):
        expected("\"<symbol>\"")
    next_token()
    if tk.type != TK_ID:
        expected("\"symbol\"")

    curr_nt = tk.str_val
    symbols.append(curr_nt)
    start_sym = curr_nt
    next_token()

    if tk.type != ord('>'):
        expected("'>'")
    next_token()

    skip_tks("::=")

    if grammar.get(curr_nt) == None:
        grammar[curr_nt] = list()
    curr_prod = list()

    parse_prods()


def augment_grammar():
    global grammar
    global start_sym

    new_start_sym = start_sym + "_s"
    create_entry(new_start_sym, grammar)
    grammar[new_start_sym].append([start_sym])
    start_sym = new_start_sym


def closure(items):
    global grammar

    clos = items.copy()

    # create a boolean array indexed
    # by the nonterminals of the grammar
    added = dict()
    for key in grammar.keys():
        added[key] = 0

    added_to_clos = 1
    while added_to_clos:
        added_to_clos = 0
        for item in clos:
            # if there is no nonterminal after the dot, continue
            if item.dot >= len(item.body) or type(item.body[item.dot]) != str:
                continue
            if added[item.body[item.dot]]:
                continue
            # this adds nonkernel items to memory
            for prod in grammar[item.body[item.dot]]:
                clos.append(
                    Item(
                        head=item.body[item.dot],
                        body=prod,
                        dot=0,
                    )
                )
            added[item.body[item.dot]] = 1
            added_to_clos = 1
    return clos


def goto(items, symbol):
    global grammar

    go = list()

    for item in items:
        if item.dot < len(item.body) and item.body[item.dot] == symbol:
            go.append(
                Item(
                    head=item.head,
                    body=item.body,
                    dot=item.dot + 1,
                )
            )
    return closure(go)


canon = list()

def compute_canon():
    global grammar
    global symbols
    global start_sym
    global canon

    # C = {CLOSURE({[S'->.S]})}
    canon = [closure([Item(head=start_sym, body=grammar[start_sym][0], dot=0)])]

    added_to_canon = 1
    while added_to_canon:
        added_to_canon = 0
        for items in canon:
            for symbol in symbols:
                go = goto(items, symbol)
                if go and go not in canon:
                    canon.append(go)
                    added_to_canon = 1


# TODO: algorithm to remove cycles and e-prods
# all nullable nonterms can be found with FIRST(nonterm)

# XXX: grammar must have no cycles or e-prods
def elim_left_rec():
    global grammar

    # list the non terms in some order
    non_terms = list()
    for nt in grammar.keys():
        non_terms.append(nt)

    for i, nt_i in enumerate(non_terms):
        # replace prods of the form nt_i -> nt_j x
        # for nt_i -> y1 x | y2 x ... | yk x
        # where x is any symbol string and yn are
        # all current nt_j productions (immediate left
        # recursion for nt_j has already been eliminated,
        # because j < i)
        for j, nt_j in enumerate(non_terms):
            if j == i:
                break
            for p_i in range(len(grammar[nt_i])):
                if grammar[nt_i][p_i][0] == nt_j:
                    prod_tail = grammar[nt_i].pop(p_i)[1:]
                    for j_prod in grammar[nt_j]:
                        grammar[nt_i].append(j_prod + prod_tail)

        # eliminate immediate left recursion for nt_i
        left_rec_prod_tails = list()
        non_left_rec_prods = list()
        for prod in grammar[nt_i]:
            if prod[0] == nt_i:
                left_rec_prod_tails.append(prod[1:])
            else:
                non_left_rec_prods.append(prod)

        if not left_rec_prod_tails:
            continue

        grammar[nt_i] = list()
        nt_i_prime = nt_i + "_1"
        create_entry(nt_i_prime, grammar)
        for nlrp in non_left_rec_prods:
            grammar[nt_i].append(nlrp + [nt_i_prime])
        for lrpt in left_rec_prod_tails:
            grammar[nt_i_prime].append(lrpt + [nt_i_prime])
        grammar[nt_i_prime].append([BN_ES])


first_tab = dict()

def first(symbol):
    global grammar
    global first_tab

    if type(symbol) == int: # symbol is a terminal
        return [symbol] # FIRST(term) = { term }

    if first_tab.get(symbol): # return FIRST(symbol) if it was already computed
        return first_tab[symbol]

    create_entry(symbol, first_tab)
    added_to_first = 1
    while added_to_first:
        added_to_first = 0
        for prod in grammar[symbol]:
            # if the first symbol in the prod is the nonterminal
            # for which we are computing first, then we check
            # if first_tab[symbol] already contains BN_ES.
            # if it does, we skip the nonterminal, if it
            # does not, we skip the production altogether
            if prod[0] == symbol:
                if BN_ES not in first_tab[symbol]:
                    continue
                prod = prod[1:]

            # add FIRST(sym) for every sym in the prod,
            # until sym cannot be reduced to the empty string
            # (BN_ES is not in first(sym))
            added_es = 1
            for sym in prod:
                if not added_es:
                    break

                added_es = 0
                for s in first(sym):
                    if s not in first_tab[symbol]:
                        first_tab[symbol].append(s)
                        added_to_first = 1
                    if s == BN_ES:
                        added_es = 1

    return first_tab[symbol]


def compute_first_tab():
    for sym in grammar.keys():
        first(sym)


def first_of_sym_string(sym_list):
    ret = list()
    all_have_es = 1
    added_es = 1
    for sym in sym_list:
        if not added_es:
            all_have_es = 0
            break
        added_es = 0
        for s in first(sym):
            if s == BN_ES:
                added_es = 1
            else:
                ret.append(s)
    # add the empty string only if every
    # symbol in the string is nullable
    if all_have_es:
        ret.append(BN_ES)
    return ret


follow_tab = dict()

def compute_follow_tab():
    global follow_tab
    global start_sym

    create_entry(start_sym, follow_tab)
    follow_tab[start_sym].append(TK_EOF)

    added_to_follow = 1
    while added_to_follow:
        added_to_follow = 0
        for nt in grammar.keys():
            for prod in grammar[nt]:
                for i, s in enumerate(prod):
                    if type(s) != str: # if s is not nonterm
                        continue

                    if follow_tab.get(s) == None:
                        create_entry(s, follow_tab)

                    # if A -> xBy add {FIRST(y) - BN_ES} to FOLLOW(B)
                    # (where x and y are strings)
                    if i < (len(prod)-1):
                        for term in first_of_sym_string(prod[i+1:]):
                            if term != BN_ES and term not in follow_tab[s]:
                                follow_tab[s].append(term)
                                added_to_follow = 1

                    # if A->xB or (A->xBy and BN_ES is in FIRST(y))
                    # then add FOLLOW(A) to FOLLOW(B)
                    if i == (len(prod)-1) or (
                            BN_ES not in first_of_sym_string(prod[i+1:])
                    ):
                        for term in follow_tab[nt]:
                            if term not in follow_tab[s]:
                                follow_tab[s].append(term)
                                added_to_follow = 1


def print_canon():
    for i, items in enumerate(canon):
        print("I_", i, sep="")
        for item in items:
            print("\t", repr_item(item), sep="")


if __name__ == "__main__":
    parse_bn()
    #elim_left_rec()
    augment_grammar()
    print_grammar()

    compute_first_tab()
    for nt in grammar.keys():
        print_first(nt)
    print()

    compute_follow_tab()
    for nt in grammar.keys():
        print_follow(nt)
    print()

    compute_canon()
    print_canon()
