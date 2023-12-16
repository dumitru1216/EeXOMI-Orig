#include "Hooked.hpp"
#include "LagCompensation.hpp"
#include "weapon.hpp"
#include "CBaseHandle.hpp"
#include "Render.hpp"
#include "PreserveKillfeed.hpp"

namespace Hooked {
	void __fastcall PaintTraverse( void* ecx, void* edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce ) {
		g_Vars.globals.szLastHookCalled = XorStr( "21" );
		std::string szPanelName = Source::m_pPanel->GetName( vguiPanel );

		static uint32_t HudZoomPanel;
		if ( !HudZoomPanel )
			if ( !szPanelName.compare( "HudZoom" ) )
				HudZoomPanel = vguiPanel;

		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

		if ( HudZoomPanel == vguiPanel && g_Vars.esp.remove_scope && LocalPlayer && LocalPlayer->m_hActiveWeapon( ).Get( ) ) {
			if ( ( ( C_WeaponCSBaseGun* )LocalPlayer->m_hActiveWeapon( ).Get( ) )->GetCSWeaponData( ).IsValid( ) && ( ( C_WeaponCSBaseGun* )LocalPlayer->m_hActiveWeapon( ).Get( ) )->GetCSWeaponData( )->m_iWeaponType == WEAPONTYPE_SNIPER_RIFLE && LocalPlayer->m_bIsScoped( ) )
				return;
		}

		// IPreserveKillfeed::Get( )->OnPaintTraverse( );

		oPaintTraverse( ecx, vguiPanel, forceRepaint, allowForce );

		if ( !szPanelName.empty( ) && !szPanelName.compare( "MatSystemTopPanel" ) ) {
			if ( g_Vars.esp.remove_scope && ( LocalPlayer && LocalPlayer->m_hActiveWeapon( ).Get( ) && ( ( C_WeaponCSBaseGun* )LocalPlayer->m_hActiveWeapon( ).Get( ) )->GetCSWeaponData( ).IsValid( ) && ( ( C_WeaponCSBaseGun* )LocalPlayer->m_hActiveWeapon( ).Get( ) )->GetCSWeaponData( )->m_iWeaponType == WEAPONTYPE_SNIPER_RIFLE && LocalPlayer->m_bIsScoped( ) ) ) {
				Source::m_pSurface->DrawSetColor( Color( 0, 0, 0, 255 ) );
				Vector2D center = Render::Get( )->GetScreenSize( );
				Source::m_pSurface->DrawLine( center.x / 2, 0, center.x / 2, center.y );
				Source::m_pSurface->DrawLine( 0, center.y / 2, center.x, center.y / 2 );
			}
		}
	}
}
