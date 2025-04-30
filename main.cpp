//! ---------
//? *  MFS  *
//! ---------

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iomanip>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <glm/gtc/type_ptr.hpp>
#include "lecturaShader_0_9.h"
#include "dibujo.h"
#include "mapa.h"
#include "colores.h"
#include <vector>
#include <string>

using namespace std;
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define pi 3.14159265359

//? constantes avion
#define VEL_MAX 120.0f
#define ACEL 1.5
#define FRENADO 0.005
#define REV_MAX -10.0f
#define MAX_PITCH 30.0f
#define MAX_ROLL 45.0f
#define PITCH_SPEED .2f
#define ROLL_SPEED .5f
#define VEL_PROY 1000.0f

//? variables camara
int tipoCamara = 1; //? 1-primera, 2-tercera, 3-arriba (DEBUG)
float alfa = 0.5;
float bbeta = 0.5;
float dist_camara = 50.0f;
float vel_camara = 0.01f;

//? variables ventana
int w_width = 1600;
int w_height = 900;

//* más variables
int momentoDia = 0; //? 0-día, 1-tarde, 2-noche
int luces = 0;		//? 0-apagadas, 1-cortas, 2-largas
int mantenerTeclaT = 0, mantenerTeclaL = 0, mantenerTeclaF = 0;
bool verFPS = false;
float centroMapa; // para debuxar arredor de 0.0
chunk **chunks;
char *mapName;

//* texturas
unsigned int texturaSuelo;
unsigned int texturaCarretera;
unsigned int texturaEsquinaCarretera;
unsigned int texturaTronco;
unsigned int texturaSkybox[12];

extern GLuint setShaders(const char *nVertx, const char *nFrag);
GLuint shaderProgram, shaderProgramSkybox;

typedef struct
{
	glm::vec3 posicion;
	float angulo_XZ;
	float inclinacion;
	float rotacion;
	float velocidad;
	glm::vec3 escala;
	unsigned int textura;
	unsigned int VAO;
} parteavion;

parteavion base = {
	{0, ALTURA_MAX * 20, 0},
	90,
	0,
	0,
	0,
	{2, 2, 12},
	0,
	0};
parteavion cabina = {{0.0, 2.0, 2}, 0, 0, 0, 0, {2.0f, 2.0f, 2.0}, 0, 0};
parteavion ventana = {{0.0, 2.0, 2.4}, 0, 0, 0, 0, {1.8f, 1.8f, 2.0}, 0, 0};
parteavion faroI = {{-1.4, 0.2, 5.0}, 0, 90, 0, 0, {0.6, 0.3, 0.5}, 0, 0};
parteavion faroD = {{1.4, 0.2, 5.0}, 0, 90, 0, 0, {0.6, 0.3, 0.5}, 0, 0};
parteavion ala = {
	{0.0f, 1.0f, 0.0f},
	0,
	0,
	0,
	0,
	{12.0f, 0.3f, 1.2f},
	0,
	0};

parteavion aleronTrasero = {
	{0.0f, 1.5f, -6.0f},
	0,
	0,
	0,
	0,
	{5.0f, 0.2f, 1.5f},
	0,
	0};

parteavion estabilizadorVertical = {
	{0.0f, 1.5f, -6.0f},
	0,
	0,
	90,
	0,
	{3.0f, 0.2f, 1.0f},
	0,
	0};

typedef struct
{
	glm::vec3 posicion;
	float angulo;
	float vel = VEL_PROY;
	bool activo = false;
} proyectil;
proyectil proy = {{0, 0, 0}, 0, 0};

unsigned int VAO;
unsigned int VAOCuadradoXZ;
unsigned int VAOEsfera;
unsigned int VAOCubo;
unsigned int VAOCarretera;
unsigned int VAOEsquina;

float t0 = glfwGetTime();
float t1;
float tdelta;
int nbFrames = 0;

void tiempo()
{
	static float sec = 0;
	t1 = glfwGetTime();
	nbFrames++;
	tdelta = t1 - t0;
	sec += tdelta;

	if (sec >= 1.0)
	{
		if (verFPS) cout << "FPS: " << nbFrames << endl;
		nbFrames = 0;
		sec = 0;
	}
	t0 = t1;
}

