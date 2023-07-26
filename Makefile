CC=gcc
CFLAGS=
LIBS=-lcap-ng -lprocps

cap_grant_ld: cap_grant_ld.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

all: cap_grant_ld
.PHONY : all


PREFIX ?= /usr/local
install: all
	install -d $(DESTDIR)$(PREFIX)/bin/
	install cap_grant_ld $(DESTDIR)$(PREFIX)/bin/
.PHONY: install

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/cap_grant_ld
.PHONY: uninstall


clean:
	rm -f cap_grant_ld
.PHONY: clean
