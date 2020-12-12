#include "glad.h" // must be before glfw.h
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

constexpr int32_t SCREEN_WIDTH = 1024;
constexpr int32_t SCREEN_HEIGHT = 800;
constexpr float fov = glm::radians(90.0f);
constexpr float LINE_FLOOR = 60.0f;

constexpr auto vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

constexpr auto fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
} 
)";

unsigned int loadShaders(const char* shaderSource, GLenum shaderType) {

	unsigned int shader{ 0 };
	int success{ 0 };
	char infoLog[1024];

	shader = glCreateShader(shaderType); // GL_VERTEX_SHADER

	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(shader, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
			<< infoLog << std::endl;
	}
	return shader;
}

unsigned int makeShaderProgram(uint32_t vertexShader, uint32_t fragmentShader) {
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

void error_callback(int error, const char* description) {
	std::cerr << "Error: " << description << " error number " << error
		<< std::endl;
}

void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action,
	int /*mods*/) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void camera(uint32_t shaderId) {
	glm::mat4 view = glm::mat4(1.0f);

	float zFar = (SCREEN_WIDTH / 2.0f) / tanf(fov / 2.0f); // was 90.0f
	glm::vec3 cameraPos =
		glm::vec3(SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, zFar);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);


	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	int modelView = glGetUniformLocation(shaderId, "view");
	glUniformMatrix4fv(modelView, 1, GL_FALSE, glm::value_ptr(view));
}


void moveLines(std::vector<glm::vec3>& lines) {
	unsigned int i = 0;
	for (auto& vert : lines) {
		if (i % 2 == 0) {
			vert.x = 0.0f;
			vert.y = 0.0f;
			vert.z = LINE_FLOOR + 00.0f;
		}
		else {
			vert.x = 350.0f;
			vert.y = 350.0f;
			vert.z = LINE_FLOOR + 00.0f;
		}
		i++;
	}
}



int main() {
	std::vector<glm::vec3> lines = {
	glm::vec3(0.0f,  0.0f, 1.0f),
	glm::vec3(100.5f,  100.5f, 1.0f),
	};

	float deltaTime = 0.0f; // Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame

	srand(time(NULL));

	if (!glfwInit()) {
		// Initialization failed
		std::cerr << "Error could not init glfw!" << std::endl;
		exit(1);
	}

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "My Title",
		nullptr, nullptr);
	if (!window) {
		std::cerr << "Error could not create window" << std::endl;
		exit(1);
		// Window or OpenGL context creation failed
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << " Error could not load glad " << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(1);
	}

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
		std::cout << "OpenGL Error/Warning: " << message << std::endl;
		}, nullptr);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, lines.size()*sizeof(glm::vec3), lines.data(),
		GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,  sizeof(glm::vec3), (void*)0);

	auto vertexShader = loadShaders(vertexShaderSource, GL_VERTEX_SHADER);
	auto fragmentShader = loadShaders(fragmentShaderSource, GL_FRAGMENT_SHADER);
	auto shaderProgram = makeShaderProgram(vertexShader, fragmentShader);

	float zFar = (SCREEN_WIDTH / 2.0) / tanf(fov / 2.0f) + 10.0f; // 100.0f
	glm::mat4 projection = glm::perspective(
		fov, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, zFar);

	//moveLines(lines);
	std::cout << "zFar=" << zFar << std::endl;
	for (auto& poly : lines) {
		std::cout << poly.x << " " << poly.y << " " << poly.z << std::endl;
	}
	glm::vec3 movement(1.0f, 0.0f, 0);

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		//moveLines(lines);


		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

	// 2. use our shader program when we want to render an object
		glUseProgram(shaderProgram);

		int modelprj = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(modelprj, 1, GL_FALSE, glm::value_ptr(projection));

		camera(shaderProgram);

		glBindVertexArray(VAO);
		for (auto& vert : lines) {
			glm::mat4 model = glm::mat4(1.0f);

			movement.x = 64.0f * deltaTime / 1.0f;
			//movement.y = 64.0f * deltaTime / 1.0f;

			vert = vert + movement;
			//std::cout << "mov.x= " << movement.x << " movement.y=" << movement.y << " mov.z=" << movement.z << std::endl;
			//std::cout << "vert.x=" << vert.x << " vert.y=" << vert.y << " vert.z=" << vert.z << std::endl;
			if (vert.x > SCREEN_WIDTH) {
				vert.x = 0.0f;				
			}
			if (vert.y > SCREEN_HEIGHT) {
				vert.y = 0.0f;				
			}
			model = glm::translate(model, vert);
			model = glm::scale(model, glm::vec3(1.0, 1.0, 1.0));

			int modelLoc = glGetUniformLocation(shaderProgram, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			glLineWidth(3.3f);
			glDrawArrays(GL_LINES, 0, 2);
			auto err = glGetError();

			if (err != GL_NO_ERROR) {
				std::cout << " error " << err << std::endl;
			}
			
		}

	glfwSwapBuffers(window);
	// Keep running
	glfwPollEvents();
}


glDeleteVertexArrays(1, &VAO);
glDeleteBuffers(1, &VBO);

glfwDestroyWindow(window);
glfwTerminate();
return 0;
}
