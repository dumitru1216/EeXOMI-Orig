#pragma once
#include "sdk.hpp"

class __declspec( novtable ) ISoundEsp : public NonCopyable {
public:
  static ISoundEsp* Get( );
  virtual void Main( ) = NULL;

protected:
  ISoundEsp( ) { };
  virtual ~ISoundEsp( ) { };
};