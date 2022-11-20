#include "test_grammar.c"
#include "test_utils.c"

#define ASCII_BOLD	"\033[1m"
#define ASCII_GREEN	"\033[32m"
#define ASCII_NORMAL	"\033[m"

int main()
{
	printf(ASCII_BOLD"TEST_UTILS\n"ASCII_NORMAL);
	test_utils();
	printf(ASCII_BOLD ASCII_GREEN"passed\n\n"ASCII_NORMAL);

	printf(ASCII_BOLD"TEST_GRAMMAR\n"ASCII_NORMAL);
	test_grammar();
	printf(ASCII_BOLD ASCII_GREEN"passed\n\n"ASCII_NORMAL);
}
