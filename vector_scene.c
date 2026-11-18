/**
 * @file vector_scene.c
 * @brief Implementation skeleton of the VectorScene ADT (2D Vector Scene)
 *
 * Algoritmos e Estruturas de Dados 2026/2027
 * Departamento de Electrónica, Telecomunicações e Informática - UA
 */

/**
 * STUDENT NAME: Eduardo Rêpas
 * STUDENT NUMBER: 132828
 * STUDENT NAME: Leandro Torralvo
 * STUDENT NUMBER: 131908
 */

#include "vector_scene.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "primitive_internal.h"

// ============================================================================
// CONCRETE STRUCT LAYOUT (OPAQUE ADT)
// ============================================================================

/**
 * @brief Concrete structure definition for VectorScene.
 * Hidden from vector_scene.h to enforce encapsulation.
 */
struct _VectorScene {
  int width;              /**< Canvas width in pixels */
  int height;             /**< Canvas height in pixels */
  Color bg_color;         /**< Default background color */
  Primitive** primitives; /**< Dynamic array of pointers to Primitive */
  size_t count;           /**< Current number of primitives in scene */
  size_t capacity;        /**< Allocated capacity for primitives array */
  int next_auto_id;       /**< Auto-increment ID generator */
};

#define INITIAL_SCENE_CAPACITY 8

// ============================================================================
// HELPER FUNCTIONS (PROVIDED TO STUDENTS)
// ============================================================================

/**
 * @brief Helper to write an RGB pixel buffer to binary PPM file (P6 format).
 */
static bool save_ppm_image(const char* filename, int width, int height,
                           const uint8_t* pixel_buffer) {
  if (!filename || !pixel_buffer || width <= 0 || height <= 0) {
    return false;
  }

  FILE* file = fopen(filename, "wb");
  if (!file) {
    return false;
  }

  fprintf(file, "P6\n%d %d\n255\n", width, height);

  size_t total_bytes = (size_t)width * height * 3;
  size_t written = fwrite(pixel_buffer, 1, total_bytes, file);
  fclose(file);

  return written == total_bytes;
}

// ============================================================================
// ACCESSORS / GETTERS (PROVIDED TO STUDENTS)
// ============================================================================

int vector_scene_get_width(const VectorScene* scene) {
  if (!scene) {
    return 0;
  }
  return scene->width;
}

int vector_scene_get_height(const VectorScene* scene) {
  if (!scene) {
    return 0;
  }
  return scene->height;
}

Color vector_scene_get_background_color(const VectorScene* scene) {
  if (!scene) {
    Color black = {0, 0, 0};
    return black;
  }
  return scene->bg_color;
}

size_t vector_scene_get_count(const VectorScene* scene) {
  if (!scene) {
    return 0;
  }
  return scene->count;
}

Primitive* vector_scene_get_primitive_by_index(const VectorScene* scene,
                                               size_t index) {
  if (!scene || index >= scene->count) {
    return NULL;
  }
  return scene->primitives[index];
}

Primitive* vector_scene_get_primitive_by_id(const VectorScene* scene, int id) {
  if (!scene || id < 0) {
    return NULL;
  }

  for (size_t i = 0; i < scene->count; i++) {
    if (scene->primitives[i] && scene->primitives[i]->id == id) {
      return scene->primitives[i];
    }
  }
  return NULL;
}

// ============================================================================
// SCENE LIFECYCLE (STUDENTS MUST COMPLETE)
// ============================================================================

VectorScene* vector_scene_create(int width, int height, Color bg_color) {
  if (width <= 0 || height <= 0) {
    return NULL;
  }
  VectorScene* vs = (VectorScene*)malloc(sizeof(VectorScene)); // Alocar a memória
  if (vs == NULL){ 
    return NULL; // Devolver se for nulo
  }
  vs->capacity = INITIAL_SCENE_CAPACITY;
  vs->width=width;
  vs->height=height;
  vs->bg_color=bg_color;
  vs->next_auto_id=0; // No início, define-se que o próximo id será 0
  vs->count=0; // Count é 0, porque não há elementos
  Primitive** p = (Primitive**)malloc((vs->capacity)*(sizeof(Primitive*))); // Alocar a memória
  if (p == NULL){
    free(vs); // Libertar o vs para não haver fugas de memória
    return NULL;
  }
  vs->primitives=p;
  return vs;
}

