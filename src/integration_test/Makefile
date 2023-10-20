CC=gcc
CFLAGS=-Iinclude -I$(COMMONS_INCLUDE) -I$(TESTING_INCLUDE) -I$(KONC4_INCLUDE) -I$(KONC4D_INCLUDE) $(GLOBAL_CFLAGS)
LDFLAGS=-lwinmm

TEST_SRC=$(wildcard test/*.c)
TEST_OBJ=$(TEST_SRC:test/%.c=output/%.o)
TEST_DEPENDS=$(TEST_SRC:test/%.c=output/%.d)
TEST_EXE=output/test_konc4d.exe

all: $(TEST_EXE)

clean:
	$(call remove,$(TEST_OBJ) $(TEST_DEPENDS) $(TEST_EXE))

.PHONY: test
test: $(TEST_EXE)
	$(TEST_EXE)

$(TEST_EXE): $(TEST_OBJ) $(COMMONS_OBJ) $(TESTING_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

output/%.d: test/%.c
	$(call make_test_depend,$@)

include $(TEST_DEPENDS)