void fuego(proyectil &proy, float angulo_real, float inclinacion)
{
	proy.angulo = -angulo_real + 90;
	proy.vel = VEL_PROY;
	proy.activo = true;
	proy.posicion = base.posicion;
}

void dibujarProyectil(proyectil &proy)
{
	if (!proy.activo)
		return;
	glm::mat4 transform;
	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");
	glBindTexture(GL_TEXTURE_2D, texturaSuelo);
	transform = glm::mat4(1.0f);
	transform = glm::translate(transform, proy.posicion);
	transform = glm::scale(transform, glm::vec3(1.5, 1.5, 1.5));

	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glBindVertexArray(VAOCubo);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	proy.posicion.x += proy.vel * cos(glm::radians(proy.angulo)) * tdelta;
	proy.posicion.z += proy.vel * sin(glm::radians(proy.angulo)) * tdelta;
	if (proy.posicion.x > MAP_LIMIT || proy.posicion.x < -MAP_LIMIT || proy.posicion.z > MAP_LIMIT || proy.posicion.z < -MAP_LIMIT)
	{
		proy.activo = false;
	}
}
//? callback para manejar eventos del ratón
void mouseCallback(GLFWwindow *window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		if (!proy.activo)
		{

			fuego(proy, base.angulo_XZ, base.inclinacion);
		}
	}
}

//! cámara
void camara()
{
	glm::vec3 direccion;
	direccion.x = cos(glm::radians(base.inclinacion)) * sin(glm::radians(base.angulo_XZ));
	direccion.y = -sin(glm::radians(base.inclinacion));
	direccion.z = cos(glm::radians(base.inclinacion)) * cos(glm::radians(base.angulo_XZ));
	direccion = glm::normalize(direccion);
	float far = 10000.0f;
	glUseProgram(shaderProgram);
	glm::vec3 cameraPos, target;

	if (tipoCamara == 1) //! primera persona
	{

		cameraPos = glm::vec3(
			base.posicion.x + 12 * direccion.x,
			base.posicion.y + 12 * direccion.y,
			base.posicion.z + 12 * direccion.z);
		target = cameraPos + direccion;
	}

	if (tipoCamara == 2) //! tercera persona
	{
		dist_camara = 50.0f;
		far = 3000.0f;
		cameraPos = glm::vec3(
			base.posicion.x - dist_camara * direccion.x,
			base.posicion.y + 13,
			base.posicion.z - dist_camara * direccion.z);

		target = glm::vec3(
			base.posicion.x,
			base.posicion.y + 2,
			base.posicion.z);
	}

	if (tipoCamara == 3) //! arriba, mirando ao avion (SOLO DEBUG)
	{
		far = 99999999.0f;
		cameraPos = glm::vec3(base.posicion.x, 30000, base.posicion.z - 40);

		target = glm::vec3(
			base.posicion.x, 0, base.posicion.z);
	}
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 view = glm::lookAt(cameraPos, target, up);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)w_width / (float)w_height, 0.1f, far * 99999);

	// --- PASAR A shaderProgram ---
	glUseProgram(shaderProgram);
	unsigned int viewLoc1 = glGetUniformLocation(shaderProgram, "view");
	unsigned int projectionLoc1 = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(viewLoc1, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLoc1, 1, GL_FALSE, glm::value_ptr(projection));

	// --- PASAR A shaderProgramSkybox ---
	glUseProgram(shaderProgramSkybox);
	unsigned int viewLoc2 = glGetUniformLocation(shaderProgramSkybox, "view");
	unsigned int projectionLoc2 = glGetUniformLocation(shaderProgramSkybox, "projection");
	glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLoc2, 1, GL_FALSE, glm::value_ptr(projection));

	// volver al shader principal por si acaso
	glUseProgram(shaderProgram);
}

