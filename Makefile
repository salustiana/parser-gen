CC = gcc
OBJS = main.c bn_parser.c ../lexer/lexer.c
CFLAGS = -Wall -Wextra -Wconversion -pedantic -std=c99 -g
INCLUDES = -iquote ./include -iquote ../lexer/include
#LIBS = -lGL -lglfw -ldl -lm -lmi

a.out: ${OBJS}
	${CC} ${OBJS} ${INCLUDES} ${CFLAGS} #${LIBS}
