#include "bn_parser.h"
#include "lexer.h"

int main(int argc, char **argv)
{
	if (argc == 1) {
		init_lexer(NULL);
		parse_bn();
		print_grammar();
	}

	while (--argc > 0) {
		init_lexer(*++argv);
		parse_bn();
		print_grammar();
	}
}
