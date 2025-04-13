//! ----------
//? *  Grúa  *
//! ----------
// todo: textura basegrua é un pouco fea, a farola non ten luz (podela quitar lol)
//  e bueno mete comentarios e tal. e é un pouco chapuza ter un VAO separado para cada tipo de cuadrado XZ

//* Autor: Manuel Pereiro Conde

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iomanip>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "lecturaShader_0_9.h"
#include "dibujo.h"
#include "colores.h"
using namespace std;
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define pi 3.14159265359
#define MAP_LIMIT 999999999

//? constantes grua
#define VEL_MAX 60
#define ACEL .5
#define FRENADO 0.3
#define REV_MAX -10
#define GIRO_MAX 60
#define VEL_GIRO 1
#define AMORTIGUACION 0.9f
#define INCLIN_MAX 30.0f
#define VEL_INCLIN 1.0f

//? variables grua
float velocidad = 0;
float ang_giro = 0;
GLfloat anguloRuedaZ = 0.0f; //! rotacion roda para simular desplazamiento
GLfloat anguloRuedaX = 0.0f; //! orientacion parcial roda para simular xiro

//? variables camara
int tipoCamara = 0; // 0: perspectiva, 1: primera persona
float alfa = 0.5;
float bbeta = 0.5;
float dist_camara = 80.0f;
float vel_camara = 0.01f;

//? variables ventana
int w_width = 1600;
int w_height = 900;

//* más variables
int momentoDia = 0; //? 0-día, 1-tarde, 2-noche
int luces = 0;		//? 0-apagadas, 1-cortas, 2-largas
int mantenerTeclaT = 0, mantenerTeclaL = 0;
float zipi = 20.0, zape = 25.0;

void entradaTeclado(GLFWwindow *window);

//* texturas
unsigned int texturaSuelo;
unsigned int texturaCarretera;
unsigned int texturaEsquinaCarretera;
unsigned int texturaTronco;
unsigned int texturaRoda;
unsigned int texturaMapa;

//* mapa obj
tinyobj::attrib_t attrib;
vector<tinyobj::shape_t> shapes;
vector<tinyobj::material_t> materials;
vector<float> vertexData;
int numVertices;
string warn, err;
#define filename_mapa "mapa/terrenoMZ.obj"
#define textura_mapa "mapa/texturaMZ.obj"

extern GLuint setShaders(const char *nVertx, const char *nFrag);
GLuint shaderProgram;

typedef struct
{
	glm::vec3 posicion;
	float angulo_real;
	float inclinacion;
	float velocidad;
	glm::vec3 escala;
	unsigned int textura;
	unsigned int VAO;
} parteGrua;

parteGrua base = {{0, 1.5, 0.5}, 0, 0, 0, {4, 2, 10}, 0, 0};
parteGrua cabina = {{0.0, 2.5, 4}, 0, 0, 0, {4.0f, 3.0f, 2.0}, 0, 0};
parteGrua ventana = {{0.0, 2.5, 4.5}, 0, 0, 0, {3.9f, 1.9f, 1.9}, 0, 0};
parteGrua brazo = {{0, 3, 0}, 0, 0, 0, {0.5, 6, 0.5}, 0, 0};
parteGrua articulacion = {{-0.4, 1.0, 0}, 35, 0, 0, {1.0, 1.0, 1.0}, 0, 0};
parteGrua articulacion2 = {{0, 2.5, 0}, 0, 90, 0, {0.7, 0.7, 0.7}, 0, 0};
parteGrua brazo2 = {{0, 3, 0}, 0, 0, 0, {0.5, 6, 0.5}, 0, 0};
parteGrua faroI = {{-1.4, 0.2, 5.0}, 0, 90, 0, {0.6, 0.3, 0.5}, 0, 0}; // Izquierdo
parteGrua faroD = {{1.4, 0.2, 5.0}, 0, 90, 0, {0.6, 0.3, 0.5}, 0, 0};  // Derecho
unsigned int VAO;
unsigned int VAOCuadradoXZ;
unsigned int VAOEsfera;
unsigned int VAOCubo;
unsigned int VAOCarretera;
unsigned int VAOEsquina;
unsigned int VAOMapa;

double t0 = glfwGetTime();
double t1;
double tdelta;
int nbFrames = 0;

