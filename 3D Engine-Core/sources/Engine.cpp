#pragma once
#include "Engine.h"
#include "externals/imgui/imgui_impl_glfw.h"
#include "externals/imgui/ImGuiFileDialog/ImGuiFileDialog.h"
#include "../sources/Global_Variable.h"

// Private functions
void Engine::initGLFW()
{
	//INIT GLFW
	if (glfwInit() == GLFW_FALSE)
	{
		std::cout << "ERROR::GLFW_INIT_FAILED" << "\n";
		glfwTerminate();
	}
}

// init window
void Engine::initWindow(const char* title, bool resizable)
{
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, this->GL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, this->GL_VERSION_MINOR);
	glfwWindowHint(GLFW_RESIZABLE, resizable);

	this->window = glfwCreateWindow(this->WINDOW_WIDTH, this->WINDOW_HEIGHT, title, NULL, NULL);
	std::cout << "3D Project with OpenGL - Console Debug" << std::endl << std::endl;

	if (this->window == nullptr)
	{
		std::cout << "ERROR::GLFW_WINDOW_INIT_FAILED" << "\n";
		glfwTerminate();
	}

	glfwGetFramebufferSize(this->window, &this->framebufferWidth, &this->framebufferHeight);
	glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
	//IMPORTANT WHITH PERSPECTIVE MATRIX!!!

	//glViewport(0, 0, framebufferWidth, framebufferHeight);

	glfwMakeContextCurrent(this->window); //IMPORTANT!!

	// tell GLFW to capture our mouse
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

// init GLAD
int Engine::initGLAD()
{
	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
}

