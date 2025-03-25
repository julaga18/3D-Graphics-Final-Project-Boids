#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"

#include "Box.cpp"
#include "Boid.hpp"
#include "AABB.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <cstdlib>
#include <ctime> 

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


bool enableNormalMap = true;

namespace texture {

	GLuint water;
	GLuint waterNormal;

	GLuint water2;
	GLuint water2Normal;

	GLuint glass;
	GLuint glassNormal;

	GLuint rocks;
	GLuint rocksNormal;

	GLuint blue, red, green;
	GLuint fishNormal;

	GLuint coral;
	GLuint coralNormal;

	GLuint marble;
	GLuint marbleNormal;

	GLuint wall, back;
	GLuint wallNormal;
}

GLuint program;
GLuint programTex;
GLuint programVAO;
GLuint programNormal;
GLuint programBackground;


Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;
Core::RenderContext cubeContext;
Core::RenderContext fishContext;
Core::RenderContext coralContext;

Core::RenderContext squareContext;
Core::RenderContext blueContext;

glm::vec3 cameraPos = glm::vec3(-15.f, 0.f, 1.0f);
glm::vec3 cameraDir = glm::vec3(1.f, 0.f, 0.f);

GLuint fishVAO, fishVBO, fishEBO;
GLuint coralVAO, coralVBO, coralEBO;
GLuint backgroundVAO, backgroundVBO, backgroundEBO;


float aspectRatio = 1.f;

float lastTime = -1.f;
float deltaTime = 0.f;

std::vector<Boid> boids; // wektor do przechowywania boidów
std::vector<glm::mat4> coralTransforms; // wektor do przechowywania transformacji koralowców

std::vector<AABB> boidAABBs;
std::vector<AABB> coralAABBs;

// Dodane globalne zmienne dla rozmiarów
glm::vec3 fishSize;
glm::vec3 coralSize;

bool drawAABBEnabled = true;

void updateDeltaTime(float time) {
	if (lastTime < 0) {
		lastTime = time;
		return;
	}

	deltaTime = time - lastTime;
	if (deltaTime > 0.1) deltaTime = 0.1;
	lastTime = time;
}

glm::mat4 createCameraMatrix()
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide, cameraDir));
	glm::mat4 cameraRotrationMatrix = glm::mat4({
		cameraSide.x,cameraSide.y,cameraSide.z,0,
		cameraUp.x,cameraUp.y,cameraUp.z ,0,
		-cameraDir.x,-cameraDir.y,-cameraDir.z,0,
		0.,0.,0.,1.,
		});
	cameraRotrationMatrix = glm::transpose(cameraRotrationMatrix);
	glm::mat4 cameraMatrix = cameraRotrationMatrix * glm::translate(-cameraPos);

	return cameraMatrix;
}

glm::mat4 createPerspectiveMatrix()
{

	glm::mat4 perspectiveMatrix;
	float n = 0.05;
	float f = 30.;
	float a1 = glm::min(aspectRatio, 1.f);
	float a2 = glm::min(1 / aspectRatio, 1.f);
	perspectiveMatrix = glm::mat4({
		1,0.,0.,0.,
		0.,aspectRatio,0.,0.,
		0.,0.,(f + n) / (n - f),2 * f * n / (n - f),
		0.,0.,-1.,0.,
		});


	perspectiveMatrix = glm::transpose(perspectiveMatrix);

	return perspectiveMatrix;
}

