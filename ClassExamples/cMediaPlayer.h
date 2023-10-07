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
	float rotatePower; // Graphics var
	void updateScene(bool isPlaying, bool isPaused);

	float volume;
	float pitch;
	float pan;
	bool isPaused;
	bool isLooping;
	std::string currAud;
	std::vector<std::string> friendlyNames;
	std::vector<std::string> credits;
	cSoundManager* soundMangr;
	FMOD::Channel* m_Channel;
};

