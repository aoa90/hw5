OUTPUT = echoservers echoclient
CFLAGS = -g -Wall -Wvla -I inc

%.o: %.c %.h
	gcc $(CFLAGS) -c -o $@ $<

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

all: $(OUTPUT)

echoservers: echoservers.o
	gcc -pthread echo.c csapp.c  $(CFLAGS) -o $@ $^ $(LFLAGS)

echoclient: echoclient.o
	gcc -pthread csapp.c $(CFLAGS) -o $@ $^ $(LFLAGS) 

clean:
	rm -f $(OUTPUT) *.o
