#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

//! cámara
void camara(int tipoCamara)
{
	glm::vec3 direccion;
	direccion.x = cos(glm::radians(base.inclinacion)) * sin(glm::radians(base.angulo_XZ));
	direccion.y = -sin(glm::radians(base.inclinacion));
	direccion.z = cos(glm::radians(base.inclinacion)) * cos(glm::radians(base.angulo_XZ));
	direccion = glm::normalize(direccion);

	glUseProgram(shaderProgram);
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