#include "utils.h"
#include "lexer.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* TODO: add tests for these utils. */

struct link {
	struct link *next;
};

/*
 * Adds lnk to the start of llist and
 * returns a pointer to lnk (which should
 * be set as the new start of llist).
 * `next` should be the first member of lnk
 * and of every link on llist.
 * panics if lnk is NULL.
 * Usage: llist = new_link(lnk, llist);
 */
void *new_link(void *lnk, void *llist)
{
	if (lnk == NULL)
		panic("NULL lnk passed to new_link");
	struct link *lk = lnk;
	lk->next = llist;
	return lk;
}

/*
 * Reverses the NULL terminated llist
 * and returns a pointer to its new
 * first element.
 * `next` should be the first member
 * of the links in llist.
 * In other words, *llist == llist->next.
 */
void *reverse_linked_list(void *llist)
{
	struct link *new, *l = llist, *rl = NULL;
	while (l != NULL) {
		new = l;
		l = l->next;
		new->next = rl;
		rl = new;
	}
	return rl;
}

char *strdup(const char *s)
{
	char *t = malloc((strlen(s) + 1) * sizeof(char));
	if (t == NULL)
		panic("could not allocate memory to duplicate string");
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

struct entry {
	struct entry *next;
	const char *key;
};

/*
 * Returns a pointer to the entry for
 * key in table, or NULL if table does
 * not contain an entry for key.
 * table should be an array of linked lists
 * of entries which have `next` for their
 * first member and `key` for their second
 * member:
 * struct entry {
 * 	void *next;
 * 	const char *key;
 * 	...
 * };)
 * The size of table should be HASHSIZE,
 * which is defined in utils.h.
 */
void *look_up(const char *key, void *table)
{
	struct entry *ep, **tab = table;

	for (ep = tab[hash(key)]; ep != NULL; ep = ep->next)
		if (strcmp(key, ep->key) == 0)
			return ep;
	return NULL;
}

/*
 * Creates a new entry in table for key.
 * panics if the key already exists
 * or if no memory is available.
 * table should be an array of linked lists
 * of entries which have `next` for their
 * first member and `key` for their second
 * member:
 * struct entry {
 * 	void *next;
 * 	const char *key;
 * 	...
 * };)
 * The size of table should be HASHSIZE,
 * which is defined in utils.h.
 */
void *create_entry(const char *key, void *table)
{
	if (look_up(key, table) != NULL)
		panic("table already has an entry \"%s\"", key);

	struct entry *ep = malloc(sizeof(struct entry));
	if (ep == NULL || (ep->key = strdup(key)) == NULL)
		panic("could not allocate memory to create new entry");

	struct entry **tab = table;
	unsigned int hash_val = hash(key);
	ep->next = tab[hash_val];
	tab[hash_val] = ep;
	return ep;
}
