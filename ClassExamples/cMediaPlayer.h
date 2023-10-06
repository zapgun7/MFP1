#pragma once

#include <string>
#include <vector>

#include "cSoundManager.h"

class cMediaPlayer
{
public:
	cMediaPlayer(cSoundManager* soundMan);
	~cMediaPlayer();
	bool startProgram(); // Starts the GUI; main loop is located in here

private:
	float volume;
	float pitch;
	float pan;
	bool isPaused;
	bool isLooping;
	std::string currAud;
	std::vector<std::string> friendlyNames;
	cSoundManager* soundMangr;
	FMOD::Channel* m_Channel;
};

