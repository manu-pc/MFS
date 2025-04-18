#ifndef MAPA_H
#define MAPA_H
#define NUM_CHUNKS 256

#include <glad/glad.h>
typedef struct chunk
{
    unsigned int VAO;
    unsigned int textura;
    unsigned int numVertices;
    unsigned int x;
    unsigned int y;
} chunk;
// Carga un mapa desde un archivo PNG y genera un VAO con los datos del mapa.
// archivo: Ruta al archivo PNG del heightmap.
// VAOMapa: Puntero al VAO donde se almacenarán los datos del mapa.
// alturaMaxima: Altura máxima que se mapeará desde los valores del heightmap.
// Devuelve el número de vértices generados.
GLuint cargarTerrenoDesdePNG(const char *archivo, GLuint *VAOMapa, float alturaMaxima = 30.0f);

// Inicializa un arreglo de punteros a chunks.
// Devuelve un puntero al arreglo de chunks inicializados.
chunk **initChunks();

// Crea un nuevo chunk con el nombre especificado.
// nombre: Nombre del chunk.
// Devuelve un puntero al chunk creado.
chunk *crearChunk();
void cargarTexturaPNG(unsigned int &textura, const char *filename);
#endif // MAPA_H