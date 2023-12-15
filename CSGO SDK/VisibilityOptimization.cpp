#include "VisibilityOptimization.hpp"
#include "source.hpp"
#include "Player.hpp"
#include "Render.hpp"

// TODO: move this to qangle
float AngleDistance( QAngle& angles, const Vector& start, const Vector& end ) {
   auto direction = end - start;
   direction.Normalize( );

   auto forward = angles.ToVectors( );
   auto dot = forward.Dot( direction );

   return ToDegrees( std::acos( dot ) );
}

namespace Engine
{
   class C_VisibilityOptimization : public VisibilityOptimization {
   public:
	  C_VisibilityOptimization( ) { }
	  virtual ~C_VisibilityOptimization( ) { }

	  virtual void Update( CViewSetup* view ); // call on overrideview
   };

   VisibilityOptimization* VisibilityOptimization::Get( ) {
	  static C_VisibilityOptimization instance;
	  return &instance;
   }

   void C_VisibilityOptimization::Update( CViewSetup* view ) {
	  auto local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local )
		 return;

	  auto display = Render::Get( )->GetScreenSize( );
	  auto ratio = display.x / display.y;
	  auto screen_fov = ToDegrees( atanf( ( ratio ) * ( 0.75f ) * tan( ToRadians( view->fov * 0.5f ) ) ) );

	  for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; ++i ) {
		 auto player = C_CSPlayer::GetPlayerByIndex( i );
		 if ( player == local )
			continue;

		 if ( !player || player->IsDormant( ) || player->IsDead( ) )
			continue;

		 Vector min, max;
		 if ( !player->ComputeHitboxSurroundingBox( min, max ) )
			continue;

		 Vector points[] =
		 {
			Vector( min.x, min.y, min.z ),
			Vector( min.x, max.y, min.z ),
			Vector( max.x, max.y, min.z ),
			Vector( max.x, min.y, min.z ),
			Vector( max.x, max.y, max.z ),
			Vector( min.x, max.y, max.z ),
			Vector( min.x, min.y, max.z ),
			Vector( max.x, min.y, max.z )
		 };

		 float min_fov = std::numeric_limits<float>::max( );
		 for ( int i = 0; i < 8; ++i ) {
			float fov = AngleDistance( view->angles, view->origin, points[ i ] );
			if ( min_fov > fov )
			   min_fov = fov;
		 }

		 player->m_bShouldDraw( ) = min_fov <= screen_fov;
	  }
   }
}
