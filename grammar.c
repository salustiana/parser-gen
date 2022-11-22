#include "grammar.h"
#include "lexer.h"
#include "utils.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct token tk;
const char *curr_head;
const char *start_sym;

struct symbol *curr_sym, es_sym = {1, EMPTY_STR, NULL};

struct sym_list *curr_prod, *nts_in_grammar, *first_of_term[TK_TYPE_COUNT];

struct prod_head_entry *productions[HASHSIZE];

struct sym_list_entry {
	struct sym_list_entry *next;
	const char *key;
	struct sym_list *sl;
} *first_of_nt[HASHSIZE], *follow_tab[HASHSIZE];

int term_in_grammar[TK_TYPE_COUNT];

void init_grammar()
{
	curr_head = start_sym = NULL;
	curr_sym = malloc(sizeof(struct symbol));
	es_sym = (struct symbol) {1, EMPTY_STR, NULL};
	curr_prod = nts_in_grammar = NULL;
	for (size_t i = 0; i < TK_TYPE_COUNT; i++) {
		first_of_term[i] = NULL;
		term_in_grammar[i] = 0;
	}
	for (size_t i = 0; i < HASHSIZE; i++) {
		productions[i] = NULL;
		first_of_nt[i] = NULL;
	}
}

int sym_in_sym_list(struct symbol *sym, struct sym_list *sl)
{
	assert(sym != NULL);
	if (sym->is_term) {
		for (; sl != NULL; sl = sl->next) {
			assert(sl->sym != NULL);
			if (!sl->sym->is_term)
				continue;
			if (sl->sym->term_type == sym->term_type)
				return 1;
		}
		return 0;
	}
	for (; sl != NULL; sl = sl->next) {
		assert(sl->sym != NULL);
		if (sl->sym->is_term)
			continue;
		assert(sl->sym->nt_name != NULL);
		if (strcmp(sl->sym->nt_name, sym->nt_name) == 0)
			return 1;
	}
	return 0;
}

#define MAX_TERMLEN	8
char *repr_sym(struct symbol *sym)
{
	char *repr;
	if (!sym->is_term) {
		repr = strdup(sym->nt_name);
		assert(repr != NULL);
		return repr;
	}

	if (sym->term_type == EMPTY_STR) {
		repr = strdup("``");
		assert(repr != NULL);
		return repr;
	}

	repr = malloc(MAX_TERMLEN);
	assert(repr != NULL);
	switch (sym->term_type) {
		/* TODO: handle missing cases */
		case TK_ID:
			snprintf(repr, MAX_TERMLEN, "`TK_ID`");
			break;
		default:
			snprintf(repr, MAX_TERMLEN, "`%c`", sym->term_type);
	}
	return repr;
}

void print_sym_list(struct sym_list *sl)
{
	char *sym_repr;
	for (struct sym_list *sp = sl; sp != NULL; sp = sp->next) {
		assert(sp->sym != NULL);
		sym_repr = repr_sym(sp->sym);
		printf(sp->sym->is_term ? "%s " : "<%s> ", sym_repr);
		free(sym_repr);
	}
}

void print_prods(struct prod_list *prods)
{
	int first = 1;
	for (struct prod_list *pp = prods; pp != NULL; pp = pp->next) {
		assert(pp->prod != NULL);
		if (first)
			first = 0;
		else
			printf("\t| ");
		print_sym_list(pp->prod);
		putchar('\n');
	}
}

void print_grammar()
{
	for (size_t i = 0; i < HASHSIZE; i++) {
		if (productions[i] == NULL)
			continue;
		struct prod_head_entry *ep = productions[i];
		for (; ep != NULL; ep = ep->next) {
			assert(ep->prods != NULL);
			printf("<%s> ::= ", ep->key);
			print_prods(ep->prods);
			putchar('\n');
		}
	}
}

