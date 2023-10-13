#include "cMediaPlayer.h"
#include "cSoundManager.h"

// GRAPHICS INCLUDES
#include "OpenGLCommon.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> 
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>

#include "Basic Shader Manager/cShaderManager.h"
#include "cVAOManager/cVAOManager.h"

//#include "GLWF_CallBacks.h" // keyboard and mouse input

#include "cMesh.h"

#include "cLightManager.h"
//#include "cLightHelper.h"
// ///////////////////

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <stdio.h>
#ifdef __APPLE__
//#define GL_SILENCE_DEPRECATION
#endif
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/include/glfw3.h>

// GRAPHICS FUNCTION SIGNATURE //////////////////////////////////
cMesh* g_pFindMeshByFriendlyName(std::string friendlyNameToFind); 
bool LoadModels(void);
void DrawObject(cMesh* pCurrentMesh, glm::mat4 matModel, GLuint shaderProgramID, double deltaTime ,float rotatePower);
//////////////////////////////////////////////////////////////
// GRAPHICS SETUP
//////////////////////////////////////////////////////////////

// Simple vec3's to maintain the POV in the window
glm::vec3 g_cameraEye = glm::vec3(0.0, 0.0, 20.0f);
glm::vec3 g_cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_upVector = glm::vec3(0.0f, 1.0f, 0.0f);



cVAOManager* g_pMeshManager = NULL; // Manages the info from the 3D object files, taking care of populating the vertex and index arrays
std::vector< cMesh* > g_vec_pMeshesToDraw; // Manages drawing info about each object (position, orientation, scale)
cLightManager* g_pTheLights = NULL; // Manages info on the 3 spotlights used in the scene
/////////////////////


cMediaPlayer::cMediaPlayer(cSoundManager* soundMan)
{
	volume = 1.0f;
	pitch = 1.0f;
	pan = 0.0f;
	isPaused = false;
	isLooping = false;
	currAud = " ";
	friendlyNames = soundMan->getFriendlyNames();
	credits = soundMan->getCredits();
	soundMangr = soundMan;
	rotatePower = 1;
}

cMediaPlayer::~cMediaPlayer()
{

}

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}




