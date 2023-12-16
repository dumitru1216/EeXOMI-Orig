#include "Hooked.hpp"
#include "Movement.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "InputSys.hpp"
#include "VisibilityOptimization.hpp"
#include "GrenadePrediction.hpp"
#include "PropManager.hpp"

void __fastcall Hooked::OverrideView( void* ECX, int EDX, CViewSetup* vsView ) {
   g_Vars.globals.szLastHookCalled = XorStr( "18" );
   C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );

   bool bOk = g_Vars.globals.RenderIsReady && vsView && local && !local->IsDead( ) && Source::m_pEngine->IsInGame( );

   if ( bOk ) {
	  IGrenadePrediction::Get( )->View( );
	  if ( g_Vars.antiaim.enabled && g_Vars.antiaim.manual && g_Vars.antiaim.mouse_override.enabled ) {
		 vsView->angles = g_Vars.globals.PreviousViewangles;
	  }

	  if ( g_Vars.misc.fakeduck && g_Vars.misc.fakeduck_bind.enabled ) {
		 Vector offset = Source::m_pGameMovement->GetPlayerViewOffset( false );
		 vsView->origin = local->GetAbsOrigin( ) + offset;
	  }

	  static auto blur_overlay = Source::m_pMatSystem->FindMaterial( XorStr( "dev/scope_bluroverlay" ), "Other textures" );
	  static auto scope_dirt = Source::m_pMatSystem->FindMaterial( XorStr( "models/weapons/shared/scope/scope_lens_dirt" ), "Other textures" );

	  blur_overlay->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, false );
	  scope_dirt->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, false );

	  auto weapon = ( C_WeaponCSBaseGun* ) ( local->m_hActiveWeapon( ).Get( ) );
	  if ( weapon ) {
		 auto weapon_data = weapon->GetCSWeaponData( );
		 if ( weapon_data.IsValid( ) ) {
			if ( local->m_bIsScoped( ) ) {
				blur_overlay->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, true );
				scope_dirt->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, true );

			   if ( g_Vars.esp.remove_scope_zoom ) {
				  if ( weapon->m_zoomLevel( ) == 2 ) {
					 vsView->fov = 45.0f;
				  } else {
					 vsView->fov = g_Vars.esp.world_fov;
				  }
			   }
			} else {
			   vsView->fov = g_Vars.esp.world_fov;
			}
		 }
	  }

	  if ( g_Vars.esp.zoom ) {
		 if ( g_Vars.esp.zoom_key.enabled )
			vsView->fov = g_Vars.esp.zoom_value;
	  }

	  Source::Movement::Get( )->ThirdPerson( );
   }

   oOverrideView( ECX, vsView );

   // NOTICE: this should be always called in the end, after all view manipulation
   if ( bOk ) {
	  auto local = C_CSPlayer::GetLocalPlayer( );

	  static auto m_nRenderMode = Engine::PropManager::Instance( ).GetProp( XorStr( "DT_BaseEntity" ), XorStr( "m_nRenderMode" ) );

	  for ( auto i = 1; i <= Source::m_pEntList->GetHighestEntityIndex( ); ++i ) {
		 auto entity = ( C_BaseEntity* ) Source::m_pEntList->GetClientEntity( i );
		 if ( entity == local )
			continue;

		 if ( !entity )
			continue;

		 auto player = ToCSPlayer( entity );
		 if ( player ) {
			if ( player->IsTeammate( local ) ) {
			   *( uint8_t* ) ( uintptr_t( player ) + m_nRenderMode ) = g_Vars.misc.disable_teammates ? 10 : 0;
			} else {
			   *( uint8_t* ) ( uintptr_t( player ) + m_nRenderMode ) = g_Vars.misc.disable_enemies ? 10 : 0;
			}
		 } else if ( entity->IsWeapon( ) ) {
			*( uint8_t* ) ( uintptr_t( entity ) + m_nRenderMode ) = g_Vars.misc.disable_weapons ? 10 : 0;
		 } else if ( entity->GetClientClass( )->m_ClassID == CCSRagdoll ) {
			*( uint8_t* ) ( uintptr_t( entity ) + m_nRenderMode ) = g_Vars.misc.disable_ragdolls ? 10 : 0;
		 }
	  }

	  Engine::VisibilityOptimization::Get( )->Update( vsView );
   }
}
