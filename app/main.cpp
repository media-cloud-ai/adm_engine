#include <iostream>

#include <bw64/bw64.hpp>

#include "renderer.hpp"
#include "parser.hpp"

using namespace admrenderer;

int dumpBw64AdmFile(const char* path) {
  auto bw64File = bw64::readFile(path);
  displayBw64FileInfos(bw64File);
  displayAdmDocument(getAdmDocument(parseAdmXmlChunk(bw64File)));
  displayChnaChunk(parseAdmChnaChunk(bw64File));
  return 0;
}

int renderAdmContent(const char* input, const char* destination, const float dialogueGain = 1.0) {
  auto bw64File = bw64::readFile(input);
  const std::string outputDirectory(destination);
  const std::string outputLayout("0+2+0"); // TODO: get it from args
  Renderer renderer(bw64File, outputLayout, outputDirectory, dialogueGain);
  renderer.process();
  return 0;
}

void displayUsage(const char* application) {
  std::cout << "Usage: " << application << " INPUT [OUTPUT] [GAIN]" << std::endl;
  std::cout << "   with INPUT   BW64/ADM audio file" << std::endl;
  std::cout << "        OUTPUT  Destination directory (optional)" << std::endl;
  std::cout << "                  - if specified, enable ADM rendering to BW64/ADM file" << std::endl;
  std::cout << "                  - otherwise, dump input BW64/ADM file information" << std::endl;
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
