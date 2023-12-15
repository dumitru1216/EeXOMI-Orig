#pragma once
#include "sdk.hpp"

namespace Engine
{
   class __declspec( novtable ) VisibilityOptimization : public NonCopyable {
   public:
	  static VisibilityOptimization* Get( );
	  virtual void Update( CViewSetup* view ) = 0; // call on overrideview
   };

}