//? actualiza a posición da grúa en función da velocidade e controla os límites do mapa
void movimiento()
{
	if (fabs(base.rotacion) > 10)
	{
		base.angulo_XZ -= base.rotacion * 0.001 * sqrt(fabs(base.velocidad));
	}

	glm::vec3 direccion;
	direccion.x = cos(glm::radians(base.inclinacion)) * sin(glm::radians(base.angulo_XZ));
	direccion.y = -sin(glm::radians(base.inclinacion));
	direccion.z = cos(glm::radians(base.inclinacion)) * cos(glm::radians(base.angulo_XZ));
	// direccion = glm::normalize(direccion);
	float delta = glm::clamp(tdelta, 0.0f, 0.05f); // máximo 50 ms/frame
	base.posicion += direccion * base.velocidad * delta;
	if (base.posicion.x > MAP_LIMIT)
		base.posicion.x = -MAP_LIMIT;
	if (base.posicion.x < -MAP_LIMIT)
		base.posicion.x = MAP_LIMIT;

	if (base.posicion.z > MAP_LIMIT)
		base.posicion.z = -MAP_LIMIT;

	if (base.posicion.z < -MAP_LIMIT)
		base.posicion.z = MAP_LIMIT;
	float y_terreno = getAlturaTerreno((base.posicion.x + centroMapa) / AUMENTO, (base.posicion.z + centroMapa) / AUMENTO);
	if (base.posicion.y < y_terreno)
	{
		base.posicion.y = y_terreno + 10 * AUMENTO;
	}
}

//* texturas
void cargarTextura(unsigned int &textura, const char *filename)
{
	glGenTextures(1, &textura);
	glBindTexture(GL_TEXTURE_2D, textura);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Error al cargar la textura (" << filename << ")\n"
			 << endl;
	}
	stbi_image_free(data);
}

//* función que debuxa unha parte do avion
// precisa os punteiros ás matrices de transformación e un stack
void dibujarParteavion(parteavion parte, glm::mat4 *transform, glm::mat4 *stack, int modificarStack)
{
	glBindTexture(GL_TEXTURE_2D, parte.textura);

	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");

	glm::mat4 rotacion = glm::yawPitchRoll(
		glm::radians(parte.angulo_XZ),
		glm::radians(parte.inclinacion),
		glm::radians(parte.rotacion));

	*transform = *stack;
	*transform = glm::translate(*transform, parte.posicion);
	*transform = *transform * rotacion;

	if (modificarStack)
		*stack = *transform;

	*transform = glm::scale(*transform, parte.escala);
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(*transform));
	glBindVertexArray(parte.VAO);

	if (parte.VAO == VAOCubo)
		glDrawArrays(GL_TRIANGLES, 0, 36);
	else
		glDrawArrays(GL_TRIANGLES, 0, 1080);
}

void dibujarSol()
{
	unsigned int solDir = glGetUniformLocation(shaderProgram, "solDir");
	unsigned int colorSol = glGetUniformLocation(shaderProgram, "solColor");
	unsigned int intSol = glGetUniformLocation(shaderProgram, "solIntensidad");
	unsigned int colorAmb = glGetUniformLocation(shaderProgram, "ambientColor");
	unsigned int intAmb = glGetUniformLocation(shaderProgram, "ambientIntensidad");
	switch (momentoDia)
	{
	case 2: // noite: non hai sol, luz tenue ambiente blanca
		glUniform1f(intSol, 0.0f);
		glUniform1f(intAmb, 0.1f);
		glUniform3f(colorAmb, 1.0f, 1.0f, 1.0f);
		glClearColor(0.01f, 0.01, 0.1f, 1.0f);

		// printf("ahora es de noche!\n");
		break;

	case 0: // día: sol blanco-amarillento colocado arriba
		glUniform1f(intSol, 0.2f);
		glUniform3f(solDir, 0.0f, 1.0f, 0.0);
		glUniform3f(colorSol, 1.0f, 0.9f, 0.8f);
		glUniform1f(intAmb, 0.8f);
		glUniform3f(colorAmb, 1.0f, 1.0f, 1.0f);
		glClearColor(0.2f, 0.5, 0.7f, 1.0f);

		// printf("ahora es de día!\n");

		break;

	case 1: // tarde: sol produce luz anaranxaada desde un lado
		glUniform1f(intSol, 2.5f);
		glUniform3f(solDir, 1.0f, 0.5f, -0.0f);
		glUniform3f(colorSol, 1.0f, 0.5f, 0.0f);
		glUniform1f(intAmb, 0.35f);
		glUniform3f(colorAmb, 1.0f, 1.0f, 1.0f);
		glClearColor(0.9f, 0.3f, 0.05f, 1.0f);

		// printf("ahora es la tarde!\n");

		break;
	}
}