bool cMediaPlayer::startProgram()
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char* glsl_version = "#version 100";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

	GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
	if (window == nullptr)
		return 1;
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1); // Enable vsync

	// /////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ///////////////////// GRAPHICS STUFF/////////////////////////////////////////////////////////
	// /////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Create the shader manager: takes care of loading from the shader files
	// Also handles creating the shader programs to use in the scene
	cShaderManager* pShaderThing = new cShaderManager();
	pShaderThing->setBasePath("assets/shaders");

	// Create variable representing vertex shader, and give it info on its file location
	cShaderManager::cShader vertexShader;
	vertexShader.fileName = "vertexShader01.glsl";

	// Same as above but fragment shader
	cShaderManager::cShader fragmentShader;
	fragmentShader.fileName = "fragmentShader01.glsl";

	// Create the shader program from the two shader variables we just created
	if (!pShaderThing->createProgramFromFile("shader01", vertexShader, fragmentShader))
	{
		std::cout << "Error: Couldn't compile or link:" << std::endl;
		std::cout << pShaderThing->getLastError();
		return -1;
	}
	// Get unique shader identifier to use the program
	GLuint shaderProgramID = pShaderThing->getIDFromFriendlyName("shader01");

	// Initialize the class that handles the raw info on the models; point it to the models file path
	::g_pMeshManager = new cVAOManager();
	::g_pMeshManager->setBasePath("assets/models");

	// Load in the only object we'll use for the scene
	sModelDrawInfo polyDrawingInfo;
	::g_pMeshManager->LoadModelIntoVAO("somepoly.ply",
		polyDrawingInfo, shaderProgramID);
	std::cout << "Loaded: " << polyDrawingInfo.numberOfVertices << " vertices" << std::endl;



	// Create our "drawable" objects; will have a friendly name that matches the object loaded just above to reerence the proper set of vertices
	// This initializes all of the draw info of our to-be-rendered objects: scale, position, orientation, etc.
	LoadModels();



	// Initialize our light manager and explicitly set the parameters for each one
	::g_pTheLights = new cLightManager();
	// 
	::g_pTheLights->SetUniformLocations(shaderProgramID);
	//------------------------SPOTLIGHT 1------------------------//
	::g_pTheLights->theLights[0].param2.x = 1.0f;   // Turn on
	::g_pTheLights->theLights[0].param1.x = 1.0f;   // 0 = point light
	::g_pTheLights->theLights[0].param1.y = 1.0f;
	::g_pTheLights->theLights[0].param1.z = 120.0f;

	::g_pTheLights->theLights[0].direction.w = 1.0f; // Light power

	::g_pTheLights->theLights[0].diffuse.x = 0.01f;
	::g_pTheLights->theLights[0].diffuse.y = 0.0f;     // Red light (don't need big value since the poly is already red)
	::g_pTheLights->theLights[0].diffuse.z = 0.0f;
	::g_pTheLights->theLights[0].specular.x = 1.0f;
	::g_pTheLights->theLights[0].specular.y = 1.0f;
	::g_pTheLights->theLights[0].specular.z = 1.0f;

	::g_pTheLights->theLights[0].atten.x = 0.0f;        // Constant attenuation
	::g_pTheLights->theLights[0].atten.y = 0.05f;        // Linear attenuation
	::g_pTheLights->theLights[0].atten.z = 0.9f;        // Quadratic attenuation

	//------------------------SPOTLIGHT 2------------------------//
	::g_pTheLights->theLights[1].param2.x = 1.0f;   // Turn on
	::g_pTheLights->theLights[1].param1.x = 1.0f;   // 0 = point light
	::g_pTheLights->theLights[1].param1.y = 1.0f;
	::g_pTheLights->theLights[1].param1.z = 20.0f;


	::g_pTheLights->theLights[1].direction.w = 1.0f; // Light power

	::g_pTheLights->theLights[1].diffuse.x = 0.01f;
	::g_pTheLights->theLights[1].diffuse.y = 0.0f;   // Bring more blue through
	::g_pTheLights->theLights[1].diffuse.z = 3.0f;
	::g_pTheLights->theLights[1].specular.x = 1.0f;
	::g_pTheLights->theLights[1].specular.y = 0.0f;
	::g_pTheLights->theLights[1].specular.z = 1.0f;

	::g_pTheLights->theLights[1].atten.x = 0.0f;        // Constant attenuation
	::g_pTheLights->theLights[1].atten.y = 0.05f;        // Linear attenuation
	::g_pTheLights->theLights[1].atten.z = 0.9f;        // Quadratic attenuation


	//------------------------SPOTLIGHT 3------------------------//
	::g_pTheLights->theLights[2].param2.x = 1.0f;   // Turn on
	::g_pTheLights->theLights[2].param1.x = 1.0f;   // 0 = point light
	::g_pTheLights->theLights[2].param1.y = 1.0f;
	::g_pTheLights->theLights[2].param1.z = 20.0f;


	::g_pTheLights->theLights[2].direction.w = 1.0f; // Light power

	::g_pTheLights->theLights[2].diffuse.x = 0.005f;
	::g_pTheLights->theLights[2].diffuse.y = 1.5f;   // Bring more green and blue through
	::g_pTheLights->theLights[2].diffuse.z = 1.0f;
	::g_pTheLights->theLights[2].specular.x = 0.0f;
	::g_pTheLights->theLights[2].specular.y = 1.0f;
	::g_pTheLights->theLights[2].specular.z = 1.0f;

	::g_pTheLights->theLights[2].atten.x = 0.0f;        // Constant attenuation
	::g_pTheLights->theLights[2].atten.y = 0.05f;        // Linear attenuation
	::g_pTheLights->theLights[2].atten.z = 0.9f;        // Quadratic attenuation


	double lastTime = glfwGetTime();
	
	// //////////////////////////////////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////////////////////////////////

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Our state
	bool show_credits_window = false;

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();



		std::vector<int> audProgLen;
		// The main window we control our audio from
		{
			ImGui::Begin("Audio Player");                         

			ImGui::Text("Currently Playing: %s", currAud.c_str());               
			ImGui::Separator();

			audProgLen = soundMangr->getAudioProgress();        // audProgLen[0] = Total audio length in ms     audProgLen[1] = Current position in audio in ms
			if (audProgLen[0] == 0)
			{
				ImGui::ProgressBar(0.0f, ImVec2(ImGui::GetWindowWidth() - 100, 0));        // Have progress bar end 100px from edge of screen; allows space for timestamp
			}
			else
			{
				ImGui::ProgressBar((float)audProgLen[1] / audProgLen[0], ImVec2(ImGui::GetWindowWidth() - 100, 0));
			}
			ImGui::SameLine();
			ImGui::Text("%02d:%02d/%02d:%02d", audProgLen[1] / 60000,           // Divide by minutes in ms to get minutes (rounded down)
											   audProgLen[1] % 60000 / 1000,    // mod by minute value to get remainder seconds in ms; then divide by 1000ms (1s) to get number of seconds (rounded down)
											   audProgLen[0] / 60000,
											   audProgLen[0] % 60000 / 1000);

			if (isPaused)
			{
				if (ImGui::Button("Play "))
				{
					isPaused = false;
					soundMangr->setPausePlay(isPaused);
				}
			}
			else
			{
				if (ImGui::Button("Pause"))
				{
					isPaused = true;
					soundMangr->setPausePlay(isPaused);
				}
			}
			ImGui::Text("\n\nAudio To Play:");
			for (unsigned int i = 0; i < 2; i++)
			{
				if (ImGui::Button(friendlyNames[i * 5].c_str())) // First audio button of a row (different than below because it doesn't have ImGui::SameLine()
				{
					m_Channel = soundMangr->PlaySound(friendlyNames[i * 5]); // Play sound
					currAud = friendlyNames[i * 5]; // Update text displaying currently playing audio
					isPaused = false; // Instantly play sound when its button is pressed
					soundMangr->setPausePlay(isPaused); // Immediately set the pause/play state, so you can start audio paused
				}
				for (unsigned int e = 0; e < 4; e++)
				{
					ImGui::SameLine();
					if (ImGui::Button(friendlyNames[i * 5 + e + 1].c_str()))
					{
						m_Channel = soundMangr->PlaySound(friendlyNames[i * 5 + e + 1]);
						currAud = friendlyNames[i * 5 + e + 1];
						isPaused = false;
						soundMangr->setPausePlay(isPaused);
					}
				}
			}


			ImGui::Text("\n\n");
			ImGui::SliderFloat("Volume", &volume, -2.0f, 2.0f);

			ImGui::SliderFloat("Pitch", &pitch, 0.0f, 10.0f);

			ImGui::SliderFloat("Pan", &pan, -1.0f, 1.0f);

			ImGui::Text("\n");
			ImGui::Checkbox("Loop", &isLooping);
			ImGui::Text("\n\n\n");


			if (show_credits_window) // Toggle credits window with one button
			{
				if (ImGui::Button("Credits"))
					show_credits_window = false;
			}
			else
			{
				if (ImGui::Button("Credits"))
					show_credits_window = true;
			}
			ImGui::SameLine();
			ImGui::Text("          "); // Spacing between credits and exit button
			ImGui::SameLine();
			if (ImGui::Button("Exit"))
				glfwSetWindowShouldClose(window, 1);


			ImGui::End();
		}


		// The credits window. We can open this from the main window created above
		if (show_credits_window)
		{
			ImGui::Begin("Audio Credits", &show_credits_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			for (int i = 0; i < credits.size()/2; i++)
			{
				ImGui::Text(credits[i * 2].c_str()); // Name of the audio
				ImGui::Text("   %s",credits[i * 2 + 1].c_str()); // Credits of the audio
				ImGui::Separator();
			}
			if (ImGui::Button("Close"))
				show_credits_window = false;
			ImGui::End();
		}

		ImGui::Render();


		// /////////////////////////////////////////////////////////////////////////////////////////////////////////
		// ////////////////////////MORE GRAPHICS STUFF//////////////////////////////////////////////////////////////////////
		// /////////////////////////////////////////////////////////////////////////////////////////////////////////
		float ratio;
		int width, height;

		// Set the shader program we're using
		glUseProgram(shaderProgramID);

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear screen before drawing

		// While drawing a pixel, see if the pixel that's already there is closer or not?
		glEnable(GL_DEPTH_TEST);
		// (Usually) the default - does NOT draw "back facing" triangles
		glCullFace(GL_BACK);
		

		// Setting uniform variables for eye location and perspective matrices ///////////////////////////
		GLint eyeLocation_UL = glGetUniformLocation(shaderProgramID, "eyeLocation");
		glUniform4f(eyeLocation_UL,
			::g_cameraEye.x, ::g_cameraEye.y, ::g_cameraEye.z, 1.0f);

		glm::mat4 matProjection = glm::perspective(0.6f,
			ratio,
			0.1f,
			1000.0f);

		glm::mat4 matView = glm::lookAt(::g_cameraEye,
			::g_cameraTarget,
			::g_upVector);

		GLint matProjection_UL = glGetUniformLocation(shaderProgramID, "matProjection");
		glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, glm::value_ptr(matProjection));

		GLint matView_UL = glGetUniformLocation(shaderProgramID, "matView");
		glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(matView));
		// ////////////////////////////////////////////////////////////////////////////////////////



		// Time per frame (more or less)
		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastTime;
		lastTime = currentTime;


		// Updates most things in the scene that change
		updateScene(soundMangr->getIsPlaying(), isPaused); 

		// Update the uniform values of our lights (effectively just for changing position, direction, and power)
		::g_pTheLights->UpdateUniformValues(shaderProgramID);

		// Draw all objects in the scene (just poly and poly mesh)
		for (unsigned int index = 0; index != ::g_vec_pMeshesToDraw.size(); index++)
		{
			cMesh* pCurrentMesh = ::g_vec_pMeshesToDraw[index];

			if (pCurrentMesh->bIsVisible)
			{

				glm::mat4 matModel = glm::mat4(1.0f);   // Identity matrix

				DrawObject(pCurrentMesh, matModel, shaderProgramID, deltaTime, rotatePower);
			}

		}

		glfwPollEvents();
		// //////////////////////////////////////////////////////////////////////////////////////
		// //////////////////////////////////////////////////////////////////////////////////////


		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		soundMangr->Update(volume, pitch, pan, isLooping); // gotta call it, important or something?
		glfwSwapBuffers(window);

	} // while (!glfwWindowShouldClose(window))

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	//soundMangr->Destroy(); destroys it in the main



	return true;
}




