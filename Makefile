# SPDX-License-Identifier: MIT
# Point d'entrée unique : build, tests (C + Python + différentiel), sanitizeurs, garde.
SHELL := /bin/bash
CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -Icore/include -Icore/src
SANFLAGS ?= -std=c11 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer -Icore/include -Icore/src

CORE_SRC := $(wildcard core/src/*.c)
CORE_HDR := $(wildcard core/include/tamis/*.h)
TEST_SRC := $(wildcard core/tests/test_*.c)
TEST_BIN := $(TEST_SRC:.c=)
SAN_BIN  := $(addprefix build/san/,$(notdir $(TEST_BIN)))
CLI      := build/tamis
CLI_SRC  := tools/tamis.c tools/sha256.c $(CORE_SRC)

.PHONY: all test sanitize garde clean cli mutants site aide
all: $(TEST_BIN) cli

# Un binaire de test = son .c + tout le core (petit projet, link simple).
core/tests/test_%: core/tests/test_%.c $(CORE_SRC) $(CORE_HDR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $< $(CORE_SRC)

build/san/test_%: core/tests/test_%.c $(CORE_SRC) $(CORE_HDR)
	@mkdir -p $(dir $@)
	$(CC) $(SANFLAGS) -o $@ $< $(CORE_SRC)

cli: $(CLI)

$(CLI): $(CLI_SRC) $(CORE_HDR) tools/sha256.h
	@mkdir -p build
	$(CC) $(CFLAGS) -Itools -o $@ $(CLI_SRC)

test: $(TEST_BIN) cli
	@tests/lance.sh
	@tests/lance_host.sh

sanitize: $(SAN_BIN)
	@tests/lance.sh build/san

garde:
	@python3 tests/garde.py

mutants:
	@python3 tests/mutants.py

site:
	@python3 tools/build_site.py
	@python3 tools/build_devpost.py

clean:
	rm -rf build $(TEST_BIN)

aide:
	@echo "make test     — construit CLI + tests et lance tout (C + Python + différentiel + CLI + fuzz + JS + site)"
	@echo "make cli      — construit build/tamis"
	@echo "make site     — régénère site/ depuis fixtures/ et docs/"
	@echo "make sanitize — relance les tests C sous ASan/UBSan"
	@echo "make mutants  — 12 mutations du core, chacune doit être tuée par les tests"
	@echo "make garde    — < 500 lignes/fichier, < 50 lignes/fonction"
	@echo "make clean    — supprime les artefacts"