void drawObjectColor(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color) {
	glUseProgram(program);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightPos"), 0, 0, 0);

	// Przełączenie na tryb linii
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	Core::DrawContext(context); // Rysowanie obiektu jako siatki

	// Przywrócenie normalnego trybu
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void drawObjectTexture(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalMapID, float alpha, bool enableNormalMap) {
	GLuint prog = programNormal;
	glUseProgram(prog);

	// Obliczenie macierzy transformacji
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	// Ustawienia pozycji świateł
	glUniform3f(glGetUniformLocation(prog, "lightPos1"), -5.0f, 10.0f, 0.0f); // Pozycja pierwszego światła
	glUniform3f(glGetUniformLocation(prog, "lightPos2"), 0.0f, 5.5f, -5.0f); // Pozycja niebieskiego światła

	// Ustawienia kolorów świateł
	glUniform3f(glGetUniformLocation(prog, "lightColor1"), 1.0f, 1.0f, 1.0f); // Białe światło
	glUniform3f(glGetUniformLocation(prog, "lightColor2"), 0.0f, 0.0f, 1.0f); // Niebieskie światło

	// Przekazanie tekstur
	Core::SetActiveTexture(textureID, "colorTexture", prog, 0);
	Core::SetActiveTexture(normalMapID, "normalSampler", prog, 1);

	glUniform1f(glGetUniformLocation(prog, "alpha"), alpha);
	glUniform1i(glGetUniformLocation(prog, "enableNormalMapping"), enableNormalMap);

	Core::DrawContext(context);
}

void drawCoralColor(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color) {
	glUseProgram(program);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightPos"), 0, 0, 0);
	Core::DrawContext(context);
}

void drawAABB(GLuint VAO, glm::mat4 modelMatrix, glm::vec3 color) {
	glUseProgram(program);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightPos"), 0, 0, 0);

	glBindVertexArray(VAO);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}


void DrawBoidTexture(const std::vector<Boid>& boids, Core::RenderContext& context, GLuint normalMapID, float alpha, bool enableNormalMap) {
	glUseProgram(programNormal);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	for (const Boid& boid : boids) {
		// Macierz modelu dla każdego boida
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), boid.position) * boid.rotationMatrix *
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		// Ustawienie transformacji
		glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(programNormal, "transformation"), 1, GL_FALSE, (float*)&transformation);
		glUniformMatrix4fv(glGetUniformLocation(programNormal, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

		// Ustawienia pozycji świateł
		glUniform3f(glGetUniformLocation(programNormal, "lightPos1"), -5.0f, 10.0f, 0.0f);
		glUniform3f(glGetUniformLocation(programNormal, "lightPos2"), 0.0f, 5.5f, -5.0f);

		// Ustawienia kolorów świateł
		glUniform3f(glGetUniformLocation(programNormal, "lightColor1"), 1.0f, 1.0f, 1.0f); 
		glUniform3f(glGetUniformLocation(programNormal, "lightColor2"), 0.0f, 0.0f, 1.0f); 

		// Przekazanie tekstur na podstawie grupy boida
		switch (boid.group % 3) { 
		case 0: Core::SetActiveTexture(texture::red, "colorTexture", programNormal, 0); break;
		case 1: Core::SetActiveTexture(texture::green, "colorTexture", programNormal, 0); break;
		case 2: Core::SetActiveTexture(texture::blue, "colorTexture", programNormal, 0); break;
		}
		
		Core::SetActiveTexture(normalMapID, "normalSampler", programNormal, 1);

		glUniform1f(glGetUniformLocation(programNormal, "alpha"), alpha);
		glUniform1i(glGetUniformLocation(programNormal, "enableNormalMapping"), enableNormalMap);

		Core::DrawContext(context);
	}
}


