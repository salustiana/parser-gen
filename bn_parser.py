import sys
from collections import namedtuple

# XXX: these are defined in "lexer.h"
TK_STR = 149
TK_ID = 150

grammar = dict()
Token = namedtuple("Token", ["type", "val"])
tk = Token(0, 0)
curr_nt = ""
curr_prod = list()
tk_n = 0

def print_token(tk):
    print(tk.type, end='\t')
    if tk.type == TK_ID or tk.type == TK_STR:
        print(tk.val)
        return
    print(chr(tk.type))

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

def skip_chars(chars):
    for char in chars:
        if tk.type != ord(char):
            expected(chars)
        next_token()

def parse_prods():
    global tk
    global curr_nt
    global curr_prod

    while True:
        if tk.type == ord('|'): # new prod for curr_nt
            grammar[curr_nt].append(curr_prod.copy())
            curr_prod = list()
            next_token()
            continue

        if tk.type == ord('<'): # parse <symbol>
            next_token()
            if tk.type != TK_ID:
                expected("\"symbol\"")
            nt = tk.val
            next_token()

            if tk.type != ord('>'):
                expected("'>'")
            next_token()

            if tk.type == ord(':'): # we are in a new def
                skip_chars("::=")
                # save curr_prod for this nonterm
                grammar[curr_nt].append(curr_prod.copy())

                curr_nt = nt # start prod for newly defined nonterm
                if grammar.get(curr_nt) == None:
                    grammar[curr_nt] = list()
                curr_prod = list()
                continue

            curr_prod.append(nt) # add symbol to current prod

        if tk.type == TK_STR: # parse literal symbol
            curr_prod.append(tk.val)
            next_token()
            continue

def parse_bn():
    global tk
    global curr_nt
    global curr_prod

    next_token()
    if tk.type != ord('<'):
        expected("\"<symbol>\"")
    next_token()
    if tk.type != TK_ID:
        expected("\"symbol\"")

    curr_nt = tk.val
    next_token()

    if tk.type != ord('>'):
        expected("'>'")
    next_token()

    skip_chars("::=")

    if grammar.get(curr_nt) == None:
        grammar[curr_nt] = list()
    curr_prod = list()

    try:
        parse_prods()
    except StopIteration:
        return

    

if __name__ == "__main__":
    parse_bn()
    for nt, prods in grammar.items():
        print("<", nt, ">", sep="")
        for prod in prods:
            print("\t", prod, sep="")
