# ============================================================================
# Makefile for AED 2026/2027 1st Project: VectorScene ADT
# ============================================================================

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c17 -g -O2
LDFLAGS = -lm

# Test suite target
TEST_SRC = unit_tests.c primitive.c vector_scene.c
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_TARGET = unit_tests

# Demo application target
DEMO_SRC = demo_app.c primitive.c vector_scene.c
DEMO_OBJ = $(DEMO_SRC:.c=.o)
DEMO_TARGET = demo_app

# Benchmark targets
BENCH_PICK_SRC = bench_pick.c primitive.c vector_scene.c
BENCH_PICK_OBJ = $(BENCH_PICK_SRC:.c=.o)
BENCH_PICK_TARGET = bench_pick

BENCH_SORT_SRC = bench_sort.c primitive.c vector_scene.c
BENCH_SORT_OBJ = $(BENCH_SORT_SRC:.c=.o)
BENCH_SORT_TARGET = bench_sort

all: $(TEST_TARGET) $(DEMO_TARGET) $(BENCH_PICK_TARGET) $(BENCH_SORT_TARGET)

$(TEST_TARGET): $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(DEMO_TARGET): $(DEMO_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BENCH_PICK_TARGET): $(BENCH_PICK_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BENCH_SORT_TARGET): $(BENCH_SORT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

test-fase1: $(TEST_TARGET)
	./$(TEST_TARGET) 1

test-fase2: $(TEST_TARGET)
	./$(TEST_TARGET) 2

test-fase3: $(TEST_TARGET)
	./$(TEST_TARGET) 3

test-fase4: $(TEST_TARGET)
	./$(TEST_TARGET) 4

test-fase5: $(TEST_TARGET)
	./$(TEST_TARGET) 5

demo: $(DEMO_TARGET)
	./$(DEMO_TARGET)

memcheck: $(TEST_TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TEST_TARGET)

clean:
	rm -f *.o $(TEST_TARGET) $(DEMO_TARGET) $(BENCH_PICK_TARGET) $(BENCH_SORT_TARGET) test_render.ppm created_scene.vsc created_scene.ppm modified_scene.vsc modified_scene.ppm

.PHONY: all test test-fase1 test-fase2 test-fase3 test-fase4 test-fase5 demo memcheck clean
