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
  // Largura e altura devem ser positivas para representar um retangulo valido.
  // O alpha representa opacidade e so pode variar entre 0.0 (transparente)
  // e 1.0 (completamente opaco).
  // Valida as dimensoes do retangulo e o intervalo permitido para alpha.
  if (width <= 0.0 || height <= 0.0 || alpha < 0.0 || alpha > 1.0) {
    // NULL sinaliza ao chamador que a criacao falhou.
    return NULL;
  }

  // Cria a parte comum da primitiva: ID, tipo, cor, alpha, z-index e AABB.
  // Aloca a estrutura base e inicializa a geometria especifica.
  Primitive* p= primitive_allocate_common(id, PRIM_RECTANGLE, color, alpha, z_index);
  if (p==NULL) {
    // A falha pode resultar de memoria insuficiente.
    return NULL;
  }

  // Guarda o canto superior esquerdo recebido pelo chamador.
  p->geom.rectangle.top_left=top_left;
  // Guarda as dimensoes que serao usadas nos testes geometricos.
  p->geom.rectangle.width=width;
  p->geom.rectangle.height=height;

  // A geometria ja esta completa; calcula a AABB antes de devolver a primitiva.
  // Calcula imediatamente a caixa envolvente inicial.
  primitive_update_aabb(p);
  // Devolve uma primitiva totalmente inicializada e pronta a utilizar.
  return p; //FUNCTION COMPLETED
}

/**
 * Cria um segmento depois de validar a opacidade.
 * A AABB e calculada a partir dos menores e maiores valores dos dois extremos.
 */
Primitive* primitive_create_segment(int id, Point p1, Point p2, Color color,
                                    double alpha, int z_index) {
  // Um segmento pode ter comprimento zero, mas a sua opacidade continua
  // obrigada a estar dentro do intervalo valido.
  if (alpha < 0.0 || alpha > 1.0) {
    // Impede a criacao de uma primitiva com dados de apresentacao invalidos.
    return NULL;
  }

  // Reserva e inicializa os campos comuns a todos os tipos de primitiva.
  Primitive* p = primitive_allocate_common(id, PRIM_SEGMENT, color, alpha, z_index);
  if (p == NULL) {
    // Nao e possivel continuar sem a estrutura base.
    return NULL;
  }

  // Um segmento e definido pelos seus dois pontos extremos.
  p->geom.segment.p1 = p1;
  p->geom.segment.p2 = p2;

  // A AABB e calculada usando os limites minimo e maximo dos extremos.
  primitive_update_aabb(p); //FUNCTION COMPLETED
  // A primitiva devolvida ja possui geometria e limites coerentes.
  return p;
}

/**
 * Cria um poligono vazio.
 * O vetor de vertices e alocado apenas quando o primeiro vertice for inserido.
 */
Primitive* primitive_create_polygon(int id, Color color, double alpha,
                                    int z_index) {
  // O poligono e criado sem vertices; eles serao adicionados dinamicamente.
  if (alpha < 0.0 || alpha > 1.0) {
    // Rejeita valores de opacidade fora do intervalo permitido.
    return NULL;
  }

  // Inicializa os campos comuns, incluindo a flag de AABB desatualizada.
  Primitive* p = primitive_allocate_common(id, PRIM_POLYGON, color, alpha, z_index);
  if (p == NULL) {
    // Liberta o chamador da responsabilidade de destruir uma alocacao parcial.
    return NULL;
  }

  // Nao existem vertices nem memoria associada neste momento.
  p->geom.polygon.num_vertices = 0;
  p->geom.polygon.vertices = NULL;

  // Define a AABB inicial de uma geometria vazia como a origem.
  // Uma lista vazia tem uma AABB inicial degenerada na origem.
  primitive_update_aabb(p); //FUNCTION COMPLETED
  // O vetor sera criado por polygon_add_vertex quando necessario.
  return p;
}

/**
 * Destroi uma primitiva e liberta a memoria que lhe pertence.
 * Poligonos requerem uma libertacao adicional para o vetor de vertices.
 */
