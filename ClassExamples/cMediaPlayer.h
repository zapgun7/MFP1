#pragma once
class cMediaPlayer
{
public:
	cMediaPlayer();
	~cMediaPlayer();
	bool startProgram(); // Starts the GUI; main loop is located in here

private:
	float volume;
	float pitch;
	float pan;
	bool isPaused;
};

