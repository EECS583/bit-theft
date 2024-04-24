CC := clang
CXX := clang++
OPT := opt
DIS := llvm-dis
DUMP := objdump
CMAKE := cmake

CFLAGS := -std=c17 -Wall -O1 -lm

PLUGIN = build/PassPlugin.so
TESTS = $(wildcard tests/*.c)

.PHONY: $(PLUGIN)
$(PLUGIN):
	$(CMAKE) --build build -j2

$(TESTS:%.c=%.bc): tests/%.bc: tests/%.c
	$(CC) -c -emit-llvm ${CFLAGS} $< -o $@

$(TESTS:%.c=%): tests/%: tests/%.bc
	$(CC) ${CFLAGS} $< -o $@

$(TESTS:%.c=%.bit_theft.bc): tests/%.bit_theft.bc: tests/%.bc $(PLUGIN)
	$(OPT) -load-pass-plugin="${PLUGIN}" -passes='bit-theft' $< -o $@

$(TESTS:%.c=%.bit_theft): tests/%.bit_theft: tests/%.bit_theft.bc
	$(CC) ${CFLAGS} $< -o $@

%.ll: %.bc
	$(DIS) $< -o $@

%.lst: %
	$(DUMP) -d $< > $@

.PHONY: clean
clean:
	find tests -type f -not -name '*.c' -delete
