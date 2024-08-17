.PHONY	:= all clean
CC		:= clang
# CFLAGS 	:= -O2
# CFLAGS 	:= -O3
# CFLAGS 	:= -Ofast
CFLAGS	:= -g -Wall -Wextra -pedantic
SRC		:= $(wildcard ./*.c)
OBJ		:= $(SRC:%.c=%.o)
YKES	:= ./

all: ykes

ykes:	$(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lm

%.o:	$(YKES)%.c
	$(CC) -I$(YKES)includes -c $< $(CFLAGS)

clean:
	rm -rf *.dSYM *.o *.d ykes