void vector_scene_destroy(VectorScene* scene) {
  if (!scene) {
    return;
  }
  for (size_t i=0; i<scene->count; i++){
    primitive_destroy(scene->primitives[i]); // Destruir todos os primitivos 1 por 1
  }
  free(scene->primitives); // Liberta-se o array de primitivos
  free(scene); // Dão-nos diretamente a scene e não um ponteiro, portanto, neste caso não se faz *scene = NULL depois de libertar a memória

}

// ============================================================================
// COLLECTION MANAGEMENT (STUDENTS MUST COMPLETE)
// ============================================================================

int vector_scene_add_primitive(VectorScene* scene, Primitive* p) {
  if (!scene || !p) {
    return -1;
  }
  if (scene->count == scene->capacity){
    size_t new_capacity = scene->capacity*2; // Se a scene estiver cheia, duplica-se a capacidade
    Primitive** primitives = (Primitive**)realloc(scene->primitives, new_capacity*sizeof(Primitive*)); // Realocação de memória
    if (primitives == NULL){
      return -1;
    }
      scene->primitives = primitives;
      scene->capacity=new_capacity;
  }
  if (p->id < 0){
    p->id = scene->next_auto_id++; // Se o id for negativo, o sistema atribui ao elemento um id
  }
  else if (p->id >= scene->next_auto_id){
    scene->next_auto_id = (p->id) + 1; // Se o id foi maior do que o next_auto_id, corrige-se o valor do next_auto_id
  }
  scene->primitives[scene->count] = p; // O índice do primitivo será scene->count, por exemplo, se adicionarmos o segundo primitivo, primitives[1] corresponde a p
  scene->count++; // Mais um primitivo
  primitive_update_aabb(p); // Chamar a função
  return p->id;
}

bool vector_scene_remove_primitive(VectorScene* scene, int id) {
  if (!scene || id < 0) {
    return false;
  }
  bool destruido = false;
  for (size_t i = 0; i < scene->count; i++){
    if (destruido){
      scene->primitives[i-1]=scene->primitives[i]; // Se o primitivo já foi destruido, todos os restantes primitivos recuam uma posição para preservar a ordem relativa
    }
    else if (scene->primitives[i]->id == id){ // Destroi-se o primitivo se o id dado corresponder ao id do primitivo desta iteração
      primitive_destroy(scene->primitives[i]); // Destruir o primitivo
      destruido = true; // Definir que o primitivo já foi destruido
    }
  }
  if (destruido){
    scene->count--;
    return true; // Se o elemento for destruido, reduz-se o count 1 unidade e dá-se return true
  }
  return false; // Se o elemento não foi destruido, return false
}

// ============================================================================
// MANIPULATION AND TRANSFORMATIONS (STUDENTS MUST COMPLETE)
// ============================================================================

bool vector_scene_translate_primitive(VectorScene* scene, int id, double dx,
                                      double dy) {
  if (!scene) {
    return false;
  }
  for (size_t i = 0; i < scene->count; i++){
    if (scene->primitives[i]->id == id){
      primitive_translate(scene->primitives[i], dx, dy); // Mudar a posição do primitivo
      return true; 
    }
  }
  return false; // return false se o elemento não foi encontrado
}

bool vector_scene_set_z_index(VectorScene* scene, int id, int new_z_index) {
  if (!scene) {
    return false;
  }
  for (size_t i = 0; i < scene->count; i++){
    if (scene->primitives[i]->id == id){
      scene->primitives[i]->z_index=new_z_index; // Redefinir o z do primitivo
      return true;
    }
  }
  return false;
}

// ============================================================================
// SPATIAL SELECTION AND SORTING (STUDENTS MUST COMPLETE & ANALYZE)
// ============================================================================

