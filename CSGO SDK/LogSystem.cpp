#include "LogSystem.hpp"
#include <sstream>
#include <fstream>

class LogSystem : public ILogSystem {
public:
   void Log( const char* file_fmt, const char* fmt, ... ) override;
private:
   std::string GetTimestamp( );

};

Encrypted_t<ILogSystem> ILogSystem::Get( ) {
   static LogSystem instance;
   return &instance;
}

void LogSystem::Log( const char* file_fmt, const char* fmt, ... ) {
   if ( !fmt ) return;

   va_list va_alist;
   char logBuf[ 1024 ] = { 0 };

   va_start( va_alist, fmt );
   _vsnprintf( logBuf + strlen( logBuf ), sizeof( logBuf ) - strlen( logBuf ), fmt, va_alist );
   va_end( va_alist );

   std::ofstream file;

   file.open( std::string( XorStr( "eexomi" ) ).append( file_fmt ), std::ios::app );

   file << GetTimestamp( ) << " " << logBuf << std::endl;

   file.close( );
}

std::string LogSystem::GetTimestamp( ) {
   std::time_t t = std::time( nullptr );
   std::tm tm;
   localtime_s( &tm, &t );
   std::locale loc( std::cout.getloc( ) );

   std::basic_stringstream<char> ss;
   ss.imbue( loc );
   ss << std::put_time( &tm, XorStr( "[%A %b %e %H:%M:%S %Y]" ) );

   return ss.str( );
}
