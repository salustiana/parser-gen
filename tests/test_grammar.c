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
	assert(strcmp(repr_sym(_sym), "`TK_ID`") == 0);

	_sym->is_term = 0;
	_sym->nt_name = "nonterm";
	assert(strcmp(repr_sym(_sym), "nonterm") == 0);

	printf("%s passed\n", __func__);
}

void test_add_sym()
{
	for (size_t i = 0; i < TK_TYPE_COUNT; i++)
		term_in_grammar[i] = 0;
	curr_prod = malloc(sizeof(struct sym_list));
	curr_prod->next = NULL;
	curr_prod->sym = NULL;

	curr_sym = malloc(sizeof(struct symbol));

	curr_sym->is_term = 0;
	curr_sym->nt_name = "nt1";
	add_sym();

	curr_sym->is_term = 1;
	curr_sym->term_type = TK_LSHFT;
	assert(!term_in_grammar[TK_LSHFT]);
	add_sym();
	assert(term_in_grammar[TK_LSHFT]);

	curr_sym->is_term = 0;
	curr_sym->nt_name = "nt2";
	add_sym();

	struct sym_list *sp = curr_prod;
	assert(strcmp(sp->sym->nt_name, "nt2") == 0);
	sp = sp->next;
	assert(sp->sym->term_type == TK_LSHFT);
	sp = sp->next;
	assert(strcmp(sp->sym->nt_name, "nt1") == 0);

	printf("%s passed\n", __func__);
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

	printf("%s passed\n", __func__);
}

void test_fill_first_of_term_tab()
{
	for (int i = 0; i < HASHSIZE; i++)
		productions[i] = NULL;
	for (size_t i = 0; i < TK_TYPE_COUNT; i++)
		term_in_grammar[i] = 0;

	curr_prod = NULL;

	curr_head = "head";
	create_entry(curr_head, productions);

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

	fill_first_of_term_tab();

	for (size_t i = 0; i < TK_TYPE_COUNT; i++) {
		switch (i) {
			case TK_CMPL: case TK_STR: case EMPTY_STR:
				assert(first_of_term[i] != NULL);
				assert(first_of_term[i]->next == NULL);
				assert(first_of_term[i]->sym != NULL);
				assert(first_of_term[i]->sym->is_term);
				assert(first_of_term[i]->sym->term_type == i);
				break;
			default:
				assert(first_of_term[i] == NULL);
		}
	}

	printf("%s passed\n", __func__);
}

void test_grammar()
{
	test_repr_sym();
	test_add_sym();
	test_add_prod();
	test_fill_first_of_term_tab();
}