/**
 * @brief Sorts an array of primitives by z_index using O(N^2) Insertion Sort.
 *
 * Iterative in-place sorting algorithm. Iterates through the primitive array
 * and inserts each element into its correct sorted position based on z_index.
 * Increments comparison and swap/move metrics in the SortStats structure.
 *
 * @param arr Array of Primitive pointers to sort.
 * @param count Number of elements in the array.
 * @param stats Pointer to SortStats structure to record performance metrics.
 */
static void insertion_sort_by_z(Primitive** arr, size_t count,
                                SortStats* stats) {
  if (count < 2){
    return; // Se o array tiver 0 ou 1 primitivo(s), não é preciso de usar o algoritmo, portanto, dá-se return
  }
  for (size_t i = 1; i < count; i++){
    Primitive* key = arr[i]; 
    size_t j = i;
    while (j > 0){ // Só há comparação se existir elemento à esquerda (j > 0), por isso o contador é incrementado aqui e não depois do ciclo
      stats->comparisons++;
      if (arr[j-1]->z_index <= key->z_index){ // O anterior já não é maior: a key fica aqui
        break;
      }
      arr[j] = arr[j - 1]; // Desloca o elemento maior uma posição para a direita
      stats->swaps++; // Conta-se cada deslocamento
      j--; // O buraco recua uma posição
    }
    arr[j] = key; // Coloca a key no buraco
  }
}

/**
 * @brief Merges two sorted subarrays of primitive pointers by z_index.
 *
 * Helper function for MergeSort that combines [left..mid] and [mid+1..right]
 * using a temporary pointer array. Increments comparisons and swaps/moves
 * counters in the SortStats structure.
 *
 * @param arr Array of Primitive pointers to merge.
 * @param temp_arr Auxiliary temporary array of Primitive pointers.
 * @param left Starting index of the first subarray.
 * @param mid Middle index separating the two subarrays.
 * @param right Ending index of the second subarray.
 * @param stats Pointer to SortStats structure to track metrics (can be NULL).
 */
static void merge_by_z(Primitive** arr, Primitive** temp_arr, size_t left,
                       size_t mid, size_t right, SortStats* stats) {
  size_t i = left; // Frente da metade esquerda [left..mid]
  size_t j = mid + 1; // Frente da metade direita [mid+1..right]
  size_t k = left; // Posição de escrita no temp_arr

  while (i <= mid && j <= right) {
    stats->comparisons++; // Única fase em que se comparam dois z_index
    if (arr[j]->z_index < arr[i]->z_index) { // O z da direita é estritamente menor: sai primeiro o da direita
      temp_arr[k++] = arr[j++];
    } else { // Caso contrário (z da esquerda menor ou igual) sai o da esquerda
      temp_arr[k++] = arr[i++];
    }
    stats->swaps++; // Cópia para o temp_arr
  }
  while (i <= mid) { // Sobras da metade esquerda (a direita esgotou-se): não há comparações
    temp_arr[k++] = arr[i++];
    stats->swaps++;
  }
  while (j <= right) { // Sobras da metade direita (a esquerda esgotou-se): não há comparações
    temp_arr[k++] = arr[j++];
    stats->swaps++;
  }
  for (size_t l = left; l <= right; l++) {
    arr[l] = temp_arr[l]; // Copia o resultado de volta para o arr
    stats->swaps++; // Troca feita
  }
}

/**
 * @brief Sorts a subarray of primitives by z_index using O(N log N) MergeSort.
 *
 * Stable recursive Divide-and-Conquer algorithm. Splits the array in half,
 * recursively sorts both halves, and invokes merge_by_z() to merge them.
 *
 * @param arr Array of Primitive pointers to sort.
 * @param temp_arr Auxiliary temporary array allocated for merging.
 * @param left Starting index of the subarray to sort.
 * @param right Ending index of the subarray to sort.
 * @param stats Pointer to SortStats structure to record performance metrics.
 */
