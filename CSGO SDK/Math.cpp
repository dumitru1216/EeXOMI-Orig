#include "Math.h"
#include "source.hpp"
#include "Player.hpp"
#include "CVariables.hpp"

#include <algorithm>
#include <numeric>
#include <xmmintrin.h>

static Vector DirBetweenLines( const Vector& a, const Vector& b, const Vector& c, const Vector& d ) {
   const Vector d1 = ( b - a );
   const Vector d2 = ( d - c );

   const Vector cross = d1.Cross( d2 );

   const Vector cross1 = d1.Cross( cross );
   const Vector cross2 = d2.Cross( cross );

   const Vector sp = c + d2 * Math::Clamp( ( a - c ).Dot( cross1 ) / ( d2.Dot( cross1 ) ), 0.f, 1.f );
   const Vector ep = a + d1 * Math::Clamp( ( c - a ).Dot( cross2 ) / ( d1.Dot( cross2 ) ), 0.f, 1.f );

   return ep - sp;
}

bool Math::CapsuleCollider::Intersect( const Vector& start, const Vector& end ) const {
#if 1
   static auto constexpr epsilon = 0.00000001f;

   const auto s1 = min;
   const auto s2 = max;
   const auto k1 = start;
   const auto k2 = end;

   auto u = s2 - s1;
   auto v = k2 - k1;
   const auto w = s1 - k1;

   const auto a = u.Dot( u );
   const auto b = u.Dot( v );
   const auto c = v.Dot( v );
   const auto d = u.Dot( w );
   const auto e = v.Dot( w );
   const auto D = a * c - b * b;
   float sn, sd = D;
   float tn, td = D;

   if ( D < epsilon ) {
	  sn = 0.0f;
	  sd = 1.0f;
	  tn = e;
	  td = c;
   } else {
	  sn = b * e - c * d;
	  tn = a * e - b * d;

	  if ( sn < 0.0f ) {
		 sn = 0.0f;
		 tn = e;
		 td = c;
	  } else if ( sn > sd ) {
		 sn = sd;
		 tn = e + b;
		 td = c;
	  }
   }

   if ( tn < 0.0f ) {
	  tn = 0.0f;

	  if ( -d < 0.0f )
		 sn = 0.0f;
	  else if ( -d > a )
		 sn = sd;
	  else {
		 sn = -d;
		 sd = a;
	  }
   } else if ( tn > td ) {
	  tn = td;

	  if ( -d + b < 0.0f )
		 sn = 0.f;
	  else if ( -d + b > a )
		 sn = sd;
	  else {
		 sn = -d + b;
		 sd = a;
	  }
   }

   const float sc = abs( sn ) < epsilon ? 0.0f : sn / sd;
   const float tc = abs( tn ) < epsilon ? 0.0f : tn / td;

   auto dp = w + u * sc - v * tc;
   auto scale = dp.Dot( dp );
   return scale < radius * radius;
#endif

#if 0
   const Vector dir = DirBetweenLines( min, max, start, end );
   return dir.LengthSquared( ) <= radius * radius;


   return IntersectSegmentCapsule( start, end, min, max, radius );
#endif
}

bool Math::IntersectSegmentSphere( const Vector& vecRayOrigin, const Vector& vecRayDelta, const Vector& vecSphereCenter, float flRadius ) {
   // Solve using the ray equation + the sphere equation
   // P = o + dt
   // (x - xc)^2 + (y - yc)^2 + (z - zc)^2 = r^2
   // (ox + dx * t - xc)^2 + (oy + dy * t - yc)^2 + (oz + dz * t - zc)^2 = r^2
   // (ox - xc)^2 + 2 * (ox-xc) * dx * t + dx^2 * t^2 +
   //		(oy - yc)^2 + 2 * (oy-yc) * dy * t + dy^2 * t^2 +
   //		(oz - zc)^2 + 2 * (oz-zc) * dz * t + dz^2 * t^2 = r^2
   // (dx^2 + dy^2 + dz^2) * t^2 + 2 * ((ox-xc)dx + (oy-yc)dy + (oz-zc)dz) t +
   //		(ox-xc)^2 + (oy-yc)^2 + (oz-zc)^2 - r^2 = 0
   // or, t = (-b +/- sqrt( b^2 - 4ac)) / 2a
   // a = DotProduct( vecRayDelta, vecRayDelta );
   // b = 2 * DotProduct( vecRayOrigin - vecCenter, vecRayDelta )
   // c = DotProduct(vecRayOrigin - vecCenter, vecRayOrigin - vecCenter) - flRadius * flRadius;

   Vector vecSphereToRay = vecRayOrigin - vecSphereCenter;

   float a = vecRayDelta.Dot( vecRayDelta );

   // This would occur in the case of a zero-length ray
   if ( a == 0.0f )
	  return vecSphereToRay.LengthSquared( ) <= flRadius * flRadius;

   float b = 2.f * vecSphereToRay.Dot( vecRayDelta );
   float c = vecSphereToRay.Dot( vecSphereToRay ) - flRadius * flRadius;
   float flDiscrim = b * b - 4.f * a * c;
   return flDiscrim >= 0.0f;
}

