#pragma once

#include <map>
#include<string>
#include <vector>

#include <fmod/include/fmod.hpp>
#include "SoundUtils.h"

// This can be a singleton, but we are not focusing on 
// design patterns at the moment.
class cSoundManager
{
public:
	// Constructor
	cSoundManager();

	// Destructor (Gets called on delete)
	~cSoundManager();

	bool Initialize(std::string loadFile);
	void Destroy();

	/*void PlaySound(FMOD::Sound* sound);*/
	FMOD::Channel* PlaySound(std::string friendlyName);

	void Update(float vol, float pit, float pan, bool isLooping);

	// Setters
	void setPitch(float newPitch);
	void setVolume(float newVolume);
	void setPan(float newPan);
	void setLoop(int loopState);
	void setPausePlay(bool isPaused);

	// Getters
	float getPitch();
	float getVolume();
	float getPan();
	int getLoop();
	bool getIsPlaying(); // Returns if channel is actively playing something
	std::vector<std::string> getFriendlyNames();
	std::vector<std::string> getCredits();

	FMOD::Sound* FindSoundBySoundName(std::string);

private:
	bool m_Initialized = false;
	bool loadSoundsFromFile(std::string filename); // Initializes the map from a given file

	FMOD::Channel* PlaySound(FMOD::Sound* sound); // 

	float currentPan;

	FMOD::System* m_System = nullptr;
	FMOD::Sound* m_Sound = nullptr;
	FMOD::Channel* m_Channel = nullptr;
	FMOD_RESULT m_Result;
	std::vector<std::string> friendlyNames;
	std::vector<std::string> credits;
	std::map<std::string, FMOD::Sound*> m_map_friendlyName_to_sound; // Map to store all loaded sounds
};