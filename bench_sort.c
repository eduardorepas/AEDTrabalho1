#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "vector_scene.h"

typedef enum { RANDOM, SORTED, REVERSED } Order;

// Cena com n círculos; o z_index de cada um depende da ordem pedida.
static VectorScene* build_scene(int n, Order order, unsigned seed) {
  VectorScene* s = vector_scene_create(100, 100, (Color){0, 0, 0});
  if (s == NULL) exit(1);
  srand(seed);
  for (int i = 0; i < n; i++) {
    int z;
    if (order == RANDOM) z = rand() % 1000000;
    else if (order == SORTED) z = i;
    else z = n - i;
    Primitive* p = primitive_create_circle(-1, (Point){50, 50}, 5,
                                           (Color){255, 0, 0}, 1.0, z);
    if (p == NULL || vector_scene_add_primitive(s, p) < 0) exit(1);
  }
  return s;
}

int main(void) {
  const char* order_names[] = {"random", "sorted", "reversed"};
  int sizes[] = {100, 200, 500, 1000, 2000, 5000,
                 10000, 20000, 50000, 100000};
  int n_sizes = sizeof(sizes) / sizeof(sizes[0]);

  printf("order,algo,N,comparisons,swaps,time_ms\n");

  for (int o = 0; o < 3; o++) {
    for (int k = 0; k < n_sizes; k++) {
      int n = sizes[k];
      int reps = (n >= 20000) ? 1 : 5;     // menos repetições nos N grandes
      for (int fast = 0; fast <= 1; fast++) {
        double comp = 0, swaps = 0, ms = 0;
        for (int r = 0; r < reps; r++) {
          VectorScene* s = build_scene(n, (Order)o, 1000 + r);
          SortStats st = {0, 0};

          clock_t t0 = clock();                       // só o sort é cronometrado
          vector_scene_sort_by_z(s, fast == 1, &st);
          ms += 1000.0 * (double)(clock() - t0) / CLOCKS_PER_SEC;

          comp += (double)st.comparisons;
          swaps += (double)st.swaps;
          vector_scene_destroy(s);
        }
        printf("%s,%s,%d,%.0f,%.0f,%.3f\n", order_names[o],
               fast ? "merge" : "insertion", n, comp / reps, swaps / reps,
               ms / reps);
        fflush(stdout);
      }
    }
  }
  return 0;
}