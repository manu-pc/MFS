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
void cargarChunk(chunk *chunk)
{
    
    
    char archivo[100];
    sprintf(archivo, "mapa/tiles/chunk_%d_%d.png", chunk->x, chunk->y);
    char archivo_textura[100];
    sprintf(archivo_textura, "mapa/tiles/tex_%d_%d.png", chunk->x, chunk->y);
    cargarTexturaPNG(chunk->textura, archivo_textura);
    printf("cargando mapa... (textura: %s)\n", archivo);
    chunk->numVertices = cargarTerrenoDesdePNG(archivo, &(chunk->VAO), 30.0f);
    printf("mapa cargado! (%d vértices)", chunk->numVertices);
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cout << "Error al cargar la textura" << endl;
    }
    stbi_image_free(data);
}

chunk **initChunks()
{
    chunk **chunks = new chunk *[NUM_CHUNKS];
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
        printf("cargando chunk %d_%d\n", i, j);
            chunks[i * 2 + j] = crearChunk();
            chunks[i * 2 + j]->x = j;
            chunks[i * 2 + j]->y = i;
            cargarChunk(chunks[i * 2 + j]);
        }
    }
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
    return nuevoChunk;
}

GLuint cargarTerrenoDesdePNG(const char *archivo, GLuint *VAOMapa, float alturaMaxima)
{
    std::vector<float> vertexData;
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(archivo, &width, &height, &channels, STBI_grey);

    if (!data)
    {
        std::cerr << "Error al cargar heightmap PNG: " << archivo << std::endl;
        exit(1);
    }
    
    std::cout << "Cargando mapa " << archivo << " (" << width << "x" << height << ")" << std::endl;

    for (int z = 0; z < height - 1; ++z)
    {
        for (int x = 0; x < width - 1; ++x)
        {
            auto getHeight = [&](int i, int j)
            {
                return (data[j * width + i] / 255.0f) * alturaMaxima;
            };

            // Generar dos triángulos por cuadrado de la malla
            glm::vec3 pos[] = {
                {x, getHeight(x, z), z},
                {x + 1, getHeight(x + 1, z), z},
                {x, getHeight(x, z + 1), z + 1},
                {x + 1, getHeight(x + 1, z + 1), z + 1}};

            glm::vec2 uv[] = {
                {x / (float)width, z / (float)height},
                {(x + 1) / (float)width, z / (float)height},
                {x / (float)width, (z + 1) / (float)height},
                {(x + 1) / (float)width, (z + 1) / (float)height}};

            auto agregarVertice = [&](int i)
            {
                vertexData.insert(vertexData.end(), {pos[i].x, pos[i].y, pos[i].z,
                                                     0.0f, 1.0f, 0.0f, // temporal: normales planas hacia arriba
                                                     uv[i].x, uv[i].y});
            };

            // Triángulo 1
            agregarVertice(1);
            agregarVertice(0);
            agregarVertice(2);

            // Triángulo 2
            agregarVertice(2);
            agregarVertice(3);
            agregarVertice(1);
        }
    }

    stbi_image_free(data);

    GLuint VBO;
    glGenVertexArrays(1, VAOMapa);
    glGenBuffers(1, &VBO);

    glBindVertexArray(*VAOMapa);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    // posición
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // normales
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    GLuint numVertices = vertexData.size() / 8;
    return numVertices;
}