void barraCarga(size_t valor, size_t total, int tamano)
{
	float progreso = static_cast<float>(valor) / total;
	int pos = static_cast<int>(progreso * tamano);

	cout << "\r[";
	for (int i = 0; i < tamano; ++i)
	{
		if (i < pos)
			cout << "|";
		else
			cout << " ";
	}
	cout << "] " << fixed << setprecision(1) << (progreso * 100.0f) << "%";
	cout.flush();
}

void tiempo()
{
	static float sec = 0;
	t1 = glfwGetTime();
	nbFrames++;
	tdelta = t1 - t0;
	sec += tdelta;

	if (sec >= 1.0)
	{
		cout << "FPS: " << nbFrames << endl;
		nbFrames = 0;
		sec = 0;
	}
	t0 = t1;
}

//! cámara
void camara()
{

	glUseProgram(shaderProgram);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)w_width / (float)w_height, 0.1f, (float)dist_camara * 200);
	unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glm::vec3 cameraPos, target;

	if (tipoCamara == 0) //! perspectiva (fija)
	{
		cameraPos = glm::vec3(
			(float)dist_camara * sin(alfa) * cos(bbeta),
			(float)dist_camara * sin(bbeta),
			(float)dist_camara * cos(alfa) * cos(bbeta));

		target = glm::vec3(0.0f, 0.0f, 0.0f);
	}
	if (tipoCamara == 1) //! primera persona
	{
		cameraPos = glm::vec3(
			base.posicion.x,
			base.posicion.y + 5,
			base.posicion.z);

		target = cameraPos + glm::vec3(
								 20 * sin(glm::radians(base.angulo_real)),
								 0,
								 20 * cos(glm::radians(base.angulo_real)));
	}

	if (tipoCamara == 2) //! tercera persona
	{
		cameraPos = glm::vec3(
			base.posicion.x - 50 * sin(glm::radians(base.angulo_real)),
			base.posicion.y + 13,
			base.posicion.z - 50 * cos(glm::radians(base.angulo_real)));

		target = glm::vec3(
			base.posicion.x,
			base.posicion.y + 2,
			base.posicion.z);
	}
	if (tipoCamara == 3) //! perspectiva, mirando a grúa
	{
		cameraPos = glm::vec3(
			(float)dist_camara * sin(alfa) * cos(bbeta),
			(float)dist_camara * sin(bbeta),
			(float)dist_camara * cos(alfa) * cos(bbeta));

		target = glm::vec3(
			base.posicion.x,
			base.posicion.y + 2,
			base.posicion.z);
	}
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 view = glm::lookAt(cameraPos, target, up);

	unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

//? --- funcións movimiento grúa ---
//? simula a gravidade no brazo, que cae cara onde esté inclinado
void brazo_aplicar_gravedad()
{
	if (articulacion.inclinacion < 0 && articulacion.inclinacion > -INCLIN_MAX * 1.5)
		articulacion.inclinacion += VEL_INCLIN / 50 * articulacion.inclinacion;
	else if (articulacion.inclinacion > 0 && articulacion.inclinacion < INCLIN_MAX)
		articulacion.inclinacion += VEL_INCLIN / 50 * articulacion.inclinacion;
}

//? simula o xiro da grúa, que se move en función da velocidade da base
void ajustar_giro()
{

	static float angulo_anterior = 0.0f;
	float delta_angulo = base.angulo_real - angulo_anterior;
	angulo_anterior = base.angulo_real;

	base.angulo_real += glm::radians(anguloRuedaX);

	articulacion.angulo_real += delta_angulo * AMORTIGUACION;

	//? se o camion está en movimiento, a palanca tende ir a direccion contraria a que se move, por inercia
	//? se está quieto a palanca tende a seguir na direccion na que está inclinada por gravedad
	if (base.velocidad > 0)
	{
		if (articulacion.inclinacion > -INCLIN_MAX * 1.5)
			articulacion.inclinacion -= VEL_INCLIN;
	}
	else if (base.velocidad < 0)
	{
		if (articulacion.inclinacion < INCLIN_MAX)
			articulacion.inclinacion += VEL_INCLIN;
	}

	if (articulacion.angulo_real > 30.0f)
		articulacion.angulo_real = 30.0f;
	else if (articulacion.angulo_real < -30.0f)
		articulacion.angulo_real = -30.0f;

	articulacion.angulo_real *= 0.95f;
	// o brazo utiliza como matriz de trnasformacion de base o stack da articulacion, asi que se gira a articulacion xirará tamén o brazo
}

