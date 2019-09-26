#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include <adm/common_definitions.hpp>
#include <adm/parse.hpp>
#include <adm/write.hpp>

#include <bw64/bw64.hpp>

#include <ear/ear.hpp>
#include <ear/dsp/dsp.hpp>

#include "renderer.hpp"

using namespace ear;
using namespace adm;
using namespace admrenderer;

int main(int argc, char **argv) {

  // Read input BWF64/ADM file
  if (argc != 3 && argc != 4) {
    std::cout << "Usage: " << argv[0] << " INPUT OUTPUT [GAIN]" << std::endl;
    std::cout << "   with INPUT   BW64/ADM audio file" << std::endl;
    std::cout << "        OUTPUT  Destination directory" << std::endl;
    std::cout << "        GAIN    Gain to apply to Dialogue (optional)" << std::endl;
    exit(1);
  }

  auto bw64File = bw64::readFile(argv[1]);
  const std::string outputDirectory(argv[2]);
  const std::string outputLayout("0+2+0"); // TODO: get it from args

  float dialogueGain = 1.0;
  if(argc == 4) {
    dialogueGain = std::atof(argv[3]);
  }
  std::cout << "DIALOGUE GAIN: " << dialogueGain << std::endl;

  render(bw64File, outputLayout, outputDirectory, dialogueGain);

  return 0;
}
