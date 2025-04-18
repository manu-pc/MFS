//! ----------
//? *  Grúa  *
//! ----------
// todo: textura baseavion é un pouco fea, a farola non ten luz (podela quitar lol)
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
#include "mapa.h"
#include "colores.h"
using namespace std;
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define pi 3.14159265359
#define MAP_LIMIT 999999999

//? constantes avion
#define VEL_MAX 60
#define ACEL .5
#define FRENADO 0.005
#define REV_MAX -10
#define MAX_PITCH 30.0f
#define MAX_ROLL 45.0f
#define PITCH_SPEED .4f
#define ROLL_SPEED 1.0f

//? variables avion
float velocidad = 0;
float ang_giro = 0;
float pitch = 0.0f; // rotation around X-axis
float yaw = 0.0f;	// rotation around Y-axis
float roll = 0.0f;	// rotation around Z-axis

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

void entradaTeclado(GLFWwindow *window);
chunk **chunks;
//* texturas
unsigned int texturaSuelo;
unsigned int texturaCarretera;
unsigned int texturaEsquinaCarretera;
unsigned int texturaTronco;

extern GLuint setShaders(const char *nVertx, const char *nFrag);
GLuint shaderProgram;

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
int test;
parteavion base = {{0, 1.5, 0.5}, 0, 0, 0, 0, {4, 2, 10}, 0, 0};
parteavion cabina = {{0.0, 2.5, 4}, 0, 0, 0, 0, {4.0f, 3.0f, 2.0}, 0, 0};
parteavion ventana = {{0.0, 2.5, 4.5}, 0, 0, 0, 0, {3.9f, 1.9f, 1.9}, 0, 0};
parteavion faroI = {{-1.4, 0.2, 5.0}, 0, 90, 0, 0, {0.6, 0.3, 0.5}, 0, 0}; // Izquierdo
parteavion faroD = {{1.4, 0.2, 5.0}, 0, 90, 0, 0, {0.6, 0.3, 0.5}, 0, 0};  // Derecho
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
	glm::vec3 direccion;
	direccion.x = cos(glm::radians(base.inclinacion)) * sin(glm::radians(base.angulo_XZ));
	direccion.y = -sin(glm::radians(base.inclinacion));
	direccion.z = cos(glm::radians(base.inclinacion)) * cos(glm::radians(base.angulo_XZ));
	direccion = glm::normalize(direccion);

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
								 20 * sin(glm::radians(base.angulo_XZ)),
								 0,
								 20 * cos(glm::radians(base.angulo_XZ)));
	}

	if (tipoCamara == 2) //! tercera persona
	{
		cameraPos = glm::vec3(
			base.posicion.x - 50 * direccion.x,
			base.posicion.y + 13,
			base.posicion.z - 50 * direccion.z);

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

//? actualiza a posición da grúa en función da velocidade e controla os límites do mapa
void movimiento()
{
	if (fabs(base.rotacion) > 10)
	{
		yaw -= base.rotacion * 0.001 * sqrt(base.velocidad);
	}
	base.inclinacion = pitch;
	base.angulo_XZ = yaw;
	base.rotacion = roll;
	glm::vec3 direccion;
	direccion.x = cos(glm::radians(base.inclinacion)) * sin(glm::radians(base.angulo_XZ));
	direccion.y = -sin(glm::radians(base.inclinacion));
	direccion.z = cos(glm::radians(base.inclinacion)) * cos(glm::radians(base.angulo_XZ));
	direccion = glm::normalize(direccion);
	base.posicion += direccion * base.velocidad * tdelta;
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



//* función que debuxa unha parte da grúa
// precisa os punteiros ás matrices de transformación e un stack
void dibujarParteavion(parteavion parte, glm::mat4 *transform, glm::mat4 *stack, int modificarStack)
{
	glBindTexture(GL_TEXTURE_2D, parte.textura);

	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");

	*transform = *stack;

	*transform = glm::translate(*transform, parte.posicion);
	*transform = glm::rotate(*transform, glm::radians(parte.angulo_XZ), glm::vec3(0.0f, 1.0f, 0.0f));

	*transform = glm::rotate(*transform, glm::radians(parte.inclinacion), glm::vec3(1.0f, 0.0f, 0.0f));
	*transform = glm::rotate(*transform, glm::radians(parte.rotacion), glm::vec3(0.0f, 0.0f, 1.0f));

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

void dibujarMapa(chunk *chunk)
{
	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");

	glUseProgram(shaderProgram);
	glBindVertexArray(chunk->VAO);

	glBindTexture(GL_TEXTURE_2D, chunk->textura);

	glm::mat4 transform = glm::mat4(1.0f); // Sin escala
	transform = glm::translate(transform, glm::vec3(chunk->x * 10, 0, chunk->y * 10));
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glDrawArrays(GL_TRIANGLES, 0, chunk->numVertices); // count = number of vertices
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
	luzFaros();

	for (int i = 0; i < 4; i++)
	{
		dibujarMapa(chunks[i]);
	}

	//* avion
	dibujarParteavion(base, &transform, &stack, 1);
	dibujarParteavion(cabina, &transform, &stack, 0);
	dibujarParteavion(ventana, &transform, &stack, 0);
	dibujarParteavion(faroI, &transform, &stack, 0);
	dibujarParteavion(faroD, &transform, &stack, 0);

	//* ruedas
	// roubaronmas en ribeira

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

	// esto é solo para que non me salte un foquin warn
	(void)window;
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

	chunks = initChunks();

	shaderProgram = setShaders("shader.vert", "shader.frag");
	cargarTextura(texturaSuelo, "texturas/suelo.jpg");
	cargarTextura(texturaCarretera, "texturas/carretera.jpg");
	cargarTextura(texturaEsquinaCarretera, "texturas/esquina.jpg");
	cargarTextura(texturaTronco, "texturas/tronco.jpg");
	cargarTextura(base.textura, "texturas/basegrua.jpg");
	cargarTextura(cabina.textura, "texturas/cabina.jpg");
	cargarTexturaPNG(ventana.textura, "texturas/ventana.png");
	cargarTextura(faroI.textura, "texturas/faro.jpg");
	faroD.textura = faroI.textura;

	glUseProgram(shaderProgram);

	dibujaEsfera();
	dibujaCubo();

	base.VAO = VAOCubo;
	cabina.VAO = VAOCubo;
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
	} // Pitch control (nose up/down)

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		if (pitch < MAX_PITCH)
			pitch += PITCH_SPEED;
	}
	else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		if (pitch > -MAX_PITCH)
			pitch -= PITCH_SPEED;
	}
	else
	{
		// Auto-level pitch
		if (pitch > 0)
			pitch -= PITCH_SPEED;
		else if (pitch < 0)
			pitch += PITCH_SPEED;
	}

	// Roll control (bank left/right)
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		if (roll < MAX_ROLL)
			roll += ROLL_SPEED;
	}
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		if (roll > -MAX_ROLL)
			roll -= ROLL_SPEED;
	}
	else
	{
		// Auto-level roll
		if (roll > 0)
			roll -= ROLL_SPEED;
		else if (roll < 0)
			roll += ROLL_SPEED;
	}

	// Throttle (speed control)q
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
		// Auto slow down
		if (base.velocidad > 1)
			base.velocidad -= FRENADO;
		else if (base.velocidad < -1)
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

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		alfa -= vel_camara;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		alfa += vel_camara;
}
