#pragma once
#include "sdk.hpp"

namespace Source
{
  class __declspec( novtable ) Miscellaneous : public NonCopyable {
  public:
	 static Miscellaneous* Get( );
	 virtual void Main( ) = NULL;

  protected:
	 Miscellaneous( ) { };
	 virtual ~Miscellaneous( ) {
	 }
  };
}
