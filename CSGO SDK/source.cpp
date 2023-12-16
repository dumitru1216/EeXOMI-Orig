#include "source.hpp"

#include "Hooked.hpp"
#include "InputSys.hpp"

#include "PropManager.hpp"
#include "Displacement.hpp"

#include "Player.hpp"

#include "Glow.hpp"
#include "GameEvent.hpp"

#include "Render.hpp"
#include "SkinChanger.hpp"
#include "KitParser.hpp"

#include "EXGui/main/setup.hpp"
#include "MenuV2.hpp"

#include "hooker.hpp"

#include "CChams.hpp"

#include "Prediction.hpp"

#include <fstream>
#include "License.hpp"

extern ClientClass* CCSPlayerClass;
extern CreateClientClassFn oCreateCCSPlayer;

struct CEventInfo {
   uint16_t classID;
   char pad_0002[ 2 ];
   float fire_delay;
   char pad_0008[ 4 ];
   void* pClientClass;
   void* pData;
   char pad_0014[ 36 ];
   CEventInfo* next;
   char pad_0038[ 8 ];
};

//56 8D 51 3C BE
matrix3x4_t g_HeadBone;

using FnProcessInterpolatedList = void( __cdecl* )( );
FnProcessInterpolatedList oProcessInterpolatedList;
void __cdecl hkProcessInterpolatedList( ) {
   g_Vars.globals.szLastHookCalled = XorStr( "38" );
   **( bool** ) ( Engine::Displacement.Data.s_bAllowExtrapolation ) = false;
   oProcessInterpolatedList( );
}

using FnResetGetWaterContentsForPointCache = void( __thiscall* )( void* );
FnResetGetWaterContentsForPointCache oResetGetWaterContentsForPointCache;
void __fastcall hkResetGetWaterContentsForPointCache( void* ecx, void* edx ) {
   g_Vars.globals.szLastHookCalled = XorStr( "39" );
   if ( !Engine::Prediction::Instance( )->InPrediction( ) )
	  oResetGetWaterContentsForPointCache( ecx );

   return;
}

matrix3x4_t HeadBone;

using FnModifyEyePosition = void( __thiscall* )( C_CSPlayer*, Vector* );
FnModifyEyePosition oModifyEyePoisition;
void __fastcall hkModifyEyePosition( C_CSPlayer* ecx, void* edx, Vector* eye_position ) {
   g_Vars.globals.szLastHookCalled = XorStr( "40" );
   auto local_player = C_CSPlayer::GetLocalPlayer( );
   if ( !local_player ) {
	  oModifyEyePoisition( ecx, eye_position );
	  return;
   }

   if ( g_Vars.globals.m_bInCreateMove )
	  oModifyEyePoisition( ecx, eye_position );

   return;

   /*auto GetBoneIndex = [local_player] ( int hitbox ) {
	  auto model = local_player->GetModel( );
	  if ( !model )
		 return -1;
	  auto hdr = Source::m_pModelInfo->GetStudiomodel( model );
	  if ( !hdr )
		 return -1;
	  auto set = hdr->pHitboxSet( local_player->m_nHitboxSet( ) );
	  return set->pHitbox( hitbox )->bone;
   };

   auto bone_idx = GetBoneIndex( HITBOX_HEAD );
   if ( bone_idx == -1 )
	  return oModifyEyePoisition( ecx, eye_position );

   matrix3x4_t bone = local_player->m_CachedBoneData( ).Element( bone_idx );
   if ( g_Vars.fakelag.enabled || g_Vars.antiaim.enabled ) {
	  local_player->m_CachedBoneData( ).Element( bone_idx ) = HeadBone;
   }

   oModifyEyePoisition( ecx, eye_position );

   if ( g_Vars.fakelag.enabled || g_Vars.antiaim.enabled ) {
	  local_player->m_CachedBoneData( ).Element( bone_idx ) = bone;
   }*/
};

using FnAddBoxOverlay = void( __thiscall* )( void*, const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, int r, int g, int b, int a, float duration );
FnAddBoxOverlay oAddBoxOverlay;
void __fastcall hkAddBoxOverlay( void* ecx, void* edx, const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, int r, int g, int b, int a, float duration ) {
   g_Vars.globals.szLastHookCalled = XorStr( "41" );

   static uintptr_t fire_bulltes = 0;
   if ( !fire_bulltes )
	  fire_bulltes = Memory::Scan( XorStr( "client.dll" ), XorStr( "3B 3D ?? ?? ?? ?? 75 4C" ) );

   if ( !g_Vars.misc.impacts_spoof || uintptr_t( _ReturnAddress( ) ) != fire_bulltes )
	  return oAddBoxOverlay( ecx, origin, mins, max, orientation, r, g, b, a, duration );

   return oAddBoxOverlay( ecx, origin, mins, max, orientation,
						  g_Vars.esp.client_impacts.r * 255, g_Vars.esp.client_impacts.g * 255, g_Vars.esp.client_impacts.b * 255, g_Vars.esp.client_impacts.a * 255,
						  duration );
}

using FnFireEvents = void( __thiscall* )( void* );
FnFireEvents oFireEvents;
void __fastcall hkFireEvents( void* ecx, void* edx ) {
   g_Vars.globals.szLastHookCalled = XorStr( "42" );
   auto local = C_CSPlayer::GetLocalPlayer( );
   if ( local && !local->IsDead( ) ) {
	  for ( CEventInfo* i = *( CEventInfo** ) ( uintptr_t( Source::m_pClientState.Xor( ) ) + 0x4DEC );
			i != nullptr;
			i = i->next ) {
		 if ( i->classID - 1 == CTEFireBullets )
			i->fire_delay = 0.0f;

	  }
   }

   oFireEvents( ecx );
}

using net_showfragments_t = bool( __thiscall* )( void* );
net_showfragments_t o_net_show_fragments;
bool __fastcall net_show_fragments( void* cvar, void* edx ) {
   g_Vars.globals.szLastHookCalled = XorStr( "43" );

   C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );

   if ( !pLocal || pLocal->IsDead( ) )
	  return o_net_show_fragments( cvar );

   if ( !Source::m_pEngine->IsInGame( ) || !g_Vars.misc.extended_backtrack || !g_Vars.misc.active )
	  return o_net_show_fragments( cvar );

   auto netchannel = Encrypted_t<INetChannel>( Source::m_pEngine->GetNetChannelInfo( ) );
   if ( !netchannel.IsValid( ) )
	  return o_net_show_fragments( cvar );

   static uint32_t last_fragment = 0;

   if ( _ReturnAddress( ) == ( uint32_t* ) ( Engine::Displacement.Data.CheckReceivingListReturn ) && last_fragment > 0 ) {
	  const auto data = &reinterpret_cast< uint32_t* >( netchannel.Xor( ) )[ 0x56 ]; // + 0x9
	  const auto bytes_fragments = reinterpret_cast< uint32_t* >( data )[ 0x43 ];

	  if ( bytes_fragments == last_fragment ) {
		 auto& buffer = reinterpret_cast< uint32_t* >( data )[ 0x42 ];
		 buffer = 0;
	  }
   }

   if ( _ReturnAddress( ) == ( uint32_t* ) ( Engine::Displacement.Data.CheckReceivingListReturn ) ) {
	  const auto data = &reinterpret_cast< uint32_t* >( netchannel.Xor( ) )[ 0x56 ]; // + 0x9
	  const auto bytes_fragments = reinterpret_cast< uint32_t* >( data )[ 0x43 ];

	  last_fragment = bytes_fragments;
   }

   return o_net_show_fragments( cvar );
}