//? actualiza a posición da grúa en función da velocidade e controla os límites do mapa
void movimiento()
{
	if (abs(base.velocidad) > 0)
		ajustar_giro();

	base.posicion.x = base.posicion.x + base.velocidad * sin(glm::radians(base.angulo_real)) * tdelta;
	base.posicion.z = base.posicion.z + base.velocidad * cos(glm::radians(base.angulo_real)) * tdelta;

	if (base.posicion.x > MAP_LIMIT)
		base.posicion.x = -MAP_LIMIT;
	if (base.posicion.x < -MAP_LIMIT)
		base.posicion.x = MAP_LIMIT;
	if (base.posicion.z > MAP_LIMIT)
		base.posicion.z = -MAP_LIMIT;
	if (base.posicion.z < -MAP_LIMIT)
		base.posicion.z = MAP_LIMIT;
	anguloRuedaZ += base.velocidad / 3;
	if (anguloRuedaZ >= 360)
		anguloRuedaZ = 0;
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
		cout << "Error al cargar la textura" << endl;
	}
	stbi_image_free(data);
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
//* debuxa unha carretera con liñas de separacion de carril
void dibujarCarretera(glm::vec3 pos, float rot, float tam)
{
	glm::mat4 transform;
	glBindTexture(GL_TEXTURE_2D, texturaCarretera);

	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");
	unsigned int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

	transform = glm::mat4(1.0f);
	transform = glm::translate(transform, pos);
	transform = glm::rotate(transform, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
	transform = glm::scale(transform, glm::vec3(tam, 0.05, 45));
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(VAOCarretera);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
void dibujarEsquinaCarretera(glm::vec3 pos, float rot, float tam)
{
	glm::mat4 transform;
	glBindTexture(GL_TEXTURE_2D, texturaEsquinaCarretera);
	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");
	unsigned int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

	transform = glm::mat4(1.0f);
	transform = glm::translate(transform, pos);
	transform = glm::rotate(transform, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
	transform = glm::scale(transform, glm::vec3(tam, 0.05, tam));
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(VAOEsquina);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

//* debuxa un arbol cun tronco de cubo e unha copa de 3 esferas
void dibujarArbol(glm::vec3 pos, float escala)
{
	glm::mat4 transform;
	glm::mat4 stack;
	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");
	unsigned int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

	//* tronco
	glBindTexture(GL_TEXTURE_2D, texturaTronco);
	transform = glm::mat4(1.0f);
	transform = glm::translate(transform, pos);
	stack = transform;
	transform = glm::scale(transform, glm::vec3(escala * 1.5, escala * 8, escala * 1.5));
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(0.55f, 0.27f, 0.07f))); // marrón
	glBindVertexArray(VAOCubo);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glm::vec3 colorCopa(0.0f, 0.5f, 0.0f);
	float desplazamiento = escala * 2.5;

	//* follas: usan a textura da herba
	glBindTexture(GL_TEXTURE_2D, texturaSuelo);

	glm::vec3 posicionesCopa[3] = {
		glm::vec3(-escala, desplazamiento * 2, 0), // Esfera izquierda
		glm::vec3(escala, desplazamiento * 2, 0),  // Esfera derecha
		glm::vec3(0, desplazamiento * 3, 0)		   // Esfera superior
	};

	for (int i = 0; i < 3; i++)
	{
		transform = stack;
		transform = glm::translate(transform, posicionesCopa[i]);
		transform = glm::scale(transform, glm::vec3(escala * 2.5, escala * 2.5, escala * 2.5));
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		glUniform3fv(colorLoc, 1, glm::value_ptr(colorCopa));
		glBindVertexArray(VAOEsfera);
		glDrawArrays(GL_TRIANGLES, 0, 1080);
	}
}

//* debuxa unha farola
void dibujarFarola(glm::vec3 pos, float escala)
{
	glm::mat4 transform;
	glm::mat4 stack;
	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");
	unsigned int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

	//* pao?
	glBindTexture(GL_TEXTURE_2D, texturaRoda);
	transform = glm::mat4(1.0f);
	transform = glm::translate(transform, pos);
	stack = transform;
	transform = glm::scale(transform, glm::vec3(escala * 1.5, escala * 32, escala * 1.5));
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(0.2, 0.2, 0.2f))); // gris
	glBindVertexArray(VAOCubo);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glm::vec3 colorCopa(0.8f, 0.8f, 0.8f);

	//* bombilla
	glBindTexture(GL_TEXTURE_2D, ventana.textura);

	transform = stack;
	transform = glm::translate(transform, glm::vec3(0, escala * 18, 0));
	transform = glm::scale(transform, glm::vec3(escala * 3, escala * 4, escala * 3));
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform3fv(colorLoc, 1, glm::value_ptr(colorCopa));
	glBindVertexArray(VAOCubo);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	if (momentoDia == 2)
	{
		// lol
	}
}

