#ifndef MAPA_H
#define MAPA_H
#define TAM_CHUNK 256
// tamaño maximo que tendrá uno de los chunks.
//? recomendado: 256

#define AUMENTO 10.0f
#define ALTURA_MAX 100.0f
#define MAP_LIMIT (sqrt(numChunks) - 0.5) * TAM_CHUNK *AUMENTO / 2.0f
// factor de aumento del tamaño de los chunks (aparecen mais grandes en openGL)
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

struct hmap
{
    unsigned char *data;
    int width;
    int height;
};

extern int numChunks;
extern long unsigned int totalVertices;

GLuint cargarTerrenoDesdeHMap(const hmap &mapa, GLuint *VAOMapa, int x, int y);
hmap cargarHeightmap(const char *archivo);
chunk **initChunks(const char *mapName);
chunk *crearChunk();
void cargarTexturaPNG(unsigned int &textura, const char *filename);
float getAlturaTerreno(float worldX, float worldZ);
#endif // MAPA_H