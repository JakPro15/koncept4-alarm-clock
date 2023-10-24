CC=gcc
CFLAGS=-Iinclude -I$(COMMONS_INCLUDE) -I$(TESTING_INCLUDE) $(GLOBAL_CFLAGS)
LDFLAGS=-lwinmm -lstdc++
SRC=$(wildcard *.c)
OBJ=$(SRC:%.c=output/%.o)
EXE=output/konc4d.exe
DEPENDS=$(SRC:%.c=output/%.d)

TEST_SRC=$(wildcard test/*.c)
TEST_OBJ=$(TEST_SRC:test/%.c=output/%.o)
TEST_DEPENDS=$(TEST_SRC:test/%.c=output/%.d)
TEST_EXE=output/test_konc4d.exe

all: $(EXE)

clean:
	$(call remove,$(OBJ) $(DEPENDS) $(TEST_OBJ) $(TEST_DEPENDS) $(EXE) $(TEST_EXE))

.PHONY: test
test: $(TEST_EXE)
	$(TEST_EXE)

$(EXE): $(OBJ) $(COMMONS_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_EXE): $(OBJ) $(TEST_OBJ) $(COMMONS_OBJ) $(TESTING_OBJ)
	$(CC) $(CFLAGS) -o $@ $(filter-out output/main.o output/message_processing.o,$(OBJ)) $(TEST_OBJ) $(COMMONS_OBJ) $(TESTING_OBJ) $(LDFLAGS)

output/%.d: %.c
	$(call make_depend,$@)

output/%.d: test/%.c
	$(call make_test_depend,$@)

include $(DEPENDS) $(TEST_DEPENDS)