static void merge_sort_by_z(Primitive** arr, Primitive** temp_arr, size_t left,
                            size_t right, SortStats* stats) {
  if (left >= right){
    return; // left não pode ser maior ou igual ao right, portanto, quando acontece, dá-se return
  }
  size_t mid = left + (right-left)/2; // Define-se o meio
  merge_sort_by_z(arr, temp_arr, left, mid, stats);
  merge_sort_by_z(arr, temp_arr, mid+1, right, stats); // Divide-se em 2 casos
  merge_by_z(arr, temp_arr, left, mid, right, stats); // Junta-se ambas as metades
}

void vector_scene_sort_by_z(VectorScene* scene, bool use_fast_sort,
                            SortStats* stats) {
  if (scene == NULL || scene->count < 2) return;
  stats->comparisons = 0; stats->swaps = 0; // Stats resetados
  if (use_fast_sort){
    Primitive** temp_arr = (Primitive**)malloc(scene->count*sizeof(Primitive*)); // Criar o array temp_arr
    if (temp_arr != NULL) {
      merge_sort_by_z(scene->primitives, temp_arr, 0, scene->count - 1, stats); // Se quiser usar o método rápido de sorting (e a criação do "temp_arr" não falhar) usa-se o merge sorting
      free(temp_arr); // Liberta-se a memória
    } else {
      insertion_sort_by_z(scene->primitives, scene->count, stats); // Troca-se o insertion sort se falhar a criação do "temp_arr"
    }
  } else {
    insertion_sort_by_z(scene->primitives, scene->count, stats); // Se não quiser usar o método rápido de sorting, usa-se diretamente o insertion sort
  }
}

Primitive* vector_scene_pick_top_primitive(const VectorScene* scene, Point pt,
                                           bool is_opt, PickStats* stats) {
  if (!scene) {
    return NULL;
  }
  stats->aabb_tests = 0;  
  stats->exact_tests = 0; // Repor os contadores antes de começar
  Primitive* maior = NULL;
  for (size_t i = 0; i < scene->count; i++) { // Percorre todas as primitivas e guarda a mais à frente que contém o ponto
    Primitive* p = scene->primitives[i];
    bool hit = false;  // Esta primitiva contém o ponto?

    if (is_opt) { // Estratégia otimizada: fase 1 (broad-phase), teste barato da AABB
      stats->aabb_tests++;
      BoundingBox box = primitive_get_aabb(p);
      if (pt.x >= box.min.x && pt.x <= box.max.x && pt.y >= box.min.y && pt.y <= box.max.y) { // Fase 2 (narrow-phase): o teste exato só corre se a AABB contiver o ponto        
        stats->exact_tests++;
        hit = primitive_contains_point(p, pt);
      }
    } else { // Estratégia direta: teste exato em todas as primitivas, sem AABB
      stats->exact_tests++;
      hit = primitive_contains_point(p, pt);
    }
    if (hit) { // Fica a de maior z_index. Em caso de empate fica a de índice maior, que é a desenhada por cima
      if (maior == NULL || p->z_index >= maior->z_index) {
        maior = p;
      }
    }
  }
  return maior; // NULL se nenhuma primitiva contém o ponto
}

// ============================================================================
// RENDERING / RASTERIZATION (STUDENTS MUST COMPLETE)
// ============================================================================

