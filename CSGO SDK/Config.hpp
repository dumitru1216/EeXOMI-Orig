#pragma once
#include <Windows.h>
#include <vector>
#include <string>

namespace ConfigManager
{
   BOOL DirectoryExists( LPCTSTR szPath );
   std::vector<std::string> GetConfigs( );
   void LoadConfig( std::string configname );
   void SaveConfig( std::string configname );
   void RemoveConfig( std::string configname );
   void CreateConfig( std::string configname );
   void OpenConfigFolder( );
}