//* función que debuxa unha parte da grúa
// precisa os punteiros ás matrices de transformación e un stack
void dibujarParteGrua(parteGrua parte, glm::mat4 *transform, glm::mat4 *stack, int modificarStack)
{
	glBindTexture(GL_TEXTURE_2D, parte.textura);

	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");

	*transform = *stack;

	*transform = glm::translate(*transform, parte.posicion);
	*transform = glm::rotate(*transform, glm::radians(parte.inclinacion), glm::vec3(1.0f, 0.0f, 0.0f));
	*transform = glm::rotate(*transform, glm::radians(parte.angulo_real), glm::vec3(0.0f, 1.0f, .0f));
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

//* debuxa as rodas da grúa
void dibujarRuedas()
{

	glm::mat4 transform;
	glm::mat4 stack;
	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");
	unsigned int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

	glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));

	transform = glm::mat4(1.0f);
	transform = glm::translate(transform, glm::vec3(base.posicion.x, base.posicion.y, base.posicion.z));
	transform = glm::rotate(transform, glm::radians(base.angulo_real - 90), glm::vec3(0.0f, 1.0f, 0.0f));
	stack = transform;

	glm::vec3 escalaRueda(1.5f, 1.5f, 0.5f);
	float radioRueda = 0.5f;

	glBindTexture(GL_TEXTURE_2D, texturaRoda);

	glm::vec3 posiciones[4] = {
		{3.5f, -radioRueda, 2.0f},
		{3.5f, -radioRueda, -2.0f},
		{-3.5f, -radioRueda, 2.0f},
		{-3.5f, -radioRueda, -2.0f}};

	for (int i = 0; i < 4; i++)
	{
		transform = stack;
		transform = glm::translate(transform, posiciones[i]);
		if (i < 2)
			// as rodas de diante xiran segun a direccion
			// reducido por 1.2f para que non xire demasiado (queda raro)
			transform = glm::rotate(transform, glm::radians(anguloRuedaX / 1.2f), glm::vec3(0.0f, 1.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(-anguloRuedaZ), glm::vec3(0.0f, 0.0f, 1.0f));

		transform = glm::scale(transform, escalaRueda); // Aplicar escala
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		glBindVertexArray(VAOCubo);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
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
		glClearColor(0.05f, 0.05, 0.2f, 1.0f);

		// printf("ahora es de noche!\n");
		break;

	case 0: // día: sol blanco-amarillento colocado arriba
		glUniform1f(intSol, 1.0f);
		glUniform3f(solDir, 0.0f, 1.0f, 0.0);
		glUniform3f(colorSol, 1.0f, 0.9f, 0.8f);
		glUniform1f(intAmb, 0.5f);
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

	glUniform3f(faroPosLoc, base.posicion.x + 3.1 * sin(glm::radians(base.angulo_real)),
				base.posicion.y + 1,
				base.posicion.z + 3.2 * cos(glm::radians(base.angulo_real)));
	glUniform3f(faroColorLoc, 1.0, 0.9, 0.7);
	float angulo_radianes = glm::radians(base.angulo_real - 90);
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

void dibujarMapa()
{
	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");

	glUseProgram(shaderProgram);
	glBindVertexArray(VAOMapa);
	glBindTexture(GL_TEXTURE_2D, texturaMapa);

	glm::mat4 transform = glm::mat4(1.0f); // Sin escala	

	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glDrawArrays(GL_TRIANGLES, 0, numVertices); // count = number of vertices
}

void display()
{
	glUseProgram(shaderProgram);
	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");
	unsigned int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
	glm::mat4 transform = glm::mat4(1.0f);
	glm::mat4 stack = glm::mat4(1.0f);

	camara();
	tiempo();
	movimiento();
	dibujarSol();
	luzFaros();

	//* suelo
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturaSuelo);

	transform = glm::translate(transform, glm::vec3(0, 0, 0));
	transform = glm::scale(transform, glm::vec3(1, 1, 1));
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(.2, .7, 0.3)));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(VAOCuadradoXZ);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	transform = glm::mat4(1.0f);

	dibujarMapa();
	//* carretera
	dibujarCarretera(glm::vec3(0, 0.1, 100), 0, 155);
	dibujarCarretera(glm::vec3(0, 0.1, -100), 0, 155);
	dibujarCarretera(glm::vec3(-100, 0.1, 0), 90, 155);
	dibujarCarretera(glm::vec3(100, 0.1, 0), 90, 155);
	dibujarEsquinaCarretera(glm::vec3(100, 0.1, 100), 90, 45);
	dibujarEsquinaCarretera(glm::vec3(-100, 0.1, 100), 0, 45);
	dibujarEsquinaCarretera(glm::vec3(100, 0.1, -100), 180, 45);
	dibujarEsquinaCarretera(glm::vec3(-100, 0.1, -100), 270, 45);

	//* arboles

	dibujarArbol(glm::vec3(150, 0, -50), 1.8);
	dibujarArbol(glm::vec3(-150, 0, 50), 2);
	dibujarArbol(glm::vec3(120, 0, 100), 1.2);
	dibujarArbol(glm::vec3(-160, 0, -80), 2.4);
	dibujarArbol(glm::vec3(80, 0, -150), 2);
	dibujarArbol(glm::vec3(-30, 0, -150), 2);
	dibujarArbol(glm::vec3(150, 0, -30), 2);
	dibujarArbol(glm::vec3(-150, 0, 30), 1.7);
	dibujarFarola(glm::vec3(150, 0, 50), 1);

	//* grua
	dibujarParteGrua(base, &transform, &stack, 1);
	dibujarParteGrua(cabina, &transform, &stack, 0);
	dibujarParteGrua(ventana, &transform, &stack, 0);
	dibujarParteGrua(faroI, &transform, &stack, 0);
	dibujarParteGrua(faroD, &transform, &stack, 0);
	dibujarParteGrua(articulacion, &transform, &stack, 1);
	dibujarParteGrua(brazo, &transform, &stack, 1);
	dibujarParteGrua(articulacion2, &transform, &stack, 1);
	dibujarParteGrua(brazo2, &transform, &stack, 0);

	//* ruedas
	dibujarRuedas();

	glBindVertexArray(0);
}

