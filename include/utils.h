#ifndef UTILS_H
#define UTILS_H

#define HASHSIZE	101

/*
 * Adds lnk to the start of llist and
 * returns a pointer to lnk (which should
 * be set as the new start of llist).
 * `next` should be the first member of lnk
 * and of every link on llist.
 * Usage: llist = new_link(lnk, llist);
 */
void *new_link(void *lnk, void *llist);

/*
 * Reverses the NULL terminated llist
 * and returns a pointer to its new
 * first element.
 * `next` should be the first member
 * of the links in llist.
 * In other words, *llist == llist->next.
 */
void *reverse_linked_list(void *llist);

char *strdup(const char *s);

/*
 * Returns a pointer to the entry for
 * key in table, or NULL if table does
 * not contain an entry for key.
 * table should be an array of linked lists
 * of entries which have `next` for their
 * first member and `key` for their second
 * member:
 * struct entry {
 * 	struct entry *next;
 * 	const char *key;
 * 	...
 * };)
 * The size of table should be HASHSIZE,
 * which is defined in utils.h.
 */
void *look_up(const char *key, void *table);

/*
 * Creates a new entry in table for key.
 * The table must not have an existing
 * entry for the given key.
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
void *create_entry(const char *key, void *table);

#endif
