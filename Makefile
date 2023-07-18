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

export COMMONS_INCLUDE=$(COMMONS_DIR:$(SRC_DIR)/%=../%)/include
COMMONS_SRC=$(wildcard $(COMMONS_DIR)/*.c)
COMMONS_OBJ_ABS=$(COMMONS_SRC:$(COMMONS_DIR)/%.c=$(COMMONS_DIR)/output/%.o)
export COMMONS_OBJ=$(COMMONS_OBJ_ABS:$(SRC_DIR)/%=../%)

export TESTING_INCLUDE=$(TESTING_DIR:$(SRC_DIR)/%=../%)/include
TESTING_SRC=$(wildcard $(TESTING_DIR)/*.c)
TESTING_OBJ_ABS=$(TESTING_SRC:$(TESTING_DIR)/%.c=$(TESTING_DIR)/output/%.o)
export TESTING_OBJ=$(TESTING_OBJ_ABS:$(SRC_DIR)/%=../%)

export GLOBAL_CFLAGS=-g -Wall -Wextra -Wpedantic -Werror -fanalyzer -Wno-analyzer-use-of-uninitialized-value

all:
	@$(MAKE) -C $(COMMONS_DIR)
	@$(MAKE) -C $(TESTING_DIR)
	@$(MAKE) -C $(KONC4D_DIR)
	@$(MAKE) -C $(KONC4_DIR)

clean:
	@$(MAKE) -C $(COMMONS_DIR) clean
	@$(MAKE) -C $(TESTING_DIR) clean
	@$(MAKE) -C $(KONC4D_DIR) clean
	@$(MAKE) -C $(KONC4_DIR) clean

test_commons: all
	@$(MAKE) -C $(COMMONS_DIR) test

test_konc4d: all
	@$(MAKE) -C $(KONC4D_DIR) test

test_konc4: all
	@$(MAKE) -C $(KONC4_DIR) test

test: test_commons test_konc4d test_konc4

active: all
	powershell.exe rm -Recurse -Force active; mkdir $(ACTIVE_DIR); cp $(KONC4D_DIR)/output/konc4d.exe active/; cp $(KONC4_DIR)/output/konc4.exe active/; cp -Recurse asset/ active/