struct MaterialSystemConfig_t {
   int m_Width;
   int m_Height;
   int m_Format;
   int m_RefreshRate;

   float m_fMonitorGamma;
   float m_fGammaTVRangeMin;
   float m_fGammaTVRangeMax;
   float m_fGammaTVExponent;
   bool m_bGammaTVEnabled;
   bool m_bTripleBuffered;
   int m_nAASamples;
   int m_nForceAnisotropicLevel;
   int m_nSkipMipLevels;
   int m_nDxSupportLevel;
   int m_nFlags;
   bool m_bEditMode;
   char m_nProxiesTestMode;
   bool m_bCompressedTextures;
   bool m_bFilterLightmaps;
   bool m_bFilterTextures;
   bool m_bReverseDepth;
   bool m_bBufferPrimitives;
   bool m_bDrawFlat;
   bool m_bMeasureFillRate;
   bool m_bVisualizeFillRate;
   bool m_bNoTransparency;
   bool m_bSoftwareLighting;
   bool m_bAllowCheats;
   char m_nShowMipLevels;
   bool m_bShowLowResImage;
   bool m_bShowNormalMap;
   bool m_bMipMapTextures;
   char m_nFullbright;
   bool m_bFastNoBump;
   bool m_bSuppressRendering;
   bool m_bDrawGray;
   bool m_bShowSpecular;
   bool m_bShowDiffuse;
   int m_nWindowedSizeLimitWidth;
   int m_nWindowedSizeLimitHeight;
   int m_nAAQuality;
   bool m_bShadowDepthTexture;
   bool m_bMotionBlur;
   bool m_bSupportFlashlight;
   bool m_bPaintEnabled;
   char pad[ 0xC ];
};

using FnOverrideConfig = bool( __thiscall* )( IMaterialSystem*, MaterialSystemConfig_t&, bool );
FnOverrideConfig oOverrideConfig;
bool __fastcall OverrideConfig( IMaterialSystem* ecx, void* edx, MaterialSystemConfig_t& config, bool bForceUpdate ) {
   g_Vars.globals.szLastHookCalled = XorStr( "44" );
   if ( ecx == Source::m_pMatSystem.Xor( ) && g_Vars.esp.fullbright ) {
	  config.m_nFullbright = true;
   }

   return oOverrideConfig( ecx, config, bForceUpdate );
}

enum {
   NO_SCOPE_NONE,
   NO_SCOPE_STATIC,
   NO_SCOPE_DYNAMIC
};

int no_scope_mode = NO_SCOPE_STATIC;

using FnDrawSetColor = void( __thiscall* )( void*, int, int, int, int );
FnDrawSetColor oDrawSetColor;
void __fastcall DrawSetColor( ISurface* thisptr, void* edx, int r, int g, int b, int a ) {
   g_Vars.globals.szLastHookCalled = XorStr( "45" );
   if ( !g_Vars.esp.esp_enable || !g_Vars.esp.remove_scope ) {
	  return oDrawSetColor( thisptr, r, g, b, a );
   }

   const auto return_address = uintptr_t( _ReturnAddress( ) );

   static auto return_to_scope_arc = Memory::Scan( XorStr( "client.dll" ), XorStr( "6A 00 FF 50 3C 8B 0D ? ? ? ? FF B7" ) ) + 5;
   static auto return_to_scope_lens = Memory::Scan( XorStr( "client.dll" ), XorStr( "FF 50 3C 8B 4C 24 20" ) ) + 3;

   static auto return_to_scope_lines_clear = Memory::Scan( XorStr( "client.dll" ), XorStr( "0F 82 ? ? ? ? FF 50 3C" ) ) + 0x9;
   static auto return_to_scope_lines_blurry = Memory::Scan( XorStr( "client.dll" ), XorStr( "FF B7 ? ? ? ? 8B 01 FF 90 ? ? ? ? 8B 4C 24 24" ) ) - 0x6;

   if ( return_address == return_to_scope_arc
		|| return_address == return_to_scope_lens ) {
	  // We don't want this to draw, so we set the alpha to 0
	  return oDrawSetColor( thisptr, r, g, b, 0 );
   }

   if ( no_scope_mode == NO_SCOPE_DYNAMIC ||
		( return_address != return_to_scope_lines_clear &&
		return_address != return_to_scope_lines_blurry ) )
	  return oDrawSetColor( thisptr, r, g, b, a );

   // We will now draw our own scope
   const auto screen_size = Render::Get( )->GetScreenSize( );
   oDrawSetColor( thisptr, 0, 0, 0, 255 );
   thisptr->DrawLine( screen_size.x * 0.5, 0, screen_size.x * 0.5, screen_size.y );
   thisptr->DrawLine( 0, screen_size.y * 0.5, screen_size.x, screen_size.y * 0.5 );

   oDrawSetColor( thisptr, r, g, b, g_Vars.esp.remove_scope_blur ? 0 : a );
}

using FnProcessMovement = void( __thiscall* )( void*, int, uint8_t* );
FnProcessMovement oProcessMovement;
void __fastcall hkProcessMovement( void* pThis, void* edx, int player, uint8_t* moveData ) {
   g_Vars.globals.szLastHookCalled = XorStr( "46" );
   /*if ( !pThis || !edx )
	  return;*/

   //*moveData &= 253u;
   oProcessMovement( pThis, player, moveData );
}

