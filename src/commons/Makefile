CC=gcc
CFLAGS=-Iinclude -I$(TESTING_INCLUDE) $(GLOBAL_CFLAGS)
SRC=$(wildcard *.c)
OBJ=$(SRC:%.c=output/%.o) output/regex.o
DEPENDS=$(SRC:%.c=output/%.d)
LDFLAGS=-lstdc++

TEST_SRC=$(wildcard test/*.c)
TEST_OBJ=$(TEST_SRC:test/%.c=output/%.o)
TEST_DEPENDS=$(TEST_SRC:test/%.c=output/%.d)
TEST_EXE=output/test_commons.exe

all: $(OBJ)

output/regex.o: regex.cpp include/regex.h include/error_handling.h include/logging.h
	g++ $(CFLAGS) -c -o $@ regex.cpp

clean:
	$(call remove,$(OBJ) $(TEST_OBJ) $(DEPENDS) $(TEST_DEPENDS) $(TEST_EXE) output/sender.exe output/receiver.exe)

.PHONY: test
test: $(TEST_EXE)
	$(TEST_EXE)

output/sender.exe: output/sender.o $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

output/receiver.exe: output/receiver.o $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_EXE): $(OBJ) $(TEST_OBJ) $(TESTING_OBJ) output/sender.exe output/receiver.exe
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(filter-out output/sender.o output/receiver.o,$(TEST_OBJ)) $(TESTING_OBJ) $(LDFLAGS)

output/%.d: %.c
	$(call make_depend,$@)

output/%.d: test/%.c
	$(call make_test_depend,$@)

include $(DEPENDS) $(TEST_DEPENDS)