void initAABB(GLuint& VAO, GLuint& VBO, GLuint& EBO, Core::RenderContext& context)
{
	// Rysujemy AABB jako siatkę (GL_LINES)
	glm::vec3 min = context.minCoords;
	glm::vec3 max = context.maxCoords;

	GLfloat vertices[] = {
		min.x, min.y, min.z,  max.x, min.y, min.z,
		max.x, max.y, min.z,  min.x, max.y, min.z,
		min.x, min.y, max.z,  max.x, min.y, max.z,
		max.x, max.y, max.z,  min.x, max.y, max.z
	};

	GLuint indices[] = {
		0, 1, 1, 2, 2, 3, 3, 0, // Front face edges
		4, 5, 5, 6, 6, 7, 7, 4, // Back face edges
		0, 4, 1, 5, 2, 6, 3, 7  // Connecting edges
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(0);
}



void renderScene(GLFWwindow* window) {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ImGui start frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowSize(ImVec2(50, 100), ImGuiCond_FirstUseEver);

	ImGui::Begin("Control Panel");
	ImGui::Checkbox("Draw AABB", &drawAABBEnabled);

	ImGui::Checkbox("Normal Map", &enableNormalMap);

	ImGui::SliderFloat("Alignment", &ALIGNMENT_WEIGHT, 0.0f, 1.0f);
	ImGui::SliderFloat("Cohesion", &COHESION_WEIGHT, 0.0f, 1.0f);
	ImGui::SliderFloat("Separation", &SEPARATION_WEIGHT, 0.0f, 1.0f);

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glm::mat4 transformation;
	float time = glfwGetTime();
	updateDeltaTime(time);

	boidAABBs.clear();
	coralAABBs.clear();

	// Aktualizacja wszystkich boidów
	for (Boid& boid : boids) {
		boid.update(boids, deltaTime);
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), boid.position) * boid.rotationMatrix *
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		AABB transformedAABB = transformAABB(blueContext.minCoords, blueContext.maxCoords, modelMatrix);
		boidAABBs.push_back(transformedAABB);
	}

	// Rysowanie boidów
	DrawBoidTexture(boids, blueContext, texture::fishNormal, 1.f, enableNormalMap);
	if (drawAABBEnabled) {
		for (Boid& boid : boids) {
			glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), boid.position) * boid.rotationMatrix *
				glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
			drawAABB(fishVAO, modelMatrix, glm::vec3(0.0f, 1.0f, 0.0f));

		}
	}

	std::vector<glm::vec3> coralAABBCollisions(coralTransforms.size(), glm::vec3(0.0f, 1.0f, 0.0f));

	// Rysowanie koralowców
	for (size_t i = 0; i < coralTransforms.size(); ++i) {
		AABB transformedAABB = transformAABB(coralContext.minCoords, coralContext.maxCoords, coralTransforms[i]);
		coralAABBs.push_back(transformedAABB);

		// Sprawdzanie kolizji z boidami
		for (size_t j = 0; j < boidAABBs.size(); ++j) {
			if (checkAABBCollision(boidAABBs[j], transformedAABB)) {
				coralAABBCollisions[i] = glm::vec3(1.0f, 0.0f, 0.0f);  // Kolizja - zmiana koloru na czerwony
			}
		}
		if (drawAABBEnabled) {
			drawAABB(coralVAO, coralTransforms[i], coralAABBCollisions[i]);
		}
	}

	for (const auto& transform : coralTransforms) {
		drawObjectTexture(coralContext, transform, texture::coral, texture::coralNormal, 1.f, enableNormalMap);
	}

	// Renderowanie GUI
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	drawObjectTexture(cubeContext, glm::translate(glm::vec3(0, -8.35f, 0)) * glm::scale(glm::vec3(0.42f, 0.411f, 1.23f)), texture::marble, texture::marbleNormal, 1.f, enableNormalMap);
	drawObjectTexture(squareContext, glm::translate(glm::vec3(0, -4.8f, 0)) * glm::scale(glm::vec3(0.041f, 0.031f, 0.121f)), texture::rocks, texture::rocksNormal, 1.f, enableNormalMap);
	drawObjectTexture(squareContext, glm::translate(glm::vec3(0, 4.23f, 0)) * glm::scale(glm::vec3(0.042f, 0.003f, 0.122f)), texture::water2, texture::water2Normal, 0.4f, enableNormalMap);
	drawObjectTexture(cubeContext, glm::translate(glm::vec3(0, 0, 0)) * glm::scale(glm::vec3(0.42f, 0.42f, 1.22f)), texture::water, texture::waterNormal, 0.4f, enableNormalMap);
	drawObjectTexture(cubeContext, glm::translate(glm::vec3(0, 0, 0)) * glm::scale(glm::vec3(0.42f, 0.42f, 1.22f)), texture::glass, texture::glassNormal, 0.f, enableNormalMap);

	glUseProgram(0);
	glfwSwapBuffers(window);
}



void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
}

