CC = gcc
OBJS = main.c parser.c grammar.c utils.c ../lexer/lexer.c
CFLAGS = -Wall -Wextra -Wconversion -pedantic -std=c99 -g
INCLUDES = -iquote ./include -iquote ../lexer/include
#LIBS = -lGL -lglfw -ldl -lm -lmi

a.out: ${OBJS}
	${CC} ${OBJS} ${INCLUDES} ${CFLAGS} #${LIBS}
