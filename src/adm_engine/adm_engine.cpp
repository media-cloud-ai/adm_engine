#ifndef ADM_ENGINE_C_API
#define ADM_ENGINE_C_API

#include <stdio.h>
#include <string.h>
#include <iostream>

#include <bw64/bw64.hpp>

#include "adm_engine/renderer.hpp"
#include "adm_engine/parser.hpp"
#include "adm_engine/utils.hpp"

using namespace admengine;

/*
 * C++ functions
 */

void _assign_string_to_pointer(const std::string& str, const char** pointer) {
  // get the string size + the ending null char
  const size_t length = str.size() + 1;
  // allocate the output pointer
  *pointer = (const char *)malloc(length);
  // copy the string to the output pointer
  std::memcpy((void*)*pointer, str.c_str(), length);
}

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API
 */

int dump_bw64_adm(const char* source_path, const char** output) {
  try {
    auto bw64File = bw64::readFile(source_path);

    const std::string admDocumentStr = getAdmDocumentAsString(getAdmDocument(parseAdmXmlChunk(bw64File)));

    _assign_string_to_pointer(admDocumentStr, output);
  } catch(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    // TODO handle errors
    return 1;
  }
  return 0;
}

typedef struct ElementGainEntry {
    const char* element_id;
    float gain;
} ElementGainEntry;


int process_rendering(const char* source_path,
                      const char* destination_path,
                      const char* element_id,
                      const ElementGainEntry* gain_mapping,
                      const unsigned int gain_mapping_size) {
  try {
    auto bw64File = bw64::readFile(source_path);

    const std::string outputLayout("0+2+0"); // TODO: get it from args
    Renderer renderer(bw64File, outputLayout, destination_path);

    if (gain_mapping != NULL && gain_mapping_size != 0) {
      for (int i = 0; i < gain_mapping_size; ++i) {
        const ElementGainEntry gain_entry = gain_mapping[i];
        renderer.setElementGain(gain_entry.element_id, gain_entry.gain);
      }
    }

    if (element_id != NULL) {
      renderer.setElementToRender(element_id);
    }

    renderer.process();
  } catch(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}


#ifdef __cplusplus
}
#endif

#endif // ADM_ENGINE_C_API