using FnDoExtraBonesProccesing = void( __thiscall* )( C_CSPlayer*, CStudioHdr*, Vector*, Quaternion*, matrix3x4_t*, void*, void* );
FnDoExtraBonesProccesing oDoExtraBonesProccesing;
void __fastcall DoExtraBonesProccesing( C_CSPlayer* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* rotations, matrix3x4_t* transforma, void* bone_list, void* ik_context ) {
   g_Vars.globals.szLastHookCalled = XorStr( "22" );

   if ( ecx->m_fEffects( ) & 8 )
	  return;

   auto animState = ecx->m_PlayerAnimState( );

   if ( !animState )
	  oDoExtraBonesProccesing( ecx, hdr, pos, rotations, transforma, bone_list, ik_context );

   const auto backup_tickcount = *reinterpret_cast< int32_t* >( animState + 8 );
   *reinterpret_cast< int32_t* >( animState + 8 ) = 0;
   oDoExtraBonesProccesing( ecx, hdr, pos, rotations, transforma, bone_list, ik_context );
   *reinterpret_cast< int32_t* >( animState + 8 ) = backup_tickcount;
}

using PhysicsSimulateFn = void( __thiscall* ) ( void* ecx );
PhysicsSimulateFn oPhysicsSimulate;
void __fastcall hkPhysicsSimulate( void* ecx, void* edx ) {
   auto local = C_CSPlayer::GetLocalPlayer( );

   if ( ecx && local && !local->IsDead( ) && local == ecx ) {
	  int m_nSimulationTick = *( int* ) ( uintptr_t( ecx ) + 0x2AC );
	  if ( Source::m_pGlobalVars->tickcount != m_nSimulationTick )
		 Engine::Prediction::Instance( )->ProccesingNetvarCompresion( );
   }

   oPhysicsSimulate( ecx );
}

typedef void( __thiscall* fnStandardBlendingRules )( void*, CStudioHdr*, Vector*, Quaternion*, float currentTime, int boneMask );
fnStandardBlendingRules oStandardBlendingRules;
void __fastcall hkStandardBlendingRules( C_CSPlayer* player, uint32_t, CStudioHdr* hdr, Vector* pos, Quaternion* q, float currentTime, int boneMask ) {
   auto local = C_CSPlayer::GetLocalPlayer( );
   if ( local && player && player->entindex( ) <= 64 && player != local ) {
	  oStandardBlendingRules( player, hdr, pos, q, currentTime, boneMask );

	  if ( player->m_iEFlags( ) & EF_NOINTERP )
		 player->m_iEFlags( ) &= ~EF_NOINTERP;
   } else
	  oStandardBlendingRules( player, hdr, pos, q, currentTime, boneMask );
}

typedef bool( __thiscall* fnIsHltv )( IVEngineClient* );
fnIsHltv oIsHltv;
bool hkIsHltv( IVEngineClient* EngineClient, uint32_t ) {
   static const auto return_to_setup_velocity = Memory::Scan( XorStr( "client.dll" ), XorStr( "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80" ) );

   //if ( _ReturnAddress( ) == ( void* ) return_to_setup_velocity && g_Vars.globals.m_bUpdatingAnimations && Source::m_pEngine->IsInGame( ) )
	  //return true;

   return oIsHltv( EngineClient );
}

#if 0
using InvalidatePhysicsRecursive_t = void( __thiscall* )( uintptr_t, int );
InvalidatePhysicsRecursive_t oInvalidatePhysicsRecursive;
void __fastcall hkInvalidatePhysicsRecursive( uintptr_t ecx, void*, int nChangeFlags ) {
   if ( !ecx || !*( int* ) ( uintptr_t( ecx ) ) || !*( int* ) ( uintptr_t( ecx ) + 0x4 ) )
	  return;

   return oInvalidatePhysicsRecursive( ecx, nChangeFlags );
};
#endif

namespace Source
{
   Encrypted_t<IBaseClientDLL> m_pClient = nullptr;
   Encrypted_t<IClientEntityList> m_pEntList = nullptr;
   Encrypted_t<IGameMovement> m_pGameMovement = nullptr;
   Encrypted_t<IPrediction> m_pPrediction = nullptr;
   Encrypted_t<IMoveHelper> m_pMoveHelper = nullptr;
   Encrypted_t<IInput> m_pInput = nullptr;
   Encrypted_t<CGlobalVars>  m_pGlobalVars = nullptr;
   Encrypted_t<ISurface> m_pSurface = nullptr;
   Encrypted_t<IVEngineClient> m_pEngine = nullptr;
   Encrypted_t<IClientMode> m_pClientMode = nullptr;
   Encrypted_t<ICVar> m_pCvar = nullptr;
   Encrypted_t<IPanel> m_pPanel = nullptr;
   Encrypted_t<IGameEventManager> m_pGameEvent = nullptr;
   Encrypted_t<IVModelRender> m_pModelRender = nullptr;
   Encrypted_t<IMaterialSystem> m_pMatSystem = nullptr;
   Encrypted_t<ISteamClient> g_pSteamClient = nullptr;
   Encrypted_t<ISteamGameCoordinator> g_pSteamGameCoordinator = nullptr;
   Encrypted_t<ISteamMatchmaking> g_pSteamMatchmaking = nullptr;
   Encrypted_t<ISteamUser> g_pSteamUser = nullptr;
   Encrypted_t<ISteamFriends> g_pSteamFriends = nullptr;
   Encrypted_t<IPhysicsSurfaceProps> m_pPhysSurface = nullptr;
   Encrypted_t<IEngineTrace> m_pEngineTrace = nullptr;
   Encrypted_t<CGlowObjectManager> m_pGlowObjManager = nullptr;
   Encrypted_t<IVModelInfo> m_pModelInfo = nullptr;
   Encrypted_t<CClientState>  m_pClientState = nullptr;
   Encrypted_t<IVDebugOverlay> m_pDebugOverlay = nullptr;
   Encrypted_t<IEngineSound> m_pEngineSound = nullptr;
   Encrypted_t<IMemAlloc> m_pMemAlloc = nullptr;
   Encrypted_t<IViewRenderBeams> m_pRenderBeams = nullptr;
   Encrypted_t<ILocalize> m_pLocalize = nullptr;
   Encrypted_t<IStudioRender> m_pStudioRender = nullptr;
   Encrypted_t<CSPlayerResource*> m_pPlayerResource = nullptr;
   Encrypted_t<ICenterPrint> m_pCenterPrint = nullptr;
   Encrypted_t<IVRenderView> m_pRenderView = nullptr;
   Encrypted_t<IClientLeafSystem> m_pClientLeafSystem = nullptr;
   Encrypted_t<IMDLCache> m_pMDLCache = nullptr;
   Encrypted_t<IViewRender> m_pViewRender = nullptr;