// GRAPHICS FUNCITONS /////////////////////////////

// Iterates through the mesh map to return the draw info; uses a friendlyname
cMesh* g_pFindMeshByFriendlyName(std::string friendlyNameToFind)
{
	for (unsigned int index = 0; index != ::g_vec_pMeshesToDraw.size(); index++)
	{
		if (::g_vec_pMeshesToDraw[index]->friendlyName == friendlyNameToFind)
		{
			// Found it
			return ::g_vec_pMeshesToDraw[index];
		}
	}
	// Didn't find it
	return NULL;
}

// Draws the object passed with pCurrentMesh and the passed shader program; deltaTime and rotatePower are for rotational speed of the poly
void DrawObject(cMesh* pCurrentMesh, glm::mat4 matModelParent, GLuint shaderProgramID, double deltaTime, float rotatePower)
{
	glm::mat4 matModel = matModelParent;

	// Gives the poly a constant up/down smooth bob
	pCurrentMesh->drawPosition.y = sin(glfwGetTime()); 


	// Translation
	glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f),
		glm::vec3(pCurrentMesh->drawPosition.x,
			pCurrentMesh->drawPosition.y ,
			pCurrentMesh->drawPosition.z));

	// Rotation matrix generation
	glm::mat4 matRotateX = glm::rotate(glm::mat4(1.0f),
		pCurrentMesh->orientation.x, // (float)glfwGetTime(),
		glm::vec3(1.0f, 0.0, 0.0f));

	// Add to the y-rotation of the poly based off of passsed variables
	pCurrentMesh->orientation.y += deltaTime/(3 - 3*(rotatePower/11)); 

	glm::mat4 matRotateY = glm::rotate(glm::mat4(1.0f),
		pCurrentMesh->orientation.y, // (float)glfwGetTime(),
		glm::vec3(0.0f, 1.0, 0.0f));

	glm::mat4 matRotateZ = glm::rotate(glm::mat4(1.0f),
		pCurrentMesh->orientation.z, // (float)glfwGetTime(),
		glm::vec3(0.0f, 0.0, 1.0f));


	// Scaling matrix
	glm::mat4 matScale = glm::scale(glm::mat4(1.0f),
		glm::vec3(pCurrentMesh->scale,
			pCurrentMesh->scale,
			pCurrentMesh->scale));
	//--------------------------------------------------------------

	// Combine all these transformation
	matModel = matModel * matTranslate;

	matModel = matModel * matRotateX;
	matModel = matModel * matRotateY;
	matModel = matModel * matRotateZ;

	matModel = matModel * matScale;


	//////////// Update Matrix uniform values in the vertex shader //////////////
	GLint matModel_UL = glGetUniformLocation(shaderProgramID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(matModel));


	// Also calculate and pass the "inverse transpose" for the model matrix
	glm::mat4 matModel_InverseTranspose = glm::inverse(glm::transpose(matModel));

	// uniform mat4 matModel_IT;
	GLint matModel_IT_UL = glGetUniformLocation(shaderProgramID, "matModel_IT");
	glUniformMatrix4fv(matModel_IT_UL, 1, GL_FALSE, glm::value_ptr(matModel_InverseTranspose));
	// /////////////////////////////////////////////////////////////////////////


	glm::vec4 trucol;
	if (pCurrentMesh->bIsWireframe) // if is the wireframe poly
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode
		glLineWidth(5); // Thicker line looks nice
		trucol = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // Set color to almost black; keep values at some non-zero to be influenced by light
	}
	else // If the full poly object
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Fill mode
		trucol = glm::vec4(255.0f, 1.0f, 1.0f, 1.0f); // Effectively set color to red; keep other values non-zero to be influenced by blue/green light
	}

	// Pass in hard-set color to vertex shader
	GLint trueColor_UL = glGetUniformLocation(shaderProgramID, "trueColor");
	glUniform4f(trueColor_UL,
		trucol.x, trucol.y, trucol.z, 1.0f);

	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgramID, "bDoNotLight");

	if (pCurrentMesh->bDoNotLight)
	{
		// Set uniform to true
		glUniform1f(bDoNotLight_UL, (GLfloat)GL_TRUE);
	}
	else
	{
		// Set uniform to false;
		glUniform1f(bDoNotLight_UL, (GLfloat)GL_FALSE);
	}

	//Setting debug colors
	GLint bUseDebugColour_UL = glGetUniformLocation(shaderProgramID, "bUseDebugColour");
	if (pCurrentMesh->bUseDebugColours)
	{
		glUniform1f(bUseDebugColour_UL, (GLfloat)GL_TRUE);
		//uniform vec4 debugColourRGBA;
		GLint debugColourRGBA_UL = glGetUniformLocation(shaderProgramID, "debugColourRGBA");
		glUniform4f(debugColourRGBA_UL,
			pCurrentMesh->wholeObjectDebugColourRGBA.r,
			pCurrentMesh->wholeObjectDebugColourRGBA.g,
			pCurrentMesh->wholeObjectDebugColourRGBA.b,
			pCurrentMesh->wholeObjectDebugColourRGBA.a);
	}
	else
	{
		glUniform1f(bUseDebugColour_UL, (GLfloat)GL_FALSE);
	}


	// Create a variable capable of storing the objects' draw info and pass it into the following function to load it with the info
	sModelDrawInfo modelInfo;
	if (::g_pMeshManager->FindDrawInfoByModelName(pCurrentMesh->meshName, modelInfo))
	{
		// Found it!!!

		glBindVertexArray(modelInfo.VAO_ID); 		//  enable VAO (and everything else)
		glDrawElements(GL_TRIANGLES,
			modelInfo.numberOfIndices,
			GL_UNSIGNED_INT,
			0);
		glBindVertexArray(0); 			            // disable VAO (and everything else)

	}

	return;
}


