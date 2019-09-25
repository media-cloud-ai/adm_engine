#pragma once

#include "adm/parse.hpp"
#include "adm/write.hpp"

#include "bw64/bw64.hpp"

namespace admrenderer {

  std::shared_ptr<adm::Document> parseAdmXml(const std::unique_ptr<bw64::Bw64Reader>& bw64File);
  std::shared_ptr<bw64::ChnaChunk> parseAdmChunk(const std::unique_ptr<bw64::Bw64Reader>& bw64File);

}