float Math::GetFov( const QAngle& viewAngle, const QAngle& aimAngle ) {

   QAngle delta = aimAngle - viewAngle;
   delta.Normalize( );
   delta.Clamp( );
   return sqrtf( powf( delta.pitch, 2.0f ) + powf( delta.yaw, 2.0f ) );
}

bool Math::IntersectSegmentCapsule( const Vector& start, const Vector& end, const Vector& min, const Vector& max, float radius ) {
   Vector d = max - min, m = start - min, n = end - start;
   float md = m.Dot( d );
   float nd = n.Dot( d );
   float dd = d.Dot( d );

   if ( md < 0.0f && md + nd < 0.0f ) {
	  return IntersectSegmentSphere( start, n, min, radius );
   }
   if ( md > dd && md + nd > dd ) {
	  return IntersectSegmentSphere( start, n, max, radius );
   }

   float t = 0.0f;
   float nn = n.Dot( n );
   float mn = m.Dot( n );
   float a = dd * nn - nd * nd;
   float k = m.Dot( m ) - radius * radius;
   float c = dd * k - md * md;
   if ( std::fabsf( a ) < FLT_EPSILON ) {
	  if ( c > 0.0f )
		 return 0;
	  if ( md < 0.0f )
		 IntersectSegmentSphere( start, n, min, radius );
	  else if ( md > dd )
		 IntersectSegmentSphere( start, n, max, radius );
	  else
		 t = 0.0f;
	  return true;
   }
   float b = dd * mn - nd * md;
   float discr = b * b - a * c;
   if ( discr < 0.0f )
	  return false;

   t = ( -b - sqrt( discr ) ) / a;
   float t0 = t;
   if ( md + t * nd < 0.0f ) {
	  return IntersectSegmentSphere( start, n, min, radius );
   } else if ( md + t * nd > dd ) {

	  return IntersectSegmentSphere( start, n, max, radius );
   }
   t = t0;
   return t > 0.0f && t < 1.0f;
}

