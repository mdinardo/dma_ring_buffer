CC=gcc
CLFAGS:=
LDFLAGS:=-Isrc/

bin/:
	mkdir -p bin

.PHONY: clean
clean:
	rm -rf bin

bin/%.o: bin/
bin/%.o: src/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: build
build: bin/ bin/stream_rb.o

.PHONY: build_debug
build_debug: CFLAGS+=-DSTREAM_RING_BUFFER_DEBUG
build_debug: build

bin/test: bin/stream_rb.o test/main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

.PHONY: test
test: bin/ build_debug bin/test
	./bin/test

