#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "lexer.h"

struct symbol_list {
	struct symbol_list *next;
	int is_term;
	enum tk_type term_val;
	const char *nt_name;
};

struct prod_list {
	struct prod_list *next;
	struct symbol_list *prod;
};

struct nt_entry {
	struct nt_entry *next;
	char *key;
	struct prod_list *prods;
};

#define HASHSIZE	101
extern struct nt_entry *grammar[HASHSIZE];

void parse_bn();

void print_grammar();

#endif
