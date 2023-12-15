#pragma once
#include "sdk.hpp"

class __declspec( novtable ) ISkinChanger : public NonCopyable {
public:
  static ISkinChanger* Get( );
  virtual void Create( ) = NULL;
  virtual void Destroy( ) = NULL;
  virtual void OnNetworkUpdate( bool start = true ) = NULL;
protected:
  ISkinChanger( ) { };
  virtual ~ISkinChanger( ) { };
};
