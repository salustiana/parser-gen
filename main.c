#include "lexer.h"
#include "parser.h"

int main(int argc, char **argv)
{
	if (argc == 1) {
		init_lexer(NULL);
		parse();
	}

	while (--argc > 0) {
		init_lexer(*++argv);
		parse();
	}
}
