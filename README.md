# MFP1
A repository for my first project in the Media Fundamentals class
Watch the demo here (sans stop button): https://www.youtube.com/watch?v=81r0ClfWX5Q

## How to Build
- Should just have to open it in VS2022 and build it in either Debug or Release in x64
- Or run the Project1.exe in the root folder


## Design Choices:
### Audio Loading
- The audio files are loaded by referencing a .txt file
- The file contains each sound's file path, the friendlyName (name used to call on the sound in code), and the type of sound to make it (sample or stream)

### Audio Storing
- I used a map to store the sound variables
  > std::map<std::string, FMOD::sound*>


## Known Issues
- GUI loads up 10 buttons for audio regardless, so having more or less will not work, the latter likely crashing :P
- The cMediaPlayer is responsible for too much: in addition to user input, I crammed all the graphics stuff in there too
  - Since the graphics portion isn't too complicated, it's not a horrid issue
