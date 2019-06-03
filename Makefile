CSRC := $(shell find src -name "*.c")
TESTC := $(shell find test_dir -name "*.c")
COBJ := $(CSRC:.c=.o)
CDEPS := $(CSRC:.c=.d)
TESTOBJ := $(TESTC:.c=.o)
TESTS := $(TESTC:.c=.bin)
CC=gcc


OPT += -O2
LFLAGS = -lm  -Lbin/ -llogger -lnvidia-ml
CFLAGS = -ggdb -Wall -Wextra -pedantic
IFLAGS = -I/usr/local/cuda/include/ -I./include/

DEFAULT: bin/liblogger.a bin/liblogger.so

test: $(TESTS)

%.o: %.c
	$(CC) -c -MMD  -fPIC $(CFLAGS) $(IFLAGS) $(OPT) $< -o $@

bin/liblogger.a: $(COBJ)
	ar rcs  $@ $<

bin/liblogger.so: $(COBJ)
	$(CC) -shared $< -o $@

%.bin: %.o
	$(CC) $(CFLAGS) $< $(LFLAGS) -o $@

clean:
	rm -f $(COBJ) $(CDEPS) $(TESTOBJ) $(TESTS)