// init openGl option
void Engine::initOpenGLOptions()
{
	// enable z coordiante
	glEnable(GL_DEPTH_TEST);

	// hide things behind
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Input
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

// init matrices
void Engine::initMatrices()
{
	this->ViewMatrix = camera.GetViewMatrix();

	this->ProjectionMatrix = glm::mat4(1.f);
	this->ProjectionMatrix = glm::perspective(
		glm::radians(this->fov),
		static_cast<float>(this->framebufferWidth) / this->framebufferHeight,
		this->nearPlane,
		this->farPlane
	);
}

// init shader
void Engine::initShaders()
{
	// build and compile shaders
	// -------------------------
	Shader* ourShader0 = new Shader("resources/shaders/vertLighting.vs.glsl", "resources/shaders/pointLighting.fs.glsl");
	this->shaders.push_back(ourShader0);

	Shader* ourShader1 = new Shader("resources/shaders/skybox_vs.glsl", "resources/shaders/skybox_fs.glsl");
	this->shaders.push_back(ourShader1);
	//this->shaders.push_back(lampShader);

	//Shader* ourShader2 = new Shader("resources/shaders/ground_vs.glsl", "resources/shaders/ground_fs.glsl");
	//this->shaders.push_back(ourShader2);
}

void Engine::initModels()
{
	this->models.push_back(new Model("resources/objects/bedroom/Bedroom 11.obj"));
}

void Engine::initPointLights()
{
	this->Light.push_back(new PointLight(glm::vec3(1.0f, 1.0f, 1.0f), 4.0f, glm::vec3(0.0f), glm::vec3(0.0f)));
}

void Engine::initLights()
{
	this->initPointLights();
}

void Engine::initUniforms()
{
	this->shaders[0]->setUniformVec3("viewPos", camera.Position);
	//INIT UNIFORMS
	this->shaders[0]->setUniformMat4("view", this->ViewMatrix, false);
	this->shaders[0]->setUniformMat4("projection", this->ProjectionMatrix, false);

	for each (PointLight * pl in this->Light)
	{
		pl->sendToShader(*this->shaders[0]);
	}
}

Engine::~Engine()
{
	glfwDestroyWindow(this->window);
	glfwTerminate();

	for (auto*& i : this->models)
		delete i;

	for (size_t i = 0; i < this->Light.size(); i++)
		delete this->Light[i];

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

//Accessors
int Engine::getWindowShouldClose()
{
	return glfwWindowShouldClose(this->window);
}

//Modifiers
void Engine::setWindowShouldClose()
{
	glfwSetWindowShouldClose(this->window, GLFW_TRUE);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Engine::updateKeyboardInput()
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
		this->dt *= 3.0f;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		this->camera.ProcessKeyboard(FORWARD, this->dt);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		this->camera.ProcessKeyboard(BACKWARD, this->dt);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		this->camera.ProcessKeyboard(LEFT, this->dt);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		this->camera.ProcessKeyboard(RIGHT, this->dt);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void Engine::updateMouseInput()
{
	//switch (this->window_cursor_state)
	//{
	//case 0:	glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//case 1:	glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	//}


	double mouseX, mouseY;
	glfwGetCursorPos(this->window, &mouseX, &mouseY);

	if (this->firstMouse)
	{
		this->lastMouseX = (float)mouseX;
		this->lastMouseY = (float)mouseY;
		this->firstMouse = false;
	}

	// Calc offset
	this->mouseOffsetX = -(float)mouseX + this->lastMouseX;
	this->mouseOffsetY = -this->lastMouseY + mouseY;

	// Set last X and Y
	this->lastMouseX = (float)mouseX;
	this->lastMouseY = (float)mouseY;

	// Move light
	if (glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		this->window_cursor_state = 0;
		this->Light[0]->setPosition(this->camera.Position);
		this->camera.ProcessMouseMovement(this->mouseOffsetX, this->mouseOffsetY);
		mouseOffsetX = mouseOffsetY = 0;
	}

	// Move light
	//else if (glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	//{
	//	this->window_cursor_state = 1;
	//}

	//else if(glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	//{
	//	this->Light[0]->setPosition(this->camera.Position);
	//}

	this->camera.ProcessMouseScroll((float)this->mouseScaleScroll);
	mouseScaleScroll = 0;
}

// Mouse scroll call back function
void Engine::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Engine::mouseScaleScroll = yoffset;
}

// Up date user's input
void Engine::updateInput()
{
	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	// ------------------------------------------------------------------------------
	glfwPollEvents();

	this->updateKeyboardInput();
	this->updateMouseInput();
	//this->camera.updateInput(dt, -1, this->mouseOffsetX, this->mouseOffsetY);
}

// Functions
// Update variable value
void Engine::updateDt()
{
	this->curTime = static_cast<float>(glfwGetTime());
	this->dt = this->curTime - this->lastTime;
	this->lastTime = this->curTime;
}

// Update user's interation on frame
void Engine::update()
{
	//UPDATE INPUT ---
	this->updateDt();
	this->updateInput();
}

// Create an ImGUI window to get user's interaction
void Engine::initIMGUI()
{
	// Setup Dear ImGui context
	//IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	//ImGui_ImplOpenGL3_Init(glsl_version);
}

// Render ImGUI
void Engine::ImGuiRender()
{
	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		//static float f = 0.0f;
		//static int counter = 0;
		ImGui::Begin("Trasnformation");
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::SetWindowSize(ImVec2(400, 150));
		glm::vec3 Trans_Val = this->models[0]->position;
		ImGui::SliderFloat3("Translation", &Trans_Val.x, -100.0f, 100.0f);
		models[0]->position = Trans_Val;

		ImGui::SliderFloat3("Scalization", &Scale_Var.x, 0.0f, 5.0f);
		models[0]->scale = Scale_Var;

		ImGui::SliderFloat3("Rotation", &Rotate_Var.x, 0, 1.0f);
		models[0]->rotation = Rotate_Var;

		ImGui::SliderFloat("Radian", &Radian, 0.0f, 360.0f);

		ImGui::End();
		// open Dialog Simple
		ImGui::Begin("Choose Object");
		ImGui::SetWindowPos(ImVec2(0, 160));
		ImGui::SetWindowSize(ImVec2(400, 80));
		if (ImGui::Button("Pick a Object model"))
			ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".obj", ".");

		// display
		if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
		{
			// action if OK
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
				filePathName.replace(filePathName.find_last_of('\\'), 1, std::string("/"));
				std::cout << filePathName;
				this->models.push_back(new Model(filePathName));
				// action
			}

			// close
			ImGuiFileDialog::Instance()->Close();
		}
		ImGui::End();
	}

	// Rendering ImGui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Update uniform for each frame
void Engine::updateUniforms()
{
	// Enable shader
	this->shaders[0]->use();

	for each (PointLight * pl in this->Light)
	{
		pl->sendToShader(*this->shaders[0]);
	}

	// Update view matrix (camera)
	this->ViewMatrix = this->camera.GetViewMatrix();

	// View/Projection transformations
	this->ProjectionMatrix = glm::perspective(glm::radians(camera.Zoom),
		static_cast<float>(this->framebufferWidth) / this->framebufferHeight,
		this->nearPlane,
		this->farPlane);

	this->shaders[0]->setUniformMat4("projection", this->ProjectionMatrix, false);
	this->shaders[0]->setUniformMat4("view", this->ViewMatrix, false);

	// Render model
	for (auto& i : this->models) 
	{
		glm::mat4 model;
		model = glm::translate(model, i->position);
		model = glm::scale(model, i->scale);
		model = glm::rotate(model, Radian, glm::vec3(0.0f, 1.0f, 0.0f));

		// Send updated uniform to shader program
		this->shaders[0]->setUniformMat4("model", model, false);
		i->Draw(*(this->shaders[0]));
	}
	// Transform the loaded model

	//Update framebuffer size and projection matrix
	glfwGetFramebufferSize(this->window, &this->framebufferWidth, &this->framebufferHeight);
}

