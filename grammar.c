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

struct symbol *curr_sym, es_sym = {1,EMPTY_STR,NULL}, eoi_sym = {1,EOI,NULL};
struct symbol *make_symbol(int is_term, enum tk_type term_type,
					const char *nt_name) {
	struct symbol *s = malloc(sizeof(struct symbol));
	s->is_term = is_term;
	s->term_type = term_type;
	s->nt_name = nt_name;
	return s;
}

struct sym_list *add_sym_to_list(struct symbol *sym, struct sym_list **slp) {
	struct sym_list *slnk = malloc(sizeof(struct sym_list));
	slnk->sym = sym;
	ADD_LINK(slnk, *slp);
	return slnk;
}

struct sym_list *curr_prod, *nts_in_grammar, *first_of_term[TK_TYPE_COUNT];

/* If an item is of the form [ A -> xB.y ], then
 * `body` points to the whole production (`xBy` sym_list),
 * and `dot` points to the sym_list remaining after the
 * dot (`y` sym_list).
 */
struct item {
	const char *head;
	struct sym_list *body;
	struct sym_list *dot;
};

struct item *make_item(const char *head, struct sym_list *body,
						struct sym_list *dot) {
	struct item *it = malloc(sizeof(struct item));
	it->head = head;
	it->body = body;
	it->dot = dot;
	return it;
}

struct goto_nt_rule_entry {
	struct goto_nt_rule_entry *next;
	const char *key;
	struct itm_list *canon_itm;
};

struct itm_list {
	struct itm_list *next;
	struct item *itm;
	struct goto_nt_rule_entry *gt_nt_rs[HASHSIZE];
	struct itm_list *gt_term_rs[TK_TYPE_COUNT];
};

struct itm_list *add_itm_to_list(struct item *itm, struct itm_list **il) {
	struct itm_list *ilnk = malloc(sizeof(struct itm_list));
	ilnk->itm = itm;
	ADD_LINK(ilnk, *il);
	return ilnk;
}

