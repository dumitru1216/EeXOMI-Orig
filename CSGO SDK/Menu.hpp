#pragma once
#if 0
#include "sdk.hpp"

class IMenu : public NonCopyable {
public:
  static IMenu* Get( );
  virtual void Main( IDirect3DDevice9* pDevice ) = 0;
  virtual void WndProcHandler( ) = 0;
  virtual void Initialize( IDirect3DDevice9* pDevice ) = 0;
  virtual void StatFrameRender( ) = 0;
  virtual void IndicatorsFrame( ) = 0;
protected:
  IMenu( ) { };
  virtual ~IMenu( ) {
  }
};
#endif