#if 1
bool Math::IntersectionBoundingBox( const Vector& src, const Vector& dir, const Vector& min, const Vector& max, Vector* hit_point ) {
   /*
   Fast Ray-Box Intersection
   by Andrew Woo
   from "Graphics Gems", Academic Press, 1990
   */

   constexpr auto NUMDIM = 3;
   constexpr auto RIGHT = 0;
   constexpr auto LEFT = 1;
   constexpr auto MIDDLE = 2;

   bool inside = true;
   char quadrant[ NUMDIM ];
   int i;

   // Rind candidate planes; this loop can be avoided if
   // rays cast all from the eye(assume perpsective view)
   Vector candidatePlane;
   for ( i = 0; i < NUMDIM; i++ ) {
	  if ( src[ i ] < min[ i ] ) {
		 quadrant[ i ] = LEFT;
		 candidatePlane[ i ] = min[ i ];
		 inside = false;
	  } else if ( src[ i ] > max[ i ] ) {
		 quadrant[ i ] = RIGHT;
		 candidatePlane[ i ] = max[ i ];
		 inside = false;
	  } else {
		 quadrant[ i ] = MIDDLE;
	  }
   }

   // Ray origin inside bounding box
   if ( inside ) {
	  if ( hit_point )
		 * hit_point = src;
	  return true;
   }

   // Calculate T distances to candidate planes
   Vector maxT;
   for ( i = 0; i < NUMDIM; i++ ) {
	  if ( quadrant[ i ] != MIDDLE && dir[ i ] != 0.f )
		 maxT[ i ] = ( candidatePlane[ i ] - src[ i ] ) / dir[ i ];
	  else
		 maxT[ i ] = -1.f;
   }

   // Get largest of the maxT's for final choice of intersection
   int whichPlane = 0;
   for ( i = 1; i < NUMDIM; i++ ) {
	  if ( maxT[ whichPlane ] < maxT[ i ] )
		 whichPlane = i;
   }

   // Check final candidate actually inside box
   if ( maxT[ whichPlane ] < 0.f )
	  return false;

   for ( i = 0; i < NUMDIM; i++ ) {
	  if ( whichPlane != i ) {
		 float temp = src[ i ] + maxT[ whichPlane ] * dir[ i ];
		 if ( temp < min[ i ] || temp > max[ i ] ) {
			return false;
		 } else if ( hit_point ) {
			( *hit_point )[ i ] = temp;
		 }
	  } else if ( hit_point ) {
		 ( *hit_point )[ i ] = candidatePlane[ i ];
	  }
   }

   // ray hits box
   return true;
#else
bool Math::IntersectionBoundingBox( const Vector& start, const Vector& end, const Vector& min, const Vector& max, Vector* hit_point ) {
   float d1, d2, f;
   auto start_solid = true;
   auto t1 = -1.0f, t2 = 1.0f;

   const float s[3] = { start.x, start.y, start.z };
   const float e[3] = { end.x, end.y, end.z };
   const float mi[3] = { min.x, min.y, min.z };
   const float ma[3] = { max.x, max.y, max.z };

   for (auto i = 0; i < 6; i++) {
	  if (i >= 3) {
		 const auto j = i - 3;

		 d1 = s[j] - ma[j];
		 d2 = d1 + e[j];
	  }
	  else {
		 d1 = -s[i] + mi[i];
		 d2 = d1 - e[i];
	  }

	  if (d1 > 0.0f && d2 > 0.0f)
		 return false;

	  if (d1 <= 0.0f && d2 <= 0.0f)
		 continue;

	  if (d1 > 0)
		 start_solid = false;

	  if (d1 > d2) {
		 f = d1;
		 if (f < 0.0f)
			f = 0.0f;

		 f /= d1 - d2;
		 if (f > t1)
			t1 = f;
	  }
	  else {
		 f = d1 / (d1 - d2);
		 if (f < t2)
			t2 = f;
	  }
   }

   return start_solid || (t1 < t2 && t1 >= 0.0f);
#endif
}

void Math::Rotate( std::array< Vector2D, 3 > & points, float rotation ) {
   const auto points_center = ( points.at( 0 ) + points.at( 1 ) + points.at( 2 ) ) / 3;
   for ( auto& point : points ) {
	  point -= points_center;

	  const auto temp_x = point.x;
	  const auto temp_y = point.y;

	  const auto theta = ToRadians( rotation );
	  float c, s;
	  DirectX::XMScalarSinCos( &s, &c, theta );

	  point.x = temp_x * c - temp_y * s;
	  point.y = temp_x * s + temp_y * c;

	  point += points_center;
   }
}
void Math::AngleVectors( const QAngle& angles, Vector& forward, Vector& right, Vector& up ) {
   float sr, sp, sy, cr, cp, cy;

   SinCos( DEG2RAD( angles[ 1 ] ), &sy, &cy );
   SinCos( DEG2RAD( angles[ 0 ] ), &sp, &cp );
   SinCos( DEG2RAD( angles[ 2 ] ), &sr, &cr );

   forward.x = ( cp * cy );
   forward.y = ( cp * sy );
   forward.z = ( -sp );
   right.x = ( -1 * sr * sp * cy + -1 * cr * -sy );
   right.y = ( -1 * sr * sp * sy + -1 * cr * cy );
   right.z = ( -1 * sr * cp );
   up.x = ( cr * sp * cy + -sr * -sy );
   up.y = ( cr * sp * sy + -sr * cy );
   up.z = ( cr * cp );
}
void Math::VectorAngles( const Vector& forward, Vector& angles ) {
   float tmp, yaw, pitch;

   if ( forward[ 1 ] == 0 && forward[ 0 ] == 0 ) {
	  yaw = 0;
	  if ( forward[ 2 ] > 0 )
		 pitch = 270;
	  else
		 pitch = 90;
   } else {
	  yaw = ( atan2( forward[ 1 ], forward[ 0 ] ) * 180 / M_PI );
	  if ( yaw < 0 )
		 yaw += 360;

	  tmp = sqrt( forward[ 0 ] * forward[ 0 ] + forward[ 1 ] * forward[ 1 ] );
	  pitch = ( atan2( -forward[ 2 ], tmp ) * 180 / M_PI );
	  if ( pitch < 0 )
		 pitch += 360;
   }

   angles[ 0 ] = pitch;
   angles[ 1 ] = yaw;
   angles[ 2 ] = 0;
}

void Math::SinCos( float a, float* s, float* c ) {
   *s = sin( a );
   *c = cos( a );
}

void Math::AngleVectors( const QAngle& angles, Vector& forward ) {
   float	sp, sy, cp, cy;

   SinCos( DEG2RAD( angles[ 1 ] ), &sy, &cy );
   SinCos( DEG2RAD( angles[ 0 ] ), &sp, &cp );

   forward.x = cp * cy;
   forward.y = cp * sy;
   forward.z = -sp;
}
float Math::AngleNormalize( float angle ) {
   if ( angle > 180.f || angle < -180.f ) {
	  auto revolutions = round( abs( angle / 360.f ) );

	  if ( angle < 0.f )
		 angle = angle + 360.f * revolutions;
	  else
		 angle = angle - 360.f * revolutions;
   }

   return angle;
}
float Math::ApproachAngle( float target, float value, float speed ) {
   target = ( target * 182.04445f ) * 0.0054931641f;
   value = ( value * 182.04445f ) * 0.0054931641f;

   // Speed is assumed to be positive
   if ( speed < 0 )
	  speed = -speed;

   float delta = target - value;
   if ( delta < -180.0f )
	  delta += 360.0f;
   else if ( delta > 180.0f )
	  delta -= 360.0f;

   if ( delta > speed )
	  value += speed;
   else if ( delta < -speed )
	  value -= speed;
   else
	  value = target;

   return value;
}
void Math::VectorTransform( const Vector& in1, const matrix3x4_t& in2, Vector& out ) {
   out[ 0 ] = in1.Dot( in2[ 0 ] ) + in2[ 0 ][ 3 ];
   out[ 1 ] = in1.Dot( in2[ 1 ] ) + in2[ 1 ][ 3 ];
   out[ 2 ] = in1.Dot( in2[ 2 ] ) + in2[ 2 ][ 3 ];
}

void Math::SmoothAngle( QAngle src, QAngle& dst, float factor ) {
   QAngle delta = dst - src;

   delta.Normalize( );

   dst = src + delta / factor;
}

QAngle Math::CalcAngle( Vector src, Vector dst ) {
   QAngle angles;
   Vector delta = src - dst;

   angles = delta.ToEulerAngles( );

   angles.Normalize( );

   return angles;
}

Vector Math::GetSmoothedVelocity( float min_delta, Vector a, Vector b ) {
   Vector delta = a - b;
   float delta_length = delta.Length( );

   if ( delta_length <= min_delta ) {
	  Vector result;
	  if ( -min_delta <= delta_length ) {
		 return a;
	  } else {
		 float iradius = 1.0f / ( delta_length + FLT_EPSILON );
		 return b - ( ( delta * iradius ) * min_delta );
	  }
   } else {
	  float iradius = 1.0f / ( delta_length + FLT_EPSILON );
	  return b + ( ( delta * iradius ) * min_delta );
   }
}

float Math::AngleDiff( float src, float dst ) {
   float i;

   for ( ; src > 180.0; src = src - 360.0 )
      ;
   for ( ; src < -180.0; src = src + 360.0 )
      ;
   for ( ; dst > 180.0; dst = dst - 360.0 )
      ;
   for ( ; dst < -180.0; dst = dst + 360.0 )
      ;
   for ( i = dst - src; i > 180.0; i = i - 360.0 )
      ;
   for ( ; i < -180.0; i = i + 360.0 )
      ;

   return i;
}
