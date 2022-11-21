#include "grammar.h"

void parse()
{
	parse_bn();
	augment_grammar();
	print_grammar();
	compute_first_tab();
	print_first_tab();
}
