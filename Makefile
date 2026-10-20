.PHONY: test clean lib dist prepare

INCLUDE_DIR = ./include/
SRC_DIR = ./src/
OBJ_DIR = ./obj/
OUT_DIR = ./bin/
DIST_DIR = ./dist/

CC = clang
CFLAGS = -g -Wall -Wextra -I$(INCLUDE_DIR)

SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)%.c, $(OBJ_DIR)%.o, $(SRCS))

dist: lib prepare
	cp $(OUT_DIR)libcg.a $(DIST_DIR)lib/libcg.a
	cp -r $(INCLUDE_DIR) $(DIST_DIR)

lib: $(OBJS) prepare
	ar rcs $(OUT_DIR)libcg.a $(OBJS)

clean:
	rm -rf $(OUT_DIR)* $(OBJ_DIR)* $(DIST_DIR)*

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $(CFLAGS) -c -o $@ $<


TEST_DIR = ./test/
LDLIBS += -lm

test_%: $(TEST_DIR)%.c lib prepare
	$(CC) $(CFLAGS) -o $(OUT_DIR)$@ $< $(OUT_DIR)libcg.a $(LDLIBS)
	$(OUT_DIR)$@

prepare:
	mkdir -p $(OBJ_DIR) $(OUT_DIR) $(DIST_DIR) $(DIST_DIR)lib