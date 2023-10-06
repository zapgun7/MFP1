#include "cSoundManager.h"

#include <iostream>
#include <fstream>

cSoundManager::cSoundManager()
{
	// Usually do nothing that requires code execution
	// Declare initial values for everything
}

cSoundManager::~cSoundManager()
{
	// Typically just for notifying things that this is being destroyed.
}

bool cSoundManager::Initialize(std::string loadFile)
{
	if (m_Initialized)
		return true;

	FMOD_RESULT result;
	result = FMOD::System_Create(&m_System);
	if (result != FMOD_OK)
	{
		printf("Failed to create the FMOD System!\n");
		return false;
	}

	result = m_System->init(1, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK)
	{
		printf("Failed to initialize the system!\n");
		// Cleanup
		result = m_System->close();
		if (result != FMOD_OK)
		{
			printf("Failed to close system!\n");
		}
		return false;
	}

	// Load sound files into map
	if (!(loadSoundsFromFile(loadFile)))
		return false;

	currentPan = 0.0f; // Initialize pan of channel, cause there's no getPan()

	m_Initialized = true;
	return true;
}

void cSoundManager::Destroy()
{
	if (!m_Initialized)
		return;

	FMOD_RESULT result;


	std::map<std::string, FMOD::Sound*>::iterator it = m_map_friendlyName_to_sound.begin();
	while(it != m_map_friendlyName_to_sound.end()) // Iterates through the map to release all the sounds
	{
		it->second->release(); // Release sound
		delete(it->second); // Delete pointer
		it++;
	}

	result = m_Sound->release();
	FMODCheckError(result);

	result = m_System->close();
	FMODCheckError(result);

	result = m_System->release();
	FMODCheckError(result);

	m_Initialized = false;
}


FMOD::Channel* cSoundManager::PlaySound(FMOD::Sound* sound) // For calling from within this class
{
	if ((!m_Initialized) || (sound == NULL))
		return NULL;

	FMOD_RESULT result;
	result = m_System->playSound(sound, 0, false, &m_Channel);
	if (result != FMOD_OK)
	{
		Destroy();
		return NULL;
	}
	return m_Channel;
}


FMOD::Channel* cSoundManager::PlaySound(std::string friendlyName) // For calling from other classes
{
	return PlaySound(FindSoundBySoundName(friendlyName));
}

void cSoundManager::Update(float vol, float pit, float pan, bool isLooping)
{
	if (!m_Initialized)
		return;

	setVolume(vol);
	setPitch(pit);
	setPan(pan);
	if (isLooping)
		setLoop(0);
	else
		setLoop(-1);
	//setLoop(isLoop);   // should have loop and pause/play on their own things, shouldn't be updated every frame
	// Add a pause/play!    or have it on its own function


	FMOD_RESULT result;
	result = m_System->update();
	if (result != FMOD_OK)
	{
		FMODCheckError(result);
		Destroy();
	}
}




void cSoundManager::setPitch(float newPitch)
{
	// Do we want to let it go below 0? probably not. maybe clamp it at .05f or somrthing, since 0 would probably stop the audio
	if (newPitch < 0.01f)
	{
		m_Channel->setPitch(0.01f);
		return;
	}
	m_Channel->setPitch(newPitch);
}


void cSoundManager::setVolume(float newVolume)
{
	if (newVolume < 0.0f) // Make sure volume doesn't go below 0
	{
		m_Channel->setVolume(0.0f);
		return;
	}
	else if (newVolume > 2.0f) // Make sure volume doesn't go above 2, I don't want to accidentally blow out my ears/headphones :)
	{
		m_Channel->setVolume(2.0f);
		return;
	}
	m_Channel->setVolume(newVolume);
}


void cSoundManager::setPan(float newPan)
{
	if (abs(newPan) > 1.0f) // Clamp the newPan between(including) -1 and 1
		newPan = abs(newPan) / newPan;
	m_Channel->setPan(newPan); // Update channel
	currentPan = newPan; // Update private variable
}


void cSoundManager::setLoop(int loopState) // -1 = loop forever; 0 = oneshot (don't loop); anything over 0 is the number of times to loop
{
	if (loopState <= -1)
	{
		m_Channel->setLoopCount(-1);
		return;
	}
	m_Channel->setLoopCount(loopState);
}


void cSoundManager::setPausePlay(bool isPaused)
{
	m_Channel->setPaused(isPaused);
	return;
}

float cSoundManager::getPitch()
{
	float temp;
	m_Channel->getPitch(&temp);
	return temp;
}


float cSoundManager::getVolume()
{
	float temp;
	m_Channel->getVolume(&temp);
	return temp;
}


float cSoundManager::getPan()
{
	return currentPan;
}


int cSoundManager::getLoop()
{
	int temp;
	m_Channel->getLoopCount(&temp);
	return temp;
}


std::vector<std::string> cSoundManager::getFriendlyNames()
{
	return friendlyNames;
}

FMOD::Sound* cSoundManager::FindSoundBySoundName(std::string soundName)
{
	std::map< std::string, FMOD::Sound*>::iterator itSound = m_map_friendlyName_to_sound.find(soundName);

	// Find it? 
	if (itSound == this->m_map_friendlyName_to_sound.end())
	{
		// Nope
		return NULL;
	}

	// Else we found the sound
	return itSound->second;
}

bool cSoundManager::loadSoundsFromFile(std::string filename)
{
	// Loads sounds from file
	// File should specify friendlyname to use with each sound and whether to use sample or stream
	// File "header" will specify how many sounds there are to load, each line thereafter will specify path and file time: ex - audio/stream/xx.wav

	std::ifstream theAudioFile(filename.c_str());
	if (!theAudioFile.is_open())
	{
		// didn't open the file.
		return false;
	}

	std::string temp;
	while (theAudioFile >> temp) // Iterate to first value we pull out of file: # of audio files
	{
		if (temp == "FilestoAdd")
		{
			break;
		}
	};

	unsigned int numOfFiles;
	theAudioFile >> numOfFiles; // Pass int int from file into variable to use later

	while (theAudioFile >> temp) // Iterate to right before we start reading for audio file info
	{
		if (temp == "Start")
		{
			break;
		}
	};

	std::string friendlyName, filePath;
	int audioTypetoMake;
	FMOD_RESULT result;
	// Each line has layout of: friendlyname path+audiofilename identifier_for_audio_type(stream or sample)
	for (unsigned int i = 0; i < numOfFiles; i++)
	{
		theAudioFile >> friendlyName;
		theAudioFile >> filePath;
		theAudioFile >> audioTypetoMake;

		if (audioTypetoMake == 0) // If == 0     sample value
		{
			result = m_System->createSound(filePath.c_str(), FMOD_CREATESAMPLE, 0, &m_Sound);
			if (result != FMOD_OK)
			{
				printf("Failed to load the sound file: %s\n", filePath);
			}
		}
		else // if == 1     stream value
		{
			result = m_System->createSound(filePath.c_str(), FMOD_CREATESTREAM, 0, &m_Sound);
			if (result != FMOD_OK)
			{
				printf("Failed to load the sound file: %s\n",filePath);
			}
		}
		m_map_friendlyName_to_sound[friendlyName] = m_Sound; // Store friendlyname and related FMod Sound pointer in map
		friendlyNames.push_back(friendlyName); // Add to vector storing just friendly names (media player uses this)
	}

	return true;

}
