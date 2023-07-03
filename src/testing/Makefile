CC=gcc
CFLAGS=-Iinclude -I$(COMMONS_INCLUDE) $(GLOBAL_CFLAGS)
SRC=$(wildcard *.c)
OBJ=$(SRC:%.c=output/%.o)
DEPENDS=$(SRC:%.c=output/%.d)

all: $(OBJ)

clean:
	$(call remove,$(OBJ) $(DEPENDS))

output/%.d: %.c
	$(call make_depend,$@)

include $(DEPENDS)
