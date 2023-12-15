#pragma once
#include "sdk.hpp"

class __declspec( novtable ) IAutoBuy : public NonCopyable {
public:
   static Encrypted_t<IAutoBuy> Get( );
   virtual void Main( ) = NULL;
protected:
   IAutoBuy( ) {

   }
   virtual ~IAutoBuy( ) {

   }
};
