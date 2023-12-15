#pragma once
#include "sdk.hpp"

class __declspec( novtable ) IPreserveKillfeed : public NonCopyable {
public:
   static IPreserveKillfeed* Get( );
   virtual void OnFrameStart( ) = 0;
   virtual void OnPaintTraverse( ) = 0;

protected:
   IPreserveKillfeed( ) { }
   virtual ~IPreserveKillfeed( ) { }
};