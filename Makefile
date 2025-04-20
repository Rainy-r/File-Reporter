CC = gcc
CFLAGS = -O0 
LIBS = -lcrypto -lssl

MACROS = -D_XOPEN_SOURCE=500 

./build/detect_dups: ./src/detect_dups.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(MACROS)

.PHONY: clean
clean:
	find . -type f -executable | xargs rm

