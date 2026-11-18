/**
 * @file primitive.c
 * @brief Implementation skeleton of the Primitive ADT (2D Vector Objects)
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

#include "primitive.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "primitive_internal.h"

// ============================================================================
// HELPER GEOMETRIC MATH FUNCTIONS (PROVIDED)
// ============================================================================

/**
 * @brief Tests exact containment of a point inside a circle.
 */
static bool math_circle_contains_point(Point center, double radius, Point pt) {
  double dx = pt.x - center.x;
  double dy = pt.y - center.y;
  return (dx * dx + dy * dy) <= (radius * radius);
}

/**
 * @brief Tests exact containment of a point inside an axis-aligned rectangle.
 */
static bool math_rect_contains_point(Point top_left, double width,
                                     double height, Point pt) {
  return (pt.x >= top_left.x && pt.x <= (top_left.x + width) &&
          pt.y >= top_left.y && pt.y <= (top_left.y + height));
}

/**
 * @brief Tests exact containment of a point near line segment (with tolerance).
 */
static bool math_segment_contains_point(Point p1, Point p2, double tolerance,
                                        Point pt) {
  double l2 = (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
  if (l2 == 0.0) {
    double dx = pt.x - p1.x;
    double dy = pt.y - p1.y;
    return sqrt(dx * dx + dy * dy) <= tolerance;
  }
  double t =
      ((pt.x - p1.x) * (p2.x - p1.x) + (pt.y - p1.y) * (p2.y - p1.y)) / l2;
  if (t < 0.0) {
    t = 0.0;
  } else if (t > 1.0) {
    t = 1.0;
  }

  double proj_x = p1.x + t * (p2.x - p1.x);
  double proj_y = p1.y + t * (p2.y - p1.y);
  double dx = pt.x - proj_x;
  double dy = pt.y - proj_y;

  return sqrt(dx * dx + dy * dy) <= tolerance;
}

/**
 * @brief Tests containment of point inside polygon using Ray-Casting.
 */
static bool math_polygon_contains_point(const Point* vertices,
                                        size_t num_vertices, Point pt) {
  if (num_vertices < 3 || vertices == NULL) {
    return false;
  }

  bool inside = false;
  for (size_t i = 0, j = num_vertices - 1; i < num_vertices; j = i++) {
    double xi = vertices[i].x, yi = vertices[i].y;
    double xj = vertices[j].x, yj = vertices[j].y;

    bool intersect = ((yi > pt.y) != (yj > pt.y)) &&
                     (pt.x < (xj - xi) * (pt.y - yi) / (yj - yi) + xi);
    if (intersect) {
      inside = !inside;
    }
  }
  return inside;
}

// ============================================================================
// INTERNAL AUXILIARY FUNCTIONS (PROVIDED)
// ============================================================================

/**
 * @brief Allocates base Primitive structure and initializes common fields.
 */
static Primitive* primitive_allocate_common(int id, PrimitiveType type,
                                            Color color, double alpha,
                                            int z_index) {
  Primitive* p = calloc(1, sizeof(Primitive));
  if (p == NULL) {
    return NULL;
  }

  p->id = id;
  p->type = type;
  p->color = color;
  p->alpha = alpha;
  p->z_index = z_index;
  p->is_aabb_dirty = true;
  p->aabb.min = (Point){0.0, 0.0};
  p->aabb.max = (Point){0.0, 0.0};

  return p;
}

// ============================================================================
// INSPECTION / GETTER FUNCTIONS (PROVIDED TO STUDENTS)
// ============================================================================

int primitive_get_id(const Primitive* p) {
  if (p == NULL) return -1;
  return p->id;
}

PrimitiveType primitive_get_type(const Primitive* p) {
  assert(p != NULL && "Primitive pointer is NULL");

  return p->type;
}

int primitive_get_z_index(const Primitive* p) {
  if (p == NULL) return 0;
  return p->z_index;
}

Color primitive_get_color(const Primitive* p) {
  if (p == NULL) {
    Color black = {0, 0, 0};
    return black;
  }
  return p->color;
}

double primitive_get_alpha(const Primitive* p) {
  if (p == NULL) return 0.0;
  return p->alpha;
}

BoundingBox primitive_get_aabb(Primitive* p) {
  assert(p != NULL && "Primitive pointer is NULL");

  // Force AABB update
  if (p->is_aabb_dirty) {
    primitive_update_aabb(p);
  }
  return p->aabb;
}

size_t polygon_get_num_vertices(const Primitive* p) {
  if (p == NULL || p->type != PRIM_POLYGON) return 0;
  return p->geom.polygon.num_vertices;
}

// ============================================================================
// CREATION AND DESTRUCTION FUNCTIONS (STUDENTS MUST COMPLETE)
// ============================================================================

/**
 * [PROVIDED AS REFERENCE MODEL]
 * Creates and dynamically allocates a Circle primitive.
 */
Primitive* primitive_create_circle(int id, Point center, double radius,
                                   Color color, double alpha, int z_index) {
  if (radius <= 0.0 || alpha < 0.0 || alpha > 1.0) {
    return NULL;
  }

  Primitive* p =
      primitive_allocate_common(id, PRIM_CIRCLE, color, alpha, z_index);
  if (p == NULL) {
    return NULL;
  }

  p->geom.circle.center = center;
  p->geom.circle.radius = radius;

  // Automatically calculate initial AABB
  primitive_update_aabb(p);

  return p;
}

Primitive* primitive_create_rectangle(int id, Point top_left, double width,
                                      double height, Color color, double alpha,
                                      int z_index) 
{
  // Valida as dimensoes do retangulo e o intervalo permitido para alpha.
  if (width <= 0.0 || height <= 0.0 || alpha < 0.0 || alpha > 1.0) {
    return NULL;
  }

  // Aloca a estrutura base e inicializa a geometria especifica.
  Primitive* p= primitive_allocate_common(id, PRIM_RECTANGLE, color, alpha, z_index);
  if (p==NULL) {
    return NULL;
  }

  p->geom.rectangle.top_left=top_left;
  p->geom.rectangle.width=width;
  p->geom.rectangle.height=height;

  // Calcula imediatamente a caixa envolvente inicial.
  primitive_update_aabb(p);
  return p; //FUNCTION COMPLETED
}

/**
 * Cria um segmento depois de validar a opacidade.
 * A AABB e calculada a partir dos menores e maiores valores dos dois extremos.
 */
Primitive* primitive_create_segment(int id, Point p1, Point p2, Color color,
                                    double alpha, int z_index) {
  if (alpha < 0.0 || alpha > 1.0) {
    return NULL;
  }

  Primitive* p = primitive_allocate_common(id, PRIM_SEGMENT, color, alpha, z_index);
  if (p == NULL) {
    return NULL;
  }

  p->geom.segment.p1 = p1;
  p->geom.segment.p2 = p2;

  primitive_update_aabb(p); //FUNCTION COMPLETED
  return p;
}

/**
 * Cria um poligono vazio.
 * O vetor de vertices e alocado apenas quando o primeiro vertice for inserido.
 */
Primitive* primitive_create_polygon(int id, Color color, double alpha,
                                    int z_index) {
  if (alpha < 0.0 || alpha > 1.0) {
    return NULL;
  }

  Primitive* p = primitive_allocate_common(id, PRIM_POLYGON, color, alpha, z_index);
  if (p == NULL) {
    return NULL;
  }

  p->geom.polygon.num_vertices = 0;
  p->geom.polygon.vertices = NULL;

  // Uma lista vazia tem uma AABB inicial degenerada na origem.
  primitive_update_aabb(p); //FUNCTION COMPLETED
  return p;
}

/**
 * Destroi uma primitiva e liberta a memoria que lhe pertence.
 * Poligonos requerem uma libertacao adicional para o vetor de vertices.
 */
void primitive_destroy(Primitive* p) {
  if (p == NULL) {
    return;
  }

  // Apenas os poligonos possuem memoria dinamica adicional.
  if (p->type == PRIM_POLYGON) {
    free(p->geom.polygon.vertices);
    p->geom.polygon.vertices = NULL;
  }

  free(p); //FUNCTION COMPLETED
}

// ============================================================================
// DYNAMIC ARRAY MANIPULATION (POLYGON) (STUDENTS MUST COMPLETE)
// ============================================================================

/**
 * Adiciona um vertice ao vetor dinamico de um poligono.
 * Quando o vetor fica cheio, a capacidade e duplicada antes da insercao.
 */
bool polygon_add_vertex(Primitive* p, Point pt) {
  if (p==NULL||p->type!=PRIM_POLYGON){
    return false;
  }

  PolygonGeom* polygon = &p->geom.polygon;
  if (polygon->num_vertices >= polygon->capacity) {
    // Duplica a capacidade para manter insercoes eficientes.
    size_t new_capacity = (polygon->capacity == 0) ? 4 : polygon->capacity * 2;
    Point* new_vertices = (Point*)realloc(polygon->vertices, new_capacity * sizeof(Point));
    if (new_vertices == NULL) {
      return false; // Memory allocation failed
    }
    polygon->vertices = new_vertices;
    polygon->capacity = new_capacity;
  }

  polygon->vertices[polygon->num_vertices] = pt;
  polygon->num_vertices++;
  // A AABB sera recalculada quando for consultada.
  p->is_aabb_dirty = true; // Mark AABB as dirty since the polygon has changed
  return true; //FUNCTION COMPLETED
}

/**
 * Remove o vertice indicado e compacta o vetor deslocando os seguintes.
 * A AABB e marcada como desatualizada porque os limites podem ter mudado.
 */
bool polygon_remove_vertex(Primitive* p, size_t index) {
  if (p == NULL || p->type != PRIM_POLYGON || index >= p->geom.polygon.num_vertices) {
    return false;
  }

  PolygonGeom* polygon = &p->geom.polygon;
  for (size_t i = index; i < polygon->num_vertices - 1; i++) {
    polygon->vertices[i] = polygon->vertices[i + 1];
  }

  polygon->num_vertices--;
  // A remocao pode alterar os limites da primitiva.
  p->is_aabb_dirty = true; // Mark AABB as dirty since the polygon has changed

  return true; //FUNCTION COMPLETED
}

// ============================================================================
// GEOMETRIC OPERATIONS AND TRANSFORMATIONS (STUDENTS MUST COMPLETE)
// ============================================================================

/**
 * Recalcula a caixa envolvente axis-aligned da primitiva.
 * O calculo so e feito quando a flag is_aabb_dirty indica dados alterados.
 */
void primitive_update_aabb(Primitive* p) {
  if (p == NULL || !p->is_aabb_dirty) {
    return;
  }

  switch (p->type) {
    case PRIM_CIRCLE:
      p->aabb.min.x = p->geom.circle.center.x - p->geom.circle.radius;
      p->aabb.min.y = p->geom.circle.center.y - p->geom.circle.radius;
      p->aabb.max.x = p->geom.circle.center.x + p->geom.circle.radius;
      p->aabb.max.y = p->geom.circle.center.y + p->geom.circle.radius;
      break;

    case PRIM_RECTANGLE:
      p->aabb.min.x = p->geom.rectangle.top_left.x;
      p->aabb.min.y = p->geom.rectangle.top_left.y;
      p->aabb.max.x = p->geom.rectangle.top_left.x + p->geom.rectangle.width;
      p->aabb.max.y = p->geom.rectangle.top_left.y + p->geom.rectangle.height;
      break;

    case PRIM_SEGMENT:
      p->aabb.min.x = fmin(p->geom.segment.p1.x, p->geom.segment.p2.x);
      p->aabb.min.y = fmin(p->geom.segment.p1.y, p->geom.segment.p2.y);
      p->aabb.max.x = fmax(p->geom.segment.p1.x, p->geom.segment.p2.x);
      p->aabb.max.y = fmax(p->geom.segment.p1.y, p->geom.segment.p2.y);
      break;

    case PRIM_POLYGON: {
      PolygonGeom* poly = &p->geom.polygon;
      if (poly->num_vertices == 0 || poly->vertices == NULL) {
        // Um poligono sem vertices nao possui extensao geometrica.
        p->aabb.min = (Point){0.0, 0.0};
        p->aabb.max = (Point){0.0, 0.0};
      } else {
        // Inicia os limites com o primeiro vertice e percorre os restantes.
        double min_x = poly->vertices[0].x;
        double min_y = poly->vertices[0].y;
        double max_x = poly->vertices[0].x;
        double max_y = poly->vertices[0].y;

        for (size_t i = 1; i < poly->num_vertices; i++) {
          if (poly->vertices[i].x < min_x) min_x = poly->vertices[i].x;
          if (poly->vertices[i].y < min_y) min_y = poly->vertices[i].y;
          if (poly->vertices[i].x > max_x) max_x = poly->vertices[i].x;
          if (poly->vertices[i].y > max_y) max_y = poly->vertices[i].y;
        }

        p->aabb.min = (Point){min_x, min_y};
        p->aabb.max = (Point){max_x, max_y};
      }
      break;
    }
  }

  p->is_aabb_dirty = false;
} //FUNCTION COMPLETED

/**
 * Aplica uma translacao a todos os pontos relevantes da geometria.
 * Depois da operacao, a AABB e invalidada para ser recalculada sob demanda.
 */
void primitive_translate(Primitive* p, double dx, double dy) {
  if (p == NULL) {
    return;
  }
  switch(p->type) {
    case PRIM_CIRCLE:
      p->geom.circle.center.x += dx;
      p->geom.circle.center.y += dy;
      break;
    case PRIM_RECTANGLE:
      p->geom.rectangle.top_left.x += dx;
      p->geom.rectangle.top_left.y += dy;
      break;
    case PRIM_SEGMENT:
      p->geom.segment.p1.x += dx;
      p->geom.segment.p1.y += dy;
      p->geom.segment.p2.x += dx;
      p->geom.segment.p2.y += dy;
      break;
    case PRIM_POLYGON:
      for (size_t i = 0; i < p->geom.polygon.num_vertices; i++) {
        p->geom.polygon.vertices[i].x += dx;
        p->geom.polygon.vertices[i].y += dy;
      }
      break;
  }

  // A translacao altera a posicao da caixa envolvente.
  p->is_aabb_dirty = true;
} //FUNCTION COMPLETED

/**
 * Escala a geometria mantendo o respetivo centro fixo.
 * Para poligonos, o centro usado e o centro da sua AABB atualizada.
 */
void primitive_scale(Primitive* p, double factor) {
  if (p == NULL || factor <= 0.0) {
    return;
  }
  switch (p->type){
    case PRIM_CIRCLE:
      p->geom.circle.radius *= factor;
      break;
    case PRIM_RECTANGLE:
      p->geom.rectangle.width *= factor;
      p->geom.rectangle.height *= factor;
      break;
    case PRIM_SEGMENT: {
      // Mantem o ponto medio fixo enquanto afasta ou aproxima os extremos.
      double mid_x = (p->geom.segment.p1.x + p->geom.segment.p2.x) / 2.0;
      double mid_y = (p->geom.segment.p1.y + p->geom.segment.p2.y) / 2.0;
      p->geom.segment.p1.x = mid_x + (p->geom.segment.p1.x - mid_x) * factor;
      p->geom.segment.p1.y = mid_y + (p->geom.segment.p1.y - mid_y) * factor;
      p->geom.segment.p2.x = mid_x + (p->geom.segment.p2.x - mid_x) * factor;
      p->geom.segment.p2.y = mid_y + (p->geom.segment.p2.y - mid_y) * factor;
      break;
    }
    case PRIM_POLYGON: {
      if (p->geom.polygon.num_vertices > 0) {
        if (p->is_aabb_dirty) {
          // O centro usado na escala deve refletir a geometria atual.
          primitive_update_aabb(p);
        }
        double center_x = (p->aabb.min.x + p->aabb.max.x) / 2.0;
        double center_y = (p->aabb.min.y + p->aabb.max.y) / 2.0;

        for (size_t i = 0; i < p->geom.polygon.num_vertices; i++) {
          p->geom.polygon.vertices[i].x =
              center_x + (p->geom.polygon.vertices[i].x - center_x) * factor;
          p->geom.polygon.vertices[i].y =
              center_y + (p->geom.polygon.vertices[i].y - center_y) * factor;
        }
      }
      break;
    }
  }

  // A escala pode alterar os limites, portanto invalida a AABB atual.
  p->is_aabb_dirty = true;
} //FUNCTION COMPLETED

/**
 * Verifica se um ponto pertence a uma primitiva.
 * Cada tipo delega o teste geometrico no auxiliar especializado correspondente.
 */
bool primitive_contains_point(const Primitive* p, Point pt) {
  if (p == NULL) {
    return false;
  }

  // Narrow-phase exact geometric test dispatch
  switch (p->type) {
    case PRIM_CIRCLE:
      return math_circle_contains_point(p->geom.circle.center,
                                        p->geom.circle.radius, pt);

    case PRIM_RECTANGLE:
      return math_rect_contains_point(p->geom.rectangle.top_left,
                                       p->geom.rectangle.width,
                                       p->geom.rectangle.height, pt);

    case PRIM_SEGMENT:
      return math_segment_contains_point(p->geom.segment.p1, p->geom.segment.p2,
                                          1e-6, pt);

    case PRIM_POLYGON:
      return math_polygon_contains_point(p->geom.polygon.vertices,
                                          p->geom.polygon.num_vertices, pt);
  }

  return false;
} //FUNCTION COMPLETED
