#include <iostream>

#include <bw64/bw64.hpp>

#include "renderer.hpp"
#include "parser.hpp"


int dumpBw64AdmFile(const char* path) {
  auto bw64File = bw64::readFile(path);
  admrenderer::displayBw64FileInfos(bw64File);
  admrenderer::displayAdmDocument(admrenderer::getAdmDocument(admrenderer::parseAdmXmlChunk(bw64File)));
  admrenderer::displayChnaChunk(admrenderer::parseAdmChnaChunk(bw64File));
  return 0;
}

int renderAdmContent(const char* input, const char* destination, const float dialogueGain = 1.0) {
  auto bw64File = bw64::readFile(input);
  const std::string outputDirectory(destination);
  const std::string outputLayout("0+2+0"); // TODO: get it from args
  admrenderer::render(bw64File, outputLayout, outputDirectory, dialogueGain);
  return 0;
}

void displayUsage(const char* application) {
  std::cout << "Usage: " << application << " INPUT OUTPUT [GAIN]" << std::endl;
  std::cout << "   with INPUT   BW64/ADM audio file" << std::endl;
  std::cout << "        OUTPUT  Destination directory" << std::endl;
  std::cout << "        GAIN    Gain to apply to Dialogue (optional)" << std::endl;
}

int main(int argc, char **argv) {
  // Read input BWF64/ADM file
  switch(argc) {
    case 2:
      return dumpBw64AdmFile(argv[1]);
    case 3:
      return renderAdmContent(argv[1], argv[2]);
    case 4:
      return renderAdmContent(argv[1], argv[2], std::atof(argv[3]));
    default:
      displayUsage(argv[0]);
      return 1;
  }
}