struct itm_list_list {
	struct itm_list_list *next;
	struct itm_list *il;
} *canon_set;

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
	curr_sym = make_symbol(0, 0, NULL);
	es_sym = (struct symbol) {1, EMPTY_STR, NULL};
	curr_prod = nts_in_grammar = NULL;
	for (size_t i = 0; i < TK_TYPE_COUNT; i++) {
		first_of_term[i] = NULL;
		term_in_grammar[i] = 0;
	}
	for (size_t i = 0; i < HASHSIZE; i++) {
		productions[i] = NULL;
		first_of_nt[i] = NULL;
		follow_tab[i] = NULL;
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

/*
 * Compares items in il with itm. If any of them
 * matches, returns 1, otherwise returns 0.
 * XXX: items are compared by checking if the
 * body pointers and dot pointers are equal.
 * It does not check for "deep copies".
 */
int itm_in_itm_list(struct item *itm, struct itm_list *il)
{
	assert(itm != NULL);
	for (; il != NULL; il = il->next) {
		assert(il->itm != NULL);
		if (il->itm->body != itm->body)
			continue;
		if (il->itm->dot != itm->dot)
			continue;
		if (strcmp(il->itm->head, itm->head) != 0)
			continue;
		return 1;
	}
	return 0;
}

#define MAX_TERMLEN	8
char *repr_sym(struct symbol *sym)
{
	assert(sym != NULL);
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

void print_item(struct item *itm) {
	assert(itm != NULL);
	printf("[ %s -> ", itm->head);
	struct sym_list *bp = itm->body;
	struct sym_list *dp = itm->dot;
	assert(bp != NULL);
	char *sym_repr;
	for (; bp != dp; bp = bp->next) {
		sym_repr = repr_sym(bp->sym);
		printf("%s ", sym_repr);
	}
	putchar('.');
	for (; dp != NULL; dp = dp->next) {
		sym_repr = repr_sym(dp->sym);
		printf("%s ", sym_repr);
	}
	putchar(']');
	free(sym_repr);
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
		LOOK_UP(e, nts->sym->nt_name, first_of_nt);
		assert(e != NULL);
		print_sym_list(e->sl);
		printf("}\n");
	}
}

void print_follow_tab()
{
	struct sym_list *nts = nts_in_grammar;
	for (; nts != NULL; nts = nts->next) {
		printf("FOLLOW(<%s>) = { ", nts->sym->nt_name);
		struct sym_list_entry *e;
		LOOK_UP(e, nts->sym->nt_name, follow_tab);
		assert(e != NULL);
		print_sym_list(e->sl);
		printf("}\n");
	}
}

void print_itm_list_goto_rules(struct itm_list *itml)
{
	for (enum tk_type tt = 0; tt < TK_TYPE_COUNT; tt++) {
		void *term_rule = itml->gt_term_rs[tt];
		if (term_rule == NULL)
			continue;
		printf("GOTO(%p, %d) = %p\n", (void *) itml, tt, term_rule);
	}
	struct goto_nt_rule_entry *gntre;
	struct sym_list *nts = nts_in_grammar;
	for (; nts != NULL; nts = nts->next) {
		LOOK_UP(gntre, nts->sym->nt_name, itml->gt_nt_rs);
		if (gntre == NULL)
			continue;
		printf("GOTO(%p, %s) = %p\n", (void *) itml,
				nts->sym->nt_name, (void *) gntre->canon_itm);
	}
}

void print_canon_set()
{
	struct itm_list_list *c = canon_set;
	for (size_t i = 0; c != NULL; c = c->next, i++) {
		printf("I%zu (%p) = {\n", i, (void *) c->il);
		for (struct itm_list *il = c->il; il != NULL; il = il->next) {
			putchar('\t');
			print_item(il->itm);
			printf(",\n");
		}
		printf("}\n");
		printf("GOTO RULES FOR I%zu:\n", i);
		print_itm_list_goto_rules(c->il);
		putchar('\n');
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
	struct prod_head_entry *cnt;
	LOOK_UP(cnt, curr_head, productions);
	assert(cnt != NULL);
	ADD_LINK(new_prod, cnt->prods);
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
	add_sym_to_list(curr_sym, &curr_prod);

	if (curr_sym->is_term)
		term_in_grammar[curr_sym->term_type] = 1;

	curr_sym = make_symbol(0, 0, NULL);
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
			struct prod_head_entry *ne;
			LOOK_UP(ne, curr_head, productions);
			if (ne == NULL) {
				ne = malloc(sizeof(struct prod_head_entry));
				INSERT_ENTRY(ne, curr_head, productions);
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
	struct prod_head_entry *ne = malloc(sizeof(struct prod_head_entry));
	INSERT_ENTRY(ne, curr_head, productions);
	ne->prods = NULL;

	assert(curr_sym != NULL);
	curr_sym->is_term = 0;
	curr_sym->nt_name = strdup(start_sym);
	add_sym();
	add_prod();
	start_sym = curr_head;
}

void fill_first_of_term_tab()
{
	for (enum tk_type tt = 0; tt < TK_TYPE_COUNT; tt++) {
		if (!term_in_grammar[tt]) {
			first_of_term[tt] = NULL;
			continue;
		}
		struct symbol *t = make_symbol(1, tt, NULL);
		first_of_term[tt] = NULL;
		add_sym_to_list(t, &first_of_term[tt]);
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
	LOOK_UP(fnte, sym->nt_name, first_of_nt);
	if (fnte != NULL) {
		assert(fnte->sl != NULL);
		return fnte->sl;
	}
	fnte = malloc(sizeof(struct sym_list_entry));
	INSERT_ENTRY(fnte, sym->nt_name, first_of_nt);
	fnte->sl = NULL;

	int added_to_first = 1;
	while (added_to_first) {

	added_to_first = 0;
	/* walk over every prod for sym */
	struct prod_head_entry *phe;
	LOOK_UP(phe, sym->nt_name, productions);
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
				LOOK_UP(e, sym->nt_name, first_of_nt);
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
					add_sym_to_list(fsl->sym, &fnte->sl);
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
			struct symbol *nt = make_symbol(0, 0, phe->key);
			add_sym_to_list(nt, &nts_in_grammar);
		}
	}
}

void compute_first_tab()
{
	fill_first_of_term_tab();
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
			if (!sym_in_sym_list(s, f))
				add_sym_to_list(s, &f);
		}
	}
	/* add the empty string only if every symbol in
	 * the sym_list is nullable.
	 */
	if (all_have_es)
		add_sym_to_list(&es_sym, &f);
	return f;
}

void compute_follow_tab()
{
	struct sym_list_entry *ssfe;
	LOOK_UP(ssfe, start_sym, follow_tab);
	assert(ssfe == NULL);
	ssfe = malloc(sizeof(struct sym_list_entry));
	INSERT_ENTRY(ssfe, start_sym, follow_tab);
	ssfe->sl = NULL;
	/* place end of input marker (EOI) into FOLLOW(start_symbol) */
	add_sym_to_list(&eoi_sym, &ssfe->sl);

	/* until nothing can be added to follow */
	int added_to_follow = 1;
	while (added_to_follow) {

	added_to_follow = 0;
	struct sym_list *ntl = nts_in_grammar;
	assert(ntl != NULL);

	/* for every nonterminal */
	for (; ntl != NULL; ntl = ntl->next) {

	struct prod_head_entry *phe;
	LOOK_UP(phe, ntl->sym->nt_name, productions);
	struct prod_list *prdp = phe->prods;
	assert(prdp != NULL);
	for (; prdp != NULL; prdp = prdp->next) {
		struct sym_list *prod = prdp->prod;
		assert(prod != NULL);
		for (; prod != NULL; prod = prod->next) {
			struct symbol *s = prod->sym;
			assert(s != NULL);
			if (s->is_term)
				continue;
			struct sym_list_entry *sfle;
			LOOK_UP(sfle, s->nt_name, follow_tab);
			if (sfle == NULL) {
				sfle = malloc(sizeof(struct sym_list_entry));
				INSERT_ENTRY(sfle, s->nt_name, follow_tab);
			}
			/* if A -> xBy add {FIRST(y) - EMPTY_STR} to
			 * FOLLOW(B) (where x and y are sym strings).
			 */
			if (prod->next != NULL) { /* if y follows B */
				struct sym_list *strf; /* FIRST(y) */
				strf = first_of_sym_list(prod->next);
				for (; strf != NULL; strf = strf->next) {
					assert(strf->sym->is_term);
					if (strf->sym->term_type == EMPTY_STR
						|| sym_in_sym_list(strf->sym,
								sfle->sl))
						continue;
					struct sym_list *slnk;
					slnk = malloc(sizeof(*slnk));
					slnk->sym = strf->sym;
					ADD_LINK(slnk, sfle->sl);
					added_to_follow = 1;
				}
			}
			/* if A->xB of (A->xBy and FIRST(y) has EMPTY_STR)
			 * then add FOLLOW(A) to FOLLOW(B).
			 */
			if (prod->next == NULL ||
					!sym_in_sym_list(&es_sym,
					first_of_sym_list(prod->next))) {
				struct sym_list_entry *phfe; /* FOLLOW(A) */
				LOOK_UP(phfe, ntl->sym->nt_name, follow_tab);
				if (phfe == NULL) {
					phfe = malloc(sizeof(*phfe));
					INSERT_ENTRY(phfe, ntl->sym->nt_name,
								follow_tab);
					phfe->sl = NULL;
					continue;
				}
				struct sym_list *phf = phfe->sl;
				for (; phf != NULL; phf = phf->next) {
					if (sym_in_sym_list(phf->sym, sfle->sl))
						continue;
					struct sym_list *slnk;
					slnk = malloc(sizeof(*slnk));
					slnk->sym = phf->sym;
					ADD_LINK(slnk, sfle->sl);
					added_to_follow = 1;
				}
			}
		}
	}

	}

	}
}

struct itm_list *closure(struct itm_list *il)
{
	struct itm_list *clos = NULL;
	/* add every item in il to clos */
	for (; il != NULL; il = il->next) {
		/* assert that il has no repeated items (is a set) */
		assert(!itm_in_itm_list(il->itm, clos));
		add_itm_to_list(il->itm, &clos);
	}

	struct sym_list *added_nts = NULL;
	int added_to_clos = 1;
	while (added_to_clos) {

	added_to_clos = 0;
	for (struct itm_list *c = clos; c != NULL; c = c->next) {
		struct item *itm = c->itm;
		/* if itm is of the form [ A -> x.By ]
		 * (where A, B are nonterms and x, y are symbol strings),
		 * then add B -> .z to clos for every production B -> z
		 * in the grammar.
		 */
		if (itm->dot == NULL || itm->dot->sym->is_term)
			continue;
		/* since adding one item of the form [ B -> .g ] implies
		 * adding every item [ B -> .z ] for every B production
		 * B -> z, then it is enough to check if B is in added_nts
		 * to know if we should skip adding B items altoghether.
		 */
		if (sym_in_sym_list(itm->dot->sym, added_nts))
			continue;
		struct symbol *nt = itm->dot->sym;
		struct prod_head_entry *phe;
		LOOK_UP(phe, nt->nt_name, productions);
		assert(phe != NULL);
		struct prod_list *prods = phe->prods;
		assert(prods != NULL);
		for (; prods != NULL; prods = prods->next) {
			struct sym_list *prod = prods->prod;
			struct item *nitm = make_item(nt->nt_name, prod, prod);
			assert(!itm_in_itm_list(nitm, clos));
			add_itm_to_list(nitm, &clos);
			added_to_clos = 1;
		}
		add_sym_to_list(itm->dot->sym, &added_nts);
	}

	}

	return clos;
}

struct itm_list *go_to(struct itm_list *il, struct symbol *sym)
{
	struct itm_list *g = NULL;
	/* for every item of the form [ A -> x.By ] in il add
	 * [ A -> xB.y ] to g (where B is sym).
	 */
	if (sym->is_term) {
		for (; il != NULL; il = il->next) {
			if (il->itm->dot == NULL)
				continue;
			struct symbol *sfd = il->itm->dot->sym;
			if (!sfd->is_term || sfd->term_type != sym->term_type)
				continue;
			struct item *nit = make_item(il->itm->head,
					il->itm->body, il->itm->dot->next);
			assert(!itm_in_itm_list(nit, g));
			add_itm_to_list(nit, &g);
		}
		return closure(g);
	}
	for (; il != NULL; il = il->next) {
		if (il->itm->dot == NULL)
			continue;
		struct symbol *sfd = il->itm->dot->sym;
		if (sfd->is_term || strcmp(sfd->nt_name, sym->nt_name) != 0)
			continue;
		struct item *nit = make_item(il->itm->head, il->itm->body,
							il->itm->dot->next);
		assert(!itm_in_itm_list(nit, g));
		add_itm_to_list(nit, &g);
	}
	return closure(g);
}

struct itm_list *find_itm_list_in_canon_set(struct itm_list *itml)
{
	struct itm_list *in_canon;
	struct itm_list_list *c = canon_set;

	for (; c != NULL; c = c->next) {
		in_canon = c->il;
		struct itm_list *il = itml;
		for (; il != NULL; il = il->next) {
			if (!itm_in_itm_list(il->itm, c->il)) {
				in_canon = NULL;
				break;
			}
		}
		if (in_canon != NULL)
			break;
	}
	return in_canon;
}

int add_goto_to_canon_set(struct itm_list *itml, struct symbol *sym)
{
	struct itm_list *go = go_to(itml, sym);
	if (go == NULL)
		return 0;

	struct itm_list *in_canon;
	if (sym->is_term) {
		in_canon = find_itm_list_in_canon_set(go);
		if (in_canon != NULL) {
			/* if go is already in canon_set, set the itml
			 * goto rule for this term to the existing
			 * itm_list in canon_set.
			 */
			itml->gt_term_rs[sym->term_type] = in_canon;
			return 0;
		}
		/* if not found, add the computed goto to
		 * the canon_set.
		 */
		struct itm_list_list *illnk;
		illnk = malloc(sizeof(struct itm_list_list));
		illnk->il = go;
		ADD_LINK(illnk, canon_set);
		/* set the itml goto rule for this term to
		 * the newly added goto in canon_set.
		 */
		itml->gt_term_rs[sym->term_type] = go;
		return 1;
	}
	in_canon = find_itm_list_in_canon_set(go);
	if (in_canon != NULL) {
		/* if go is already in canon_set, set the itml
		 * goto_rule for this nonterm to the existing
		 * itm_list in canon_set.
		 */
		struct goto_nt_rule_entry *gntre;
		LOOK_UP(gntre, sym->nt_name, itml->gt_nt_rs);
		if (gntre != NULL)
			return 0;
		gntre = malloc(sizeof(struct goto_nt_rule_entry));
		gntre->canon_itm = in_canon;
		INSERT_ENTRY(gntre, sym->nt_name, itml->gt_nt_rs);
		return 0;
	}
	/* if not found, add the computed goto to the canon_set */
	struct itm_list_list *illnk;
	illnk = malloc(sizeof(struct itm_list_list));
	illnk->il = go;
	ADD_LINK(illnk, canon_set);
	/* set the itml goto rule for this nonterm to
	 * the newly added goto in canon_set.
	 */
	struct goto_nt_rule_entry *gntre;
	LOOK_UP(gntre, sym->nt_name, itml->gt_nt_rs);
	if (gntre != NULL)
		return 0;
	gntre = malloc(sizeof(struct goto_nt_rule_entry));
	gntre->canon_itm = go;
	INSERT_ENTRY(gntre, sym->nt_name, itml->gt_nt_rs);
	return 1;
}

void compute_canon_set()
{
	canon_set = NULL;
	/* add CLOSURE({ [S'->.S] }) to canon_set */
	struct prod_head_entry *sphe;
	LOOK_UP(sphe, start_sym, productions);
	assert(sphe != NULL);
	assert(sphe->prods->next == NULL);
	struct item *si = make_item(start_sym, sphe->prods->prod,
						sphe->prods->prod);
	struct itm_list *sil = NULL;
	add_itm_to_list(si, &sil);
	struct itm_list_list *sill = malloc(sizeof(struct itm_list_list));
	sill->il = closure(sil);
	ADD_LINK(sill, canon_set);

	int added_to_canon = 1;
	while (added_to_canon) {

	added_to_canon = 0;

	struct itm_list_list *canon = canon_set;
	for (; canon != NULL; canon = canon->next) {
		for (enum tk_type tt = 0; tt < TK_TYPE_COUNT; tt++) {
			if (!term_in_grammar[tt])
				continue;
			struct symbol *sym = make_symbol(1, tt, NULL);
			if (add_goto_to_canon_set(canon->il, sym))
				added_to_canon = 1;
		}
		struct sym_list *nts = nts_in_grammar;
		for (; nts != NULL; nts = nts->next) {
			if (add_goto_to_canon_set(canon->il, nts->sym))
				added_to_canon = 1;
		}
	}

	}
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
	struct prod_head_entry *ne = malloc(sizeof(struct prod_head_entry));
	INSERT_ENTRY(ne, curr_head, productions);
	ne->prods = NULL;
	parse_prods();
	augment_grammar();
	fill_nts_in_grammar_list();
	compute_first_tab();
	compute_follow_tab();
	compute_canon_set();
}
