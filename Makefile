CC = gcc
CFLAGS = --std=c2x -Wall -Wextra -Wunused -Wno-override-init -g -O1

FORMAT = clang-format

SRC_DIR = ./src
OBJ_DIR = ./obj

SRC_FILES = $(wildcard ${SRC_DIR}/*.c)
OBJ_FILES = $(patsubst %.c,${OBJ_DIR}/%.o,$(notdir ${SRC_FILES}))

TARGET = shelly

${TARGET}: ${OBJ_FILES}
	${CC} ${CFLAGS} -o $@ $^

${OBJ_DIR}:
	mkdir -p ${OBJ_DIR}

${OBJ_DIR}/%.o: ${SRC_DIR}/%.c | ${OBJ_DIR}
	${CC} ${CFLAGS} -o $@ -c $<

all: ${TARGET}

format:
	${FORMAT} -i ${SRC_DIR}/* || true

clean: format
	rm -rf ${TARGET} ${OBJ_DIR}