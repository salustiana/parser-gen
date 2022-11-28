#include "utils.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int is_mem_null(void *s, size_t n)
{
	char null_mem[n];
	memset(null_mem, 0, n);
	return memcmp(s, null_mem, n) == 0;
}

struct link {
	struct link *next;
};

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
	assert(t != NULL);
	strcpy(t, s);
	return t;
}

char *extended_str(const char *base, const char *ext)
{
	assert(base != NULL);
	assert(ext != NULL);
	char *s = malloc(strlen(base) + strlen(ext));
	size_t i;
	for (i = 0; base[i] != '\0'; i++)
		s[i] = base[i];
	for (size_t j = 0; ext[j] != '\0'; i++, j++)
		s[i] = ext[j];
	s[i] = '\0';
	return s;
}

unsigned int hash(const char *s)
{
	assert(s != NULL);
	unsigned int hash_val;
	for (hash_val = 0; *s != '\0'; s++)
		hash_val = (unsigned int) *s + 31*hash_val;
	return hash_val % HASHSIZE;
}

struct entry {
	struct entry *next;
	const char *key;
};
