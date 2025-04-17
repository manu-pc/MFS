#include "stb_image.h"
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

//? devolve num de vertices, garda en VAOMapa
#include <fstream>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>


GLuint cargarTerrenoDesdePNG(const char* archivo, GLuint *VAOMapa, float alturaMaxima = 30.0f)
{
    std::vector<float> vertexData;
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true); 
    unsigned char* data = stbi_load(archivo, &width, &height, &channels, STBI_grey);

    if (!data) {
        std::cerr << "Error al cargar heightmap PNG: " << archivo << std::endl;
        exit(1);
    }

    std::cout << "Cargando mapa " << archivo << " (" << width << "x" << height << ")" << std::endl;

    for (int z = 0; z < height - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            auto getHeight = [&](int i, int j) {
                return (data[j * width + i] / 255.0f) * alturaMaxima;
            };

            // Generar dos triángulos por cuadrado de la malla
            glm::vec3 pos[] = {
                {x,     getHeight(x, z),     z},
                {x + 1, getHeight(x + 1, z), z},
                {x,     getHeight(x, z + 1), z + 1},
                {x + 1, getHeight(x + 1, z + 1), z + 1}
            };

            glm::vec2 uv[] = {
                {x / (float)width, z / (float)height},
                {(x + 1) / (float)width, z / (float)height},
                {x / (float)width, (z + 1) / (float)height},
                {(x + 1) / (float)width, (z + 1) / (float)height}
            };

            auto agregarVertice = [&](int i) {
                vertexData.insert(vertexData.end(), {
                    pos[i].x, pos[i].y, pos[i].z,
                    0.0f, 1.0f, 0.0f, // temporal: normales planas hacia arriba
                    uv[i].x, uv[i].y
                });
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normales
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    GLuint numVertices = vertexData.size() / 8;
    return numVertices;
}
