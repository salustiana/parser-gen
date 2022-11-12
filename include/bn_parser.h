#ifndef BN_PARSER_H
#define BN_PARSER_H

#include "lexer.h"

char *strdup(const char *s);

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

struct nt_entry *look_up(const char *key);

/*
 * Creates a new nt_entry for the given key,
 * with a NULL prods pointer.
 * Returns NULL if the key already exists
 * or if no memory is available.
 */
struct nt_entry *create_entry(const char *key);

/*
 * Skips the single char tokens
 * given in tk_str and panics if one
 * of them does not match the input.
 * Example: skip_tks("::=") skips
 * the tokens TK_COLN, TK_COLN, TK_ASSIGN.
 * XXX: only works for tokens whose type is
 * given by their character representation.
 */
void skip_tks(const char *tk_str);

extern struct token tk;

#define HASHSIZE	101
extern struct nt_entry *grammar[HASHSIZE];

void parse_bn();

void print_grammar();

#endif