void luzFaros()
{
	unsigned int farosEncendidosLoc = glGetUniformLocation(shaderProgram, "farosEncendidos");
	unsigned int faroPosLoc = glGetUniformLocation(shaderProgram, "faroPos");
	unsigned int faroColorLoc = glGetUniformLocation(shaderProgram, "faroColor");
	unsigned int faroIntensidadLoc = glGetUniformLocation(shaderProgram, "faroIntensidad");
	unsigned int faroDireccionLoc = glGetUniformLocation(shaderProgram, "faroDireccion");
	unsigned int faroCorteLoc = glGetUniformLocation(shaderProgram, "faroCorte");
	unsigned int faroBordeSuaveLoc = glGetUniformLocation(shaderProgram, "faroBordeSuave");

	glUniform3f(faroPosLoc, base.posicion.x + 3.1 * sin(glm::radians(base.angulo_XZ)),
				base.posicion.y + 1,
				base.posicion.z + 3.2 * cos(glm::radians(base.angulo_XZ)));
	glUniform3f(faroColorLoc, 1.0, 0.9, 0.7);
	float angulo_radianes = glm::radians(base.angulo_XZ - 90);
	glm::vec3 faroDireccion = glm::normalize(glm::vec3(-glm::cos(-angulo_radianes), 0.4f, -glm::sin(-angulo_radianes)));
	glUniform3fv(faroDireccionLoc, 1, glm::value_ptr(faroDireccion));

	switch (luces)
	{
	case 0:
		glUniform1i(farosEncendidosLoc, 0);
		break;
	case 1:
		glUniform1i(farosEncendidosLoc, 1);
		glUniform1f(faroIntensidadLoc, 3.0);
		glUniform1f(faroCorteLoc, glm::cos(glm::radians(220.0)));
		glUniform1f(faroBordeSuaveLoc, glm::cos(glm::radians(200.0)));
		break;
	case 2:
		glUniform1i(farosEncendidosLoc, 1);
		glUniform1f(faroIntensidadLoc, 10.0);
		glUniform1f(faroCorteLoc, glm::cos(glm::radians(215.0)));
		glUniform1f(faroBordeSuaveLoc, glm::cos(glm::radians(210.0)));
		break;
	}
}

bool esChunkVisible(chunk *chunk)
{
	glm::vec2 dir_camara = {
		sin(glm::radians(base.angulo_XZ)),
		cos(glm::radians(base.angulo_XZ))};

	float x_camara = base.posicion.x - dist_camara * dir_camara.x;
	float z_camara = base.posicion.z - dist_camara * dir_camara.y;

	float x_chunk = chunk->x * TAM_CHUNK * AUMENTO - centroMapa;
	float z_chunk = chunk->y * TAM_CHUNK * AUMENTO - centroMapa;

	float dist = sqrt(pow(x_camara - x_chunk, 2) + pow(z_camara - z_chunk, 2));

	if (dist < (2.0f * TAM_CHUNK * AUMENTO))
		return true;

	glm::vec2 dir_chunk = glm::normalize(glm::vec2(x_chunk - x_camara, z_chunk - z_camara));
	float angle = acos(glm::clamp(glm::dot(dir_camara, dir_chunk), -1.0f, 1.0f)); // aseguramos que acos no se pase

	float max_dist = (8.0f * TAM_CHUNK * AUMENTO) * exp(-angle * 0.8f);
	return dist < max_dist;
}

void dibujarChunk(chunk *chunk)
{

	if (!esChunkVisible(chunk))
		return;
	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");

	glUseProgram(shaderProgram);
	glBindVertexArray(chunk->VAO);

	glBindTexture(GL_TEXTURE_2D, chunk->textura);

	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::translate(transform, glm::vec3(chunk->x * TAM_CHUNK * AUMENTO - centroMapa, 0, chunk->y * TAM_CHUNK * AUMENTO - centroMapa));
	transform = glm::scale(transform, glm::vec3(AUMENTO, AUMENTO, AUMENTO));
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glDrawArrays(GL_TRIANGLES, 0, chunk->numVertices);
}

