/**
 * bench_pick.c
 * Benchmark experimental para comparar as duas estratégias de picking
 * (Direta vs. Filtragem Hierárquica AABB) através da estrutura PickStats.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "primitive.h"
#include "vector_scene.h"

static double rand_double(double min, double max) {
    return min + ((double)rand() / (double)RAND_MAX) * (max - min);
}
static VectorScene* create_random_scene(int n, int width, int height) {
    Color bg = {255, 255, 255};
    VectorScene* scene = vector_scene_create(width, height, bg);
    if (!scene) return NULL;
    for (int i = 0; i < n; i++) {
        Color col = {
            (uint8_t)(rand() % 256),
            (uint8_t)(rand() % 256),
            (uint8_t)(rand() % 256)
        };
        double alpha = rand_double(0.4, 1.0);
        int z_index = rand() % 100;
        int shape = i % 4;
        Primitive* p = NULL;
        // Primitivas distribuídas pela tela (dimensões típicas 15–45 px)
        double cx = rand_double(20.0, (double)width - 20.0);
        double cy = rand_double(20.0, (double)height - 20.0);
        double size = rand_double(15.0, 45.0);
        if (shape == 0) {
            p = primitive_create_circle(-1, (Point){cx, cy}, size * 0.5, col, alpha, z_index);
        } else if (shape == 1) {
            p = primitive_create_rectangle(-1, (Point){cx - size * 0.5, cy - size * 0.5}, size, size * 0.8, col, alpha, z_index);
        } else if (shape == 2) {
            p = primitive_create_segment(-1, (Point){cx - size * 0.5, cy}, (Point){cx + size * 0.5, cy + size * 0.3}, col, alpha, z_index);
        } else {
            p = primitive_create_polygon(-1, col, alpha, z_index);
            if (p) {
                polygon_add_vertex(p, (Point){cx, cy - size * 0.5});
                polygon_add_vertex(p, (Point){cx + size * 0.5, cy + size * 0.4});
                polygon_add_vertex(p, (Point){cx - size * 0.5, cy + size * 0.4});
            }
        }
        if (p) {
            vector_scene_add_primitive(scene, p);
        }
    }
    return scene;
}
int main(void) {
    srand(42); // Semente fixa para reprodutibilidade
    const int WIDTH = 800;
    const int HEIGHT = 600;
    const int M_QUERIES = 500;
    const int N_VALUES[] = {1000, 10000, 50000};
    const int NUM_N = 3;
    // Gerar M pontos de clique aleatórios na tela
    Point* queries = malloc((size_t)M_QUERIES * sizeof(Point));
    if (!queries) return 1;
    for (int m = 0; m < M_QUERIES; m++) {
        queries[m].x = rand_double(0.0, (double)WIDTH);
        queries[m].y = rand_double(0.0, (double)HEIGHT);
    }
    printf("N,strategy,M,aabb_tests,exact_tests,time_ms\n");
    for (int idx = 0; idx < NUM_N; idx++) {
        int n = N_VALUES[idx];
        VectorScene* scene = create_random_scene(n, WIDTH, HEIGHT);
        if (!scene) continue;
        // 1. Estratégia Direta (is_opt = false)
        PickStats stats_direct = {0, 0};
        clock_t t0 = clock();
        for (int m = 0; m < M_QUERIES; m++) {
            PickStats st = {0, 0};
            vector_scene_pick_top_primitive(scene, queries[m], false, &st);
            stats_direct.aabb_tests += st.aabb_tests;
            stats_direct.exact_tests += st.exact_tests;
        }
        clock_t t1 = clock();
        double time_direct_ms = ((double)(t1 - t0) / CLOCKS_PER_SEC) * 1000.0;
        printf("%d,direct,%d,%zu,%zu,%.3f\n", n, M_QUERIES, stats_direct.aabb_tests, stats_direct.exact_tests, time_direct_ms);
        // 2. Estratégia Otimizada (is_opt = true)
        PickStats stats_opt = {0, 0};
        clock_t t2 = clock();
        for (int m = 0; m < M_QUERIES; m++) {
            PickStats st = {0, 0};
            vector_scene_pick_top_primitive(scene, queries[m], true, &st);
            stats_opt.aabb_tests += st.aabb_tests;
            stats_opt.exact_tests += st.exact_tests;
        }
        clock_t t3 = clock();
        double time_opt_ms = ((double)(t3 - t2) / CLOCKS_PER_SEC) * 1000.0;
        printf("%d,optimized,%d,%zu,%zu,%.3f\n", n, M_QUERIES, stats_opt.aabb_tests, stats_opt.exact_tests, time_opt_ms);
        vector_scene_destroy(scene);
    }
    free(queries);
    return 0;
}