//*
void cargarMapa()
{
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
								filename_mapa, "./mapa", true);

	if (!warn.empty())
		cout << "WARN: " << warn << endl;
	if (!err.empty())
		cerr << "ERR: " << err << endl;
	if (!ret)
		exit(1);

	size_t totalIndices = 0;
	size_t indicesProcesados = 0;
	for (const auto &shape : shapes)
		totalIndices += shape.mesh.indices.size();

	for (const auto &shape : shapes)
	{
		for (const auto &index : shape.mesh.indices)
		{

			// Posiciones (v)
			float vx = attrib.vertices[3 * index.vertex_index + 0];
			float vy = attrib.vertices[3 * index.vertex_index + 1];
			float vz = attrib.vertices[3 * index.vertex_index + 2];

			// Normales (vn)
			float nx = 0.0f, ny = 0.0f, nz = 0.0f;
			if (index.normal_index >= 0)
			{
				nx = attrib.normals[3 * index.normal_index + 0];
				ny = attrib.normals[3 * index.normal_index + 1];
				nz = attrib.normals[3 * index.normal_index + 2];
			}

			// Coordenadas de textura (vt)
			float tx = 0.0f, ty = 0.0f;
			if (index.texcoord_index >= 0)
			{
				tx = attrib.texcoords[2 * index.texcoord_index + 0];
				ty = attrib.texcoords[2 * index.texcoord_index + 1];
			}

			// Intercalamos: posición (3) + normal (3) + texcoord (2)
			vertexData.insert(vertexData.end(), {vx, vy, vz, nx, ny, nz, tx, ty});

			++indicesProcesados;
			if (indicesProcesados % 500 == 0 || indicesProcesados == totalIndices) // actualiza cada 500
				barraCarga(indicesProcesados, totalIndices, 50);
		}
	}
	cout << endl;
	GLuint VBO;
	glGenVertexArrays(1, &VAOMapa);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAOMapa);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

	// layout(location = 0): posición
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	// layout(location = 1): normales
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// layout(location = 2): textura
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	numVertices = vertexData.size() / 8;
	cargarTexturaPNG(texturaMapa, "mapa/texturaMZ.png");
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
}