void dibujarSkybox(glm::vec3 centro, float tam)
{
	glUseProgram(shaderProgramSkybox);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(VAOCuadradoXZ);

	unsigned int transformLoc = glGetUniformLocation(shaderProgramSkybox, "model");

	for (int i = 0; i < 6; ++i)
	{
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(1, 0, 0));
		transform = glm::translate(transform, centro);

		switch (i)
		{
		case 0: // Derecha (+X)
			transform = glm::translate(transform, glm::vec3(tam / 2.0f, 0.0f, 0.0f));
			transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(0, 0, 1));
			transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(0, 1, 0));
			break;
		case 1: // Izquierda (-X)
			transform = glm::translate(transform, glm::vec3(-tam / 2.0f, 0.0f, 0.0f));
			transform = glm::rotate(transform, glm::radians(-90.0f), glm::vec3(0, 0, 1));
			transform = glm::rotate(transform, glm::radians(-90.0f), glm::vec3(0, 1, 0));
			break;
		case 2: // Arriba (+Y)
			transform = glm::translate(transform, glm::vec3(0.0f, tam / 2.0f, 0.0f));
			transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(1, 0, 0));
			break;
		case 3: // Abajo (-Y)
			transform = glm::translate(transform, glm::vec3(0.0f, -tam / 2.0f, 0.0f));
			break;
		case 4: // Frente (-Z)
			transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, -tam / 2.0f));
			transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1, 0, 0));
			break;
		case 5: // Atrás (+Z)
			transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, tam / 2.0f));
			transform = glm::rotate(transform, glm::radians(-90.0f), glm::vec3(1, 0, 0));
			break;
		}

		transform = glm::scale(transform, glm::vec3(tam, tam, tam));

		glBindTexture(GL_TEXTURE_2D, texturaSkybox[i + 6 * momentoDia]);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(shaderProgramSkybox, "skyboxTexture"), 0);
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	glUseProgram(shaderProgram);
}

void display()
{
	glUseProgram(shaderProgram);

	glm::mat4 transform = glm::mat4(1.0f);
	glm::mat4 stack = glm::mat4(1.0f);

	camara();
	tiempo();
	movimiento();
	dibujarSol();

	if (momentoDia != 2)
		dibujarSkybox({base.posicion.x, 0, base.posicion.z}, sqrt(numChunks) * TAM_CHUNK * AUMENTO * 100);
	luzFaros();
	dibujarProyectil(proy);

	for (int i = 0; i < numChunks; i++)
	{
		dibujarChunk(chunks[i]);
	}

	//* avion
	dibujarParteavion(base, &transform, &stack, 1);
	dibujarParteavion(cabina, &transform, &stack, 0);
	dibujarParteavion(ventana, &transform, &stack, 0);
	dibujarParteavion(faroI, &transform, &stack, 0);
	dibujarParteavion(faroD, &transform, &stack, 0);
	dibujarParteavion(ala, &transform, &stack, 0);
	dibujarParteavion(aleronTrasero, &transform, &stack, 0);
	dibujarParteavion(estabilizadorVertical, &transform, &stack, 0);

	glBindVertexArray(0);
}

