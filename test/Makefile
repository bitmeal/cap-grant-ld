CC=gcc
CFLAGS=
LIBS=-lcap-ng

libcap_so: cap_so.c
	$(CC) -c -o$@.o $^ $(CFLAGS)
	$(CC) -shared -o $@.so $@.o $(CFLAGS) $(LIBS)

loader: libcap_so loader.c
	$(CC) -c -o$@.o loader.c $(CFLAGS)
	$(CC) -o$@ $@.o -L. -lcap_so $(CFLAGS)

all: loader
.PHONY : all

clean:
	rm -f ./*.so ./*.o loader
.PHONY: clean