// Changes all the values to "animate" the scene
// Changes spotlight positions, updates their direction to point to the poly
// Adjusts the poly rotate speed and the lights' power based on the pause/play state of the audio
void cMediaPlayer::updateScene(bool isPlaying, bool isPaused)
{
	float rotateRampSpeed = 0.03f; // Might want to look into easing functions
	float lightRampSpeed = 0.03f;

	///////////// If/else to handle rotation speed and light power////////////
	if (isPlaying && !isPaused) // If audio is currently playing
	{
		if (rotatePower < 10) // Ramp up rotational speed
			rotatePower += rotateRampSpeed;
		if (::g_pTheLights->theLights[0].direction.w < 10.0f) // Power up the lights
		{
			for (unsigned int i = 0; i < 3; i++)
			{
				::g_pTheLights->theLights[i].direction.w += lightRampSpeed;
			}
		}
	}
	else // If no audio is playing
	{
		if (rotatePower > 1) // Ramp down rotational speed of poly
			rotatePower -= rotateRampSpeed;
		if (::g_pTheLights->theLights[0].direction.w > 1.0f) // Power down the lightss
		{
			for (unsigned int i = 0; i < 3; i++)
			{
				::g_pTheLights->theLights[i].direction.w -= lightRampSpeed;
			}
		}
	}
	/////////////////////////////////////////////////////////////////////


	int rotateSpeed = 2; // Hard set value to control speed of lights
	int amplitude = 8; // Distance of lights away from poly roughly

	double currTime = glfwGetTime();

	// Nested sin/cos/tan creates distinct paths for the lights to travel on
	::g_pTheLights->theLights[0].position.x = amplitude * sin(sin(3 * sin(currTime / rotateSpeed)));
	::g_pTheLights->theLights[0].position.z = amplitude * sin(sin(3 * sin(currTime / rotateSpeed + 1.57f)));
	::g_pTheLights->theLights[0].position.y = amplitude * sin(2 * cos(4 * sin(currTime / (rotateSpeed * 5))));

	::g_pTheLights->theLights[1].position.x = -amplitude * cos(3 * cos(2 * cos(currTime / rotateSpeed + 1.57f)));
	::g_pTheLights->theLights[1].position.z = -amplitude * cos(3 * cos(2 * cos(currTime / rotateSpeed)));
	::g_pTheLights->theLights[1].position.y = -amplitude * sin(2 * cos(4 * sin(currTime / (rotateSpeed * 5) + 1.57f)));

	::g_pTheLights->theLights[2].position.x = -amplitude * cos(3 * sin(cos(currTime / (rotateSpeed * 3) + 1.57f)));
	::g_pTheLights->theLights[2].position.z = amplitude * cos(3 * sin(cos(currTime / (rotateSpeed * 3))));
	::g_pTheLights->theLights[2].position.y = -amplitude * tan(0.7 * tan(cos(currTime / (rotateSpeed * 5))));



	// Point all the lights to look at the poly
	glm::vec3 polypos = glm::vec3(::g_vec_pMeshesToDraw[0]->drawPosition.x, ::g_vec_pMeshesToDraw[0]->drawPosition.y, ::g_vec_pMeshesToDraw[0]->drawPosition.z);
	::g_pTheLights->theLights[0].direction.x = polypos.x - ::g_pTheLights->theLights[0].position.x;
	::g_pTheLights->theLights[0].direction.y = polypos.y - ::g_pTheLights->theLights[0].position.y; 
	::g_pTheLights->theLights[0].direction.z = polypos.z - ::g_pTheLights->theLights[0].position.z;
	::g_pTheLights->theLights[1].direction.x = polypos.x - ::g_pTheLights->theLights[1].position.x;
	::g_pTheLights->theLights[1].direction.y = polypos.y - ::g_pTheLights->theLights[1].position.y; // Try to put walls, floor, ceiling behind the poly
	::g_pTheLights->theLights[1].direction.z = polypos.z - ::g_pTheLights->theLights[1].position.z; // Make it absorb barely any light, so the "room" reflects the little bit of light scattered by the light sources
	::g_pTheLights->theLights[2].direction.x = polypos.x - ::g_pTheLights->theLights[2].position.x;
	::g_pTheLights->theLights[2].direction.y = polypos.y - ::g_pTheLights->theLights[2].position.y;
	::g_pTheLights->theLights[2].direction.z = polypos.z - ::g_pTheLights->theLights[2].position.z;


	return;
}
