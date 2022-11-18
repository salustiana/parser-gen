#include "grammar.h"
#include "lexer.h"
#include "utils.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/* TODO:
 * - Check for NULL returns from new_link, create_entry, repr_sym and strdup.
 */

struct token tk;
char *curr_head;
const char *start_sym;

struct symbol {
	int is_term;
	enum tk_type term_type;
	const char *term_name;
	const char *nt_name;
} *curr_sym;

struct sym_entry {
	struct sym_entry *next;
	const char *key;
	struct symbol *sym;
} *symbols[HASHSIZE];

struct sym_list {
	struct sym_list *next;
	struct symbol *sym;
} *curr_prod;

struct prod_list {
	struct prod_list *next;
	struct sym_list *prod;
};

struct nt_entry {
	struct nt_entry *next;
	const char *key;
	struct prod_list *prods;
} *grammar[HASHSIZE];

#define MAX_TERMLEN	8
char *repr_sym(struct symbol *sym)
{
	if (sym->is_term) {
		char *repr = malloc(MAX_TERMLEN);
		assert(repr != NULL);
		if (sym->term_type == TK_ID)
			snprintf(repr, MAX_TERMLEN, "`%s`", sym->term_name);
		else
			snprintf(repr, MAX_TERMLEN, "`%c`", sym->term_type);
		return repr;
	}
	else
		return strdup(sym->nt_name);
}

void print_prod(struct sym_list *prod)
{
	char *sym_repr;
	for (struct sym_list *sp = prod; sp != NULL; sp = sp->next) {
		sym_repr = repr_sym(sp->sym);
		printf(sp->sym->is_term ? "%s " : "<%s> ", sym_repr);
		free(sym_repr);
	}
	putchar('\n');
}

void print_prods(struct prod_list *prods)
{
	int first = 1;
	for (struct prod_list *pp = prods; pp != NULL; pp = pp->next) {
		if (pp->prod == NULL)
			panic("trying to print NULL prod");
		if (first)
			first = 0;
		else
			printf("\t| ");
		print_prod(pp->prod);
	}
	putchar('\n');
}

void print_grammar()
{
	for (int i = 0; i < HASHSIZE; i++) {
		if (grammar[i] == NULL)
			continue;
		for (struct nt_entry *ep = grammar[i]; ep != NULL; ep = ep->next) {
			if (ep->prods == NULL)
				panic("nonterminal <%s> has no productions", ep->key);
			printf("<%s> ::= ", ep->key);
			print_prods(ep->prods);
		}
	}
}

/*
 * Skips the single char tokens
 * given in tk_str and panics if one
 * of them does not match the input.
 * Example: skip_tks("::=") skips
 * the tokens TK_COLN, TK_COLN, TK_ASSIGN.
 * XXX: only works for tokens whose type is
 * given by their character representation.
 */
void skip_tks(const char *tk_str)
{
	if (*tk_str == '\0')
		return;
	do {
		if (tk.type != (enum tk_type) *tk_str)
			panic("expected %s\n", tk_str);
		next_token(&tk);
	} while (*++tk_str);
}

/*
 * Adds the curr_prod to the
 * grammar entry for curr_head
 * and sets curr_prod to NULL
 * (last element of a new linked list).
 */
void add_prod()
{
	if (curr_prod == NULL)
		panic("trying to add curr_prod when it is NULL");
	curr_prod = reverse_linked_list(curr_prod);

	struct prod_list *new_prod = malloc(sizeof(struct prod_list));
	if (new_prod == NULL)
		panic("could not allocate memory for a new production");
	new_prod->prod = curr_prod;
	struct nt_entry *cnt = look_up(curr_head, grammar);
	cnt->prods = new_link(new_prod, cnt->prods);
	curr_prod = NULL;
}

/*
 * Adds curr_sym to the curr_prod and to the
 * symbols table (if not already there), then
 * sets curr_sym to newly allocated memory.
 * XXX: the symbol is added as the first
 * element of the linked list, so curr_prod
 * is stored in reverse.
 */
void add_sym()
{
	/* add to curr_prod */
	struct sym_list *new_sym = malloc(sizeof(struct sym_list));
	new_sym->sym = curr_sym;
	curr_prod = new_link(new_sym, curr_prod);
	/* add to symbols */
	const char *sym_key = repr_sym(curr_sym);
	if (look_up(sym_key, symbols) == NULL) {
		struct sym_entry *ep = create_entry(sym_key, symbols);
		ep->sym = curr_sym;
	}
	curr_sym = malloc(sizeof(struct symbol));
}

void parse_prods()
{
	int more_input = 1;
	while (more_input) {
		switch (tk.type) {
		case TK_BAR:	/* new prod for current nonterm */
			add_prod();
			next_token(&tk);
			continue;
		case TK_LESS:	/* parse nonterm */
			next_token(&tk);
			if (tk.type != TK_ID)
				panic("expected nonterm");
			curr_sym->nt_name = strdup(tk.str_val);
			next_token(&tk);
			if (tk.type != TK_GRT)
				panic("expected '>'");
			more_input = next_token(&tk); /* BN could end here */
			if (tk.type == TK_COLN) { /* we are in a new def */
				skip_tks("::=");
				add_prod();
				/* start prod for new def */
				curr_head = strdup(curr_sym->nt_name);
				if (look_up(curr_head, grammar) == NULL) {
					struct nt_entry *ne;
					ne = create_entry(curr_head, grammar);
					ne->prods = NULL;
				}
				continue;
			}
			curr_sym->is_term = 0;
			add_sym(); /* add the nonterm to curr_prod */
			break;
		case TK_BACTK:	/* parse terminal */
			next_token(&tk);
			curr_sym->is_term = 1;
			curr_sym->term_type = tk.type;
			if (curr_sym->term_type == TK_ID)
				curr_sym->term_name = strdup(tk.str_val);
			add_sym(); /* add the term to curr_prod */
			next_token(&tk);
			if (tk.type != TK_BACTK)
				panic("expected '`'");
			more_input = next_token(&tk); /* BN could end here */
			break;
		default:
			// TODO: produce a proper error message
			print_token(tk);
			panic("token %d not supported by BN syntax", tk.type);
		}
		if (!more_input)
			add_prod();
	}
}

void parse_bn()
{
	next_token(&tk);
	skip_tks("<");
	if (tk.type != TK_ID)
		panic("expected starting nonterm");
	start_sym = strdup(tk.str_val);
	next_token(&tk);
	skip_tks(">::=");
	curr_head = strdup(start_sym);
	struct nt_entry *ne = create_entry(curr_head, grammar);
	ne->prods = NULL;
	curr_sym = malloc(sizeof(struct symbol));
	parse_prods();
}
