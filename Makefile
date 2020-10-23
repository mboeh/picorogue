picorogue-c: picorogue.c
	$(CC) -std=c99 -Wall -Werror -pedantic $< -o $@