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
#include <GLFW/glfw3.h>

#include "mapa.h"
using namespace std;
int tamFila;
int numChunks;
long unsigned int totalVertices = 0;

hmap mapa;
// carga en un chunk los datos de un heightmap (tipo hmap) y la textura (se pasa el nombre del archivo)
//  el fragmento de hmap y la textura no tienen que ser del mismo tamaño
void cargarChunk(chunk *chunk, const hmap &mapa, const char *mapName)
{
    printf("\r\rCargando chunk %d %d...           ", chunk->x, chunk->y);
    fflush(stdout); // para imprimir todo nunha misma línea

    char archivo_textura[100];
    sprintf(archivo_textura, "mapas/%s/tiles/tex_%d_%d.png", mapName, chunk->x, chunk->y);
    cargarTexturaPNG(chunk->textura, archivo_textura);

    printf("(textura: %s)", archivo_textura);
    fflush(stdout);

    chunk->numVertices = cargarTerrenoDesdeHMap(mapa, &(chunk->VAO), chunk->x, chunk->y);
    totalVertices += chunk->numVertices;
}

// carga una textura PNG (4 canales) en la variable textura. se usa tanto para las texturas de los chunks como para otras texturas, como la ventana
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
        int rowSize = width * 4; // 4 bytes por píxel (RGBA)
        unsigned char *rowTemp = new unsigned char[rowSize];
        for (int y = 0; y < height; ++y)
        {
            unsigned char *row = data + y * rowSize;
            for (int x = 0; x < width / 2; ++x)
            {
                // se voltea horizontalmente - es necesario para las texturas del mapa y no importa para las ddemás texturas
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

// crea un array dinámico de punteros a chunks y lo inicializa a partir de un archivo "hmap.png"
chunk **initChunks(const char *mapName)
{
    char filename_mapa[100];
    sprintf(filename_mapa, "mapas/%s/hmap.png", mapName);
    // debe exisitir en la carpeta del mapa un hmap.png, y una carpeta tiles con las texturas
    mapa = cargarHeightmap(filename_mapa);

    int tamFila = (int)sqrt(numChunks);
    chunk **chunks = new chunk *[numChunks];

    for (int i = 0; i < tamFila; i++)
    {
        for (int j = 0; j < tamFila; j++)
        {
            int idx = i * tamFila + j;
            chunks[idx] = crearChunk();
            chunks[idx]->x = i;
            chunks[idx]->y = j;
            cargarChunk(chunks[idx], mapa, mapName);
            glfwPollEvents();
        }
    }

    stbi_image_free(mapa.data);
    return chunks;
}

// crea un nuevo chunk y lo inicializa a 0
// devuelve un puntero al chunk creado
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

// carga un heightmap PNG y lo devuelve como un struct hmap
// el heightmap debe ser un PNG de 1 canal (grayscale) y se carga como un array de unsigned char
// el objeto hmap tambien guarda el ancho y alto del heightmap
hmap cargarHeightmap(const char *archivo)
{
    hmap mapa;
    int channels;
    stbi_set_flip_vertically_on_load(true); // esto voltea verticalmente
    mapa.data = stbi_load(archivo, &mapa.width, &mapa.height, &channels, STBI_grey);
    if (!mapa.data)
    {
        std::cerr << "Error al cargar heightmap PNG: " << archivo << ". Comprueba si existe el archivo y si es un heightmap válido." << std::endl;
        exit(1);
    }

    // volteo horizontal
    for (int y = 0; y < mapa.height; ++y)
    {
        for (int x = 0; x < mapa.width / 2; ++x)
        {
            int leftIndex = y * mapa.width + x;
            int rightIndex = y * mapa.width + (mapa.width - 1 - x);
            std::swap(mapa.data[leftIndex], mapa.data[rightIndex]);
        }
    }

    numChunks = ceil(mapa.width / (float)TAM_CHUNK) * ceil(mapa.height / (float)TAM_CHUNK);
    printf("Cargado heightmap PNG: %s. Se dividirá en %d chunks.\n", archivo, numChunks);

    return mapa;
}

// crea la geometría (VAO) del terreno a partir de un heightmap
// se le pasan las coordenadas del chunk (x, y) y el VAO donde se guardará la geometría
// a partir del heightmap que tiene el terreno completo, esta funcion carga la geometría sólo de un chunk
GLuint cargarTerrenoDesdeHMap(const hmap &mapa, GLuint *VAOMapa, int x, int y)
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
                return (mapa.data[j * mapa.width + i] / 255.0f * ALTURA_MAX);
            };

            float localX = v - startX;
            float localZ = z - startZ;

            glm::vec3 pos[] = {
                {localX, getHeight(v, z), localZ},
                {localX + 1, getHeight(v + 1, z), localZ},
                {localX, getHeight(v, z + 1), localZ + 1},
                {localX + 1, getHeight(v + 1, z + 1), localZ + 1}};

            float epsilon = 0.001f;
            // margen de error para que encajen mejor los chunks
            glm::vec2 uv[] = {
                {localX / TAM_CHUNK + epsilon, localZ / TAM_CHUNK + epsilon},
                {(localX + 1) / TAM_CHUNK - epsilon, localZ / TAM_CHUNK + epsilon},
                {localX / TAM_CHUNK + epsilon, (localZ + 1) / TAM_CHUNK - epsilon},
                {(localX + 1) / TAM_CHUNK - epsilon, (localZ + 1) / TAM_CHUNK - epsilon}};

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

// a partir de las coordenadas del mundo (x, z) devuelve la altura del terreno, para colisiones
float getAlturaTerreno(float x, float y)
{
    if (x < 0 || x >= mapa.width || y < 0 || y >= mapa.height)
    {
        std::cerr << "Coordenadas fuera de rango: (" << x << ", " << y << ")" << std::endl;
        return 0;  // nunca se debería dar este caso gracias a map_limit pero por si acaso
    }

    float index = y * mapa.width + x;
    int roundedIndex = static_cast<int>(round(index));
    return mapa.data[roundedIndex] / 255.0f * AUMENTO * ALTURA_MAX;
}
