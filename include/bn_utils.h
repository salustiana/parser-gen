#ifndef BN_UTILS_H
#define BN_UTILS_H

#include "lexer.h"

struct symbol_list {
	int is_term;
	enum tk_type term_val;
	const char *nt_name;
	size_t rem_syms;
	struct symbol_list *next;
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

struct nt_entry *look_up(const char *key);

/*
 * Creates a new nt_entry for the given key,
 * with a NULL prods pointer.
 * Returns NULL if the key already exists
 * or if no memory is available.
 */
struct nt_entry *create_entry(const char *key);

#define HASHSIZE	101
extern struct nt_entry *grammar[HASHSIZE];

#endif
