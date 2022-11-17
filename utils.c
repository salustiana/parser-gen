#include "utils.h"

#include <stddef.h>
#include <stdlib.h>

struct link {
	struct link *next;
};

/*
 * Reverses the NULL terminated llist
 * and returns a pointer to its new
 * first element.
 * llist should point to the next pointer,
 * in other words, *llist == llist->next
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