   WNDPROC oldWindowProc;
   HWND hWindow = nullptr;

#if 0
   Memory::VmtSwap::Shared m_pClientSwap = nullptr;
   Memory::VmtSwap::Shared m_pClientModeSwap = nullptr;
   Memory::VmtSwap::Shared m_pPredictionSwap = nullptr;
   Memory::VmtSwap::Shared m_pPanelSwap = nullptr;
   Memory::VmtSwap::Shared m_pD3D9Swap = nullptr;
   Memory::VmtSwap::Shared m_pResetSwap = nullptr;
   Memory::VmtSwap::Shared m_pVguiSurfaceSwap = nullptr;
   Memory::VmtSwap::Shared m_pGameEventSwap = nullptr;
   Memory::VmtSwap::Shared m_pMatSystemSwap = nullptr;
   Memory::VmtSwap::Shared m_pBSPQuerySwap = nullptr;
   Memory::VmtSwap::Shared m_pClientStateSwap = nullptr;
   Memory::VmtSwap::Shared m_pNetChannelSwap = nullptr;
   Memory::VmtSwap::Shared m_pStudioRenderSwap = nullptr;
   Memory::VmtSwap::Shared m_pEngineSoundSwap = nullptr;
   Memory::VmtSwap::Shared m_pEngineSwap = nullptr;
#endif

   RecvPropHook::Shared m_pDidSmokeEffectSwap = nullptr;
   RecvPropHook::Shared m_pFlAbsYawSwap = nullptr;
   RecvPropHook::Shared m_pPlaybackRateSwap = nullptr;

   void m_bDidSmokeEffect( const CRecvProxyData* pData, void* pStruct, void* pOut ) {
	  Source::m_pDidSmokeEffectSwap->GetOriginalFunction( )( pData, pStruct, pOut );

	  if ( g_Vars.esp.remove_smoke )
		 *( uintptr_t* ) ( ( uintptr_t ) pOut ) = true;
   }

#if 0
   DWORD UnkAnimation;
   uintptr_t AnimSystemOriginal = 0;
   int __cdecl hkAnimationSystem( uintptr_t unk ) {
	  // check if disable interpolation for correct animation 
	  if ( ( *( char** ) ( unk + 1036 ) )[ 0 ] != XorStr( 'l' )
		   && ( *( char** ) ( unk + 1036 ) )[ 2 ] != XorStr( 'c' )
		   && ( *( char** ) ( unk + 1036 ) )[ 4 ] != XorStr( 'l' ) ) {
		 // disable interpolation for correct animation
		 int data[] = { 544235885, 1952412276, 1852399986, 1667722857, 1702129253, 114, 0 };

		 Memory::VCall<void( __thiscall* )( void*, int* )>( Source::m_pEngine, 108 )( Source::m_pEngine,
																					  data );
		 return 0;
	  } else {
		 return ( ( decltype( &hkAnimationSystem ) ) UnkAnimation )( unk );
	  }
   }
#endif

