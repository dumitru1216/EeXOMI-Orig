#pragma once
#include "sdk.hpp"

class __declspec( novtable ) IExtendedEsp : public NonCopyable {
public:
  static Encrypted_t<IExtendedEsp> Get( );
  virtual void Start() = NULL;
  virtual void Finish() = NULL;
protected:
  IExtendedEsp( ) {

  }

  virtual ~IExtendedEsp( ) {

  }
};