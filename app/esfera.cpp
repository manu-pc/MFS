

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <iostream>
#include "esfera.h"
#include "camaragrua.h"
#include "lecturaShader_0_9.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
extern void entradaTeclado(GLFWwindow *window);
extern void mouse_callback(GLFWwindow *window, double xpos, double ypos);
extern void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

extern GLuint setShaders(const char *nVertx, const char *nFrag);
GLuint shaderProgram;
#define numVertices 8640
// unsigned int VBO, VAO, EBO;
unsigned int VAO;
unsigned int VAOTriangulo = 0;

void dibujaEjes()
{
	unsigned int VBO, EBO;

	float vertices[] = {
		// Vertices          //Colores
		0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // 0
		.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,	// x
		0.0f, .5f, 0.0f, 0.0f, 1.0f, 0.0f,	// y
		0.0f, .5f, 0.0f, 0.0f, 0.0f, 1.0f,	// z
		.5f, .5f, 0.5f, 1.0f, 1.0f, 1.0f	// 1,1,1 bueno realmente la mitad
	};
	unsigned int indices[] = {// empieza desde cero
							  0, 1,
							  0, 2,
							  0, 3,
							  0, 4};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object
	glBindVertexArray(VAO);

	// bind the Vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position de los attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	// position del color Color

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// ya no no necesito el VAo y hago el un unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	// Borro los buffers
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
void dibujaEsfera()
{
	static unsigned int VBO = 0;
	if (vertices_esfera == nullptr)
	{
		std::cout << "Error: vertices_esfera es NULL!" << std::endl;
		return;
	}

	if (VAOTriangulo == 0)
	{
		glGenVertexArrays(1, &VAOTriangulo);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAOTriangulo);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(float), vertices_esfera, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glBindVertexArray(VAOTriangulo);
	glDrawArrays(GL_TRIANGLES, 0, numVertices / 6);
	glBindVertexArray(0);
}
void openGlInit()
{
	// Habilito aqui el depth en vez de arriba para los usuarios de linux y mac mejor arriba
	// Incializaciones varias
	glClearDepth(1.0f);					  // Valor z-buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // valor limpieza buffer color
	glEnable(GL_DEPTH_TEST);			  // z-buffer
	glEnable(GL_CULL_FACE);				  // ocultacion caras back
	glCullFace(GL_BACK);
}

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creo la ventana

	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Grúa", NULL, NULL);
	glfwMakeContextCurrent(window);

	// glad
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return 1;
	}

	// generarShader();
	shaderProgram = setShaders("shader.vert", "shader.frag");

	// Verificación de uniformes en el shader
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

	if (modelLoc == -1 || viewLoc == -1 || projLoc == -1)
	{
		std::cout << "Error: No se encontraron las variables en el shader!" << std::endl;
	}
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	dibujaEjes();
	dibujaEsfera();
	openGlInit();

	// uncomment this call to draw in wireframe polygons.
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Lazo de la ventana mientras no la cierre
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		entradaTeclado(window);

		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// render
		// ------
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		// dibujo los ejes

		glUseProgram(shaderProgram);
		// Dibujo tiangulos

		glBindVertexArray(VAOTriangulo);
		glDrawArrays(GL_TRIANGLES, 0, numVertices);

		glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
		glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, 0);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:

	glDeleteVertexArrays(1, &VAO);

	// glfw: terminate, clearing all

	glfwTerminate();
	return 0;
}

// input