void primitive_destroy(Primitive* p) {
  // Destruir NULL e uma operacao segura e nao requer qualquer acao.
  if (p == NULL) {
    return;
  }

  // Apenas o poligono possui uma segunda alocacao fora da estrutura principal.
  // Apenas os poligonos possuem memoria dinamica adicional.
  if (p->type == PRIM_POLYGON) {
    // Liberta o vetor dinamico de pontos, se tiver sido alocado.
    free(p->geom.polygon.vertices);
    // Evita deixar um ponteiro invalido dentro da estrutura durante a limpeza.
    p->geom.polygon.vertices = NULL;
  }

  // Liberta finalmente a estrutura Primitive.
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
  // So primitivas do tipo poligono possuem um vetor de vertices.
  if (p==NULL||p->type!=PRIM_POLYGON){
    // False indica argumento invalido ou tipo de primitiva incorreto.
    return false;
  }

  // Cria um alias para aceder a geometria do poligono de forma mais legivel.
  PolygonGeom* polygon = &p->geom.polygon;
  // Se num_vertices atingir capacity, nao ha espaco para uma nova insercao.
  if (polygon->num_vertices >= polygon->capacity) {
    // Duplica a capacidade para manter insercoes eficientes.
    size_t new_capacity = (polygon->capacity == 0) ? 4 : polygon->capacity * 2;
    // realloc preserva os pontos existentes e tenta obter espaco adicional.
    Point* new_vertices = (Point*)realloc(polygon->vertices, new_capacity * sizeof(Point));
    if (new_vertices == NULL) {
      // O vetor original continua valido; a insercao simplesmente falha.
      return false; // Memory allocation failed
    }
    // Atualiza o ponteiro apenas depois de confirmar que realloc teve sucesso.
    polygon->vertices = new_vertices;
    // Regista a nova quantidade de pontos que o vetor pode armazenar.
    polygon->capacity = new_capacity;
  }

  // Coloca o novo ponto na primeira posicao livre do vetor.
  polygon->vertices[polygon->num_vertices] = pt;
  // Aumenta a contagem para incluir o ponto acabado de inserir.
  polygon->num_vertices++;
  // A AABB sera recalculada quando for consultada.
  p->is_aabb_dirty = true; // Mark AABB as dirty since the polygon has changed
  // A insercao foi concluida com sucesso.
  return true; //FUNCTION COMPLETED
}

/**
 * Remove o vertice indicado e compacta o vetor deslocando os seguintes.
 * A AABB e marcada como desatualizada porque os limites podem ter mudado.
 */
