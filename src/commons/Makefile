CC=gcc
CFLAGS=-Iinclude -I$(TESTING_INCLUDE) $(GLOBAL_CFLAGS)
SRC=$(wildcard *.c)
OBJ=$(SRC:%.c=output/%.o)
DEPENDS=$(SRC:%.c=output/%.d)

TEST_SRC=$(wildcard test/*.c)
TEST_OBJ=$(TEST_SRC:test/%.c=output/%.o)
TEST_DEPENDS=$(TEST_SRC:test/%.c=output/%.d)
TEST_EXE=output/test_commons.exe

all: $(OBJ)

clean:
	$(call remove,$(OBJ) $(DEPENDS) $(TEST_DEPENDS))

test: $(TEST_EXE)
	$(TEST_EXE)

$(TEST_EXE): $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(TEST_OBJ) $(TESTING_OBJ) $(LDFLAGS)

output/%.d: %.c
	$(call make_depend,$@)

output/%.d: test/%.c
	$(call make_test_depend,$@)

include $(DEPENDS) $(TEST_DEPENDS)
