#include "../grammar.c"

#include <stdio.h>
#include <string.h>

struct symbol *_sym;

void test_repr_sym()
{
	_sym = malloc(sizeof(struct symbol));
	_sym->is_term = 1;
	_sym->term_type = TK_LBRCE;
	assert(strcmp(repr_sym(_sym), "`{`") == 0);

	_sym->is_term = 1;
	_sym->term_type = TK_ID;
	_sym->term_name = "term";
	assert(strcmp(repr_sym(_sym), "`term`") == 0);

	_sym->is_term = 0;
	_sym->nt_name = "nonterm";
	assert(strcmp(repr_sym(_sym), "nonterm") == 0);
}

void test_add_sym()
{
	for (int i = 0; i < HASHSIZE; i++)
		symbols[i] = NULL;
	curr_prod = malloc(sizeof(struct sym_list));
	curr_prod->next = NULL;
	curr_prod->sym = NULL;

	curr_sym = malloc(sizeof(struct symbol));

	curr_sym->is_term = 0;
	curr_sym->nt_name = "nt1";
	assert(look_up(repr_sym(curr_sym), symbols) == NULL);
	add_sym();

	curr_sym->is_term = 1;
	curr_sym->term_type = TK_LSHFT;
	assert(look_up(repr_sym(curr_sym), symbols) == NULL);
	add_sym();

	curr_sym->is_term = 0;
	curr_sym->nt_name = "nt2";
	assert(look_up(repr_sym(curr_sym), symbols) == NULL);
	add_sym();

	assert(look_up("nt2", symbols) != NULL);
	// TODO: assert(look_up("`<<`", symbols) != NULL);
	assert(look_up("nt1", symbols) != NULL);

	struct sym_list *sp = curr_prod;
	assert(strcmp(sp->sym->nt_name, "nt2") == 0);
	sp = sp->next;
	assert(sp->sym->term_type == TK_LSHFT);
	sp = sp->next;
	assert(strcmp(sp->sym->nt_name, "nt1") == 0);
}

void test_add_prod()
{
	for (int i = 0; i < HASHSIZE; i++)
		productions[i] = NULL;

	curr_prod = NULL;

	curr_head = "head";
	assert(look_up(curr_head, productions) == NULL);
	create_entry(curr_head, productions);
	assert(look_up(curr_head, productions) != NULL);

	curr_sym = malloc(sizeof(struct symbol));

	curr_sym->is_term = 0;
	curr_sym->nt_name = "nonterm";
	add_sym();

	curr_sym->is_term = 1;
	curr_sym->term_type = TK_CMPL;
	add_sym();

	curr_sym->is_term = 1;
	curr_sym->term_type = TK_STR;
	add_sym();

	add_prod();

	curr_sym->is_term = 1;
	curr_sym->term_type = EMPTY_STR;
	add_sym();

	add_prod();

	struct prod_head_entry *ep = look_up(curr_head, productions);
	assert(ep != NULL);

	struct prod_list *pp;
	struct sym_list *sp;

	/* assert that first prod is [``] */
	pp = ep->prods;
	sp = pp->prod;
	assert(sp != NULL);

	assert(sp->sym->is_term == 1);
	assert(strcmp(repr_sym(sp->sym), "``") == 0);
	assert(sp->next == NULL);

	/* assert that second (and last) prod is [nonterm `~` TK_STR] */
	pp = pp->next;
	assert(pp != NULL);
	assert(pp->next == NULL);
	sp = pp->prod;
	assert(sp != NULL);

	assert(sp->sym->is_term == 0);
	assert(strcmp(sp->sym->nt_name, "nonterm") == 0);
	sp = sp->next;
	assert(sp->sym->is_term == 1);
	assert(sp->sym->term_type == TK_CMPL);
	sp = sp->next;
	assert(sp->sym->is_term == 1);
	assert(sp->sym->term_type == TK_STR);
	assert(sp->next == NULL);
}

void test_grammar()
{
	test_repr_sym();
	printf("test_repr_sym() passed\n");
	test_add_sym();
	printf("test_add_sym() passed\n");
	test_add_prod();
	printf("test_add_prod() passed\n");
}