bool polygon_remove_vertex(Primitive* p, size_t index) {
  // Verifica simultaneamente o ponteiro, o tipo e o intervalo do indice.
  if (p == NULL || p->type != PRIM_POLYGON || index >= p->geom.polygon.num_vertices) {
    // Um indice fora dos limites nao pode ser removido.
    return false;
  }

  // Alias para a geometria do poligono, evitando expressoes repetidas.
  PolygonGeom* polygon = &p->geom.polygon;
  // Move cada elemento posterior uma posicao para a esquerda.
  for (size_t i = index; i < polygon->num_vertices - 1; i++) {
    polygon->vertices[i] = polygon->vertices[i + 1];
  }

  // O ultimo elemento ficou duplicado e deixa de fazer parte da sequencia logica.
  polygon->num_vertices--;
  // A remocao pode alterar os limites da primitiva.
  p->is_aabb_dirty = true; // Mark AABB as dirty since the polygon has changed

  // A remocao e confirmada ao chamador.
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
  // Nao recalcula uma primitiva inexistente nem uma AABB ja atualizada.
  if (p == NULL || !p->is_aabb_dirty) {
    return;
  }

  // Cada geometria tem uma forma diferente de obter os seus limites.
  switch (p->type) {
    case PRIM_CIRCLE:
      // O circulo estende o raio em todas as quatro direcoes a partir do centro.
      p->aabb.min.x = p->geom.circle.center.x - p->geom.circle.radius;
      p->aabb.min.y = p->geom.circle.center.y - p->geom.circle.radius;
      p->aabb.max.x = p->geom.circle.center.x + p->geom.circle.radius;
      p->aabb.max.y = p->geom.circle.center.y + p->geom.circle.radius;
      break;

    case PRIM_RECTANGLE:
      // O retangulo ja e axis-aligned: os seus cantos definem diretamente a AABB.
      p->aabb.min.x = p->geom.rectangle.top_left.x;
      p->aabb.min.y = p->geom.rectangle.top_left.y;
      p->aabb.max.x = p->geom.rectangle.top_left.x + p->geom.rectangle.width;
      p->aabb.max.y = p->geom.rectangle.top_left.y + p->geom.rectangle.height;
      break;

    case PRIM_SEGMENT:
      // Para o segmento, cada limite e o menor ou maior coordenada dos extremos.
      p->aabb.min.x = fmin(p->geom.segment.p1.x, p->geom.segment.p2.x);
      p->aabb.min.y = fmin(p->geom.segment.p1.y, p->geom.segment.p2.y);
      p->aabb.max.x = fmax(p->geom.segment.p1.x, p->geom.segment.p2.x);
      p->aabb.max.y = fmax(p->geom.segment.p1.y, p->geom.segment.p2.y);
      break;

    case PRIM_POLYGON: {
      // A AABB do poligono e obtida percorrendo todos os seus vertices.
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
          // Cada comparacao atualiza apenas o limite que foi ultrapassado.
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

  // A caixa corresponde agora a geometria atual e nao precisa de novo calculo.
  p->is_aabb_dirty = false;
} //FUNCTION COMPLETED

/**
 * Aplica uma translacao a todos os pontos relevantes da geometria.
 * Depois da operacao, a AABB e invalidada para ser recalculada sob demanda.
 */
void primitive_translate(Primitive* p, double dx, double dy) {
  // Sem primitiva, nao existe geometria para mover.
  if (p == NULL) {
    return;
  }

  // Aplica o mesmo deslocamento a todas as coordenadas relevantes.
  switch(p->type) {
    case PRIM_CIRCLE:
      // Num circulo, apenas o centro muda; o raio permanece igual.
      p->geom.circle.center.x += dx;
      p->geom.circle.center.y += dy;
      break;
    case PRIM_RECTANGLE:
      // Num retangulo, move-se o canto superior esquerdo.
      p->geom.rectangle.top_left.x += dx;
      p->geom.rectangle.top_left.y += dy;
      break;
    case PRIM_SEGMENT:
      // Os dois extremos devem ser transladados para preservar o segmento.
      p->geom.segment.p1.x += dx;
      p->geom.segment.p1.y += dy;
      p->geom.segment.p2.x += dx;
      p->geom.segment.p2.y += dy;
      break;
    case PRIM_POLYGON:
      // Todos os vertices recebem o mesmo deslocamento.
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
  // A escala deve ser positiva; fatores nulos ou negativos sao rejeitados.
  if (p == NULL || factor <= 0.0) {
    return;
  }

  // A formula de escala depende da geometria concreta.
  switch (p->type){
    case PRIM_CIRCLE:
      // O centro nao muda; apenas o raio e multiplicado pelo fator.
      p->geom.circle.radius *= factor;
      break;
    case PRIM_RECTANGLE:
      // O canto superior esquerdo permanece fixo nesta representacao.
      p->geom.rectangle.width *= factor;
      p->geom.rectangle.height *= factor;
      break;
    case PRIM_SEGMENT: {
      // Mantem o ponto medio fixo enquanto afasta ou aproxima os extremos.
      // Calcula o ponto medio antes de modificar qualquer extremo.
      double mid_x = (p->geom.segment.p1.x + p->geom.segment.p2.x) / 2.0;
      double mid_y = (p->geom.segment.p1.y + p->geom.segment.p2.y) / 2.0;
      p->geom.segment.p1.x = mid_x + (p->geom.segment.p1.x - mid_x) * factor;
      p->geom.segment.p1.y = mid_y + (p->geom.segment.p1.y - mid_y) * factor;
      p->geom.segment.p2.x = mid_x + (p->geom.segment.p2.x - mid_x) * factor;
      p->geom.segment.p2.y = mid_y + (p->geom.segment.p2.y - mid_y) * factor;
      break;
    }
    case PRIM_POLYGON: {
      // Um poligono vazio nao tem vertices para escalar.
      if (p->geom.polygon.num_vertices > 0) {
        if (p->is_aabb_dirty) {
          // O centro usado na escala deve refletir a geometria atual.
          primitive_update_aabb(p);
        }
        double center_x = (p->aabb.min.x + p->aabb.max.x) / 2.0;
        double center_y = (p->aabb.min.y + p->aabb.max.y) / 2.0;

        // Cada vertice e afastado ou aproximado ao centro pela mesma proporcao.
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
  // Uma primitiva inexistente nao pode conter qualquer ponto.
  if (p == NULL) {
    return false;
  }

  // Narrow-phase exact geometric test dispatch
  // Encaminha o ponto para o algoritmo especifico de cada forma.
  switch (p->type) {
    case PRIM_CIRCLE:
      // Teste por distancia ao centro comparada com o raio.
      return math_circle_contains_point(p->geom.circle.center,
                                        p->geom.circle.radius, pt);

    case PRIM_RECTANGLE:
      // Teste por comparacao com os limites horizontais e verticais.
      return math_rect_contains_point(p->geom.rectangle.top_left,
                                       p->geom.rectangle.width,
                                       p->geom.rectangle.height, pt);

    case PRIM_SEGMENT:
      // Teste pela distancia ao ponto mais proximo do segmento.
      return math_segment_contains_point(p->geom.segment.p1, p->geom.segment.p2,
                                          1e-6, pt);

    case PRIM_POLYGON:
      // Teste de inclusao por ray-casting sobre os lados do poligono.
      return math_polygon_contains_point(p->geom.polygon.vertices,
                                          p->geom.polygon.num_vertices, pt);
  }

  // Tipos desconhecidos sao tratados como nao contendo o ponto.
  return false;
} //FUNCTION COMPLETED
