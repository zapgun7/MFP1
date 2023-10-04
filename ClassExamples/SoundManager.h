#pragma once

#include <map>
#include<string>

#include <fmod.hpp>
#include "SoundUtils.h"

// This can be a singleton, but we are not focusing on 
// design patterns at the moment.
class SoundManager
{
public:
	// Constructor
	SoundManager();

	// Destructor (Gets called on delete)
	~SoundManager();

	bool Initialize(std::string loadFile);
	void Destroy();

	void PlaySound(FMOD::Sound* sound);

	void Update();

	// Setters
	void setPitch(float newPitch);
	void setVolume(float newVolume);
	void setPan(float newPan);
	void setLoop(int loopState);

	// Getters
	float getPitch();
	float getVolume();
	float getPan();
	int getLoop();

	FMOD::Sound* FindSoundBySoundName(std::string);

private:
	bool m_Initialized = false;
	bool loadSoundsFromFile(std::string filename); // Initializes the map from a given file

	float currentPan;

	FMOD::System* m_System = nullptr;
	FMOD::Sound* m_Sound = nullptr;
	FMOD::Channel* m_Channel = nullptr;
	FMOD_RESULT m_Result;
	std::map<std::string, FMOD::Sound*> m_map_friendlyName_to_sound; // Map to store all loaded sounds
};