#include "stb_image.h"
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "string.h"
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <math.h>
#include "mapa.h"
using namespace std;
int tamFila;
int numChunks;

void cargarChunk(chunk *chunk, const hmap &mapa)
{
    printf("Cargandando chunmk %d %d\n", chunk->x, chunk->y);
    char archivo_textura[100];
    sprintf(archivo_textura, "mapa/tiles/tex_%d_%d.png", chunk->x, chunk->y);
    cargarTexturaPNG(chunk->textura, archivo_textura);
    printf("ºCargada textura %s\n", archivo_textura);
    chunk->numVertices = cargarTerrenoDesdeHMap(mapa, &(chunk->VAO), 30.0f, chunk->x, chunk->y);
}

void cargarTexturaPNG(unsigned int &textura, const char *filename)
{
    glGenTextures(1, &textura);
    glBindTexture(GL_TEXTURE_2D, textura);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, STBI_rgb_alpha);
    if (data)
    {
        // se voltea horizontamente para que se cargue bien el mapa
        int rowSize = width * 4; // 4 bytes por píxel (RGBA)
        unsigned char *rowTemp = new unsigned char[rowSize];
        for (int y = 0; y < height; ++y)
        {
            unsigned char *row = data + y * rowSize;
            for (int x = 0; x < width / 2; ++x)
            {
                int left = x * 4;
                int right = (width - 1 - x) * 4;
                for (int i = 0; i < 4; ++i)
                {
                    std::swap(row[left + i], row[right + i]);
                }
            }
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        delete[] rowTemp;
    }
    else
    {
        std::cout << "Error al cargar la textura" << std::endl;
    }

    stbi_image_free(data);
}

chunk **initChunks()
{
    hmap mapa = cargarHeightmap("mapa/tiles/hmap.png");

    int tamFila = (int)sqrt(numChunks);
    chunk **chunks = new chunk *[numChunks];
    printf("pito!\n");
    // Cargar mapa heightmap UNA vez
 
    for (int i = 0; i < tamFila; i++)
    {
        printf("Cargando chunk %d\n", i);
        for (int j = 0; j < tamFila; j++)
        {
            printf("Cargando chunk %d %d\n", i, j);
            int idx = i * tamFila + j;
            chunks[idx] = crearChunk();
            chunks[idx]->x = j;
            chunks[idx]->y = i;
            cargarChunk(chunks[idx], mapa);
        }
    }

    stbi_image_free(mapa.data);
    return chunks;
}

chunk *crearChunk()
{
    chunk *nuevoChunk = new chunk;
    nuevoChunk->VAO = 0;
    nuevoChunk->textura = 0;
    nuevoChunk->numVertices = 0;
    nuevoChunk->x = 0;
    nuevoChunk->y = 0;
    printf("Creando chunk\n");
    return nuevoChunk;
}

hmap cargarHeightmap(const char *archivo)
{
    hmap mapa;
    int channels;
    stbi_set_flip_vertically_on_load(true);
    printf("Cargando heightmap PNG: %s\n", archivo);
    mapa.data = stbi_load(archivo, &mapa.width, &mapa.height, &channels, STBI_grey);
    printf("Width: %d, Height: %d, Channels: %d\n", mapa.width, mapa.height, channels);
    if (!mapa.data)
    {
        std::cerr << "Error al cargar heightmap PNG: " << archivo << std::endl;
        exit(1);
    }
    numChunks = ceil(mapa.width / (float)TAM_CHUNK) * ceil( mapa.height / (float)TAM_CHUNK);

    return mapa;
}

GLuint cargarTerrenoDesdeHMap(const hmap &mapa, GLuint *VAOMapa, float alturaMaxima, int x, int y)
{
    std::vector<float> vertexData;

    int startX = x * TAM_CHUNK;
    int startZ = y * TAM_CHUNK;
    int endX = std::min(startX + TAM_CHUNK, mapa.width - 1);
    int endZ = std::min(startZ + TAM_CHUNK, mapa.height - 1);

    for (int z = startZ; z < endZ; ++z)
    {
        for (int v = startX; v < endX; ++v)
        {
            auto getHeight = [&](int i, int j)
            {
                return (mapa.data[j * mapa.width + i] / 255.0f) * alturaMaxima;
            };

            float localX = v - startX;
            float localZ = z - startZ;

            glm::vec3 pos[] = {
                {localX, getHeight(v, z), localZ},
                {localX + 1, getHeight(v + 1, z), localZ},
                {localX, getHeight(v, z + 1), localZ + 1},
                {localX + 1, getHeight(v + 1, z + 1), localZ + 1}};

                glm::vec2 uv[] = {
                    {localX / TAM_CHUNK, localZ / TAM_CHUNK},
                    {(localX + 1) / TAM_CHUNK, localZ / TAM_CHUNK},
                    {localX / TAM_CHUNK, (localZ + 1) / TAM_CHUNK},
                    {(localX + 1) / TAM_CHUNK, (localZ + 1) / TAM_CHUNK}};
                
            auto agregarVertice = [&](int i)
            {
                vertexData.insert(vertexData.end(), {pos[i].x, pos[i].y, pos[i].z,
                                                     0.0f, 1.0f, 0.0f,
                                                     uv[i].x, uv[i].y});
            };

            agregarVertice(1);
            agregarVertice(0);
            agregarVertice(2);
            agregarVertice(2);
            agregarVertice(3);
            agregarVertice(1);
        }
    }

    GLuint VBO;
    glGenVertexArrays(1, VAOMapa);
    glGenBuffers(1, &VBO);

    glBindVertexArray(*VAOMapa);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    return vertexData.size() / 8;
}
