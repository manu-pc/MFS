#ifndef MAPA_H
#define MAPA_H

#include <glad/glad.h>

// Carga un mapa desde un archivo PNG y genera un VAO con los datos del mapa.
// archivo: Ruta al archivo PNG del heightmap.
// VAOMapa: Puntero al VAO donde se almacenarán los datos del mapa.
// alturaMaxima: Altura máxima que se mapeará desde los valores del heightmap.
// Devuelve el número de vértices generados.
GLuint cargarTerrenoDesdePNG(const char* archivo, GLuint *VAOMapa, float alturaMaxima = 30.0f);

#endif // MAPA_H