// Main
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *window = glfwCreateWindow(w_width, w_height, "sputnik", NULL, NULL);
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

	shaderProgram = setShaders("shader.vert", "shader.frag");
	cargarTextura(texturaSuelo, "texturas/suelo.jpg");
	cargarTextura(texturaCarretera, "texturas/carretera.jpg");
	cargarTextura(texturaEsquinaCarretera, "texturas/esquina.jpg");
	cargarTextura(texturaTronco, "texturas/tronco.jpg");
	cargarTextura(texturaRoda, "texturas/roda.jpg");
	articulacion2.textura = texturaRoda;
	articulacion.textura = texturaRoda;
	cargarTextura(base.textura, "texturas/basegrua.jpg");
	cargarTextura(cabina.textura, "texturas/cabina.jpg");
	cargarTextura(brazo.textura, "texturas/brazo.jpg");
	brazo2.textura = brazo.textura;
	articulacion.textura = texturaRoda;
	articulacion2.textura = texturaRoda;
	cargarTexturaPNG(ventana.textura, "texturas/ventana.png");
	cargarTextura(faroI.textura, "texturas/faro.jpg");
	faroD.textura = faroI.textura;

	printf("cargando mapa...\n");
	cargarMapa();
	printf("mapa cargado!");

	glUseProgram(shaderProgram);

	CuadradoXZ();
	CuadradoXZCarretera();
	CuadradoXZEsquina();
	dibujaEsfera();
	dibujaCubo();

	base.VAO = VAOCubo;
	cabina.VAO = VAOCubo;
	brazo.VAO = VAOCubo;
	articulacion.VAO = VAOEsfera;
	articulacion2.VAO = VAOEsfera;
	brazo2.VAO = VAOCubo;
	ventana.VAO = VAOCubo;
	faroI.VAO = VAOEsfera;
	faroD.VAO = VAOEsfera;

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
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{

		if (base.velocidad < VEL_MAX)
		{
			base.velocidad += ACEL;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{

		if (base.velocidad > REV_MAX)
		{
			base.velocidad -= ACEL;
		}
	}
	else
	{
		brazo_aplicar_gravedad();
		if (base.velocidad > 1)
			base.velocidad -= FRENADO;
		else if (base.velocidad < -1)
			base.velocidad += FRENADO;
		else
			base.velocidad = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		if (anguloRuedaX < GIRO_MAX)
			anguloRuedaX += VEL_GIRO;
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		if (anguloRuedaX > -GIRO_MAX)
			anguloRuedaX -= VEL_GIRO;
	}
	else
	{
		if (anguloRuedaX > 0)
			anguloRuedaX -= VEL_GIRO;
		else if (anguloRuedaX < 0)
			anguloRuedaX += VEL_GIRO;
	}

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		tipoCamara = 1;
	}
	else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	{
		tipoCamara = 0;
	}
	else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	{
		tipoCamara = 2;
	}
	else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
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
	if ((glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) && !mantenerTeclaT)
	{
		momentoDia = (momentoDia + 1) % 3;
		mantenerTeclaT = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
	{
		mantenerTeclaT = 0;
	}
	if ((glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) && !mantenerTeclaL)
	{
		luces = (luces + 1) % 3;
		mantenerTeclaL = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE)
	{
		mantenerTeclaL = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		zipi += 1.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		zipi -= 1.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		zape += 1.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
	{
		zape -= 1.0f;
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		base.posicion.y += 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		base.posicion.y -= 0.1f;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		alfa -= vel_camara;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		alfa += vel_camara;
}
