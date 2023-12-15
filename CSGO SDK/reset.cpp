#include "Hooked.hpp"
#include "Render.hpp"

HRESULT __stdcall Hooked::Reset( IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters ) {
   g_Vars.globals.szLastHookCalled = XorStr( "31" );
   // lock render mutex
   Render::Get( )->BeginScene( );

   ImGui_ImplDX9_InvalidateDeviceObjects( );

   auto ret = oReset( pDevice, pPresentationParameters );

   if ( ret == D3D_OK ) {
	  ImGui_ImplDX9_CreateDeviceObjects( );
	  Render::Get( )->OnReset( );
   }

   Render::Get( )->EndScene( );

   return ret;
}
