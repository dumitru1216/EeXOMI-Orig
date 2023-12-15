#pragma once

#include "auto.hpp"
#include "DataMap.hpp"

#define horizon_concat(x, y) x##y
#define horizon_concatiate(x, y) horizon_concat(x, y)
#define horizon_pad(size)                                         \
private:                                                          \
  std::uint8_t horizon_concatiate(__pad, __COUNTER__)[size] = {}; \
                                                                  \
public:


namespace Horizon::Memory
{
  std::uintptr_t Scan( const std::string& image_name, const std::string& signature );
  std::uintptr_t Scan( const std::uintptr_t image, const std::string& signature );
  unsigned int FindInDataMap( datamap_t *pMap, const char *name );
}
