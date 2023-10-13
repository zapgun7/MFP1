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


// Creates and initializes the FMOD::System variable; sets private pan variable, since there's no getPan channel function
// Calls loadSoundsFromFile, which loads all audio specified in a text file into a map
bool cSoundManager::Initialize(std::string loadFile)
{
	if (m_Initialized)
		return true;

	FMOD_RESULT result;
	result = FMOD::System_Create(&m_System); // Create system
	if (result != FMOD_OK)
	{
		printf("Failed to create the FMOD System!\n");
		return false;
	}

	result = m_System->init(1, FMOD_INIT_NORMAL, nullptr); // Initialize system
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

// Cleans up project by releasing and closing all variables that should
void cSoundManager::Destroy()
{
	if (!m_Initialized)
		return;

	FMOD_RESULT result;


	std::map<std::string, FMOD::Sound*>::iterator it = m_map_friendlyName_to_sound.begin();
	while(it != m_map_friendlyName_to_sound.end()) // Iterates through the map to release all the sounds
	{
		it->second->release(); // Release sound
		//delete(it->second); // Delete pointer         // Causing an ungraceful program termination (when closing window)
		it++;
	}

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


// Update is called every frame from the main program loop in cMediaPlayer
// Updates all settings managed by cMediaPlayer
// Also calls the important FMOD::System->update()
void cSoundManager::Update(float vol, float pit, float pan, bool isLooping)
{
	if (!m_Initialized)
		return;

	setVolume(vol);
	setPitch(pit);
	setPan(pan);
 	if (isLooping)
 		setLoop(-1);
 	else
 		setLoop(0);


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
	// Do we want to let it go below 0? probably not. maybe clamp it at .05f or something, since 0 would probably stop the audio
// 	if (newPitch < 0.01f)
// 	{
// 		m_Channel->setPitch(0.01f);
// 		return;
// 	}
	m_Channel->setPitch(newPitch);
}


void cSoundManager::setVolume(float newVolume)
{
	if (newVolume < -2.0f) // Make sure volume doesn't go below -2
	{
		m_Channel->setVolume(-2.0f);
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
	if (loopState < 0)
	{
		m_Channel->setMode(FMOD_LOOP_NORMAL); // Set channel mode to looping
		m_Channel->setLoopCount(-1);
		return;
	}
	m_Channel->setLoopCount(loopState);
	return;
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


bool cSoundManager::getIsPlaying()
{
	bool temp;
	m_Channel->isPlaying(&temp);
	return temp;
}


// Returns a vector with the first index containing       vector[0] = current audio length in ms
std::vector<int> cSoundManager::getAudioProgress()//      vector[1] = current position in audio in ms
{
	std::vector<int> retVec;
	unsigned int audPos;
	unsigned int audLen;
	FMOD::Sound *temp;
	m_Channel->getPosition(&audPos, FMOD_TIMEUNIT_MS); // get position in ms
	m_Channel->getCurrentSound(&temp);
	temp->getLength(&audLen, FMOD_TIMEUNIT_MS);
	if (audPos == NULL)
	{
		retVec.push_back(0.0f);
		retVec.push_back(0.0f);
		return retVec;
	}
	retVec.push_back(audLen);
	retVec.push_back(audPos);
	return retVec;
}

// Returns stored string vector of audio friendlynames for cMediaPlayer to use
std::vector<std::string> cSoundManager::getFriendlyNames()
{
	return friendlyNames;
}

// Returns stored string vector of audio credits for cMediaPlayer to use
std::vector<std::string> cSoundManager::getCredits()
{
	return credits;
}


// Queries the sound map with the given friendlyname, returning the relevant sound pointer
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


// Loads all audio specified by the given text file; text file specifiies whether it is stream or sample (bigger files will be stream)
// All audio is loaded into a map to be easily retrieved later
// This function also loads a string vector of the friendlynames and the credits, the credits also being read from a file
bool cSoundManager::loadSoundsFromFile(std::string filename)
{
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
				printf("Failed to load the sound file: %s\n", filePath.c_str());
			}
		}
		else // if == 1     stream value
		{
			result = m_System->createSound(filePath.c_str(), FMOD_CREATESTREAM, 0, &m_Sound);
			if (result != FMOD_OK)
			{
				printf("Failed to load the sound file: %s\n",filePath.c_str());
			}
		}
		m_map_friendlyName_to_sound[friendlyName] = m_Sound; // Store friendlyname and related FMod Sound pointer in map
		friendlyNames.push_back(friendlyName); // Add to vector storing just friendly names (media player uses this)
	}
	theAudioFile.close();

	// Now loading the credits into a private vector
	std::string tempStr;
	std::ifstream theCreditsFile("audio/Credits.txt");
	if (!theCreditsFile.is_open())
	{
		// didn't open the file.
		return false;
	}
	while (theCreditsFile >> tempStr)
	{
		for (unsigned int i = 0; i < 2; i++) // Iterates through each "chunk" of credits, while loop checks if another "chunk" exists
		{								     // Could've just looped off of numOfFiles, but still works and gives a different way to tackle a file like this
			credits.push_back(tempStr);
			theCreditsFile >> tempStr;
		}
	}
	theCreditsFile.close();

	return true;

}
