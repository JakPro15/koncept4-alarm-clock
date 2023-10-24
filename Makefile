NULL :=
export SPACE := $(NULL) #
export COMMA := ,$(SPACE)

export define remove
	@echo remove $$(1)
	@powershell.exe rm -ErrorAction Ignore $$(subst $$(SPACE),$$(COMMA),$$(strip $$(1))); exit 0
endef

export define make_depend
	@echo make_depend $$(1)
	@$$(CC) $$(CFLAGS) -MM "$$(1:output/%.d=%.c)" -MQ "$$(1:%.d=%.o)" > $$(1)
	@echo 	$$$$(CC) $$$$(CFLAGS) -c -o $$$$@ $$$$(@:output/%%.o=%%.c)>> $$(1)
	@$$(CC) $$(CFLAGS) -MM "$$(1:output/%.d=%.c)" -MQ "$$(1)" >> $$(1)
endef

export define make_test_depend
	@echo make_depend $$(1)
	@$$(CC) $$(CFLAGS) -MM "$$(1:output/%.d=test/%.c)" -MQ "$$(1:%.d=%.o)" > $$(1)
	@echo 	$$$$(CC) $$$$(CFLAGS) -c -o $$$$@ $$$$(@:output/%%.o=test/%%.c)>> $$(1)
	@$$(CC) $$(CFLAGS) -MM "$$(1:output/%.d=test/%.c)" -MQ "$$(1)" >> $$(1)
endef

SRC_DIR=src
COMMONS_DIR=$(SRC_DIR)/commons
TESTING_DIR=$(SRC_DIR)/testing
ACTIVE_DIR=active
KONC4_DIR=$(SRC_DIR)/konc4
KONC4D_DIR=$(SRC_DIR)/konc4d
INTEGRATION_DIR=$(SRC_DIR)/integration_test

export KONC4_INCLUDE=$(KONC4_DIR:$(SRC_DIR)/%=../%)/include
KONC4_SRC=$(wildcard $(KONC4_DIR)/*.c)
KONC4_OBJ_ABS=$(KONC4_SRC:$(KONC4_DIR)/%.c=$(KONC4_DIR)/output/%.o)
export KONC4_OBJ=$(KONC4_OBJ_ABS:$(SRC_DIR)/%=../%)

export KONC4D_INCLUDE=$(KONC4D_DIR:$(SRC_DIR)/%=../%)/include
KONC4D_SRC=$(wildcard $(KONC4D_DIR)/*.c)
KONC4D_OBJ_ABS=$(KONC4D_SRC:$(KONC4D_DIR)/%.c=$(KONC4D_DIR)/output/%.o)
export KONC4D_OBJ=$(KONC4D_OBJ_ABS:$(SRC_DIR)/%=../%)

export COMMONS_INCLUDE=$(COMMONS_DIR:$(SRC_DIR)/%=../%)/include
COMMONS_SRC=$(wildcard $(COMMONS_DIR)/*.c)
COMMONS_OBJ_ABS=$(COMMONS_SRC:$(COMMONS_DIR)/%.c=$(COMMONS_DIR)/output/%.o) $(COMMONS_DIR)/output/regex.o
export COMMONS_OBJ=$(COMMONS_OBJ_ABS:$(SRC_DIR)/%=../%)

export TESTING_INCLUDE=$(TESTING_DIR:$(SRC_DIR)/%=../%)/include
TESTING_SRC=$(wildcard $(TESTING_DIR)/*.c)
TESTING_OBJ_ABS=$(TESTING_SRC:$(TESTING_DIR)/%.c=$(TESTING_DIR)/output/%.o)
export TESTING_OBJ=$(TESTING_OBJ_ABS:$(SRC_DIR)/%=../%)

CFLAGS_EXCEPT_OPTIMIZATION=-Wall -Wextra -Wpedantic -Werror -fanalyzer -Wno-stringop-overflow -Wno-analyzer-use-of-uninitialized-value -Wno-analyzer-malloc-leak -Wno-maybe-uninitialized
ifdef DEBUG
export GLOBAL_CFLAGS= -g -Og $(CFLAGS_EXCEPT_OPTIMIZATION)
$(info DEBUG compilation)
else
export GLOBAL_CFLAGS= -O3 -flto -ffat-lto-objects -fuse-linker-plugin $(CFLAGS_EXCEPT_OPTIMIZATION)
$(info RELEASE compilation)
endif

all:
	@$(MAKE) -C $(TESTING_DIR)
	@$(MAKE) -C $(COMMONS_DIR)
	@$(MAKE) -C $(KONC4D_DIR)
	@$(MAKE) -C $(KONC4_DIR)
	@$(MAKE) -C $(INTEGRATION_DIR)

clean:
	@$(MAKE) -C $(TESTING_DIR) clean
	@$(MAKE) -C $(COMMONS_DIR) clean
	@$(MAKE) -C $(KONC4D_DIR) clean
	@$(MAKE) -C $(KONC4_DIR) clean
	@$(MAKE) -C $(INTEGRATION_DIR) clean

stop_konc4d: all
	src\konc4\output\konc4.exe < stop_command.txt

test_commons: stop_konc4d
	@$(MAKE) -C $(COMMONS_DIR) test

test_konc4d: stop_konc4d
	@$(MAKE) -C $(KONC4D_DIR) test

test_konc4: stop_konc4d
	@$(MAKE) -C $(KONC4_DIR) test

test: test_commons test_konc4d test_konc4

active: stop_konc4d
	powershell.exe rm -Recurse -Force active; mkdir $(ACTIVE_DIR); cp $(KONC4D_DIR)/output/konc4d.exe active/; cp $(KONC4_DIR)/output/konc4.exe active/; cp -Recurse asset/ active/

test_integration: active
	@$(MAKE) -C $(INTEGRATION_DIR) test
