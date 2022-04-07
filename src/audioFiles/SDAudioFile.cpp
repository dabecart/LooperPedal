#include "SDAudioFile.h"

const String SDAudioFile::PROCESSED_FOLDER = "/proc";

SDAudioFile::SDAudioFile(){}

bool SDAudioFile::open(char *filePath){
  fileStatus = FILE_OPENING;
  fileName = String(filePath);

  if(!fetchSDAudioFileData()) return false;
  refreshBuffer();
  fileStatus = FILE_READY;
  return true;
}

bool SDAudioFile::hasFileEnded(){
  return fileStatus == FILE_ENDED;
}

void SDAudioFile::calculateTotalIteration(uint32_t maxFileSize){
  maxIterations = maxFileSize/fileSize;
}

uint16_t SDAudioFile::getSample(){
  /* If it's the case that inside the circular buffer it's the end of the song
     then, the file will be tagged as 'ended' and so, in the next iteration, if
     it remains as such state, it will return silence.   
  */
  if(buf.getReadIndex() == finalReadIndexOfFile){
    if(currentIteration < maxIterations){
      fileStatus = FILE_ENDED;
      currentIteration = 0;
    }else{
      // Same fileStatus.
      currentIteration++;
    }
    finalReadIndexOfFile = 0xFFFF;
    return buf.get();
  }

  if(fileStatus == FILE_PLAYING) return buf.get();
  //else return 0x80; // For unsigned 8 bit audio.
  else return 0x8000; // For unsigned 16 bit audio.
}

void SDAudioFile::refreshBuffer(){
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

bool SDAudioFile::fetchSDAudioFileData(){
  if(fileName.endsWith(".wav")){
    WavFile wavFile(fileName);
    WAV_FILE_INFO wavInfo = wavFile.processToRawFile();
    fileName = String(wavInfo.fileName);
    //fileSize = wavInfo.dataSize;
  } else if(fileName.endsWith(".raw")){
    // Nothing at the moment.
  }else{
    error("Filetype not suported!");
    return false;
  }

  // Must be sure it is a .raw file.
  dataFile = SD.open(fileName, FILE_READ);
  fileSize = dataFile.size();

  if(fileSize == 0){
    debug("File %s is empty!\n", fileName.c_str());
    return false;
  }

  fileDirectionToBuffer = 0;
  //Byte res. = audioRes/8 (+ 1 if audioRes%8 > 0)
  byteAudioResolution = (audioResolution>>3) + ((audioResolution & 0x07)>0);
  debug("Loaded %s (size %d bytes) loaded! [AUDIO RESOLUTION: %d (%d bytes)]\n", fileName.c_str(), fileSize, audioResolution, byteAudioResolution);
  return true;
}

String SDAudioFile::getStatusString(){
  String status = "";
  switch (fileStatus) {
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

void SDAudioFile::debug(const char* x, ... ) {
  if(!DEBUG_FILE_MESSAGES) return;
  va_list args;
  va_start(args, x);
  Utilities::debug(x, args);
  va_end(args);
}

void SDAudioFile::error(const char* x, ... ) {
  va_list args;
  va_start(args, x);
  Utilities::error(x, args);
  va_end(args);
  for(;;){}
}