void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}

	aiMesh* mesh = scene->mMeshes[0];
	context.initFromAssimpMesh(scene->mMeshes[0]);

	// Inicjalizacja wartości min/max
	glm::vec3 minCoords(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 maxCoords(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		glm::vec3 vertex(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

		minCoords.x = std::min(minCoords.x, vertex.x);
		minCoords.y = std::min(minCoords.y, vertex.y);
		minCoords.z = std::min(minCoords.z, vertex.z);

		maxCoords.x = std::max(maxCoords.x, vertex.x);
		maxCoords.y = std::max(maxCoords.y, vertex.y);
		maxCoords.z = std::max(maxCoords.z, vertex.z);
	}

	context.minCoords = minCoords;
	context.maxCoords = maxCoords;


	// Obliczenie wielkości modelu
	glm::vec3 modelSize = maxCoords - minCoords;

	context.modelSize = modelSize;

	// Wyświetlenie wyników
	std::cout << "Model Size: " << modelSize.x << " x " << modelSize.y << " x " << modelSize.z << std::endl;
	std::cout << "Min Coords: (" << minCoords.x << ", " << minCoords.y << ", " << minCoords.z << ")\n";
	std::cout << "Max Coords: (" << maxCoords.x << ", " << maxCoords.y << ", " << maxCoords.z << ")\n";
}



void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_5_1.vert", "shaders/shader_5_1.frag");
	programTex = shaderLoader.CreateProgram("shaders/shader_5_1_tex.vert", "shaders/shader_5_1_tex.frag");
	programVAO = shaderLoader.CreateProgram("shaders/shader_vao.vert", "shaders/shader_vao.frag");
	programNormal = shaderLoader.CreateProgram("shader_normal.vert", "shader_normal.frag");

	loadModelToContext("./models/cube.obj", cubeContext);
	loadModelToContext("./models/fish.obj", fishContext);
	loadModelToContext("./models/coral.obj", coralContext);
	loadModelToContext("./models/square.glb", squareContext);
	loadModelToContext("./models/blue.obj", blueContext);

	texture::water = Core::LoadTexture("./textures/w.png");
	texture::water2 = Core::LoadTexture("./textures/w2.png");
	texture::glass = Core::LoadTexture("./textures/gl.jpg");
	texture::rocks = Core::LoadTexture("./textures/rocks_diff.jpg");
	texture::blue = Core::LoadTexture("./textures/b.png");
	texture::red = Core::LoadTexture("./textures/red.png");
	texture::green = Core::LoadTexture("./textures/green.png");
	texture::marble = Core::LoadTexture("./textures/marbler.png");
	texture::coral = Core::LoadTexture("./textures/coral.png");

	texture::waterNormal = Core::LoadTexture("./textures/wn.png");
	texture::water2Normal = Core::LoadTexture("./textures/w2n.png");
	texture::glassNormal = Core::LoadTexture("./textures/gln.jpg");
	texture::rocksNormal = Core::LoadTexture("./textures/rocks_nor.jpg");
	texture::fishNormal = Core::LoadTexture("./textures/bn.png");
	texture::marbleNormal = Core::LoadTexture("./textures/mn.jpg");
	texture::coralNormal = Core::LoadTexture("./textures/coral_normal.png");

	//InitImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");


	// Inicjalizacja VAO, VBO i EBO dla różnych kontekstów
	initAABB(fishVAO, fishVBO, fishEBO, blueContext);
	initAABB(coralVAO, coralVBO, coralEBO, coralContext);

	// Inicjalizacja pozycji koralowców
	coralTransforms.push_back(
		glm::translate(glm::vec3(0, -4.0f, 0)) * 
		glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * 
		glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::scale(glm::vec3(0.02f))
	);
	coralTransforms.push_back(
		glm::translate(glm::vec3(0, -4.0f, -5.0f)) * 
		glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * 
		glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::scale(glm::vec3(0.04f)) 
	);
	coralTransforms.push_back(
		glm::translate(glm::vec3(0, -4.0f, 5.0f)) * 
		glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * 
		glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::scale(glm::vec3(0.03f)) 
	);

	// Inicjalizacja boidów
	for (int i = 0; i < 40; ++i) {
		glm::vec3 position(
			(rand() % 21 - 10) * 0.4f, // Zmieniamy zakres na -4 do 4
			(rand() % 21 - 10) * 0.4f, // Zmieniamy zakres na -4 do 4
			(rand() % 21 - 10) * 1.2f  // Zmieniamy zakres na -4 do 4
		);

		glm::vec3 velocity(((rand() % 200 - 100) / 100.0f) * MAX_SPEED,
			((rand() % 200 - 100) / 100.0f) * MAX_SPEED,
			((rand() % 200 - 100) / 100.0f) * MAX_SPEED);

		int group = rand() % 3;

		boids.push_back(Boid(position, velocity, group));
	}

}

void cleanup()
{
	glDeleteBuffers(1, &fishVBO);
	glDeleteBuffers(1, &fishEBO);
	glDeleteVertexArrays(1, &fishVAO);

	glDeleteBuffers(1, &coralVBO);
	glDeleteBuffers(1, &coralEBO);
	glDeleteVertexArrays(1, &coralVAO);
}

void shutdown(GLFWwindow* window)
{
	cleanup();
	shaderLoader.DeleteProgram(program);
	shaderLoader.DeleteProgram(programTex);
	shaderLoader.DeleteProgram(programVAO);
	shaderLoader.DeleteProgram(programNormal);
}

//obsluga wejscia
void processInput(GLFWwindow* window)
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 CameraUp = glm::vec3(0.f, 1.f, 0.f);
	float angleSpeed = 0.005f;
	float moveSpeed = 0.05f;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		cameraPos += cameraSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		cameraPos -= cameraSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		cameraPos += CameraUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		cameraPos -= CameraUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraDir = glm::vec3(glm::eulerAngleY(angleSpeed) * glm::vec4(cameraDir, 0));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraDir = glm::vec3(glm::eulerAngleY(-angleSpeed) * glm::vec4(cameraDir, 0));


}

// glowna petla
void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		renderScene(window);
		glfwPollEvents();
	}
}
//}