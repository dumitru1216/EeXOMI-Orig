#pragma once
#include "sdk.hpp"

class __declspec( novtable ) IGrenadePrediction : public NonCopyable {
public:
  static IGrenadePrediction* Get( );
  virtual void Tick( int buttons ) = 0;
  virtual void View( ) = 0;
  virtual void Paint( ) = 0;
protected:
  IGrenadePrediction( ) { };
  virtual ~IGrenadePrediction( ) {
  }
};
