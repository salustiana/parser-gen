#include "../utils.c"

#include <stdio.h>

struct _llist {
	struct _llist *next;
	int ival;
	char cval;
};

void test_ADD_LINK()
{
	struct _llist *lp = NULL, *np;
	for (int i = 0; i < 5; i++) {
		np = malloc(sizeof(struct _llist));
		np->ival = i;
		np->cval = (char) ('a' + i);
		ADD_LINK(np, lp);
	}
	int i = 4;
	char s[] = "abcde";
	for (struct _llist *p = lp; p != NULL; p = p->next) {
		assert(p->ival == i);
		assert(p->cval == s[i]);
		--i;
	}

	printf("%s passed\n", __func__);
}

void test_reverse_linked_list()
{
	struct _llist *lp = NULL, *np;
	for (int i = 0; i < 5; i++) {
		np = malloc(sizeof(struct _llist));
		np->ival = i;
		np->cval = (char) ('a' + i);
		ADD_LINK(np, lp);
	}
	int i = 0;
	char s[] = "abcde";
	lp = reverse_linked_list(lp);
	for (struct _llist *p = lp; p != NULL; p = p->next) {
		assert(p->ival == i);
		assert(p->cval == s[i]);
		++i;
	}

	printf("%s passed\n", __func__);
}

void test_LOOK_UP()
{
	struct _entry {
		struct _entry *next;
		const char *key;
		int ival;
	} *_tab[HASHSIZE];

	for (int i = 0; i < HASHSIZE; i++)
		_tab[i] = NULL;

	struct _entry *dest;
	LOOK_UP(dest, "key", _tab);
	assert(dest == NULL);

	struct _entry *ep = malloc(sizeof(struct _entry));
	ep->next = NULL;
	ep->key = "key";
	ep->ival = 8;
	unsigned int hash_val = hash(ep->key);
	ep->next = _tab[hash_val];
	_tab[hash_val] = ep;

	LOOK_UP(dest, "key", _tab);
	assert(dest != NULL);
	assert(dest->ival == 8);

	printf("%s passed\n", __func__);
}

void test_INSERT_ENTRY()
{
	struct _entry {
		struct _entry *next;
		const char *key;
		int ival;
	} *_tab[HASHSIZE];

	for (int i = 0; i < HASHSIZE; i++)
		_tab[i] = NULL;

	struct _entry *e, *_e;
	LOOK_UP(e, "key", _tab);
	assert(e == NULL);

	_e = malloc(sizeof(struct _entry));
	INSERT_ENTRY(_e, "key", _tab);
	LOOK_UP(e, "key", _tab);
	assert(e != NULL);

	printf("%s passed\n", __func__);
}

void test_utils()
{
	test_ADD_LINK();
	test_reverse_linked_list();
	test_LOOK_UP();
	test_INSERT_ENTRY();
}