bool vector_scene_render(const VectorScene* scene,
                         const char* output_filename) {
  if (!scene || !output_filename) {
    return false;
  }
  int w = scene->width;
  int h = scene->height;

  uint8_t* buffer = malloc((size_t)w * h * 3); // Buffer de pixeis
  if (buffer == NULL) {
    return false;
  }
  for (size_t i = 0; i < (size_t)w * h; i++) { // Preencher tudo com a cor de fundo
    buffer[i * 3] = scene->bg_color.r;
    buffer[i * 3 + 1] = scene->bg_color.g;
    buffer[i * 3 + 2] = scene->bg_color.b;
  }

  Primitive** sorted = NULL;
  if (scene->count > 0) { // Algoritmo do Pintor
    sorted = malloc(scene->count * sizeof(Primitive*));
    if (sorted == NULL) {
      free(buffer); // Libertar o buffer antes de dar return
      return false;
    }
    memcpy(sorted, scene->primitives, scene->count * sizeof(Primitive*));
    SortStats unused = {0, 0};
    insertion_sort_by_z(sorted, scene->count, &unused);
  }

  for (size_t i = 0; i < scene->count; i++) { // Desenhar do fundo para a frente (menor z primeiro)
    Primitive* p = sorted[i];
    BoundingBox box = primitive_get_aabb(p); // AABB atualizada se necessário
    if (box.max.x < 0 || box.min.x > w - 1 || box.max.y < 0 || box.min.y > h - 1) { // Se a AABB está completamente fora da imagem, não há nada para desenhar
      continue;
    }
    double x0 = floor(box.min.x), x1 = ceil(box.max.x);
    double y0 = floor(box.min.y), y1 = ceil(box.max.y); // Converter a AABB em índices de píxeis e cortá-la aos limites da imagem (clipping), para nunca escrever fora do buffer
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > w - 1) x1 = w - 1;
    if (y1 > h - 1) y1 = h - 1;
    int x_min = (int)x0, x_max = (int)x1;
    int y_min = (int)y0, y_max = (int)y1;
    Color c = primitive_get_color(p);
    double a = primitive_get_alpha(p);
    for (int y = y_min; y <= y_max; y++) { // Só se testam os píxeis dentro da AABB (já cortada)
      for (int x = x_min; x <= x_max; x++) {
        Point center = {x + 0.5, y + 0.5}; // Testa-se o centro do píxel
        if (primitive_contains_point(p, center)) {
          size_t idx = ((size_t)y * w + x) * 3;
          buffer[idx] = (uint8_t)(a * c.r + (1.0 - a) * buffer[idx] + 0.5);
          buffer[idx + 1] = (uint8_t)(a * c.g + (1.0 - a) * buffer[idx + 1] + 0.5);
          buffer[idx + 2] = (uint8_t)(a * c.b + (1.0 - a) * buffer[idx + 2] + 0.5);
        }
      }
    }
  }

  bool ok = save_ppm_image(output_filename, w, h, buffer);
  free(sorted);
  free(buffer);
  return ok;
}

// ============================================================================
// FILE INPUT / OUTPUT (PROVIDED TO STUDENTS)
// ============================================================================

bool vector_scene_save_text(const VectorScene* scene, const char* filename) {
  if (!scene || !filename) {
    return false;
  }

  FILE* file = fopen(filename, "w");
  if (!file) {
    return false;
  }

  Color bg = scene->bg_color;
  fprintf(file, "SCENE %d %d %d %d %d\n", scene->width, scene->height, bg.r,
          bg.g, bg.b);

  for (size_t i = 0; i < scene->count; i++) {
    Primitive* p = scene->primitives[i];
    if (!p) {
      continue;
    }

    Color c = p->color;

    switch (p->type) {
      case PRIM_CIRCLE:
        fprintf(file, "CIRC %d %d %d %d %d %.3f %.2f %.2f %.2f\n", p->id,
                p->z_index, c.r, c.g, c.b, p->alpha, p->geom.circle.center.x,
                p->geom.circle.center.y, p->geom.circle.radius);
        break;

      case PRIM_RECTANGLE:
        fprintf(file, "RECT %d %d %d %d %d %.3f %.2f %.2f %.2f %.2f\n", p->id,
                p->z_index, c.r, c.g, c.b, p->alpha,
                p->geom.rectangle.top_left.x, p->geom.rectangle.top_left.y,
                p->geom.rectangle.width, p->geom.rectangle.height);
        break;

      case PRIM_SEGMENT:
        fprintf(file, "SEG %d %d %d %d %d %.3f %.2f %.2f %.2f %.2f\n", p->id,
                p->z_index, c.r, c.g, c.b, p->alpha, p->geom.segment.p1.x,
                p->geom.segment.p1.y, p->geom.segment.p2.x,
                p->geom.segment.p2.y);
        break;

      case PRIM_POLYGON:
        fprintf(file, "POLY %d %d %d %d %d %.3f %zu", p->id, p->z_index, c.r,
                c.g, c.b, p->alpha, p->geom.polygon.num_vertices);

        for (size_t v = 0; v < p->geom.polygon.num_vertices; v++) {
          fprintf(file, " %.2f %.2f", p->geom.polygon.vertices[v].x,
                  p->geom.polygon.vertices[v].y);
        }
        fprintf(file, "\n");
        break;
    }
  }

  fclose(file);
  return true;
}

