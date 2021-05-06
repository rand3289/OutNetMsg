# -Werror -ansi -pedantic -Wall -Wextra -Wno-unused-parameter
CC = g++  # notice CFLAGS contains -g which will compile everything in debug mode!
CFLAGS = -g --std=c++20 -Wall -Wextra -Wno-unused-parameter -pthread -Ilib
DEPS = config.h state.h outnet.h lib/buffer.h lib/sock.h lib/client.h lib/sign.h lib/tweetnacl.h lib/json.hpp lib/utils.h
OBJ = main.o webs.o config.o state.o outnet.o lib/sock.o lib/client.o lib/sign.o lib/tweetnacl.o lib/utils.o

# Linux generates a warning when using -static
# Using 'gethostbyname' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking
ifdef OS # windows defines this environment variable
	LDFLAGS = -lwsock32 -static
endif

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

onmsg: $(OBJ)
	$(CC) -o $@ $^ -lpthread $(LDFLAGS)

clean:
	/bin/rm -f $(OBJ) onmsg onmsg.exe
