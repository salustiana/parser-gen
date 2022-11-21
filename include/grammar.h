#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "lexer.h"
#include "utils.h"

/* XXX: EMPTY_STR is chosen to be
 * a value not present in enum tk_type.
 */
#define EMPTY_STR	'@'

struct symbol {
	int is_term;
	enum tk_type term_type;
	const char *nt_name;
};
extern struct symbol *curr_sym;

struct sym_list {
	struct sym_list *next;
	struct symbol *sym;
};
extern struct sym_list *nts_in_grammar;


struct prod_list {
	struct prod_list *next;
	struct sym_list *prod;
};

struct prod_head_entry {
	struct prod_head_entry *next;
	const char *key;
	struct prod_list *prods;
};
extern struct prod_head_entry *productions[HASHSIZE];

void parse_bn();

void augment_grammar();

void compute_first_tab();

void print_grammar();

void print_first_tab();

#endif
