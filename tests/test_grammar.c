#include "../grammar.c"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

void test_sym_in_sym_list()
{
	struct symbol *s;
	struct sym_list *list = NULL;

	s = make_symbol(1, TK_INT, NULL);
	assert(!sym_in_sym_list(s, list));
	add_sym_to_list(s, &list);
	assert(sym_in_sym_list(s, list));

	s = make_symbol(1, TK_GRT, NULL);
	assert(!sym_in_sym_list(s, list));
	add_sym_to_list(s, &list);
	assert(sym_in_sym_list(s, list));

	s = make_symbol(0, 0, "nt1");
	assert(!sym_in_sym_list(s, list));
	add_sym_to_list(s, &list);
	assert(sym_in_sym_list(s, list));

	s = make_symbol(0, 0, "nt2");
	assert(!sym_in_sym_list(s, list));
	add_sym_to_list(s, &list);
	assert(sym_in_sym_list(s, list));

	printf("%s passed\n", __func__);
}

void test_repr_sym()
{
	struct symbol *s = make_symbol(1, TK_LBRCE, NULL);
	assert(strcmp(repr_sym(s), "`{`") == 0);

	s->is_term = 1;
	s->term_type = TK_ID;
	assert(strcmp(repr_sym(s), "`TK_ID`") == 0);

	s->is_term = 0;
	s->nt_name = "nonterm";
	assert(strcmp(repr_sym(s), "nonterm") == 0);

	printf("%s passed\n", __func__);
}

