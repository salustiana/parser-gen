#include "grammar.h"

void parse()
{
	parse_bn();
	augment_grammar();
	print_grammar();
}
