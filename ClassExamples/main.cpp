#include "cSoundManager.h"
#include <iostream>
#include <conio.h>
#include "cMediaPlayer.h"

cSoundManager *g_SoundManager;
cMediaPlayer *g_MediaPlayer;

// void PlaySound(const char* soundfile)
// {
// 	g_SoundManager.LoadSound(soundfile);
// 	g_SoundManager.PlaySound();
// 
// 	printf("Sound playing, press ESC to quit . . .");
// 	while (true)
// 	{
// 		g_SoundManager.Update();
// 
// 		if (_kbhit())
// 		{
// 			int key = _getch();
// 			if (key == 27/*ESCAPE*/)
// 			{
// 				break;
// 			}
// 		}
// 	}
// }


// Main entry point to our program
// argc is the number of arguments
// argv is the arguments
// 
// Examples of input:
// Example01.exe (1, "")
// Example01.exe with some parameters (4, {"Example01", "with", "some", "Parameters"})

int main(int argc, char** argv)
{
	g_SoundManager = new cSoundManager();
	g_SoundManager->Initialize("audio/aud_files.txt");
// 	g_SoundManager.PlaySound("churchbell");
// 	g_SoundManager.setPitch(3.0f);
// 	PlaySound("audio/jaguar.wav");
// 	PlaySound("audio/singing.wav");
// 	PlaySound("audio/swish.wav");
// 	while (true)
// 	{
// 		g_SoundManager.Update();
// 		
// 		if (_kbhit())
// 		{
// 			int key = _getch();
// 			if (key == 27/*ESCAPE*/)
// 			{
// 				break;
// 			}
// 		}
// 	}
// 	g_SoundManager.PlaySound(g_SoundManager.FindSoundBySoundName("fire"));
// 	while (true)
// 	{
// 		g_SoundManager.Update();
// 
// 		if (_kbhit())
// 		{
// 			int key = _getch();
// 			if (key == 27/*ESCAPE*/)
// 			{
// 				break;
// 			}
// 		}
// 	}
	g_MediaPlayer = new cMediaPlayer(g_SoundManager);


	g_MediaPlayer->startProgram();
	g_SoundManager->Destroy();
	delete g_SoundManager;
	delete g_MediaPlayer;
	// 0 means successful, anything else is typically an "error"
	return 0;
}