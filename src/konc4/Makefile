CC=gcc
CFLAGS=-Iinclude -I$(COMMONS_INCLUDE) -I$(TESTING_INCLUDE) $(GLOBAL_CFLAGS)
LDFLAGS=-lwinmm
SRC=$(wildcard *.c)
OBJ=$(SRC:%.c=output/%.o)
EXE=output/konc4.exe
DEPENDS=$(SRC:%.c=output/%.d)

TEST_SRC=$(wildcard test/*.c)
TEST_OBJ=$(TEST_SRC:test/%.c=output/%.o)
TEST_DEPENDS=$(TEST_SRC:test/%.c=output/%.d)
TEST_EXE=output/test_konc4.exe

all: $(EXE)

clean:
	$(call remove,$(OBJ) $(TEST_OBJ) $(DEPENDS) $(TEST_DEPENDS) $(EXE) $(TEST_EXE))

.PHONY: test
test: $(TEST_EXE)
	$(TEST_EXE)

output/input_looper.exe: output/input_looper.o
	$(CC) $(CFLAGS) -o $@ $^ $(filter-out output/main.o,$(OBJ)) $(COMMONS_OBJ) $(LDFLAGS)

$(EXE): $(OBJ) $(COMMONS_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_EXE): $(OBJ) $(TEST_OBJ) $(COMMONS_OBJ) $(TESTING_OBJ) ../commons/output/receiver.exe output/input_looper.exe
	$(CC) $(CFLAGS) -o $@ $(filter-out output/main.o,$(OBJ)) $(filter-out output/input_looper.o,$(TEST_OBJ)) $(COMMONS_OBJ) $(TESTING_OBJ) $(LDFLAGS)

output/%.d: %.c
	$(call make_depend,$@)

output/%.d: test/%.c
	$(call make_test_depend,$@)

include $(DEPENDS) $(TEST_DEPENDS)
