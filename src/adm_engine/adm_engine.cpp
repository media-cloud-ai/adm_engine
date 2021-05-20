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

int _dump_bw64_adm_file(const std::string& inputFilePath, const char** output) {
  try {
    auto bw64File = bw64::readFile(inputFilePath);

    const std::string admDocumentStr = getAdmDocumentAsString(getAdmDocument(parseAdmXmlChunk(bw64File)));

    _assign_string_to_pointer(admDocumentStr, output);
  } catch(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}


int _create_renderer(void** renderer, const std::string& inputFilePath, const std::string& outputDirectory) {
  try {
    const std::string outputLayout("0+2+0"); // TODO: get it from args

    auto bw64File = openBw64File(inputFilePath);

    *renderer = new Renderer(bw64File, outputLayout, outputDirectory);
  } catch(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

int _set_element_to_render(void** renderer, const std::string& elementId) {
  try {
    Renderer* r = static_cast<Renderer*>(*renderer);
    r->setElementToRender(elementId);
  } catch(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

int _set_element_gain(void** renderer, const std::string& elementId, const float gain) {
  try {
    Renderer* r = static_cast<Renderer*>(*renderer);
    r->setElementGain(elementId, gain);
  } catch(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

int _process_rendering(void** renderer) {
  try {
    Renderer* r = static_cast<Renderer*>(*renderer);
    r->process();
  } catch(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API
 */

int dump_bw64_adm(const char* source_path, const char** output) {
  return _dump_bw64_adm_file(source_path, output);
}

int initialize_renderer(void** renderer, const char* source_path, const char* destination_path) {
  return _create_renderer(renderer, source_path, destination_path);
}

int set_element_to_render(void** renderer, const char* element_id) {
  return _set_element_to_render(renderer, element_id);
}

int set_element_gain(void** renderer, const char* element_id, const float gain) {
  return _set_element_gain(renderer, element_id, gain);
}

int process_rendering(void** renderer) {
  return _process_rendering(renderer);
}


#ifdef __cplusplus
}
#endif

#endif // ADM_ENGINE_C_API
