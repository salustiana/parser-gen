""" Grammar for an arithmetic expression

	<expr> ::= <expr> `+` <term>
		    | <expr> `-` <term>
		    | <term>

	<term> ::= <term> `*` <fact>
		    | <term> `/` <fact>
		    | <fact>

	<fact> ::= `float`
		    | `(` <expr> `)`
"""

""" After eliminating left recursion

	<expr> ::= <term> <expr_1> 

	<term> ::= <fact> <term_1> 

	<fact> ::= `float` 
		    | `(` <expr> `)` 

	<expr_1> ::= `+` <term> <expr_1> 
		    | `-` <term> <expr_1> 
		    | `` 

	<term_1> ::= `*` <fact> <term_1> 
		    | `/` <fact> <term_1> 
		    | `` 
"""

look = ''
tks = list()
tk_i = 0
stack = list()
rax = 0
rbx = 0


def next_tk():
    global tks, tk_i

    if tk_i >= len(tks):
        return None

    tk = tks[tk_i]
    tk_i += 1
    return tk

def match(tk):
    global look

    if look != tk:
        raise Exception("expected `" + str(tk) + "`")
    look = next_tk()

def parse_expr(expr_str):
    global tks, look

    tks = tokenize_expr(expr_str)
    look = next_tk()
    expr()
    return rax

def expr():
    term()
    expr_1()

def term():
    fact()
    term_1()

def fact():
    global rax, rbx

    if look == '(':
        match('(')
        expr()
        match(')')
        return

    if type(look) != float:
        raise Exception("expected float")
    rax = look
    match(look)

def expr_1():
    global rax, rbx

    if look == '+':
        stack.append(rax)
        match('+')
        term()
        rbx = stack.pop()
        rax += rbx
        expr_1()
        return
    if look == '-':
        stack.append(rax)
        match('-')
        term()
        rbx = stack.pop()
        rax -= rbx
        rax = -rax
        expr_1()
        return

def term_1():
    global rax, rbx

    if look == '*':
        stack.append(rax)
        match('*')
        fact()
        rbx = stack.pop()
        rax *= rbx
        term_1()
        return
    if look == '/':
        stack.append(rax)
        match('/')
        fact()
        rbx = rax
        rax = stack.pop()
        rax /= rbx
        term_1()
        return

def tokenize_expr(expr_str):
    terms = ['+', '-', '*', '/', 'float', '(', ')']
    whitespace = [' ', '\t', '\n']

    expr_tks = list()
    curr_tk = ""
    for char in expr_str:
        if char in whitespace:
            continue

        if char in terms:
            if curr_tk:
                expr_tks.append(float(curr_tk))
                curr_tk = ""
            expr_tks.append(char)
            continue

        curr_tk += char

    # last token
    if curr_tk:
        expr_tks.append(float(curr_tk))

    return expr_tks


if __name__ == "__main__":
    import sys

    print(parse_expr(sys.argv[1]))
