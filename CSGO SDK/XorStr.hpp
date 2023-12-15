#pragma once

#include "ADVobfuscator/MetaString.h"

#ifdef USE_XOR
#define DefXorStr(str) DEF_OBFUSCATED(str)
#define XorStr(str) (DEF_OBFUSCATED(str).decrypt())
#else
#define DefXorStr(str) str
#define XorStr(str)    str
#endif