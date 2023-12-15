#pragma once
#include "sdk.hpp"

class __declspec( novtable ) IExtendenBacktrack : public NonCopyable {
public:
   static IExtendenBacktrack* Get( );
   virtual void SetSuitableInSequence( INetChannel* channel ) = NULL;
   virtual float CalculatePing( INetChannel* channel ) = NULL;
   virtual void FlipState( INetChannel* channel ) = NULL;

protected:
   IExtendenBacktrack( ) {

   }
   virtual ~IExtendenBacktrack( ) {

   }
};