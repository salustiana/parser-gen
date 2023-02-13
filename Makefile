CC = gcc
OBJS = main.c parser.c grammar.c utils.c ./lexer/lexer.c
CFLAGS = -Wall -Wextra -Wconversion -pedantic -std=c99 -g
INCLUDES = -iquote ./include -iquote ./lexer/include

a.out: ${OBJS}
	${CC} ${OBJS} ${INCLUDES} ${CFLAGS} && make test

test.out: ${OBJS} ./tests
	${CC} ./tests/test_main.c ./lexer/lexer.c ${INCLUDES} ${CFLAGS} -o test.out

test: test.out
	./test.out
