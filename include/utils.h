#ifndef UTILS_H
#define UTILS_H

char *strdup(const char *s);

/*
 * Adds lnk to the start of llist.
 * `next` should be the first member of lnk
 * and of every link on llist.
 * Returns NULL if either lnk or llist is NULL.
 */
void *add_link(void *lnk, void *llist);

/*
 * Reverses the NULL terminated llist
 * and returns a pointer to its new
 * first element.
 * llist should point to the next pointer,
 * in other words, *llist == llist->next
 */
void *reverse_linked_list(void *llist);

#define HASHSIZE	101

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
void *look_up(const char *key, void *table);

/*
 * Creates a new entry in table for key.
 * Returns NULL if the key already exists
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
void *create_entry(const char *key, void *table);

#endif
