#pragma once
#include "sdk.hpp"

class __declspec( novtable ) ILogSystem : public NonCopyable {
public:
   static Encrypted_t<ILogSystem> Get( );
   virtual void Log( const char* file_fmt, const char* fmt, ... ) = NULL;

protected:
   ILogSystem( ) {

   }
   virtual ~ILogSystem( ) {
      
   }
};