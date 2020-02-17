#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>
#include <cglm/call.h>

static GLuint VBO, VAO, IBO, program, uniformModel, uniformProjection;
float triOffset = 0.0f;
float triMaxOffset = 0.7f;
float triIncrement = 0.005f;
float triRotation = 0.0f;

double toRadians(double degrees)
{
	return degrees * (acos(-1) / (double)180);
}

char* readFile(const char path[])
{
	unsigned int len = 1;
	unsigned int index = 0;
	char* str = (char *)malloc(sizeof(char) * len);
	char* temp;
	char c;
	FILE* file = fopen(path, "r");
	if (!file)
		goto error;
	c = getc(file);
	while (c != EOF)
	{
		str[index] = c; index++;
		if (index == len) {
			len *= 2;
			if (NULL == (temp = (char *)realloc(str, sizeof(char) * len)))
				goto error;
			else
				str = temp;
		}
		c = getc(file);
	}
	if (NULL == (temp = (char *)realloc(str, sizeof(char) * (index + 1))))
		goto error;
	else
		str = temp;
	str[index] = '\0';
	fclose(file);

	return str;
error:
	free(str);
	free(temp);
	fclose(file);
	return NULL;
}

void CreateTriangle()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
	GLuint theShader = glCreateShader(shaderType);

	const GLchar* theCode[1];
	theCode[0] = shaderCode;

	GLint codeLength[1];
	codeLength[0] = strlen(shaderCode);

	glShaderSource(theShader, 1, theCode, codeLength);
	glCompileShader(theShader);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(theShader, 1024, NULL, eLog);
		fprintf(stderr, "Error compiling the %d shader: '%s'\n", shaderType, eLog);
		return;
	}

	glAttachShader(theProgram, theShader);
}

void CompileShaders()
{
	program = glCreateProgram();

	if (!program)
		printf("Error creating shader program!");

	char* vShader = readFile("./shaders/vertex.shader");
	AddShader(program, vShader, GL_VERTEX_SHADER);
	free(vShader);
	char* fShader = readFile("./shaders/fragment.shader");
	AddShader(program, fShader, GL_FRAGMENT_SHADER);
	free(fShader);

	GLint result = 0;
	GLchar eLog[1024];

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(program, sizeof(eLog), NULL, eLog);
		printf("Error linking program '%s'\n", eLog);
		return;
	}

	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(program, sizeof(eLog), NULL, eLog);
		printf("Error validating program '%s'\n", eLog);
		return;
	}

	uniformModel = glGetUniformLocation(program, "model");
	uniformProjection = glGetUniformLocation(program, "projection");

}

struct Keys {
	bool A;
	bool D;
};
struct Keys keys;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_REPEAT) return;
	if (key == GLFW_KEY_A)
		keys.A = (action == GLFW_PRESS);
	if (key == GLFW_KEY_D)
		keys.D = (action == GLFW_PRESS);
}

int main()
{
	GLFWwindow* window;
	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(640, 480, "", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
	glViewport(0, 0, bufferWidth, bufferHeight);

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}
	glEnable(GL_DEPTH_TEST);

	CreateTriangle();
	CompileShaders();

	mat4 perspective = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	glm_perspective(100.0f, (GLfloat)bufferWidth/(GLfloat)bufferHeight, 0.1f, 100.0f, perspective);

	keys.A = false;
	keys.D = false;
	glfwSetKeyCallback(window, key_callback);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(0.05f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);
		float direction = (float)(keys.D - keys.A);
		triOffset += triIncrement * direction;
		if (fabs(triOffset) > triMaxOffset)
			triOffset = triMaxOffset * direction;

		mat4 model = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
		vec3 translation = { triOffset, 0.0f, -2.5f };
		glm_translate(model, translation);
		vec3 rotation = {1.0f, 0.0f, 0.0f};
		glm_rotate(model, toRadians(triRotation),rotation);
		triRotation += 0.1f;
		if (triRotation > 360)
			triRotation -= 360;
		vec3 scale = {0.4f, 0.4f, 1.0f};
		glm_scale(model,scale);

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, *model);
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, *perspective);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glUseProgram(0);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
