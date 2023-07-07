CC=gcc
CFLAGS=-Iinclude -I$(COMMONS_INCLUDE) -I$(TESTING_INCLUDE) $(GLOBAL_CFLAGS)
LDFLAGS=-lwinmm
SRC=$(wildcard *.c)
OBJ=$(SRC:%.c=output/%.o)
DEPENDS=$(SRC:%.c=output/%.d)

TEST_SRC=$(wildcard test/*.c)
TEST_OBJ=$(TEST_SRC:test/%.c=output/%.o)
TEST_DEPENDS=$(TEST_SRC:test/%.c=output/%.d)
TEST_EXE=output/test_konc4d.exe

all: $(OBJ)
	$(CC) $(CFLAGS) -o output/konc4d.exe $(OBJ) $(COMMONS_OBJ) $(LDFLAGS)

test: $(TEST_EXE)
	$(TEST_EXE)

$(TEST_EXE): $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o output/test_konc4d.exe $(filter-out output/main.o,$(OBJ)) $(TEST_OBJ) $(COMMONS_OBJ) $(TESTING_OBJ) $(LDFLAGS)

clean:
	$(call remove,$(OBJ) $(DEPENDS) $(TEST_OBJ) $(TEST_DEPENDS))

output/%.d: %.c
	$(call make_depend,$@)

output/%.d: test/%.c
	$(call make_test_depend,$@)

include $(DEPENDS) $(TEST_DEPENDS)
