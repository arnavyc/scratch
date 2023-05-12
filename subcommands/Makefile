CC = cc
CFLAGS = -Og -g

subcmd-main: subcmd.o main.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -f *.o subcmd-main

