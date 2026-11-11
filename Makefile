# SPDX-License-Identifier: MIT
# Point d'entrée unique : build, tests (C + Python + différentiel), sanitizeurs, garde.
SHELL := /bin/bash
CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -Icore/include
SANFLAGS ?= -std=c11 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer -Icore/include

CORE_SRC := $(wildcard core/src/*.c)
CORE_HDR := $(wildcard core/include/tamis/*.h)
TEST_SRC := $(wildcard core/tests/test_*.c)
TEST_BIN := $(TEST_SRC:.c=)
SAN_BIN  := $(addprefix build/san/,$(notdir $(TEST_BIN)))

.PHONY: all test sanitize garde clean aide
all: $(TEST_BIN)

# Un binaire de test = son .c + tout le core (petit projet, link simple).
core/tests/test_%: core/tests/test_%.c $(CORE_SRC) $(CORE_HDR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $< $(CORE_SRC)

build/san/test_%: core/tests/test_%.c $(CORE_SRC) $(CORE_HDR)
	@mkdir -p $(dir $@)
	$(CC) $(SANFLAGS) -o $@ $< $(CORE_SRC)

test: $(TEST_BIN)
	@tests/lance.sh
	@tests/lance_host.sh

sanitize: $(SAN_BIN)
	@tests/lance.sh build/san

garde:
	@python3 tests/garde.py

clean:
	rm -rf build $(TEST_BIN)

aide:
	@echo "make test     — construit et lance tous les tests (C + Python + différentiel)"
	@echo "make sanitize — relance les tests C sous ASan/UBSan"
	@echo "make garde    — < 500 lignes/fichier, < 50 lignes/fonction"
	@echo "make clean    — supprime les artefacts"
