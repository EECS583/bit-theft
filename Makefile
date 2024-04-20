CC := clang
CXX := clang++
OPT := opt
DIS := llvm-dis

PLUGIN = "build/PassPlugin.so"

tests/%.ll: tests/%.c
	$(CC) -emit-llvm -S $< -O1 -o $@

tests/%.steal.bc: tests/%.ll
	$(OPT) -load-pass-plugin="${PLUGIN}" -passes='bit-theft' $< -o $@

tests/%.steal.ll: tests/%.steal.bc
	$(DIS) $< -o $@

clean:
	rm -f tests/*.bc tests/*.ll
