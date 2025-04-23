#ifndef MAPA_H
#define MAPA_H
#define TAM_CHUNK 254
// tamaño maximo que tendrá uno de los chunks.
//? recomendado: 254
// mayor tamaño implica mayor calidad y peor rendimiento
#include <glad/glad.h>
typedef struct chunk
{
    unsigned int VAO;
    unsigned int textura;
    unsigned int numVertices;
    unsigned int x;
    unsigned int y;
} chunk;

struct hmap {
    unsigned char* data;
    int width;
    int height;
};

// Carga un mapa desde un archivo PNG y genera un VAO con los datos del mapa.
// archivo: Ruta al archivo PNG del heightmap.
// VAOMapa: Puntero al VAO donde se almacenarán los datos del mapa.
// alturaMaxima: Altura máxima que se mapeará desde los valores del heightmap.
// Devuelve el número de vértices generados.
GLuint cargarTerrenoDesdeHMap(const hmap &mapa, GLuint *VAOMapa, float alturaMaxima, int x, int y);
hmap cargarHeightmap(const char *archivo);

// Inicializa un arreglo de punteros a chunks.
// Devuelve un puntero al arreglo de chunks inicializados.
chunk **initChunks();

// Crea un nuevo chunk con el nombre especificado.
// nombre: Nombre del chunk.
// Devuelve un puntero al chunk creado.
chunk *crearChunk();
void cargarTexturaPNG(unsigned int &textura, const char *filename);
#endif // MAPA_H