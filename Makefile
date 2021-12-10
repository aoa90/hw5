OUTPUT = client
CFLAGS = -g -Wall -Wvla -I inc -D_REENTRANT
LFLAGS = -L lib -lSDL2 -lSDL2_image -lSDL2_ttf

%.o: %.c %.h
	gcc $(CFLAGS) -c -o $@ $<

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

all: $(OUTPUT)

runclient: $(OUTPUT)
	LD_LIBRARY_PATH=lib ./client

client: client.o
	gcc $(CFLAGS) -o $@ $^ $(LFLAGS)

server: server.o
	gcc $(CFLAGS) -o $@ $^ $(LFLAGS) server

server.o: server.c
	gcc -g -Wall -Wvla -fsanitize=address -c server.c

clean:
	rm -f $(OUTPUT) *.o
