#include "bn_utils.h"
#include "lexer.h"

#include <stdlib.h>
#include <string.h>

struct nt_entry *grammar[HASHSIZE];

char *strdup(const char *s)
{
	char *t;
	t = malloc((strlen(s) + 1) * sizeof(char));
	if (t != NULL)
		strcpy(t, s);
	return t;
}

unsigned int hash(const char *s)
{
	unsigned int hash_val;
	for (hash_val = 0; *s != '\0'; s++)
		hash_val = (unsigned int) *s + 31*hash_val;
	return hash_val % HASHSIZE;
}

struct nt_entry *look_up(const char *key)
{
	struct nt_entry *ep;
	for (ep = grammar[hash(key)]; ep != NULL; ep = ep->next)
		if (strcmp(key, ep->key) == 0)
			return ep;
	return NULL;
}

/*
 * Creates a new nt_entry for the given key,
 * with a NULL prods pointer.
 * Returns NULL if the key already exists
 * or if no memory is available.
 */
struct nt_entry *create_entry(const char *key)
{
	if (look_up(key) != NULL)
		return NULL;

	struct nt_entry *ep = malloc(sizeof(struct nt_entry));
	if (ep == NULL || (ep->key = strdup(key)) == NULL)
		return NULL;

	ep->prods = NULL;
	unsigned int hash_val = hash(key);
	ep->next = grammar[hash_val];
	grammar[hash_val] = ep;
	return ep;
}
