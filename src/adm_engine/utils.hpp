#pragma once

#include <string>
#include <bw64/bw64.hpp>

namespace admengine {

std::string replaceSpecialCharacters(std::string& text);

std::unique_ptr<bw64::Bw64Reader> openBw64File(const std::string& filePath);

}
