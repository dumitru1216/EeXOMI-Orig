#if 1

#pragma once
#include "sdk.hpp"

class __declspec( novtable ) MenuV2 : public NonCopyable {
public:
   static Encrypted_t<MenuV2> Get( );
   virtual void MenuRender( IDirect3DDevice9* pDevice ) = NULL;
   virtual void WndProcHandler( ) = 0;
   virtual void Initialize( IDirect3DDevice9* pDevice ) = 0;
protected:
   MenuV2( ) {

   }
   virtual ~MenuV2( ) {

   }
};

#endif