void openGlInit()
{
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

//? callback para actualizar tamaño da ventá
void cambioTamaño(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
	w_height = height;
	w_width = width;
	camara();

	// esto é solo para que non me salte un  warn
	(void)window;
}

int menuInicio()
{
	cout << "▗▖  ▗▖ ▗▄▖ ▗▄▄▄▄▖ ▗▄▖ ▗▄▄▖ ▗▄▄▄▖ ▗▄▄▖ ▗▄▖  ▗▄▄▖\n";
	cout << "▐▛▚▞▜▌▐▌ ▐▌   ▗▞▘▐▌ ▐▌▐▌ ▐▌  █  ▐▌   ▐▌ ▐▌▐▌   \n";
	cout << "▐▌  ▐▌▐▛▀▜▌ ▗▞▘  ▐▛▀▜▌▐▛▀▚▖  █  ▐▌   ▐▌ ▐▌ ▝▀▚▖\n";
	cout << "▐▌  ▐▌▐▌ ▐▌▐▙▄▄▄▖▐▌ ▐▌▐▌ ▐▌▗▄█▄▖▝▚▄▄▖▝▚▄▞▘▗▄▄▞▘\n";
	cout << "                                               \n";
	cout << "                                               \n";
	cout << "                                               \n";
	cout << "▗▄▄▄▖▗▖   ▗▄▄▄▖ ▗▄▄▖▗▖ ▗▖▗▄▄▄▖                 \n";
	cout << "▐▌   ▐▌     █  ▐▌   ▐▌ ▐▌  █                   \n";
	cout << "▐▛▀▀▘▐▌     █  ▐▌▝▜▌▐▛▀▜▌  █                   \n";
	cout << "▐▌   ▐▙▄▄▖▗▄█▄▖▝▚▄▞▘▐▌ ▐▌  █                   \n";
	cout << "                                               \n";
	cout << "                                               \n";
	cout << "                                               \n";
	cout << " ▗▄▄▖▗▄▄▄▖▗▖  ▗▖▗▖ ▗▖▗▖    ▗▄▖▗▄▄▄▖▗▄▖ ▗▄▄▖    \n";
	cout << "▐▌     █  ▐▛▚▞▜▌▐▌ ▐▌▐▌   ▐▌ ▐▌ █ ▐▌ ▐▌▐▌ ▐▌   \n";
	cout << " ▝▀▚▖  █  ▐▌  ▐▌▐▌ ▐▌▐▌   ▐▛▀▜▌ █ ▐▌ ▐▌▐▛▀▚▖   \n";
	cout << "▗▄▄▞▘▗▄█▄▖▐▌  ▐▌▝▚▄▞▘▐▙▄▄▖▐▌ ▐▌ █ ▝▚▄▞▘▐▌ ▐▌   \n";
	cout << "                                               \n";
	cout << "                                               \n";
	cout << "                                               \n";
	cout << "        Pulsa ENTER para comenzar...           \n";
	cout << "                                               \n";
	getchar();
	printf("Nombre del mapa: ");
	scanf("%ms", &mapName);
	printf("Has seleccionado el mapa: %s\n", mapName);
}

void cargarTexturas()
{

	cargarTextura(base.textura, "texturas/baseavion.jpg");
	cargarTextura(cabina.textura, "texturas/cabina.jpg");
	cargarTexturaPNG(ventana.textura, "texturas/ventana.png");
	cargarTextura(faroI.textura, "texturas/faro.jpg");

	// skybox

	const char *caras[12] = {"px", "nx", "py", "ny", "pz", "nz", "px-1", "nx-1", "py-1", "ny-1", "pz-1", "nz-1"};
	for (int i = 0; i < 12; ++i)
	{
		string ruta = "texturas/" + string(caras[i]) + ".jpg";
		cargarTextura(texturaSkybox[i], ruta.c_str());
	}

	faroD.textura = faroI.textura;
	cargarTextura(ala.textura, "texturas/ala.jpg");
	cargarTextura(aleronTrasero.textura, "texturas/ala.jpg");
	cargarTextura(estabilizadorVertical.textura, "texturas/ala.jpg");
}
void entradaTeclado(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		if (dist_camara > 10.0f)
			dist_camara -= 1.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		if (dist_camara < 200.0f)
			dist_camara += 1.0f;
	}
	// inclinacion
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		if (base.inclinacion < MAX_PITCH)
			base.inclinacion += PITCH_SPEED;
		else
			base.inclinacion = MAX_PITCH;
	}
	else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		if (base.inclinacion > -MAX_PITCH)
			base.inclinacion -= PITCH_SPEED;
		else
			base.inclinacion = -MAX_PITCH;
	}
	else
	{
		if (base.inclinacion > 1 && !glfwGetKey(window, GLFW_KEY_Q))
			base.inclinacion -= PITCH_SPEED / 2;
		else if (base.inclinacion < -1 && !glfwGetKey(window, GLFW_KEY_Q))
			base.inclinacion += PITCH_SPEED / 2;
	}

	// roll
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		if (base.rotacion < MAX_ROLL)
			base.rotacion += ROLL_SPEED;
	}
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		if (base.rotacion > -MAX_ROLL)
			base.rotacion -= ROLL_SPEED;
	}
	else
	{

		if (base.rotacion > 2)
			base.rotacion -= ROLL_SPEED;
		else if (base.rotacion < -2)
			base.rotacion += ROLL_SPEED;
		else
			base.rotacion = 0;
	}

	// velocidad
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		if (base.velocidad < VEL_MAX)
			base.velocidad += ACEL;
	}
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		if (base.velocidad > REV_MAX)
			base.velocidad -= ACEL;
	}
	else
	{
		if (base.velocidad > 5)
			base.velocidad -= FRENADO;
		else if (base.velocidad < -5)
			base.velocidad += FRENADO;
		else
			base.velocidad = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		tipoCamara = 1;
	}

	else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	{
		tipoCamara = 2;
	}
	else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	{
		tipoCamara = 3;
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		if (bbeta <= 1.5f)
		{
			bbeta += vel_camara;
		}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		if (bbeta >= 0.1f)
		{
			bbeta -= vel_camara;
		}
	//? t - cambiar momento do día
	if ((glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) && !mantenerTeclaT)
	{
		momentoDia = (momentoDia + 1) % 3;
		mantenerTeclaT = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
	{
		mantenerTeclaT = 0;
	}

	//? l - cambiar luces
	if ((glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) && !mantenerTeclaL)
	{
		luces = (luces + 1) % 3;
		mantenerTeclaL = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE)
	{
		mantenerTeclaL = 0;
	}

	//? f - controlar si ver FPS
	if ((glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) && !mantenerTeclaF)
	{
		verFPS = !verFPS;
		mantenerTeclaF = 1;
	}
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
	{
		mantenerTeclaF = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		alfa -= vel_camara;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		alfa += vel_camara;
}

//!! main
int main(int argc, char **argv)
{

	menuInicio();

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *window = glfwCreateWindow(50, 50, "MFS", NULL, NULL);
	if (window == NULL)
	{
		cout << "Error al crear la ventana" << endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, cambioTamaño);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Error al inicializar GLAD" << endl;
		return -1;
	}

	openGlInit();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	printf("Cargando chunks...\n");
	chunks = initChunks(mapName);
	centroMapa = (sqrt(numChunks) / 2 * TAM_CHUNK * AUMENTO);
	printf("\nChunks cargados! (Total: %ld vértices)\n", totalVertices);

	glfwSetMouseButtonCallback(window, mouseCallback);
	shaderProgram = setShaders("shader.vert", "shader.frag");
	shaderProgramSkybox = setShaders("shaderSkybox.vert", "shaderSkybox.frag");
	glUseProgram(shaderProgram);

	cargarTexturas();
	glfwSetWindowSize(window, w_width, w_height);

	dibujaEsfera();
	dibujaCubo();
	CuadradoXZ();

	base.VAO = VAOCubo;
	cabina.VAO = VAOCubo;
	ventana.VAO = VAOCubo;
	faroI.VAO = VAOEsfera;
	faroD.VAO = VAOEsfera;
	ala.VAO = VAOCubo;
	aleronTrasero.VAO = VAOCubo;
	estabilizadorVertical.VAO = VAOCubo;


	printf("--- MAZARICOS FLIGHT SIMULATOR ---\n");
	printf("Controles:\n");
	printf("  - Movimiento:\n");
	printf("    Q: Acelerar\n");
	printf("    E: Frenar/Retroceder\n");
	printf("    W: Subir\n");
	printf("    S: Bajar\n");
	printf("    A: Girar a la izquierda\n");
	printf("    D: Girar a la derecha\n");
	printf("  - Cámara:\n");
	printf("    Z: Primera persona (predeterminado)\n");
	printf("    X: Tercera persona\n");
	printf("    C: Vista de mundo (DEBUG)\n");
	printf("  - Otros:\n");
	printf("    T: Cambiar momento del día (Día/Tarde/Noche)\n");
	printf("    L: Cambiar luces (Apagadas/Cortas/Largas)\n");
	printf("    F: Mostrar/Ocultar FPS\n");
	printf("    ESC: Salir del programa\n");
	while (!glfwWindowShouldClose(window))
	{
		entradaTeclado(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAOCuadradoXZ);
	glDeleteVertexArrays(1, &VAOEsfera);
	glDeleteVertexArrays(1, &VAOCubo);
	glfwTerminate();
	return 0;
}
