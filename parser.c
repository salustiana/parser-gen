#include "grammar.h"

#include <stdio.h>

void parse()
{
	parse_bn();
	print_grammar();
	print_first_tab();
	putchar('\n');
	print_follow_tab();
}
