#include "../utils.c"

#include <stdio.h>

struct _llist {
	struct _llist *next;
	int ival;
	char cval;
};

void test_new_link()
{
	struct _llist *lp = NULL, *np;
	for (int i = 0; i < 5; i++) {
		np = malloc(sizeof(struct _llist));
		np->ival = i;
		np->cval = (char) ('a' + i);
		lp = new_link(np, lp);
	}
	int i = 4;
	char s[] = "abcde";
	for (struct _llist *p = lp; p != NULL; p = p->next) {
		assert(p->ival == i);
		assert(p->cval == s[i]);
		--i;
	}
	printf("\ttest_new_link() passed\n");
}

void test_reverse_linked_list()
{
	struct _llist *lp = NULL, *np;
	for (int i = 0; i < 5; i++) {
		np = malloc(sizeof(struct _llist));
		np->ival = i;
		np->cval = (char) ('a' + i);
		lp = new_link(np, lp);
	}
	int i = 0;
	char s[] = "abcde";
	lp = reverse_linked_list(lp);
	for (struct _llist *p = lp; p != NULL; p = p->next) {
		assert(p->ival == i);
		assert(p->cval == s[i]);
		++i;
	}
	printf("\ttest_reverse_linked_list() passed\n");
}

struct _entry {
	struct _entry *next;
	const char *key;
	int ival;
} *_tab[HASHSIZE];

void test_look_up()
{
	for (int i = 0; i < HASHSIZE; i++)
		_tab[i] = NULL;

	assert(look_up("key", _tab) == NULL);
	struct _entry *ep = malloc(sizeof(struct _entry));
	ep->next = NULL;
	ep->key = "key";
	ep->ival = 8;
	unsigned int hash_val = hash(ep->key);
	ep->next = _tab[hash_val];
	_tab[hash_val] = ep;
	assert(look_up("key", _tab) != NULL);
	assert(((struct _entry *) look_up("key", _tab))->ival == 8);
	printf("\ttest_look_up() passed\n");
}

void test_create_entry()
{
	for (int i = 0; i < HASHSIZE; i++)
		_tab[i] = NULL;

	assert(look_up("key", _tab) == NULL);
	create_entry("key", _tab);
	assert(look_up("key", _tab) != NULL);
	printf("\ttest_create_entry() passed\n");
}

void test_utils()
{
	printf("test_utils() start\n");
	test_new_link();
	test_reverse_linked_list();
	test_look_up();
	test_create_entry();
	printf("test_utils() passed\n");
}