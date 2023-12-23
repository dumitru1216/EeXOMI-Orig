#include "Hooked.hpp"
#include "Menu.hpp"
#include <intrin.h>
#include "imgui_impl_win32.h"
#include "InputSys.hpp"
#include "Render.hpp"

bool run_once = false;
uint64_t Cookie = 0;

void SetCookie( ) {
   auto cs = ( uintptr_t ) Source::m_pClientState.Xor( );
   Cookie = *( uint64_t* ) ( cs + 0x3B8 + 0x8 );
}

void Reconnect( ) { // dont need this
   auto cs = ( uintptr_t ) Source::m_pClientState.Xor( );
   *( uint64_t* ) ( cs + 0x3B8 + 0x8 ) = Cookie;

   static auto client = GetModuleHandle( "client.dll" );
   auto& BanFlag = *( DWORD* ) ( ( DWORD ) client + ( DWORD ) 0x514ED1C );
   auto& xzchoeto = *( bool* ) ( ( DWORD ) client + ( DWORD ) 0x5150276 );//5257B68
   auto& setupinabandon = *( DWORD* ) ( ( DWORD ) client + ( DWORD ) 0x525E150 );

   DWORD tempBan = BanFlag |= 1;
   BanFlag = ( tempBan << 1 );
   xzchoeto = 1;
   if ( setupinabandon & 1 )
	  setupinabandon |= 0u;

   static auto ActionReconnectToOngoingMatchAddress = Memory::Scan( ( uintptr_t ) client, "A1 ?? ?? ?? ?? D1 E8 A8 01 74 4D" );

   typedef unsigned int( __stdcall * ActionReconnectToOngoingMatchFn )( uintptr_t, uintptr_t );
   ActionReconnectToOngoingMatchFn ActionReconnectToOngoingMatch = ( ActionReconnectToOngoingMatchFn ) ActionReconnectToOngoingMatchAddress;

   ActionReconnectToOngoingMatch( 0, 0 );
}

HRESULT __stdcall Hooked::EndScene( IDirect3DDevice9* pDevice ) {
   if ( !pDevice )
      goto end;

   g_Vars.globals.szLastHookCalled = XorStr( "8" );
   g_Vars.globals.m_pD3D9Device = pDevice;

   static auto wanted_ret_address = _ReturnAddress( );

   if ( g_Vars.misc.obs_bypass && _ReturnAddress( ) == wanted_ret_address )
	  goto end;

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

   IMenu::Get( )->Main( pDevice );

   //IMenu::Get( )->Main( pDevice );

   //if ( g_Vars.esp.stat_logger )
	  //IMenu::Get( )->StatFrameRender( );

   //if ( g_Vars.esp.spectator_list || g_Vars.esp.keybind_list )
	  //IMenu::Get( )->IndicatorsFrame( );

   ImGui::Render( );

   ImGui_ImplDX9_RenderDrawData( ImGui::GetDrawData( ) );

   state->Release( );
   state->Apply( );

   //pDevice->SetRenderState( D3DRS_SRGBWRITEENABLE, true );
   pDevice->SetVertexDeclaration( vertDec );
   pDevice->SetVertexShader( vertShader );

   for ( auto i = 0; i < size; i++ )
	  pDevice->SetRenderState( backup_states[ i ], old_states[ i ] );

//   // working btw
//#if 0
//   if ( Cookie == 0 )
//	  SetCookie( );
//
//   if ( InputSys::Get( )->WasKeyPressed( VirtualKeys::End ) )
//	  Reconnect( );
//#endif

end:
   return oEndScene( pDevice );
}
