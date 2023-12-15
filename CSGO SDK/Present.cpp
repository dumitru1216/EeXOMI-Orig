#include "Hooked.hpp"
#include "MenuV2.hpp"
#include <intrin.h>
#include "imgui_impl_win32.h"
#include "InputSys.hpp"
#include "Render.hpp"

HRESULT __stdcall Hooked::Present( LPDIRECT3DDEVICE9 pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion ) {
   g_Vars.globals.szLastHookCalled = XorStr( "27" );
   g_Vars.globals.m_pD3D9Device = pDevice;

   IDirect3DStateBlock9* state = nullptr;
   IDirect3DVertexDeclaration9* vertDec; IDirect3DVertexShader9* vertShader;

   DWORD colorwrite, srgbwrite;
   static const D3DRENDERSTATETYPE backup_states[] = { D3DRS_COLORWRITEENABLE, D3DRS_ALPHABLENDENABLE, D3DRS_SRCBLEND, D3DRS_DESTBLEND, D3DRS_BLENDOP, D3DRS_FOGENABLE };
   static const int size = sizeof( backup_states ) / sizeof( DWORD );
   pDevice->CreateStateBlock( D3DSBT_PIXELSTATE, &state );
   DWORD old_states[ size ] = { 0 };

   for ( auto i = 0; i < size; i++ )
	  pDevice->GetRenderState( backup_states[ i ], &old_states[ i ] );

   pDevice->GetVertexDeclaration( &vertDec );
   pDevice->GetVertexShader( &vertShader );

   // removes the source engine color correction
   pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0xffffffff );
   pDevice->SetRenderState( D3DRS_SRGBWRITEENABLE, false );

   pDevice->SetSamplerState( NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
   pDevice->SetSamplerState( NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
   pDevice->SetSamplerState( NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP );
   pDevice->SetSamplerState( NULL, D3DSAMP_SRGBTEXTURE, NULL );

   Render::Get( )->RenderScene( );

   ImGui_ImplDX9_NewFrame( );
   ImGui_ImplWin32_NewFrame( );
   ImGui::NewFrame( );

   ImGui::GetIO( ).MouseDrawCursor = g_Vars.globals.menuOpen;

   /*IMenu::Get( )->Main( pDevice );

   if ( g_Vars.esp.stat_logger )
	  IMenu::Get( )->StatFrameRender( );

   if ( g_Vars.esp.spectator_list || g_Vars.esp.keybind_list )
	  IMenu::Get( )->IndicatorsFrame( );*/

   ImGui::Render( );

   ImGui_ImplDX9_RenderDrawData( ImGui::GetDrawData( ) );

   state->Release( );
   state->Apply( );

   //pDevice->SetRenderState( D3DRS_SRGBWRITEENABLE, true );
   pDevice->SetVertexDeclaration( vertDec );
   pDevice->SetVertexShader( vertShader );

   for ( auto i = 0; i < size; i++ )
	  pDevice->SetRenderState( backup_states[ i ], old_states[ i ] );

   return oPresent( pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion );
}