#pragma once
#include "sdk.hpp"

class __declspec( novtable ) IEsp : public NonCopyable {
public:
   static Encrypted_t<IEsp> Get( );
   virtual void Main( ) = NULL;
   virtual void SetAlpha( int idx ) = NULL;
   virtual float GetAlpha( int idx ) = NULL;
protected:
   IEsp( ) { };
   virtual ~IEsp( ) {
   }
};
