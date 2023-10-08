#include "cSoundManager.h"
#include <iostream>
#include <conio.h>
#include "cMediaPlayer.h"

cSoundManager *g_SoundManager;// Will control and manage the audio
cMediaPlayer *g_MediaPlayer; // Will control user experience (input and visuals)



int main(int argc, char** argv)
{
	g_SoundManager = new cSoundManager();
	if (!g_SoundManager->Initialize("audio/aud_files.txt"))
	{
		std::cout << "Could not initialize the sound manager" << std::endl;
		return 1;
	 }

	g_MediaPlayer = new cMediaPlayer(g_SoundManager);

	if (!g_MediaPlayer->startProgram()) // Calls the main loop of this program
	{
		std::cout << "Could not start the cMediaPlayer program" << std::endl;
	}
	g_SoundManager->Destroy(); 
	delete g_SoundManager;
	delete g_MediaPlayer;
	// 0 means successful, anything else is typically an "error"
	return 0;
}