void print_first_tab()
{
	struct sym_list *nts = nts_in_grammar;
	for (; nts != NULL; nts = nts->next) {
		printf("FIRST(<%s>) = { ", nts->sym->nt_name);
		struct sym_list_entry *e;
		e = look_up(nts->sym->nt_name, first_of_nt);
		print_sym_list(e->sl);
		printf("}\n");
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
	assert(curr_prod != NULL);
	curr_prod = reverse_linked_list(curr_prod);

	struct prod_list *new_prod = malloc(sizeof(struct prod_list));
	assert(new_prod != NULL);
	new_prod->prod = curr_prod;
	struct prod_head_entry *cnt = look_up(curr_head, productions);
	assert(cnt != NULL);
	cnt->prods = new_link(new_prod, cnt->prods);
	curr_prod = NULL;
}

/*
 * Adds curr_sym to the curr_prod.
 * If curr_sym was a terminal, it sets
 * term_in_grammar[curr_sym.term_type] = 1.
 * Finally, it sets curr_sym to newly
 * allocated memory.
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

	if (curr_sym->is_term)
		term_in_grammar[curr_sym->term_type] = 1;

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
		break;
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
			assert(curr_prod == NULL);
			curr_head = strdup(curr_sym->nt_name);
			if (look_up(curr_head, productions) == NULL) {
				struct prod_head_entry *ne;
				ne = create_entry(curr_head, productions);
				ne->prods = NULL;
			}
			break;
		}
		curr_sym->is_term = 0;
		add_sym(); /* add the nonterm to curr_prod */
		break;
	case TK_BACTK:	/* parse terminal */
		next_token(&tk);
		curr_sym->is_term = 1;
		if (tk.type == TK_BACTK) { /* parse empty string */
			curr_sym->term_type = EMPTY_STR;
			add_sym();
			/* BN could end here */
			more_input = next_token(&tk);
			break;
		}
		curr_sym->term_type = tk.type;
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

void augment_grammar()
{
	assert(start_sym != NULL);
	assert(*start_sym != '\0');

	curr_prod = NULL;
	curr_head = extended_str(start_sym, "_s");
	struct prod_head_entry *ne;
	ne = create_entry(curr_head, productions);
	ne->prods = NULL;

	assert(curr_sym != NULL);
	curr_sym->is_term = 0;
	curr_sym->nt_name = strdup(start_sym);
	add_sym();
	add_prod();
}

void fill_first_of_term_tab()
{
	for (size_t i = 0; i < TK_TYPE_COUNT; i++) {
		if (!term_in_grammar[i]) {
			first_of_term[i] = NULL;
			continue;
		}
		struct symbol *t = malloc(sizeof(struct symbol));
		t->is_term = 1;
		t->term_type = (enum tk_type) i;
		struct sym_list *sl = malloc(sizeof(struct sym_list));
		sl->next = NULL;
		sl->sym = t;
		first_of_term[i] = sl;
	}
}

struct sym_list *first(struct symbol *sym)
{
	/* FIRST(term) = {term} */
	if (sym->is_term) {
		assert(term_in_grammar[sym->term_type]);
		assert(first_of_term[sym->term_type] != NULL);
		return first_of_term[sym->term_type];
	}
	/* return FIRST(nt) if it had already been computed */
	struct sym_list_entry *fnte;
	if ((fnte = look_up(sym->nt_name, first_of_nt)) != NULL) {
		assert(fnte->sl != NULL);
		return fnte->sl;
	}
	fnte = create_entry(sym->nt_name, first_of_nt);
	fnte->sl = NULL;

	int added_to_first = 1;
	while (added_to_first) {

	added_to_first = 0;
	/* walk over every prod for sym */
	struct prod_head_entry *phe;
	phe = look_up(sym->nt_name, productions);
	assert(phe != NULL);
	struct prod_list *prdp = phe->prods;
	assert(prdp != NULL);
	for (; prdp != NULL; prdp = prdp->next) {
		struct sym_list *prod = prdp->prod;
		assert(prod != NULL);
		assert(prod->sym != NULL);
		/* if the prod is left-recursive, check
		 * if FIRST(sym) already contains EMPTY_STR.
		 * if it does, skip sym in the prod; if it
		 * does not, skip the prod altogether.
		 */
		if (!sym->is_term && !prod->sym->is_term) {
			assert(sym->nt_name != NULL);
			assert(prod->sym->nt_name != NULL);
			if (strcmp(prod->sym->nt_name, sym->nt_name) == 0) {
				struct sym_list_entry *e;
				e = look_up(sym->nt_name, first_of_nt);
				if (e == NULL || !sym_in_sym_list(&es_sym,
							e->sl))
					continue;
				prod = prod->next; /* skip sym in prod */
				if (prod == NULL)
					panic("redundant production %s ::= %s",
						sym->nt_name, sym->nt_name);
			}
		}
		/* add FIRST(s) for every s in the prod, stop when
		 * FIRST(s) does not contain the EMPTY_STR (when s
		 * cannot be reduced to the empty string).
		 */
		int added_es = 1;
		for (; prod != NULL; prod = prod->next) {
			if (!added_es)
				break;
			added_es = 0;
			struct sym_list *fsl = first(prod->sym);
			assert(fsl != NULL);
			for (; fsl != NULL; fsl = fsl->next) {
				if (!sym_in_sym_list(fsl->sym, fnte->sl)) {
					struct sym_list *sl;
					sl = malloc(sizeof(struct sym_list));
					sl->sym = fsl->sym;
					fnte->sl = new_link(sl,
							fnte->sl);
					added_to_first = 1;
				}
				if (fsl->sym->is_term &&
						fsl->sym->term_type == EMPTY_STR)
					added_es = 1;
			}
		}
	}

	}
	return fnte->sl;
}

void fill_nts_in_grammar_list()
{
	for (size_t i = 0; i < HASHSIZE; i++) {
		if (productions[i] == NULL)
			continue;
		struct prod_head_entry *phe = productions[i];
		for (; phe != NULL; phe = phe->next) {
			struct symbol *nt = malloc(sizeof(struct symbol));
			nt->is_term = 0;
			nt->nt_name = phe->key;
			struct sym_list *ntl = malloc(sizeof(struct sym_list));
			ntl->sym = nt;

			nts_in_grammar = new_link(ntl, nts_in_grammar);
		}
	}
}

void compute_first_tab()
{
	fill_first_of_term_tab();
	fill_nts_in_grammar_list();
	struct sym_list *nts = nts_in_grammar;
	assert(nts != NULL);
	for (; nts != NULL; nts = nts->next)
		first(nts->sym);
}

struct sym_list *first_of_sym_list(struct sym_list *sl)
{
	struct sym_list *f = NULL;
	int all_have_es = 1;
	int had_es = 1;
	for (; sl != NULL; sl = sl->next) {
		assert(sl->sym != NULL);
		if (!had_es) {
			all_have_es = 0;
			break;
		}
		had_es = 0;
		struct sym_list *sf = first(sl->sym);
		assert(sf != NULL);
		for (; sf != NULL; sf = sf->next) {
			struct symbol *s = sf->sym;
			assert(s != NULL);
			if (s->is_term && s->term_type == EMPTY_STR) {
				had_es = 1;
				continue;
			}
			if (!sym_in_sym_list(s, f)) {
				struct sym_list *slnk;
				slnk = malloc(sizeof(struct sym_list));
				slnk->sym = s;
				f = new_link(slnk, f);
			}
		}
	}
	/* add the empty string only if every symbol in
	 * the sym_list is nullable.
	 */
	if (all_have_es) {
		struct sym_list *slnk = malloc(sizeof(struct sym_list));
		slnk->sym = &es_sym;
		f = new_link(slnk, f);
	}
	return f;
}

void parse_bn()
{
	init_grammar();
	next_token(&tk);
	skip_tks("<");
	if (tk.type != TK_ID)
		panic("expected starting nonterm");
	start_sym = strdup(tk.str_val);
	next_token(&tk);
	skip_tks(">::=");
	curr_head = strdup(start_sym);
	struct prod_head_entry *ne = create_entry(curr_head, productions);
	ne->prods = NULL;
	parse_prods();
	augment_grammar();
	compute_first_tab();
}
