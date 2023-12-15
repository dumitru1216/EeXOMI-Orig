#pragma once
#include "sdk.hpp"
#include <DirectXMath.h>

#define RAD2DEG(x) DirectX::XMConvertToDegrees(x)
#define DEG2RAD(x) DirectX::XMConvertToRadians(x)

namespace Math
{
   bool IntersectSegmentSphere( const Vector& vecRayOrigin, const Vector& vecRayDelta, const Vector& vecSphereCenter, float radius );
   float GetFov( const QAngle& viewAngle, const QAngle& aimAngle );
   bool IntersectSegmentCapsule( const Vector& start, const Vector& end, const Vector& min, const Vector& max, float radius );
   bool IntersectionBoundingBox( const Vector& start, const Vector& dir, const Vector& min, const Vector& max, Vector* hit_point = nullptr );

   void Rotate( std::array<Vector2D, 3> & points, float rotation );

   void AngleVectors( const QAngle& angles, Vector& forward, Vector& right, Vector& up );

   void VectorAngles( const Vector& forward, Vector& angles );

   void SinCos( float a, float* s, float* c );

   void AngleVectors( const QAngle& angles, Vector& forward );

   float AngleNormalize( float angle );

   float ApproachAngle( float target, float value, float speed );

   void VectorTransform( const Vector& in1, const matrix3x4_t& in2, Vector& out );

   void SmoothAngle( QAngle src, QAngle& dst, float factor );

   QAngle CalcAngle( Vector src, Vector dst );

   Vector GetSmoothedVelocity( float min_delta, Vector a, Vector b );

   float AngleDiff( float src, float dst );

   __forceinline static float Interpolate( const float from, const float to, const float percent ) {
	  return to * percent + from * ( 1.f - percent );
   }

   __forceinline static Vector Interpolate( const Vector from, const Vector to, const float percent ) {
	  return to * percent + from * ( 1.f - percent );
   }

   __forceinline static void MatrixSetOrigin( Vector pos, matrix3x4_t& matrix ) {
	  matrix[ 0 ][ 3 ] = pos.x;
	  matrix[ 1 ][ 3 ] = pos.y;
	  matrix[ 2 ][ 3 ] = pos.z;
   }

   __forceinline static Vector MatrixGetOrigin( const matrix3x4_t& src ) {
	  return { src[ 0 ][ 3 ], src[ 1 ][ 3 ], src[ 2 ][ 3 ] };
   }

   struct CapsuleCollider {
	  Vector min;
	  Vector max;
	  float radius;

	  bool Intersect( const Vector& a, const Vector& b ) const;
   };

   // mixed types involved.
   template < typename T >
   T Clamp( const T& val, const T& minVal, const T& maxVal ) {
	  if ( ( T ) val < minVal )
		 return minVal;
	  else if ( ( T ) val > maxVal )
		 return maxVal;
	  else
		 return val;
   }

   template < typename T >
   T Hermite_Spline(
	  T p1,
	  T p2,
	  T d1,
	  T d2,
	  float t ) {
	  float tSqr = t * t;
	  float tCube = t * tSqr;

	  float b1 = 2.0f * tCube - 3.0f * tSqr + 1.0f;
	  float b2 = 1.0f - b1; // -2*tCube+3*tSqr;
	  float b3 = tCube - 2 * tSqr + t;
	  float b4 = tCube - tSqr;

	  T output;
	  output = p1 * b1;
	  output += p2 * b2;
	  output += d1 * b3;
	  output += d2 * b4;

	  return output;
   }

   template < typename T >
   T Hermite_Spline( T p0, T p1, T p2, float t ) {
	  return Hermite_Spline( p1, p2, p1 - p0, p2 - p1, t );
   }
}
