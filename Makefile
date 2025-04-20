CC = gcc
CFLAGS = -O0 -g
LIBS = -lcrypto -lssl

MACROS = -D_XOPEN_SOURCE=500 

detect_dups: detect_dups.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(MACROS)

.PHONY: clean
clean:
	find . -type f -executable | xargs rm