// Create a skybox for our engine
void Engine::initSkyBox()
{
	// skybox VAO
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// load textures
// -------------
	vector<std::string> faces;
	faces.push_back("resources/skybox/right.jpg");
	faces.push_back("resources/skybox/left.jpg");
	faces.push_back("resources/skybox/top.jpg");
	faces.push_back("resources/skybox/bottom.jpg");
	faces.push_back("resources/skybox/back.jpg");
	faces.push_back("resources/skybox/front.jpg");

	cubemapTexture = Model::LoadCubemap(faces);

	this->shaders[1]->use();
	this->shaders[1]->setInt("skybox", 0);
}

// Create a ground for engine
void Engine::initGround()
{
	// skybox VAO
	glGenVertexArrays(1, &groundVAO);
	glGenBuffers(1, &groundVBO);
	glBindVertexArray(groundVAO);
	glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GrassVertices), GrassVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));


	// load textures
	string path = "resources/textures/grass.png";
	this->groundTexture = Model::loadTexture(path.c_str());

}

// Main Render function
void Engine::render()
{
	// UPDATE --- 
	//updateInput(window);

	// DRAW ---
	// Clear
	glClearColor(0.1f, 0.1f, 0.1f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// **************************** Render Model ******************************
	///////////////////////////////////////////////////////////////////////////

	glPushMatrix();
	// Update the uniforms : Send model, view, projection matrix to shader program
	this->updateUniforms();

	glPopMatrix();
	glBindVertexArray(0);



	// this->shaders[2]->use();
	// draw scene as normal
	glm::mat4 model_m = glm::mat4(1.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);

	// *********************** Render Ground *********************************
	///////////////////////////////////////////////////////////////////////////
	glBindVertexArray(0);

	//this->shaders[2]->use();
	//this->shaders[2]->setUniformMat4("model", model_m, false);
	//this->shaders[2]->setUniformMat4("view", view, false);
	//this->shaders[2]->setUniformMat4("projection", projection, false);
	//glBindVertexArray(this->groundVAO);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, this->groundTexture);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	//glBindVertexArray(0);

	// *********************** Render Skybox ******************************
	///////////////////////////////////////////////////////////////////////////


	// Render Sky box
	// Draw skybox as last
	glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
	this->shaders[1]->use();
	view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
	this->shaders[1]->setUniformMat4("view", view, false);
	this->shaders[1]->setUniformMat4("projection", projection, false);

	// skybox cube
	glBindVertexArray(this->skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default

	ImGuiRender();

	// End Draw
	glfwSwapBuffers(window);
	//glFlush();
	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glfwPollEvents();
}

// Static functions
void Engine::framebuffer_resize_callback(GLFWwindow* window, int fbW, int fbH)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, fbW, fbH);
}

//Constructors / Destructors
Engine::Engine(
	const char* title,
	const int WINDOW_WIDTH, const int WINDOW_HEIGHT,
	const int GL_VERSION_MAJOR, const int GL_VERSION_MINOR,
	bool resizable
)
	:
	WINDOW_WIDTH(WINDOW_WIDTH),
	WINDOW_HEIGHT(WINDOW_HEIGHT),
	GL_VERSION_MAJOR(GL_VERSION_MAJOR),
	GL_VERSION_MINOR(GL_VERSION_MINOR),
	camera(glm::vec3(0.f, 2.f, 5.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f))
{
	// Init variables
	this->window = nullptr;
	this->framebufferWidth = this->WINDOW_WIDTH;
	this->framebufferHeight = this->WINDOW_HEIGHT;

	// Set up view-port
	this->fov = 90.f;
	this->nearPlane = 0.1f;
	this->farPlane = 1000.f;

	// Initialize moving specifications
	this->dt = 0.f;
	this->curTime = 0.f;
	this->lastTime = 0.f;

	// Initialize mouse
	this->lastMouseX = 0.0;
	this->lastMouseY = 0.0;
	this->mouseOffsetX = 0.0;
	this->mouseOffsetY = 0.0;
	this->firstMouse = true;

	// Initilaize our engine system
	this->initGLFW();
	this->initWindow(title, resizable);
	this->initGLAD();
	this->initOpenGLOptions();

	// Initialize components of my engine
	this->initMatrices();
	this->initShaders();
	this->initModels();
	this->initLights();
	this->initUniforms();
	this->initSkyBox();
	this->initIMGUI();
	//this->initGround();
}