VectorScene* vector_scene_load_text(const char* filename) {
  if (!filename) {
    return NULL;
  }

  FILE* file = fopen(filename, "r");
  if (!file) {
    return NULL;
  }

  char line[1024];
  VectorScene* scene = NULL;

  while (fgets(line, sizeof(line), file)) {
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
      continue;
    }

    char tag[16];
    if (sscanf(line, "%15s", tag) != 1) {
      continue;
    }

    if (strcmp(tag, "SCENE") == 0) {
      int w, h, r, g, b;
      if (sscanf(line, "SCENE %d %d %d %d %d", &w, &h, &r, &g, &b) == 5) {
        Color bg = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
        scene = vector_scene_create(w, h, bg);
      }
    } else if (scene != NULL) {
      int id, z_index, r, g, b;
      double alpha;

      if (strcmp(tag, "CIRC") == 0) {
        double cx, cy, radius;
        if (sscanf(line, "CIRC %d %d %d %d %d %lf %lf %lf %lf", &id, &z_index,
                   &r, &g, &b, &alpha, &cx, &cy, &radius) == 9) {
          Point center = {cx, cy};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p = primitive_create_circle(id, center, radius, color,
                                                 alpha, z_index);
          if (p) {
            vector_scene_add_primitive(scene, p);
          }
        }
      } else if (strcmp(tag, "RECT") == 0) {
        double x, y, w, h;
        if (sscanf(line, "RECT %d %d %d %d %d %lf %lf %lf %lf %lf", &id,
                   &z_index, &r, &g, &b, &alpha, &x, &y, &w, &h) == 10) {
          Point top_left = {x, y};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p = primitive_create_rectangle(id, top_left, w, h, color,
                                                    alpha, z_index);
          if (p) {
            vector_scene_add_primitive(scene, p);
          }
        }
      } else if (strcmp(tag, "SEG") == 0) {
        double x1, y1, x2, y2;
        if (sscanf(line, "SEG %d %d %d %d %d %lf %lf %lf %lf %lf", &id,
                   &z_index, &r, &g, &b, &alpha, &x1, &y1, &x2, &y2) == 10) {
          Point p1 = {x1, y1};
          Point p2 = {x2, y2};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p =
              primitive_create_segment(id, p1, p2, color, alpha, z_index);
          if (p) {
            vector_scene_add_primitive(scene, p);
          }
        }
      } else if (strcmp(tag, "POLY") == 0) {
        size_t num_verts;
        int read_header = sscanf(line, "POLY %d %d %d %d %d %lf %zu", &id,
                                 &z_index, &r, &g, &b, &alpha, &num_verts);
        if (read_header == 7) {
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p = primitive_create_polygon(id, color, alpha, z_index);
          if (p) {
            char* ptr = line;
            for (int k = 0; k < 7; k++) {
              while (*ptr && *ptr != ' ' && *ptr != '\t') {
                ptr++;
              }
              while (*ptr && (*ptr == ' ' || *ptr == '\t')) {
                ptr++;
              }
            }

            for (size_t v = 0; v < num_verts; v++) {
              double vx, vy;
              if (sscanf(ptr, "%lf %lf", &vx, &vy) == 2) {
                Point pt = {vx, vy};
                polygon_add_vertex(p, pt);

                for (int k = 0; k < 2; k++) {
                  while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' &&
                         *ptr != '\r') {
                    ptr++;
                  }
                  while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' ||
                                  *ptr == '\r')) {
                    ptr++;
                  }
                }
              }
            }
            vector_scene_add_primitive(scene, p);
          }
        }
      }
    }
  }

  fclose(file);
  return scene;
}
