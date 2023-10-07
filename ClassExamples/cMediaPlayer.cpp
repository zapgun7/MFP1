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
glm::vec3 g_cameraEye = glm::vec3(0.0, 0.0, 20.0f);
glm::vec3 g_cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_upVector = glm::vec3(0.0f, 1.0f, 0.0f);



cVAOManager* g_pMeshManager = NULL;
std::vector< cMesh* > g_vec_pMeshesToDraw;
cLightManager* g_pTheLights = NULL;
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

	cShaderManager* pShaderThing = new cShaderManager();
	pShaderThing->setBasePath("assets/shaders");

	cShaderManager::cShader vertexShader;
	vertexShader.fileName = "vertexShader01.glsl";

	cShaderManager::cShader fragmentShader;
	fragmentShader.fileName = "fragmentShader01.glsl";

	if (!pShaderThing->createProgramFromFile("shader01", vertexShader, fragmentShader))
	{
		std::cout << "Error: Couldn't compile or link:" << std::endl;
		std::cout << pShaderThing->getLastError();
		return -1;
	}
	GLuint shaderProgramID = pShaderThing->getIDFromFriendlyName("shader01");

	::g_pMeshManager = new cVAOManager();

	::g_pMeshManager->setBasePath("assets/models");


	sModelDrawInfo polyDrawingInfo;
	::g_pMeshManager->LoadModelIntoVAO("somepoly.ply",
		polyDrawingInfo, shaderProgramID);
	std::cout << "Loaded: " << polyDrawingInfo.numberOfVertices << " vertices" << std::endl;



	// 
	LoadModels();


	::g_pTheLights = new cLightManager();
	// 
	::g_pTheLights->SetUniformLocations(shaderProgramID);

	::g_pTheLights->theLights[0].param2.x = 1.0f;   // Turn on
	::g_pTheLights->theLights[0].param1.x = 1.0f;   // 0 = point light
	::g_pTheLights->theLights[0].param1.y = 1.0f;
	::g_pTheLights->theLights[0].param1.z = 120.0f;

	::g_pTheLights->theLights[0].position.x = 8.0f;
	::g_pTheLights->theLights[0].position.y = 5.0f;
	::g_pTheLights->theLights[0].position.z = 5.0f;
	::g_pTheLights->theLights[0].direction.w = 1.0f; // Light power

	::g_pTheLights->theLights[0].diffuse.x = 0.01f;
	::g_pTheLights->theLights[0].diffuse.y = 0.0f;
	::g_pTheLights->theLights[0].diffuse.z = 0.0f;
	::g_pTheLights->theLights[0].specular.x = 1.0f;
	::g_pTheLights->theLights[0].specular.y = 1.0f;
	::g_pTheLights->theLights[0].specular.z = 1.0f;

	::g_pTheLights->theLights[0].atten.x = 0.0f;        // Constant attenuation
	::g_pTheLights->theLights[0].atten.y = 0.05f;        // Linear attenuation
	::g_pTheLights->theLights[0].atten.z = 0.9f;        // Quadratic attenuation


	::g_pTheLights->theLights[1].param2.x = 1.0f;   // Turn on
	::g_pTheLights->theLights[1].param1.x = 1.0f;   // 0 = point light
	::g_pTheLights->theLights[1].param1.y = 1.0f;
	::g_pTheLights->theLights[1].param1.z = 20.0f;

	::g_pTheLights->theLights[1].position.x = -8.0f;
	::g_pTheLights->theLights[1].position.y = 5.0f;
	::g_pTheLights->theLights[1].position.z = 0.0f;
	::g_pTheLights->theLights[1].direction.w = 1.0f; // Light power

	::g_pTheLights->theLights[1].diffuse.x = 0.01f;
	::g_pTheLights->theLights[1].diffuse.y = 0.0f;
	::g_pTheLights->theLights[1].diffuse.z = 3.0f;
	::g_pTheLights->theLights[1].specular.x = 1.0f;
	::g_pTheLights->theLights[1].specular.y = 0.0f;
	::g_pTheLights->theLights[1].specular.z = 1.0f;

	::g_pTheLights->theLights[1].atten.x = 0.0f;        // Constant attenuation
	::g_pTheLights->theLights[1].atten.y = 0.05f;        // Linear attenuation
	::g_pTheLights->theLights[1].atten.z = 0.9f;        // Quadratic attenuation

	::g_pTheLights->theLights[2].param2.x = 1.0f;   // Turn on
	::g_pTheLights->theLights[2].param1.x = 1.0f;   // 0 = point light
	::g_pTheLights->theLights[2].param1.y = 1.0f;
	::g_pTheLights->theLights[2].param1.z = 20.0f;

	::g_pTheLights->theLights[2].position.x = -8.0f;
	::g_pTheLights->theLights[2].position.y = 5.0f;
	::g_pTheLights->theLights[2].position.z = 0.0f;
	::g_pTheLights->theLights[2].direction.w = 1.0f; // Light power

	::g_pTheLights->theLights[2].diffuse.x = 0.005f;
	::g_pTheLights->theLights[2].diffuse.y = 1.5f;
	::g_pTheLights->theLights[2].diffuse.z = 1.0f;
	::g_pTheLights->theLights[2].specular.x = 1.0f;
	::g_pTheLights->theLights[2].specular.y = 0.0f;
	::g_pTheLights->theLights[2].specular.z = 1.0f;

	::g_pTheLights->theLights[2].atten.x = 0.0f;        // Constant attenuation
	::g_pTheLights->theLights[2].atten.y = 0.05f;        // Linear attenuation
	::g_pTheLights->theLights[2].atten.z = 0.9f;        // Quadratic attenuation


	//    glm::vec3 cameraEye = glm::vec3(10.0, 5.0, -15.0f);
	float yaxisRotation = 0.0f;

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
	bool show_demo_window = false;
	bool show_credits_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		//glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();




		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);






		// 2. The main window we control our audio from
		{
			//static float f = 0.0f;
			//static int counter = 0;

			ImGui::Begin("Audio Player");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("Currently Playing: %s", currAud.c_str());               // Display some text (you can use a format strings too)
			ImGui::Separator();

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
				if (ImGui::Button(friendlyNames[i * 5].c_str()))
				{
					m_Channel = soundMangr->PlaySound(friendlyNames[i * 5]); // Play sound
					currAud = friendlyNames[i * 5]; // Update text displaying currently playing audio
					soundMangr->setPausePlay(isPaused); // Immediately set the pause/play state, so you can start audio paused
				}
				for (unsigned int e = 0; e < 4; e++)
				{
					ImGui::SameLine();
					if (ImGui::Button(friendlyNames[i * 5 + e + 1].c_str()))
					{
						m_Channel = soundMangr->PlaySound(friendlyNames[i * 5 + e + 1]);
						currAud = friendlyNames[i * 5 + e + 1];
						soundMangr->setPausePlay(isPaused);
					}
				}
			}


			ImGui::Text("\n\n");
			ImGui::SliderFloat("Volume", &volume, -2.0f, 2.0f);

			ImGui::SliderFloat("Pitch", &pitch, -10.0f, 10.0f);

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



			// 			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			// 			ImGui::Checkbox("Another Window", &show_another_window);
			// 
			// 			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			// 			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
			// 
			// 			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			// 				counter++;
			// 			ImGui::SameLine();
			// 			ImGui::Text("counter = %d", counter);
			// 
			// 			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}




		// 3. The credits window. We can open this from the main window created above
		if (show_credits_window)
		{
			ImGui::Begin("Audio Credits", &show_credits_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			for (int i = 0; i < credits.size()/2; i++)
			{
				ImGui::Text(credits[i * 2].c_str());
				ImGui::Text("   %s",credits[i * 2 + 1].c_str());
				ImGui::Separator();
			}
			if (ImGui::Button("Close"))
				show_credits_window = false;
			ImGui::End();
		}


		std::cout << rotatePower << std::endl;



		// Rendering
		ImGui::Render();
// 		int display_w, display_h;
// 		glfwGetFramebufferSize(window, &display_w, &display_h);
// 		glViewport(0, 0, display_w, display_h);
// 		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
// 		glClear(GL_COLOR_BUFFER_BIT);
		// /////////////////////////////////////////////////////////////////////////////////////////////////////////
		// ////////////////////////GRAPHICS STUFF//////////////////////////////////////////////////////////////////////
		// /////////////////////////////////////////////////////////////////////////////////////////////////////////
		float ratio;
		int width, height;

		glUseProgram(shaderProgramID);

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// While drawing a pixel, see if the pixel that's already there is closer or not?
		glEnable(GL_DEPTH_TEST);
		// (Usually) the default - does NOT draw "back facing" triangles
		glCullFace(GL_BACK);
		

		::g_pTheLights->UpdateUniformValues(shaderProgramID);

		//uniform vec4 eyeLocation;
		GLint eyeLocation_UL = glGetUniformLocation(shaderProgramID, "eyeLocation");
		glUniform4f(eyeLocation_UL,
			::g_cameraEye.x, ::g_cameraEye.y, ::g_cameraEye.z, 1.0f);



		//       //mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
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

		// Time per frame (more or less)
		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastTime;
		//        std::cout << deltaTime << std::endl;
		lastTime = currentTime;


		// *********************************************************************
		// Draw all the objects
		updateScene(soundMangr->getIsPlaying(), isPaused);
		::g_pTheLights->UpdateUniformValues(shaderProgramID);
		for (unsigned int index = 0; index != ::g_vec_pMeshesToDraw.size(); index++)
		{
			cMesh* pCurrentMesh = ::g_vec_pMeshesToDraw[index];

			if (pCurrentMesh->bIsVisible)
			{

				glm::mat4 matModel = glm::mat4(1.0f);   // Identity matrix

				DrawObject(pCurrentMesh, matModel, shaderProgramID, deltaTime, rotatePower);
			}//if (pCurrentMesh->bIsVisible)

		}
		// *********************************************************************





		//glfwSwapBuffers(window);
		glfwPollEvents();
		// //////////////////////////////////////////////////////////////////////////////////////
		// //////////////////////////////////////////////////////////////////////////////////////
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		soundMangr->Update(volume, pitch, pan, isLooping); // gotta call it, important or something?
		glfwSwapBuffers(window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	//soundMangr->Destroy(); destroys it in the main?



	return true;
}




// GRAPHICS FUNCITONS /////////////////////////////
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
void DrawObject(cMesh* pCurrentMesh, glm::mat4 matModelParent, GLuint shaderProgramID, double deltaTime, float rotatePower)
{

	//         mat4x4_identity(m);
	glm::mat4 matModel = matModelParent;


	pCurrentMesh->drawPosition.y = sin(glfwGetTime()); // constant bob


	// Translation
	glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f),
		glm::vec3(pCurrentMesh->drawPosition.x,
			pCurrentMesh->drawPosition.y ,
			pCurrentMesh->drawPosition.z));


	// Rotation matrix generation
	glm::mat4 matRotateX = glm::rotate(glm::mat4(1.0f),
		pCurrentMesh->orientation.x, // (float)glfwGetTime(),
		glm::vec3(1.0f, 0.0, 0.0f));

	pCurrentMesh->orientation.y += deltaTime/(3 - 3*(rotatePower/11)); // not constant >:(

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

	//        m = m * rotateZ;
	//        m = m * rotateY;
	//        m = m * rotateZ;



	   //mat4x4_mul(mvp, p, m);
	//    glm::mat4 mvp = matProjection * matView * matModel;

	//    GLint mvp_location = glGetUniformLocation(shaderProgramID, "MVP");
	//    //glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
	//    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));

	GLint matModel_UL = glGetUniformLocation(shaderProgramID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(matModel));


	// Also calculate and pass the "inverse transpose" for the model matrix
	glm::mat4 matModel_InverseTranspose = glm::inverse(glm::transpose(matModel));

	// uniform mat4 matModel_IT;
	GLint matModel_IT_UL = glGetUniformLocation(shaderProgramID, "matModel_IT");
	glUniformMatrix4fv(matModel_IT_UL, 1, GL_FALSE, glm::value_ptr(matModel_InverseTranspose));

	glm::vec4 trucol;
	if (pCurrentMesh->bIsWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(5);
		trucol = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		trucol = glm::vec4(255.0f, 1.0f, 1.0f, 1.0f);
	}

	// uniform mat4 matModel_IT;
	GLint trueColor_UL = glGetUniformLocation(shaderProgramID, "trueColor");
	glUniform4f(trueColor_UL,
		trucol.x, trucol.y, trucol.z, 1.0f);

	//        glPointSize(10.0f);


			// uniform bool bDoNotLight;
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

	//uniform bool bUseDebugColour;	
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
void cMediaPlayer::updateScene(bool isPlaying, bool isPaused)
{
	float rotateRampSpeed = 0.03f; // Might want to look into easing functions
	float lightRampSpeed = 0.03f;

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

	int rotateSpeed = 2;
	int amplitude = 8;

	::g_pTheLights->theLights[0].position.x = amplitude * sin(sin(3 * sin(glfwGetTime() / rotateSpeed)));
	::g_pTheLights->theLights[0].position.z = amplitude * sin(sin(3 * sin(glfwGetTime() / rotateSpeed + 1.57f)));
	::g_pTheLights->theLights[0].position.y = amplitude * sin(2 * cos(4 * sin(glfwGetTime() / (rotateSpeed * 5))));

	::g_pTheLights->theLights[1].position.x = -amplitude * cos(3 * cos(2 * cos(glfwGetTime() / rotateSpeed + 1.57f)));
	::g_pTheLights->theLights[1].position.z = -amplitude * cos(3 * cos(2 * cos(glfwGetTime() / rotateSpeed)));
	::g_pTheLights->theLights[1].position.y = -amplitude * sin(2 * cos(4 * sin(glfwGetTime() / (rotateSpeed * 5) + 1.57f)));

	::g_pTheLights->theLights[2].position.x = -amplitude * cos(3 * sin(cos(glfwGetTime() / (rotateSpeed * 3) + 1.57f)));
	::g_pTheLights->theLights[2].position.z = amplitude * cos(3 * sin(cos(glfwGetTime() / (rotateSpeed * 3))));
	::g_pTheLights->theLights[2].position.y = -amplitude * tan(0.7 * tan(cos(glfwGetTime() / (rotateSpeed * 5))));


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
