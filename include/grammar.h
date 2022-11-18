#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "utils.h"

/* XXX: EMPTY_STR is chosen to be
 * a value not present in enum tk_type.
 */
#define EMPTY_STR	'@'

void parse_bn();

void augment_grammar();

void print_grammar();

#endif
