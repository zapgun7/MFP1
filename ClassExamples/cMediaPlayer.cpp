#include "cMediaPlayer.h"
#include "cSoundManager.h"


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#ifdef __APPLE__
//#define GL_SILENCE_DEPRECATION
#endif
#include <glad.h>
#define GLFW_INCLUDE_NONE
#include <glfw3.h>


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
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();




		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);






		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
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




		// 3. Show another simple window.
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







		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
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
