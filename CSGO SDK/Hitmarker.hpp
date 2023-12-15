#pragma once
#include "sdk.hpp"

typedef struct _ImpactInfo {

  float x, y, z;
  float time;
}ImpactInfo, *PImpactInfo;

typedef struct _HitmarkerInfo {

  ImpactInfo pImpact;
  float alpha;
  int alpha2;
  int damage;
  float moved;
}HitmarkerInfo, *PHitmarkerInfo;

class __declspec( novtable ) IHitmarker : public NonCopyable {
public:
  static IHitmarker* Get( );
  virtual void GameEventCallback( IGameEvent* pEvent ) = NULL;
  virtual void Paint( ) = NULL;
protected:

  IHitmarker( ) { }
  virtual ~IHitmarker( ) { } 
};