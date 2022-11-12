#include "bn_parser.h"
#include "bn_utils.h"
#include "lexer.h"

#include <stdlib.h>

struct token tk;

struct non_term {
	const char *name;
	struct symbol_list *curr_prod;
} curr_nt;

void add_prod()
{
	// TODO: move this to bn_utils
	struct prod_list *new_prod = malloc(sizeof(struct prod_list));
	new_prod->next = look_up(curr_nt.name)->prods;
	new_prod->prod = malloc(sizeof(struct symbol_list));
	curr_nt.curr_prod = new_prod->prod;
	curr_nt.curr_prod->rem_syms = 0;
}

void parse_prods()
{
	// TODO
	add_prod();
	while (next_token(&tk)) {
		if (tk.type == TK_BAR) {
			add_prod();
			continue;
		}
		/* parse next symbol in prod */
		if (tk.type == TK_LESS) {	/* parse non term */
			next_token(&tk);
			if (tk.type != TK_ID)
				panic("expected non terminal symbol");
			// TODO: char *nt = strdup(tk.str_val);
			next_token(&tk);
			if (tk.type != TK_GRT)
				panic("expected '>'");
			next_token(&tk);
			if (tk.type == TK_COLN) {	/* we are in a new definition "<nt> :=" */
				next_token(&tk);
				if (tk.type != TK_ASSIGN)
					panic("expected '='");
				// TODO: curr_nt.name = create_entry(nt)->key;
				// TODO: new_def(nt);
				continue;
			}
			/* store the non term in the current prod */
			/* XXX: the prod is stored in reverse order */
			struct symbol_list *new_sym = malloc(sizeof(struct symbol_list));
			new_sym->is_term = 0;
			// TODO: new_sym->nt_name = nt;
			new_sym->rem_syms = curr_nt.curr_prod->rem_syms + 1;
			new_sym->next = curr_nt.curr_prod;
			curr_nt.curr_prod = new_sym;
		}
		if (tk.type == TK_STR) {	/* parse literal */
			/* store the term in the current prod */
			/* XXX: the prod is stored in reverse order */
			struct symbol_list *new_sym = malloc(sizeof(struct symbol_list));
			new_sym->is_term = 1;
			new_sym->term_val = TK_LPAR;	/* TODO: implement this */
			new_sym->rem_syms = curr_nt.curr_prod->rem_syms + 1;
			new_sym->next = curr_nt.curr_prod;
			curr_nt.curr_prod = new_sym;
		}
	}
}

void parse_bn()
{
	// TODO
}
