#include "grammar.h"
#include "lexer.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>

/* TODO:
 * - Check for NULL returns from add_link, create_entry and strdup.
 * - Use add_link instead of manually adding links.
 * - Create a table of symbols in the grammar (to compute GOTO).
 * 	Solve: symbols being ints and strings.
 * - Symbol representation in print_prod. Solve: if terminal is
 *   	of type TK_ID, the value is lost. (change term_val to term_type
 *   	and add term_val to hold string value of terms in symbol_list?)
 */

struct token tk;
const char *start_sym;
char *curr_nt;

struct nt_entry {
	struct nt_entry *next;
	const char *key;
	struct prod_list *prods;
} *grammar[HASHSIZE];

struct symbol_list {
	struct symbol_list *next;
	int is_term;
	enum tk_type term_val;
	const char *nt_name;
} *curr_prod;

struct prod_list {
	struct prod_list *next;
	struct symbol_list *prod;
};

void print_prod(struct symbol_list *prod)
{
	for (struct symbol_list *sp = prod; sp != NULL; sp = sp->next) {
		if (sp->is_term)
			printf("%c ", sp->term_val);
		else
			printf("<%s> ", sp->nt_name);
	}
	putchar('\n');
}

void print_prods(struct prod_list *prods)
{
	int first = 1;
	for (struct prod_list *pp = prods; pp != NULL; pp = pp->next) {
		if (pp->prod == NULL)
			panic("trying to print empty prod");
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
 * Adds the curr_prod to the grammar
 * entry for curr_nt, and sets
 * curr_prod to NULL (last element
 * of the linked list).
 */
void add_prod()
{
	if (curr_prod == NULL)
		panic("curr_prod is NULL");
	curr_prod = reverse_linked_list(curr_prod);
	struct prod_list *new_prod = malloc(sizeof(struct prod_list));
	if (new_prod == NULL)
		panic("could not allocate memory for a new production");
	new_prod->prod = curr_prod;
	struct nt_entry *cnt = look_up(curr_nt, grammar);
	new_prod->next = cnt->prods;
	cnt->prods = new_prod;
	curr_prod = NULL;
}

/*
 * Adds the nonterm symbol nt to
 * curr_prod.
 * XXX: the symbol is added as the
 * first element, so curr_prod is
 * stored in reverse.
 */
void add_non_term(const char *nt)
{
	struct symbol_list *new_sym = malloc(sizeof(struct symbol_list));
	new_sym->is_term = 0;
	new_sym->nt_name = nt;
	new_sym->next = curr_prod;
	curr_prod = new_sym;
}

/*
 * Adds the terminal symbol term to
 * curr_prod.
 * XXX: the symbol is added as the
 * first element, so curr_prod is
 * stored in reverse.
 */
void add_term(enum tk_type term)
{
	struct symbol_list *new_sym = malloc(sizeof(struct symbol_list));
	new_sym->is_term = 1;
	new_sym->term_val = term;
	new_sym->next = curr_prod;
	curr_prod = new_sym;
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
			char *nt = strdup(tk.str_val);
			next_token(&tk);
			if (tk.type != TK_GRT)
				panic("expected '>'");
			more_input = next_token(&tk); /* BN could end here */
			if (tk.type == TK_COLN) { /* we are in a new def */
				skip_tks("::=");
				add_prod();
				curr_nt = nt; /* start prod for new def */
				if (look_up(curr_nt, grammar) == NULL) {
					struct nt_entry *ne;
					ne = create_entry(curr_nt, grammar);
					ne->prods = NULL;
				}
				continue;
			}
			add_non_term(nt); /* add the nonterm to curr_prod */
			break;
		case TK_BACTK:	/* parse terminal */
			next_token(&tk);
			add_term(tk.type); /* add the term to curr_prod */
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
	curr_nt = strdup(start_sym);
	struct nt_entry *ne = create_entry(curr_nt, grammar);
	ne->prods = NULL;
	parse_prods();
}
