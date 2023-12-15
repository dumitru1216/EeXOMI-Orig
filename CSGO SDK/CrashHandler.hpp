#pragma once
#include "sdk.hpp"

class __declspec( novtable ) ICrashHandler : public NonCopyable {
public:
   static Encrypted_t<ICrashHandler> Get( );
   virtual long __stdcall OnCrashProgramm( struct _EXCEPTION_POINTERS* ) = NULL;
   
protected:
   ICrashHandler( ) {

   }
   virtual ~ICrashHandler( ) {

   }
};