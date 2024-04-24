CC := clang
CXX := clang++
OPT := opt
DIS := llvm-dis
CMAKE := cmake

PLUGIN = build/PassPlugin.so
TESTS = $(wildcard tests/*.c)
FLAGS = -lm

.PHONY: $(PLUGIN)
$(PLUGIN):
	$(CMAKE) --build build -j2

$(TESTS:%.c=%.ll): tests/%.ll: tests/%.c
	$(CC) -Os -S -emit-llvm $< -o $@

$(TESTS:%.c=%): tests/%: tests/%.ll
	$(CC) $< -o $@

$(TESTS:%.c=%.bit_theft.bc): tests/%.bit_theft.bc: tests/%.ll $(PLUGIN)
	$(OPT) -load-pass-plugin="${PLUGIN}" -passes='bit-theft' $< -o $@

$(TESTS:%.c=%.bit_theft): tests/%.bit_theft: tests/%.bit_theft.bc
	$(CC) -Os $< -o $@ ${FLAGS}

$(TESTS:%.c=%.bit_theft.ll): tests/%.bit_theft.ll: tests/%.bit_theft.bc
	$(DIS) $< -o $@

.PHONY: clean
clean:
	rm -f tests/*.bc tests/*.ll tests/*.bit_theft $(TESTS:%.c=%)
