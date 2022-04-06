#include "AudioFile.h"

const String AudioFile::PROCESSED_FOLDER = "/proc";

AudioFile::AudioFile(){}

bool AudioFile::open(char *filePath){
  fileState = FILE_OPENING;
  fileName = String(filePath);

  fetchAudioFileData();
  refreshBuffer();
  fileState = FILE_READY;
  return true;
}

void AudioFile::setTo(const uint8_t state){
  fileState = state;
}

bool AudioFile::hasFileEnded(){
  return fileState == FILE_ENDED;
}

uint16_t AudioFile::getSample(){
  /* If it's the case that inside the circular buffer it's the end of the song
     then, the file will be tagged as 'ended' and so, in the next iteration, if
     it remains as such state, it will return silence.   
  */
  if(buf.getReadIndex() == finalReadIndexOfFile){
    fileState = FILE_ENDED;
    finalReadIndexOfFile = 0xFFFF;
    return buf.get();
  }

  if(fileState == FILE_PLAYING) return buf.get();
  //else return 0x80; // For unsigned 8 bit audio.
  else return 0x8000; // For unsigned 16 bit audio.
}

void AudioFile::refreshBuffer(){
  if(buf.getFreeSpace() < BUFFER_REFRESH) return;
  dataFile.seek(fileDirectionToBuffer);
  uint8_t bufData[BUFFER_REFRESH*2];
  dataFile.read(bufData, BUFFER_REFRESH*2);
  for(uint16_t i = 0; i < BUFFER_REFRESH; i+=2){
    buf.put(bufData[i] | (bufData[i+1] << 8));

    fileDirectionToBuffer += 2;

    if(fileDirectionToBuffer >= fileSize){
      fileDirectionToBuffer = 0;
      dataFile.seek(0);
      finalReadIndexOfFile = buf.getWriteIndex();
      break;
    }
  }
}

bool AudioFile::fetchAudioFileData(){
  if(fileName.endsWith(".wav")){
    WavFile wavFile(fileName);
    WAV_FILE_INFO wavInfo = wavFile.processToRawFile();
    fileName = String(wavInfo.fileName);
    fileSize = wavInfo.dataSize;
  }

  dataFile = SD.open(fileName, FILE_READ);

  fileDirectionToBuffer = 0;
  //Byte res. = audioRes/8 (+ 1 if audioRes%8 > 0)
  byteAudioResolution = (audioResolution>>3) + ((audioResolution & 0x07)>0);
  debug("Loaded %s (size %d bytes) loaded! [AUDIO RESOLUTION: %d (%d bytes)]\n", fileName.c_str(), fileSize, audioResolution, byteAudioResolution);
  return true;
}

uint32_t AudioFile::getFileSize(){
  return fileSize;
}

uint32_t AudioFile::getCurrentFileDirection(){
  return fileDirectionToBuffer;
}

AUDIO_FILE_INFO AudioFile::getAudioFileInfo(){
  AUDIO_FILE_INFO ret = {
    .fileName = fileName.c_str(),
    .currentFileDirection = fileDirectionToBuffer,
    .size = fileSize,
    .progress = (fileDirectionToBuffer*100UL)/fileSize,
    .state = getStatusString(),
    .bitRes = audioResolution,
  };
  return ret;
}

String AudioFile::getStatusString(){
  String status = "";
  switch (fileState) {
  case FILE_OPENING:
    status = "OPENING";
    break;
  case FILE_READY:
    status = "READY";
    break;
  case FILE_ENDED:
    status = "ENDED";
    break;
  case FILE_PAUSED:
    status = "PAUSED";
    break;
  case FILE_PLAYING:
    status = "PLAYING";
    break;
  default:
    status = "UNKNOWN";
    break;
  }
  return status;
}

void AudioFile::debug(const char* x, ... ) {
  if(!DEBUG_FILE_MESSAGES) return;
  va_list args;
  va_start(args, x);
  Utilities::debug(x, args);
  va_end(args);
}

void AudioFile::error(const char* x, ... ) {
  va_list args;
  va_start(args, x);
  Utilities::error(x, args);
  va_end(args);
  for(;;){}
}
