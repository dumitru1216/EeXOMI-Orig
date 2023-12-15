#include "Config.hpp"
#include "json.h"
#include <fstream>
#include <iomanip>
#include "CVariables.hpp"

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>

#include <shlobj.h>


BOOL ConfigManager::DirectoryExists( LPCTSTR szPath ) {
   DWORD dwAttrib = GetFileAttributes( szPath );

   return ( dwAttrib != INVALID_FILE_ATTRIBUTES &&
	  ( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) );
}

std::vector<std::string> ConfigManager::GetConfigs( ) {
   static bool created_cfg = true;
   if ( created_cfg ) {
	  namespace fs = std::experimental::filesystem;
	  fs::path full_path( fs::current_path( ) );
	  std::wstring str = full_path.wstring( ) + L"\\EeXOMI_HVH";

	  CreateDirectoryW( str.c_str( ), nullptr );
	  str += L"\\Configs";
	  CreateDirectoryW( str.c_str( ), nullptr );

	  created_cfg = false;
   }

   std::string config_extension = ".JSON";
   std::vector<std::string> names;

   WIN32_FIND_DATAA find_data;
   HANDLE preset_file = FindFirstFileA( ( "EeXOMI_HVH\\Configs\\*" + config_extension ).c_str( ), &find_data );

   if ( preset_file != INVALID_HANDLE_VALUE ) {
	  do {
		 if ( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			continue;

		 std::string s = find_data.cFileName;
		 int pos = s.find( ".json" );

		 s.erase( s.begin( ) + pos, s.begin( ) + pos + 5 );

		 names.push_back( s.c_str( ) );
	  } while ( FindNextFileA( preset_file, &find_data ) );

	  FindClose( preset_file );
   }

   return names;
}

void ConfigManager::LoadConfig( std::string configname ) {
   std::ifstream input_file = std::ifstream( ( "EeXOMI_HVH\\Configs\\" + configname + ".json" ).c_str( ) );
   if ( !input_file.good( ) )
	  return;

   nlohmann::json j;

   try {
	  input_file >> g_Vars.m_json;
	  input_file.close( );
   } catch ( ... ) {
	  input_file.close( );
	  return;
   }

   for ( auto& child : g_Vars.m_children ) {
	  child->Load( g_Vars.m_json[ child->GetName( ) ] );
   }
}

void ConfigManager::SaveConfig( std::string configname ) {
   std::ofstream o( ( "EeXOMI_HVH\\Configs\\" + configname + ".json" ).c_str( ) );
   if ( !o.is_open( ) )
	  return;

   g_Vars.m_json.clear( );
   for ( auto& child : g_Vars.m_children ) {
	  child->Save( );

	  auto json = child->GetJson( );
	  g_Vars.m_json[ child->GetName( ) ] = ( json );
   }

   o.clear( );
   o << g_Vars.m_json.dump( 1 ).c_str( );
   o.close( );

   g_Vars.m_json.clear( );
}

void ConfigManager::RemoveConfig( std::string configname ) {
   std::remove( ( "EeXOMI_HVH\\Configs\\" + configname + ".json" ).c_str( ) );
}

void ConfigManager::CreateConfig( std::string configname ) {
   std::ofstream o( ( "EeXOMI_HVH\\Configs\\" + configname + ".json" ).c_str( ) );
}

void ConfigManager::OpenConfigFolder( ) {
   namespace fs = std::experimental::filesystem;
   fs::path full_path( fs::current_path( ) );

   std::wstring str = full_path.wstring( ) + L"\\EeXOMI_HVH\\Configs\\";

   PIDLIST_ABSOLUTE pidl;
   if ( SUCCEEDED( SHParseDisplayName( str.c_str( ), 0, &pidl, 0, 0 ) ) ) {
	  // we don't want to actually select anything in the folder, so we pass an empty
	  // PIDL in the array. if you want to select one or more items in the opened
	  // folder you'd need to build the PIDL array appropriately
	  ITEMIDLIST idNull = { 0 };
	  LPCITEMIDLIST pidlNull[ 1 ] = { &idNull };
	  SHOpenFolderAndSelectItems( pidl, 1, pidlNull, 0 );
	  ILFree( pidl );
   }
}