void test_add_sym()
{
	init_grammar();

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
	init_grammar();

	curr_head = "head";
	struct prod_head_entry *phe;
	LOOK_UP(phe, curr_head, productions);
	assert(phe == NULL);
	struct prod_head_entry *_e = malloc(sizeof(struct prod_head_entry));
	INSERT_ENTRY(_e, curr_head, productions);
	LOOK_UP(phe, curr_head, productions);
	assert(phe != NULL);

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

	struct prod_head_entry *ep;
	LOOK_UP(ep, curr_head, productions);
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
	init_grammar();

	curr_head = "head";
	struct prod_head_entry *_e = malloc(sizeof(struct prod_head_entry));
	INSERT_ENTRY(_e, curr_head, productions);

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

void test_first_for_terms()
{
	init_grammar();

	curr_head = "head";
	struct prod_head_entry *_e = malloc(sizeof(struct prod_head_entry));
	INSERT_ENTRY(_e, curr_head, productions);

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

	curr_sym->is_term = 1;
	curr_sym->term_type = EMPTY_STR;
	assert(first(curr_sym) != NULL);
	assert(first(curr_sym)->next == NULL);
	assert(first(curr_sym)->sym != NULL);
	assert(first(curr_sym)->sym->is_term);
	assert(first(curr_sym)->sym->term_type == EMPTY_STR);

	curr_sym->is_term = 1;
	curr_sym->term_type = TK_CMPL;
	assert(first(curr_sym) != NULL);
	assert(first(curr_sym)->next == NULL);
	assert(first(curr_sym)->sym != NULL);
	assert(first(curr_sym)->sym->is_term);
	assert(first(curr_sym)->sym->term_type == TK_CMPL);

	curr_sym->is_term = 1;
	curr_sym->term_type = TK_STR;
	assert(first(curr_sym) != NULL);
	assert(first(curr_sym)->next == NULL);
	assert(first(curr_sym)->sym != NULL);
	assert(first(curr_sym)->sym->is_term);
	assert(first(curr_sym)->sym->term_type == TK_STR);

	printf("%s passed\n", __func__);
}

void test_fill_nts_in_grammar_list()
{
	init_grammar();

	curr_head = "head";
	struct prod_head_entry *_e = malloc(sizeof(struct prod_head_entry));
	INSERT_ENTRY(_e, curr_head, productions);

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

	assert(nts_in_grammar == NULL);
	fill_nts_in_grammar_list();
	assert(nts_in_grammar != NULL);
	struct symbol *s = make_symbol(0, 0, "head");
	assert(sym_in_sym_list(s, nts_in_grammar));

	printf("%s passed\n", __func__);
}

void test_compute_first_tab()
{
	init_lexer("./tests/arith_expr.bn");
	init_grammar();
	parse_bn();

	struct sym_list_entry *fnte;
	struct sym_list *f;
	struct symbol *s = malloc(sizeof(struct symbol));

	/* assert FIRST(expr) has 2 elements: { `(` and `TK_ID` } */
	LOOK_UP(fnte, "expr", first_of_nt);
	assert(fnte != NULL);
	f = fnte->sl;
	assert(f != NULL);
	assert(f->next != NULL);
	assert(f->next->next == NULL);
	s->is_term = 1;
	s->term_type = TK_ID;
	assert(sym_in_sym_list(s, f));
	s->term_type = TK_LPAR;
	assert(sym_in_sym_list(s, f));

	/* assert FIRST(term) has 2 elements: { `(` and `TK_ID` } */
	LOOK_UP(fnte, "term", first_of_nt);
	assert(fnte != NULL);
	f = fnte->sl;
	assert(f != NULL);
	assert(f->next != NULL);
	assert(f->next->next == NULL);
	s->is_term = 1;
	s->term_type = TK_ID;
	assert(sym_in_sym_list(s, f));
	s->term_type = TK_LPAR;
	assert(sym_in_sym_list(s, f));

	/* assert FIRST(fact) has 2 elements: { `(` and `TK_ID` } */
	LOOK_UP(fnte, "fact", first_of_nt);
	assert(fnte != NULL);
	f = fnte->sl;
	assert(f != NULL);
	assert(f->next != NULL);
	assert(f->next->next == NULL);
	s->is_term = 1;
	s->term_type = TK_ID;
	assert(sym_in_sym_list(s, f));
	s->term_type = TK_LPAR;
	assert(sym_in_sym_list(s, f));

	printf("%s passed\n", __func__);
}

void test_first_of_sym_list()
{
	init_lexer("./tests/arith_expr.bn");
	init_grammar();
	parse_bn();


	/* first_of_sym_list("expr"->"fact"->TK_RPAR) == { `(`, `TK_ID` } */

	struct sym_list *sl = NULL;
	struct symbol *s = make_symbol(1, TK_RPAR, NULL);
	add_sym_to_list(s, &sl);
	s = make_symbol(0, 0, "fact");
	add_sym_to_list(s, &sl);
	s = make_symbol(0, 0, "expr");
	add_sym_to_list(s, &sl);
	struct sym_list *fosl = first_of_sym_list(sl);
	assert(fosl != NULL);
	assert(fosl->next != NULL);
	assert(fosl->next->next == NULL);

	s = make_symbol(1, TK_LPAR, NULL);
	assert(sym_in_sym_list(s, fosl));

	s->term_type = TK_ID;
	assert(sym_in_sym_list(s, fosl));

	s->term_type = TK_RPAR;
	assert(!sym_in_sym_list(s, fosl));

	s->term_type = TK_ASTK;
	assert(!sym_in_sym_list(s, fosl));

	/* first_of_sym_list(TK_RPAR->"expr") == { `)` } */
	s = make_symbol(0, 0, "expr");
	add_sym_to_list(s, &sl);
	s = make_symbol(1, TK_RPAR, "fact");
	add_sym_to_list(s, &sl);
	fosl = first_of_sym_list(sl);
	assert(fosl != NULL);
	assert(fosl->next == NULL);

	s->is_term = 1;
	s->term_type = TK_RPAR;
	assert(sym_in_sym_list(s, fosl));

	s->term_type = TK_LPAR;
	assert(!sym_in_sym_list(s, fosl));

	s->term_type = TK_ID;
	assert(!sym_in_sym_list(s, fosl));

	s->term_type = TK_ASTK;
	assert(!sym_in_sym_list(s, fosl));

	printf("%s passed\n", __func__);
}

void test_compute_follow_tab()
{
	init_lexer("./tests/arith_expr.bn");
	init_grammar();
	parse_bn();
	struct sym_list_entry *fle;
	struct sym_list *f;
	struct symbol *s = malloc(sizeof(struct symbol));

	/* assert FOLLOW(expr) has 4 element: { `+`, `-`, `)`, `$` } */
	LOOK_UP(fle, "expr", follow_tab);
	assert(fle != NULL);
	f = fle->sl;
	assert(f != NULL);
	assert(f->next != NULL);
	assert(f->next->next != NULL);
	assert(f->next->next->next != NULL);
	assert(f->next->next->next->next == NULL);
	s->is_term = 1;
	s->term_type = TK_PLUS;
	assert(sym_in_sym_list(s, f));
	s->term_type = TK_MINUS;
	assert(sym_in_sym_list(s, f));
	s->term_type = TK_RPAR;
	assert(sym_in_sym_list(s, f));
	s->term_type = EOI;
	assert(sym_in_sym_list(s, f));

	/* assert FOLLOW(term) has 6 element:
	 * { `+`, `-`, `*`, `/`, `)`, `$` }
	 */
	LOOK_UP(fle, "term", follow_tab);
	assert(fle != NULL);
	f = fle->sl;
	assert(f != NULL);
	assert(f->next != NULL);
	assert(f->next->next != NULL);
	assert(f->next->next->next != NULL);
	assert(f->next->next->next->next != NULL);
	assert(f->next->next->next->next->next != NULL);
	assert(f->next->next->next->next->next->next == NULL);
	s->is_term = 1;
	s->term_type = TK_ASTK;
	assert(sym_in_sym_list(s, f));
	s->term_type = TK_DIV;
	assert(sym_in_sym_list(s, f));
	s->term_type = TK_RPAR;
	assert(sym_in_sym_list(s, f));
	s->term_type = EOI;
	assert(sym_in_sym_list(s, f));

	printf("%s passed\n", __func__);
}

void test_parse_bn()
{
	init_lexer("./tests/arith_expr.bn");
	parse_bn();

	assert(term_in_grammar[TK_PLUS]);
	assert(term_in_grammar[TK_MINUS]);
	assert(term_in_grammar[TK_ASTK]);
	assert(term_in_grammar[TK_DIV]);
	assert(term_in_grammar[TK_LPAR]);
	assert(term_in_grammar[TK_RPAR]);
	assert(term_in_grammar[TK_ID]);

	assert(!term_in_grammar[TK_LBRCE]);
	assert(!term_in_grammar[TK_CMPL]);
	assert(!term_in_grammar[EMPTY_STR]);

	struct prod_head_entry *phe;
	struct prod_list *prdp;
	struct symbol *s = malloc(sizeof(struct symbol));

	s->is_term = 0;

	s->nt_name = "expr_s"; /* augmented grammar */
	assert(sym_in_sym_list(s, nts_in_grammar));
	LOOK_UP(phe, s->nt_name, productions);
	assert(phe != NULL);
	assert(phe->next == NULL);
	prdp = phe->prods;
	assert(prdp != NULL);
	assert(prdp->next == NULL);
	assert(prdp->prod->next == NULL);
	assert(!prdp->prod->sym->is_term);
	assert(strcmp(prdp->prod->sym->nt_name, "expr") == 0);

	s->nt_name = "expr";
	assert(sym_in_sym_list(s, nts_in_grammar));
	LOOK_UP(phe, s->nt_name, productions);
	assert(phe != NULL);
	prdp = phe->prods;
	assert(prdp != NULL);
	assert(!prdp->prod->sym->is_term);
	for (; prdp != NULL; prdp = prdp->next) {
		if (!strcmp(prdp->prod->sym->nt_name, "term")) {
			assert(prdp->prod->next == NULL);
			continue;
		}
		if (!strcmp(prdp->prod->sym->nt_name, "expr")) {
			assert(prdp->prod->next->sym->is_term);
			assert(strcmp(prdp->prod->next->next->sym->nt_name,
								"term") == 0);
			assert(prdp->prod->next->next->next == NULL);
		}
	}

	s->nt_name = "term";
	assert(sym_in_sym_list(s, nts_in_grammar));
	LOOK_UP(phe, s->nt_name, productions);
	assert(phe != NULL);
	prdp = phe->prods;
	assert(prdp != NULL);

	s->nt_name = "fact";
	assert(sym_in_sym_list(s, nts_in_grammar));
	LOOK_UP(phe, s->nt_name, productions);
	assert(phe != NULL);
	prdp = phe->prods;
	assert(prdp != NULL);

	s->nt_name = "not_in_grammar";
	assert(!sym_in_sym_list(s, nts_in_grammar));
	LOOK_UP(phe, s->nt_name, productions);
	assert(phe == NULL);

	printf("%s passed\n", __func__);
}

void test_print_item()
{
	curr_prod = NULL;
	curr_sym->is_term = 0;
	curr_sym->nt_name = "nt1";
	add_sym();

	curr_sym->is_term = 1;
	curr_sym->term_type = TK_CMPL;
	add_sym();

	curr_sym->is_term = 0;
	curr_sym->nt_name = "nt2";
	add_sym();

	struct item it = {"head", curr_prod, curr_prod->next->next};

	int out_pipe[2];
	int saved_stdout = dup(STDOUT_FILENO);
	assert(pipe(out_pipe) == 0);
	dup2(out_pipe[1], STDOUT_FILENO);
	close(out_pipe[1]);

	print_item(&it);
	fflush(stdout);
	char out_str[25];
	read(out_pipe[0], out_str, 25);
	out_str[24] = '\0';
	assert(strcmp("[ head -> nt2 `~` .nt1 ]", out_str) == 0);
	dup2(saved_stdout, STDOUT_FILENO);

	printf("%s passed\n", __func__);
}

void test_itm_in_itm_list()
{
	/* item list = it1->it2->it3->NULL
	 * it1:
	 * 	head="head1",
	 * 	body=nt1->TK_CMPL->nt2->NULL,
	 * 	dot=nt2->NULL
	 * it2:
	 * 	head="head2",
	 * 	body=nt1->TK_LESS->nt2->NULL,
	 * 	dot=nt2->NULL
	 * it3:
	 * 	head="head3",
	 * 	body=nt1->TK_GRT->nt2->NULL,
	 * 	dot=nt2->NULL
	 */
	struct item *it1, *it2, *it3;
	struct sym_list *b = NULL;
	struct sym_list *d;
	struct symbol *s;
	s = make_symbol(0, 0, "nt2");
	add_sym_to_list(s, &b);
	s = make_symbol(1, TK_CMPL, NULL);
	add_sym_to_list(s, &b);
	s = make_symbol(0, 0, "nt1");
	add_sym_to_list(s, &b);
	d = b->next->next;
	it1 = make_item("head1", b, d);

	b = d = NULL;
	s = make_symbol(0, 0, "nt2");
	add_sym_to_list(s, &b);
	add_sym_to_list(s, &d);
	s = make_symbol(1, TK_LESS, NULL);
	add_sym_to_list(s, &b);
	s = make_symbol(0, 0, "nt1");
	add_sym_to_list(s, &b);
	it2 = make_item("head2", b, d);

	b = d = NULL;
	s = make_symbol(0, 0, "nt2");
	add_sym_to_list(s, &b);
	add_sym_to_list(s, &d);
	s = make_symbol(1, TK_GRT, NULL);
	add_sym_to_list(s, &b);
	s = make_symbol(0, 0, "nt1");
	add_sym_to_list(s, &b);
	it3 = make_item("head3", b, d);

	struct itm_list *il = NULL;
	add_itm_to_list(it1, &il);
	add_itm_to_list(it2, &il);
	add_itm_to_list(it3, &il);

	assert(itm_in_itm_list(it1, il));
	assert(itm_in_itm_list(it2, il));
	assert(itm_in_itm_list(it3, il));

	struct item *it4 = make_item("head4", it1->body, it1->dot);
	assert(!itm_in_itm_list(it4, il));

	struct item *it;
	it = make_item("head1", it1->body, it1->body->next);
	assert(!itm_in_itm_list(it, il));

	it = make_item("head1", it1->body, it1->body);
	assert(!itm_in_itm_list(it, il));

	it = make_item("head1", it1->dot, it1->body);
	assert(!itm_in_itm_list(it, il));

	it = make_item("head1", NULL, it1->body);
	assert(!itm_in_itm_list(it, il));

	it = make_item("head1", it1->body, it1->dot);
	assert(itm_in_itm_list(it, il));

	it = make_item("head1", it1->body, it1->body->next->next);
	assert(itm_in_itm_list(it, il));

	printf("%s passed\n", __func__);
}

void test_closure()
{
	init_lexer("./tests/arith_expr.bn");
	init_grammar();
	parse_bn();

	/* check closure of [ E' -> .E ] */
	struct prod_head_entry *phe;
	LOOK_UP(phe, "expr_s", productions);
	struct item *si;
	si = make_item("expr_s", phe->prods->prod, phe->prods->prod);
	struct itm_list *sil = NULL;
	add_itm_to_list(si, &sil);

	struct itm_list *c = closure(sil);

	/* [ E' -> .E ] must be in c */
	assert(itm_in_itm_list(si, c));

	struct item *it = malloc(sizeof(struct item));

	/* an item for every E prod must be in c:
	 * [ E -> .T ], [ E -> .E + T ], [ E -> .E - T ]
	 */
	LOOK_UP(phe, "expr", productions);
	it->head = "expr";
	printf("CLOSURE({ ");
	print_item(si);
	printf(" }) = {\n");
	for (; phe->prods != NULL; phe->prods = phe->prods->next) {
		it->body = phe->prods->prod;
		it->dot = phe->prods->prod;
		putchar('\t');
		print_item(it);
		printf(",\n");
		assert(itm_in_itm_list(it, c));
	}

	/* an item for every T prod must be in c:
	 * [ T -> .F ], [ T -> .T * F ], [ T -> .T / F ]
	 */
	LOOK_UP(phe, "term", productions);
	it->head = "term";
	for (; phe->prods != NULL; phe->prods = phe->prods->next) {
		it->body = phe->prods->prod;
		it->dot = phe->prods->prod;
		putchar('\t');
		print_item(it);
		printf(",\n");
		assert(itm_in_itm_list(it, c));
	}

	/* an item for every F prod must be in c:
	 * [ F -> .id ], [ F -> .( E ) ]
	 */
	LOOK_UP(phe, "fact", productions);
	it->head = "fact";
	for (; phe->prods != NULL; phe->prods = phe->prods->next) {
		it->body = phe->prods->prod;
		it->dot = phe->prods->prod;
		putchar('\t');
		print_item(it);
		printf(",\n");
		assert(itm_in_itm_list(it, c));
	}
	printf("}\n");

	printf("%s passed\n", __func__);
}

void test_find_itm_list_in_canon_set()
{
	canon_set = NULL;
	/* add { [ E' -> E. ], [ E -> E .+ T ] } to canon_set */
	struct prod_head_entry *phe;
	LOOK_UP(phe, "expr_s", productions);
	struct item *it1;
	it1 = make_item("expr_s", phe->prods->prod,
				phe->prods->prod->next);

	struct item *it2 = make_item("expr", NULL, NULL);
	add_sym_to_list(make_symbol(0, 0, "term"), &it2->body);
	add_sym_to_list(make_symbol(1, TK_PLUS, NULL), &it2->body);
	add_sym_to_list(make_symbol(0, 0, "expr"), &it2->body);
	it2->dot = it2->body->next;

	struct itm_list *itl = NULL;
	add_itm_to_list(it2, &itl);
	add_itm_to_list(it1, &itl);

	struct itm_list_list *itllnk = malloc(sizeof(struct itm_list_list));
	itllnk->il = itl;
	ADD_LINK(itllnk, canon_set);

	assert(find_itm_list_in_canon_set(itl) == itl);

	printf("%s passed\n", __func__);
}

void test_go_to()
{
	init_lexer("./tests/arith_expr.bn");
	init_grammar();
	parse_bn();

	/* check goto of {[ E' -> E. ], [ E -> E .+ T ] } */
	struct prod_head_entry *phe;
	LOOK_UP(phe, "expr_s", productions);
	struct item *it1;
	it1 = make_item("expr_s", phe->prods->prod,
				phe->prods->prod->next);

	struct item *it2 = make_item("expr", NULL, NULL);
	add_sym_to_list(make_symbol(0, 0, "term"), &it2->body);
	add_sym_to_list(make_symbol(1, TK_PLUS, NULL), &it2->body);
	add_sym_to_list(make_symbol(0, 0, "expr"), &it2->body);
	it2->dot = it2->body->next;

	struct itm_list *itl = NULL;
	add_itm_to_list(it2, &itl);
	add_itm_to_list(it1, &itl);

	struct itm_list *go = go_to(itl, it2->dot->sym);
	printf("GOTO({ ");
	print_item(it1);
	printf(", ");
	print_item(it2);
	printf(" }, `+`) = {\n");
	for (; go != NULL; go = go->next) {
		putchar('\t');
		print_item(go->itm);
		printf(",\n");
	}
	printf("}\n");

	printf("%s: check output above\n", __func__);
}

void test_compute_canon_set()
{
	init_lexer("./tests/arith_expr.bn");
	init_grammar();
	parse_bn();

	compute_canon_set();

	printf("CANON SET:\n");
	print_canon_set();

	printf("%s: check output above\n", __func__);
}

void test_compute_action_tab()
{
	init_lexer("./tests/reduced_arith_expr.bn");
	init_grammar();
	parse_bn();

	print_action_tab();

	printf("%s: check output above\n", __func__);
}

void test_grammar()
{
	test_sym_in_sym_list();
	test_repr_sym();
	test_add_sym();
	test_add_prod();
	test_fill_first_of_term_tab();
	test_fill_nts_in_grammar_list();
	test_first_for_terms();
	test_compute_first_tab();
	test_compute_follow_tab();
	test_first_of_sym_list();
	test_parse_bn();
	test_print_item();
	test_itm_in_itm_list();
	test_closure();
	test_find_itm_list_in_canon_set();
	test_go_to();
	test_compute_canon_set();
	test_compute_action_tab();
}