   bool Create( void* reserved ) {
	  auto& pPropManager = Engine::PropManager::Instance( );

	  m_pClient = ( IBaseClientDLL* ) CreateInterface( XorStr( "client.dll" ), XorStr( "VClient" ) );

	  if ( !m_pClient.IsValid( ) ) {
		 Win32::Error( XorStr( "IBaseClientDLL is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  if ( !pPropManager->Create( m_pClient.Xor( ) ) ) {
		 Win32::Error( XorStr( "Engine::PropManager::Create failed (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pEntList = ( IClientEntityList* ) CreateInterface( XorStr( "client.dll" ), XorStr( "VClientEntityList" ) );

	  if ( !m_pEntList.IsValid( ) ) {
		 Win32::Error( XorStr( "IClientEntityList is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pGameMovement = ( IGameMovement* ) CreateInterface( XorStr( "client.dll" ), XorStr( "GameMovement" ) );

	  if ( !m_pGameMovement.IsValid( ) ) {
		 Win32::Error( XorStr( "IGameMovement is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pPrediction = ( IPrediction* ) CreateInterface( XorStr( "client.dll" ), XorStr( "VClientPrediction" ) );

	  if ( !m_pPrediction.IsValid( ) ) {
		 Win32::Error( XorStr( "IPrediction is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pInput = *reinterpret_cast< IInput** > ( ( *reinterpret_cast< uintptr_t** >( m_pClient.Xor( ) ) )[ 15 ] + 0x1 );
	  if ( !m_pInput.IsValid( ) ) {
		 Win32::Error( XorStr( "IInput is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pGlobalVars = ( **reinterpret_cast< CGlobalVars*** >( ( *reinterpret_cast< uintptr_t** >( m_pClient.Xor( ) ) )[ 0 ] + 0x1B ) );

	  if ( !m_pGlobalVars.IsValid( ) ) {
		 Win32::Error( XorStr( "CGlobalVars is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pEngine = ( IVEngineClient* ) CreateInterface( XorStr( "engine.dll" ), XorStr( "VEngineClient" ) );

	  if ( !m_pEngine.IsValid( ) ) {
		 Win32::Error( XorStr( "IVEngineClient is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pPanel = ( IPanel* ) CreateInterface( XorStr( "vgui2.dll" ), XorStr( "VGUI_Panel" ) );

	  if ( !m_pPanel.IsValid( ) ) {
		 Win32::Error( XorStr( "IPanel is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pSurface = ( ISurface* ) CreateInterface( XorStr( "vguimatsurface.dll" ), XorStr( "VGUI_Surface" ) );

	  if ( !m_pSurface.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pSurface is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pClientMode = **( IClientMode*** ) ( ( *( DWORD** ) m_pClient.Xor( ) )[ 10 ] + 0x5 );

	  if ( !m_pClientMode.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pClientMode is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pCvar = ( ICVar* ) CreateInterface( XorStr( "vstdlib.dll" ), XorStr( "VEngineCvar" ) );

	  if ( !m_pCvar.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pCvar is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pGameEvent = ( IGameEventManager* ) CreateInterface( XorStr( "engine.dll" ), XorStr( "GAMEEVENTSMANAGER002" ), true );

	  if ( !m_pGameEvent.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pGameEvent is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pModelRender = ( IVModelRender* ) CreateInterface( XorStr( "engine.dll" ), XorStr( "VEngineModel" ) );

	  if ( !m_pModelRender.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pModelRender is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pMatSystem = ( IMaterialSystem* ) CreateInterface( XorStr( "materialsystem.dll" ), XorStr( "VMaterialSystem" ) );

	  if ( !m_pMatSystem.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pMatSystem is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pPhysSurface = ( IPhysicsSurfaceProps* ) CreateInterface( XorStr( "vphysics.dll" ), XorStr( "VPhysicsSurfaceProps" ) );

	  if ( !m_pPhysSurface.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pPhysSurface is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pEngineTrace = ( IEngineTrace* ) CreateInterface( XorStr( "engine.dll" ), XorStr( "EngineTraceClient" ) );

	  if ( !m_pEngineTrace.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pEngineTrace is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  if ( !Engine::CreateDisplacement( reserved ) ) {
		 Win32::Error( XorStr( "Engine::CreateCreateDisplacement failed (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pMoveHelper = ( IMoveHelper* ) ( Engine::Displacement.Data.m_uMoveHelper );

	  if ( !m_pMoveHelper.IsValid( ) ) {
		 Win32::Error( XorStr( "IMoveHelper is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pGlowObjManager = ( CGlowObjectManager* ) Engine::Displacement.Data.m_uGlowObjectManager;

	  if ( !m_pGlowObjManager.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pGlowObjManager is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pModelInfo = ( IVModelInfo* ) CreateInterface( XorStr( "engine.dll" ), XorStr( "VModelInfoClient" ) );

	  if ( !m_pModelInfo.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pModelInfo is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  // A1 FC BC 58 10  mov eax, g_ClientState
	  m_pClientState = Encrypted_t<CClientState>( **( CClientState*** )( ( *( std::uintptr_t** )m_pEngine.Xor( ) )[ 14 ] + 0x1 ) );
	  if ( !m_pClientState.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pClientState is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pDebugOverlay = ( IVDebugOverlay* ) CreateInterface( XorStr( "engine.dll" ), XorStr( "VDebugOverlay" ) );

	  if ( !m_pDebugOverlay.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pDebugOverlay is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pMemAlloc = *( IMemAlloc** ) ( GetProcAddress( GetModuleHandle( XorStr( "tier0.dll" ) ), XorStr( "g_pMemAlloc" ) ) );

	  if ( !m_pMemAlloc.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pMemAlloc is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pEngineSound = ( IEngineSound* ) CreateInterface( XorStr( "engine.dll" ), XorStr( "IEngineSoundClient" ) );

	  if ( !m_pEngineSound.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pEngineSound is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pRenderBeams = *( IViewRenderBeams** ) ( Engine::Displacement.Data.m_uRenderBeams );

	  if ( !m_pRenderBeams.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pRenderBeams is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pLocalize = ( ILocalize* ) CreateInterface( XorStr( "localize.dll" ), XorStr( "Localize_" ) );

	  if ( !m_pLocalize.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pLocalize is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pStudioRender = ( IStudioRender* ) CreateInterface( XorStr( "studiorender.dll" ), XorStr( "VStudioRender" ) );

	  if ( !m_pStudioRender.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pStudioRender is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pCenterPrint = *( ICenterPrint** ) ( Engine::Displacement.Data.m_uCenterPrint );

	  if ( !m_pCenterPrint.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pCenterPrint is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pRenderView = ( IVRenderView* ) CreateInterface( XorStr( "engine.dll" ), XorStr( "VEngineRenderView" ) );

	  if ( !m_pRenderView.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pRenverView is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pClientLeafSystem = ( IClientLeafSystem* ) CreateInterface( XorStr( "client.dll" ), XorStr( "ClientLeafSystem" ) );

	  if ( !m_pClientLeafSystem.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pClientLeafSystem is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pMDLCache = ( IMDLCache* ) CreateInterface( XorStr( "datacache.dll" ), XorStr( "MDLCache" ) );

	  if ( !m_pMDLCache.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pMDLCache is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  m_pViewRender = **( IViewRender*** )( Memory::Scan( XorStr( "client.dll" ), XorStr( "FF 50 4C 8B 06 8D 4D F4" ) ) - 6 );

	  if ( !m_pViewRender.IsValid( ) ) {
		 Win32::Error( XorStr( "m_pViewRender is nullptr (Source::%s)" ), __FUNCTION__ );
		 return false;
	  }

	  auto D3DDevice9 = **( IDirect3DDevice9*** ) Engine::Displacement.Data.m_D3DDevice;
	  if ( !D3DDevice9 )
		 return false;

	  // pMethod
	  D3DDEVICE_CREATION_PARAMETERS params;
	  D3DDevice9->GetCreationParameters( &params );
	  hWindow = params.hFocusWindow;

	  if ( !InputSys::Get( )->Initialize( D3DDevice9 ) ) {
		 return false;
	  }

	  g_pSteamClient = ( ( ISteamClient * ( __cdecl* )( void ) )GetProcAddress( GetModuleHandle( XorStr( "steam_api.dll" ) ), XorStr( "SteamClient" ) ) )( );
	  HSteamUser hSteamUser = ( ( HSteamUser( __cdecl* )( void ) )GetProcAddress( GetModuleHandle( XorStr( "steam_api.dll" ) ), XorStr( "SteamAPI_GetHSteamUser" ) ) )( );
	  HSteamUser hSteamPipe = ( ( HSteamPipe( __cdecl* )( void ) )GetProcAddress( GetModuleHandle( XorStr( "steam_api.dll" ) ), XorStr( "SteamAPI_GetHSteamPipe" ) ) )( );
	  g_pSteamGameCoordinator = ( ISteamGameCoordinator* ) g_pSteamClient->GetISteamGenericInterface( hSteamUser, hSteamPipe, XorStr( "SteamGameCoordinator001" ) );
	  g_pSteamMatchmaking = ( ISteamMatchmaking* ) g_pSteamClient->GetISteamMatchmaking( hSteamUser, hSteamPipe, XorStr( "SteamMatchMaking009" ) );
	  g_pSteamUser = g_pSteamClient->GetISteamUser( hSteamPipe, hSteamUser, XorStr( "SteamUser019" ) );
	  g_pSteamFriends = g_pSteamClient->GetISteamFriends( hSteamPipe, hSteamUser, XorStr( "SteamFriends015" ) );

	  Render::Get( )->Initialize( D3DDevice9 );

	  MenuV2::Get( )->Initialize( D3DDevice9 );

	  GameEvent::Get( )->Register( );

	  initialize_kits( );

	  ISkinChanger::Get( )->Create( );

	  for ( auto clientclass = Source::m_pClient->GetAllClasses( );
			clientclass != nullptr;
			clientclass = clientclass->m_pNext ) {
		 if ( !strcmp( clientclass->m_pNetworkName, XorStr( "CCSPlayer" ) ) ) {
			CCSPlayerClass = clientclass;
			oCreateCCSPlayer = ( CreateClientClassFn ) clientclass->m_pCreateFn;

			clientclass->m_pCreateFn = Hooked::hkCreateCCSPlayer;
			break;
		 }
	  }

	  if ( Source::m_pEngine->IsInGame( ) ) {
		 for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; ++i ) {
			auto entity = C_CSPlayer::GetPlayerByIndex( i );
			if ( !entity || !entity->IsPlayer( ) )
			   continue;

			auto& new_hook = Hooked::player_hooks[ i ];
			new_hook.clientHook.Create( entity );
			new_hook.renderableHook.Create( ( void* ) ( ( uintptr_t ) entity + 0x4 ) );
			new_hook.networkableHook.Create( ( void* ) ( ( uintptr_t ) entity + 0x8 ) );
			new_hook.SetHooks( );
		 }
	  }

	  for ( ClientClass* pClass = Source::m_pClient->GetAllClasses( ); pClass; pClass = pClass->m_pNext ) {
		 if ( !strcmp( pClass->m_pNetworkName, XorStr( "CPlayerResource" ) ) ) {
			RecvTable* pClassTable = pClass->m_pRecvTable;

			for ( int nIndex = 0; nIndex < pClassTable->m_nProps; nIndex++ ) {
			   RecvProp* pProp = &pClassTable->m_pProps[ nIndex ];

			   if ( !pProp || strcmp( pProp->m_pVarName, XorStr( "m_iTeam" ) ) )
				  continue;

			   m_pPlayerResource = Encrypted_t<CSPlayerResource*>( *reinterpret_cast< CSPlayerResource*** >( std::uintptr_t( pProp->m_pDataTable->m_pProps->m_ProxyFn ) + 0x10 ) );
			   break;
			}
			break;
		 }
	  }

   #if 0
	  // some animfix from onetap.su
	  auto animsys = **( uintptr_t** ) ( Memory::Scan( XorStr( "client.dll" ),
										 XorStr( "8B 0D ?? ?? ?? ?? FF 35 ?? ?? ?? ?? 8B 01 FF 50 30 C7 05 ?? ?? ?? ?? ?? ?? ?? ?? C6 05 ?? ?? ?? ??" ) ) + 2 );

	  int inputData[ 2 ] = { 1852731235, 7627621 };

	  typedef void* ( __thiscall* UnkAnimationFn )( uintptr_t, int* );
	  AnimSystemOriginal = ( uintptr_t ) Memory::VCall<UnkAnimationFn>( ( void* ) animsys, 18 )( animsys, inputData );

	  UnkAnimation = ( DWORD ) * ( void** ) ( ( DWORD ) AnimSystemOriginal + ( DWORD ) 0x18 );
	  ( *( void** ) ( ( DWORD ) AnimSystemOriginal + ( DWORD ) 0x18 ) ) = ( DWORD* ) &hkAnimationSystem;
   #endif

	  // init config system
	  g_Vars.Create( );

	  g_Vars.viewmodel_fov->fnChangeCallback.m_Size = 0;
	  g_Vars.viewmodel_offset_x->fnChangeCallback.m_Size = 0;
	  g_Vars.viewmodel_offset_y->fnChangeCallback.m_Size = 0;
	  g_Vars.viewmodel_offset_z->fnChangeCallback.m_Size = 0;

	  RecvProp* prop = nullptr;
	  pPropManager->GetProp( XorStr( "DT_SmokeGrenadeProjectile" ), XorStr( "m_bDidSmokeEffect" ), &prop );
	  m_pDidSmokeEffectSwap = std::make_shared<RecvPropHook>( prop, &Hooked::m_nSmokeEffectTickBegin );

	  pPropManager->GetProp( XorStr( "DT_CSRagdoll" ), XorStr( "m_flAbsYaw" ), &prop );
	  m_pFlAbsYawSwap = std::make_shared<RecvPropHook>( prop, &Hooked::RecvProxy_m_flAbsYaw );

	  auto& database = pPropManager->database;
	  auto BaseOverlay = std::find( database.begin( ), database.end( ), XorStr( "DT_BaseAnimatingOverlay" ) );

	  if ( database.end( ) != BaseOverlay ) {
		 auto OverlayVars = BaseOverlay->child_tables.front( );
		 auto AnimOverlays = OverlayVars.child_tables.front( );
		 auto anim_layer_6 = AnimOverlays.child_tables.at( 6 );

		 auto playback = anim_layer_6.child_props.at( 2 );
		 m_pPlaybackRateSwap = std::make_shared< RecvPropHook >( playback, &Hooked::RecvProxy_PlaybackRate );
	  }

	  using namespace Hooked;

	  MH_Initialize( );

	  oGetScreenAspectRatio = Hooked::HooksManager.HookVirtual<decltype( oGetScreenAspectRatio )>( m_pEngine.Xor( ), &Hooked::hkGetScreenAspectRatio, Index::EngineClient::GetScreenAspectRatio );
	  oFireEvents = Hooked::HooksManager.HookVirtual<decltype( oFireEvents )>( m_pEngine.Xor( ), &hkFireEvents, 59 );
	  oDispatchUserMessage = Hooked::HooksManager.HookVirtual<decltype( oDispatchUserMessage )>( m_pClient.Xor( ), &hkDispatchUserMessage, 38 );
	  oIsConnected = Hooked::HooksManager.HookVirtual<decltype( oIsConnected )>( m_pEngine.Xor( ), &hkIsConnected, 27 );
	  oIsBoxVisible = Hooked::HooksManager.HookVirtual<decltype( oIsBoxVisible )>( m_pEngine.Xor( ), &Hooked::hkIsBoxVisible, 32 );

	  oCreateMove = Hooked::HooksManager.HookVirtual<decltype( oCreateMove )>( m_pClientMode.Xor( ), &Hooked::CreateMove, Index::CClientModeShared::CreateMove );
	  oDoPostScreenEffects = Hooked::HooksManager.HookVirtual<decltype( oDoPostScreenEffects )>( m_pClientMode.Xor( ), &Hooked::DoPostScreenEffects, Index::CClientModeShared::DoPostScreenSpaceEffects );
	  oOverrideView = Hooked::HooksManager.HookVirtual<decltype( oOverrideView )>( m_pClientMode.Xor( ), &Hooked::OverrideView, Index::CClientModeShared::OverrideView );
	  oRenderView = Hooked::HooksManager.HookVirtual<decltype( oRenderView )>( m_pViewRender.Xor( ), &Hooked::hkRenderView, 6 );
	  oPaintTraverse = Hooked::HooksManager.HookVirtual<decltype( oPaintTraverse )>( m_pPanel.Xor( ), &Hooked::PaintTraverse, Index::IPanel::PaintTraverse );

	  oFrameStageNotify = Hooked::HooksManager.HookVirtual<decltype( oFrameStageNotify )>( m_pClient.Xor( ), &Hooked::FrameStageNotify, Index::IBaseClientDLL::FrameStageNotify );
	  oRunCommand = Hooked::HooksManager.HookVirtual<decltype( oRunCommand )>( m_pPrediction.Xor( ), &Hooked::RunCommand, Index::IPrediction::RunCommand );
	  oProcessMovement = Hooked::HooksManager.HookVirtual<decltype( oProcessMovement )>( m_pGameMovement.Xor( ), &hkProcessMovement, Index::IGameMovement::ProcessMovement );

	  oDrawSetColor = Hooked::HooksManager.HookVirtual<decltype( oDrawSetColor )>( m_pSurface.Xor( ), &DrawSetColor, 60 / 4 );
	  oEndScene = Hooked::HooksManager.HookVirtual<decltype( oEndScene )>( D3DDevice9, &Hooked::EndScene, Index::DirectX::EndScene );
	  oReset = Hooked::HooksManager.HookVirtual<decltype( oReset )>( D3DDevice9, &Hooked::Reset, Index::DirectX::Reset );

	  oLockCursor = Hooked::HooksManager.HookVirtual<decltype( oLockCursor )>( m_pSurface.Xor( ), &Hooked::LockCursor, Index::VguiSurface::LockCursor );

	  oBeginFrame = Hooked::HooksManager.HookVirtual<decltype( oBeginFrame )>( m_pMatSystem.Xor( ), &Hooked::BeginFrame, Index::MatSystem::BeginFrame );
	  oOverrideConfig = Hooked::HooksManager.HookVirtual<decltype( oOverrideConfig )>( m_pMatSystem.Xor( ), &OverrideConfig, 21 );

	  oListLeavesInBox = Hooked::HooksManager.HookVirtual<decltype( oListLeavesInBox )>( Source::m_pEngine->GetBSPTreeQuery( ), &Hooked::ListLeavesInBox, Index::BSPTreeQuery::ListLeavesInBox );

	  oDrawModelExecute = Hooked::HooksManager.HookVirtual<decltype( oDrawModelExecute )>( m_pModelRender.Xor( ), &Hooked::DrawModelExecute, Index::ModelDraw::DrawModelExecute );

	  oAddBoxOverlay = Hooked::HooksManager.HookVirtual<decltype( oAddBoxOverlay )>( m_pDebugOverlay.Xor( ), &hkAddBoxOverlay, 1 );

	  auto meme = ( void* )( uintptr_t( m_pClientState.Xor( ) ) + 0x8 );
	  oPacketStart = Hooked::HooksManager.HookVirtual<decltype( oPacketStart )>( meme, &Hooked::PacketStart, 5 );
	  oPacketEnd = Hooked::HooksManager.HookVirtual<decltype( oPacketEnd )>( meme, &Hooked::PacketEnd, 6 );

	  auto rel32_resolve = [ ]( uintptr_t ptr ) {
		  auto offset = *( uintptr_t* )( ptr + 0x1 );
		  return ( uintptr_t* )( ptr + 5 + offset );
		  };

	  //0F B7 05 ? ? ? ? 3D ? ? ? ? 74 3F 
	  //g_pSoundServices           = *( ISoundServices** )( utils::PatternScan( engine,            XorStr( "B9 ? ? ? ? 80 65 FC FE 6A 00" ) ) + 1 );
	  auto soundservice = *( uintptr_t** )( Engine::Displacement.Data.m_SoundService + 1 );
	  auto interpolate = Engine::Displacement.Data.m_InterpolateServerEntities;
	  auto reset_contents_cache = Engine::Displacement.Data.m_ResetContentsCache;
	  auto process_interpolated_list = Engine::Displacement.Data.m_ProcessInterpolatedList;
	  auto cl_move = Memory::Scan( XorStr( "engine.dll" ), XorStr( "55 8B EC 81 EC ?? ?? ?? ?? 53 56 57 8B 3D ?? ?? ?? ?? 8A" ) );

	  //55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 57 8B F9 89 7C 24 18
	  oSendNetMsg = Hooked::HooksManager.CreateHook<decltype( oSendNetMsg ) >( &Hooked::SendNetMsg, ( void* )Engine::Displacement.Data.m_SendNetMsg );
	  oSendDatagram = Hooked::HooksManager.CreateHook<decltype( oSendDatagram ) >( &Hooked::SendDatagram, ( void* )Engine::Displacement.Data.SendDatagram );

	  oProcessPacket = Hooked::HooksManager.CreateHook<decltype( oProcessPacket ) >( &Hooked::ProcessPacket, ( void* )Engine::Displacement.Data.ProcessPacket );
	  oInterpolateServerEntities = Hooked::HooksManager.CreateHook<decltype( oInterpolateServerEntities ) >( &Hooked::InterpolateServerEntities, ( void* )interpolate );

	  // oOnSoundStarted = Hooked::HooksManager.HookVirtual<decltype( oOnSoundStarted )>( soundservice, &Hooked::OnSoundStarted, 27 );

	  oProcessInterpolatedList = Hooked::HooksManager.CreateHook<decltype( oProcessInterpolatedList ) >( &hkProcessInterpolatedList, ( void* )process_interpolated_list );
	  oCL_Move = Hooked::HooksManager.CreateHook<decltype( oCL_Move ) >( &CL_Move, ( void* )cl_move );

	 //auto modify_eye_pos = Memory::Scan( XorStr( "client.dll" ), XorStr( "E8 ? ? ? ? 8B 06 8B CE FF 90 ? ? ? ? 85 C0 74 4E" ) );//Engine::Displacement.Data.m_ModifyEyePos;
	 //oModifyEyePoisition = Hooked::HooksManager.CreateHook<decltype( oModifyEyePoisition ) >( &hkModifyEyePosition, ( void* )modify_eye_pos );

	  auto physics_simulate_adr = rel32_resolve( Memory::Scan( XorStr( "client.dll" ), XorStr( "E8 ? ? ? ? 80 BE ? ? ? ? ? 0F 84 ? ? ? ? 8B 06" ) ) );
	  oPhysicsSimulate = Hooked::HooksManager.CreateHook<decltype( oPhysicsSimulate ) >( &hkPhysicsSimulate, ( void* )physics_simulate_adr );


	  oIsHltv = Hooked::HooksManager.HookVirtual<decltype( oIsHltv )>( m_pEngine.Xor( ), &hkIsHltv, Index::EngineClient::IsHltv );
   #if 0

	  || !g_Vars.globals.HackIsReady
		 auto invalid = Memory::Scan( XorStr( "client.dll" ), XorStr( "55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56" ) );
	  oInvalidatePhysicsRecursive = Hooked::HooksManager.CreateHook<decltype( oInvalidatePhysicsRecursive ) >( &hkInvalidatePhysicsRecursive, ( void* ) invalid );
   #endif

	  /* auto _net_show_fragments = Source::m_pCvar->FindVar( XorStr( "net_showfragments" ) );
	   if ( _net_show_fragments ) {
		  o_net_show_fragments = Hooked::HooksManager.HookVirtual<decltype( o_net_show_fragments )>( _net_show_fragments, &net_show_fragments, 13 );
	   }*/

   #if 0
	  auto addsound = rel32_resolve( Memory::Scan( XorStr( "engine.dll" ), XorStr( "   " ) ) );
	  oCL_AddSound = Hooked::HooksManager.CreateHook<decltype( oCL_AddSound ) >( &Hooked::CL_AddSound, ( void* ) addsound );
   #endif


	  Hooked::HooksManager.Enable( );

   #if 0
	  m_pClientSwap = std::make_shared<Memory::VmtSwap>( m_pClient );
	  m_pPredictionSwap = std::make_shared<Memory::VmtSwap>( m_pPrediction );
	  m_pPanelSwap = std::make_shared<Memory::VmtSwap>( m_pPanel );
	  m_pClientModeSwap = std::make_shared<Memory::VmtSwap>( m_pClientMode );
	  m_pD3D9Swap = std::make_shared<Memory::VmtSwap>( D3DDevice9 );
	  m_pVguiSurfaceSwap = std::make_shared<Memory::VmtSwap>( m_pSurface );
	  m_pMatSystemSwap = std::make_shared<Memory::VmtSwap>( m_pMatSystem );
	  m_pBSPQuerySwap = std::make_shared<Memory::VmtSwap>( Source::m_pEngine->GetBSPTreeQuery( ) );
	  m_pStudioRenderSwap = std::make_shared<Memory::VmtSwap>( m_pStudioRender );
	  m_pClientStateSwap = std::make_shared<Memory::VmtSwap>( ( const void* ) ( uintptr_t( m_pClientState.Xor( ) ) + 0x8 ) );
	  m_pEngineSoundSwap = std::make_shared<Memory::VmtSwap>( m_pEngineSound );
	  m_pEngineSwap = std::make_shared<Memory::VmtSwap>( m_pEngine );

	  m_pEngineSwap->Hook( &Hooked::hkGetScreenAspectRatio, Index::EngineClient::GetScreenAspectRatio );

	  m_pClientModeSwap->Hook( &Hooked::CreateMove, Index::CClientModeShared::CreateMove );
	  m_pClientModeSwap->Hook( &Hooked::DoPostScreenEffects, Index::CClientModeShared::DoPostScreenSpaceEffects );
	  m_pClientModeSwap->Hook( &Hooked::OverrideView, Index::CClientModeShared::OverrideView );

	  m_pEngineSoundSwap->Hook( &Hooked::hkEmitSound, 5 );

	  m_pClientSwap->Hook( &Hooked::FrameStageNotify, Index::IBaseClientDLL::FrameStageNotify );
	  //m_pClientSwap->Hook( &Hooked::WriteUsercmdDeltaToBuffer, 24 );
	  //m_pClientSwap->Hook( &Hooked::CreateMoveInput, Index::IBaseClientDLL::CreateMove );

	  m_pPredictionSwap->Hook( &Hooked::RunCommand, Index::IPrediction::RunCommand );

	  m_pPanelSwap->Hook( &Hooked::PaintTraverse, Index::IPanel::PaintTraverse );

	  m_pD3D9Swap->Hook( &Hooked::Present, Index::DirectX::Present );
	  m_pD3D9Swap->Hook( &Hooked::Reset, Index::DirectX::Reset );

	  m_pVguiSurfaceSwap->Hook( &Hooked::LockCursor, Index::VguiSurface::LockCursor );

	  m_pMatSystemSwap->Hook( &Hooked::BeginFrame, Index::MatSystem::BeginFrame );

	  m_pBSPQuerySwap->Hook( &Hooked::ListLeavesInBox, Index::BSPTreeQuery::ListLeavesInBox );

	  m_pStudioRenderSwap->Hook( &Hooked::DrawModel, Index::StudioRender::DrawModel );

	  m_pClientStateSwap->Hook( &Hooked::PacketStart, 5 );

   #if 0
	  m_pInputSwap->Hook( &Hooked::WriteUsercmdDeltaToBuffer, 5 );
	  m_pSVCheats->Hook( &Hooked::SvCheatsGetBool, 13 );
	  m_pClientStateSwap->Hook( &Hooked::SetReservationCookie, 63 );
   #endif

   #endif

   #ifndef DevelopMode
	  CLicense lic;
	  std::string str = XorStr( "/stat/injectcnt/?login=" ) + g_Vars.globals.c_login;
	  lic.GetUrlData( str );
   #endif // !DevelopMode

	  return true;
   }

   void Destroy( ) {
   #if 0
	  m_pClientSwap.reset( );
	  m_pPredictionSwap.reset( );
	  m_pPanelSwap.reset( );
	  m_pClientModeSwap.reset( );
	  m_pD3D9Swap.reset( );
	  m_pVguiSurfaceSwap.reset( );
	  m_pBSPQuerySwap.reset( );
	  m_pStudioRenderSwap.reset( );
	  m_pClientStateSwap.reset( );
	  m_pEngineSoundSwap.reset( );
	  m_pEngineSwap.reset( );

	  if ( m_pNetChannelSwap )
		 m_pNetChannelSwap.reset( );
   #endif

	  Hooked::HooksManager.Restore( );

	  // когда убираем хуки
	  CCSPlayerClass->m_pCreateFn = oCreateCCSPlayer;
	  Hooked::player_hooks.clear( );

	  /* ( *( void** )( ( DWORD )AnimSystemOriginal + ( DWORD )0x18 ) ) = ( DWORD* )UnkAnimation;*/
	 // m_pDidSmokeEffectSwap.reset( );

	  GameEvent::Get( )->Shutdown( );
	  GlowOutline::Get( )->Shutdown( );
	  ISkinChanger::Get( )->Destroy( );
	  InputSys::Get( )->Destroy( );

	  MH_Uninitialize( );
   }

   void* CreateInterface( const std::string& image_name, const std::string& name, bool force /*= false */ ) {
	  auto image = GetModuleHandleA( image_name.c_str( ) );

	  if ( !image )
		 return nullptr;

	  auto fn = ( CreateInterfaceFn ) ( GetProcAddress( image, XorStr( "CreateInterface" ) ) );

	  if ( !fn )
		 return nullptr;

	  if ( force )
		 return fn( name.c_str( ), nullptr );

	  char format[ 1024 ] = { };

	  for ( auto i = 0u; i < 1000u; i++ ) {
		 sprintf_s( format, XorStr( "%s%03u" ), name.c_str( ), i );

		 auto factory = fn( format, nullptr );

		 if ( factory ) {
			printf( XorStr( "%s: 0x%X\n" ), name.c_str( ), factory );
			return factory;
		 }
	  }

	  return nullptr;
   }

}
