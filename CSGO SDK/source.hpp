#pragma once

// little include hack
#include "UtlBuffer.hpp"
#include "UtlMemory.hpp"
#include "UtlVector.hpp"

#include "sdk.hpp"
#include "recv_swap.hpp"

class IClientMode {
public:
   virtual ~IClientMode( ) { }
   virtual int ClientModeCSNormal( void* ) = 0;
   virtual void Init( ) = 0;
   virtual void InitViewport( ) = 0;
   virtual void Shutdown( ) = 0;
   virtual void Enable( ) = 0;
   virtual void Disable( ) = 0;
   virtual void Layout( ) = 0;
   virtual IPanel* GetViewport( ) = 0;
   virtual void* GetViewportAnimationController( ) = 0;
   virtual void ProcessInput( bool bActive ) = 0;
   virtual bool ShouldDrawDetailObjects( ) = 0;
   virtual bool ShouldDrawEntity( C_BaseEntity* pEnt ) = 0;
   virtual bool ShouldDrawLocalPlayer( C_BaseEntity* pPlayer ) = 0;
   virtual bool ShouldDrawParticles( ) = 0;
   virtual bool ShouldDrawFog( void ) = 0;
   virtual void OverrideView( CViewSetup* pSetup ) = 0;
   virtual int KeyInput( int down, int keynum, const char* pszCurrentBinding ) = 0;
   virtual void StartMessageMode( int iMessageModeType ) = 0;
   virtual IPanel* GetMessagePanel( ) = 0;
   virtual void OverrideMouseInput( float* x, float* y ) = 0;
   virtual bool CreateMove( float flInputSampleTime, void* usercmd ) = 0;
   virtual void LevelInit( const char* newmap ) = 0;
   virtual void LevelShutdown( void ) = 0;
};
namespace Source
{

   extern Encrypted_t<IBaseClientDLL> m_pClient;
   extern Encrypted_t<IClientEntityList> m_pEntList;
   extern Encrypted_t<IGameMovement> m_pGameMovement;
   extern Encrypted_t<IPrediction> m_pPrediction;
   extern Encrypted_t<IMoveHelper> m_pMoveHelper;
   extern Encrypted_t<IInput> m_pInput;
   extern Encrypted_t< CGlobalVars >  m_pGlobalVars;
   extern Encrypted_t<ISurface> m_pSurface;
   extern Encrypted_t<IVEngineClient> m_pEngine;
   extern Encrypted_t<IClientMode> m_pClientMode;
   extern Encrypted_t<ICVar> m_pCvar;
   extern Encrypted_t<IPanel> m_pPanel;
   extern Encrypted_t<IGameEventManager> m_pGameEvent;
   extern Encrypted_t<IVModelRender> m_pModelRender;
   extern Encrypted_t<IMaterialSystem> m_pMatSystem;
   extern Encrypted_t<ISteamClient> g_pSteamClient;
   extern Encrypted_t<ISteamGameCoordinator> g_pSteamGameCoordinator;
   extern Encrypted_t<ISteamMatchmaking> g_pSteamMatchmaking;
   extern Encrypted_t<ISteamUser> g_pSteamUser;
   extern Encrypted_t<ISteamFriends> g_pSteamFriends;
   extern Encrypted_t<IPhysicsSurfaceProps> m_pPhysSurface;
   extern Encrypted_t<IEngineTrace> m_pEngineTrace;
   extern Encrypted_t<CGlowObjectManager> m_pGlowObjManager;
   extern Encrypted_t<IVModelInfo> m_pModelInfo;
   extern Encrypted_t< CClientState >  m_pClientState;
   extern Encrypted_t<IVDebugOverlay> m_pDebugOverlay;
   extern Encrypted_t<IEngineSound> m_pEngineSound;
   extern Encrypted_t<IMemAlloc> m_pMemAlloc;
   extern Encrypted_t<IViewRenderBeams> m_pRenderBeams;
   extern Encrypted_t<ILocalize> m_pLocalize;
   extern Encrypted_t<IStudioRender> m_pStudioRender;
   extern Encrypted_t<CSPlayerResource*>  m_pPlayerResource;
   extern Encrypted_t<ICenterPrint> m_pCenterPrint;
   extern Encrypted_t<IVRenderView> m_pRenderView;
   extern Encrypted_t<IClientLeafSystem> m_pClientLeafSystem;
   extern Encrypted_t<IMDLCache> m_pMDLCache;
   extern Encrypted_t<IViewRender> m_pViewRender;
   extern WNDPROC oldWindowProc;
   extern HWND hWindow;

#if 0
   extern Memory::VmtSwap::Shared m_pClientSwap;
   extern Memory::VmtSwap::Shared m_pPredictionSwap;
   extern Memory::VmtSwap::Shared m_pPanelSwap;
   extern Memory::VmtSwap::Shared m_pD3D9Swap;
   extern Memory::VmtSwap::Shared m_pClientModeSwap;
   extern Memory::VmtSwap::Shared m_pResetSwap;
   extern Memory::VmtSwap::Shared m_pVguiSurfaceSwap;
   extern Memory::VmtSwap::Shared m_pGameEventSwap;
   extern Memory::VmtSwap::Shared m_pMatSystemSwap;
   extern Memory::VmtSwap::Shared m_pBSPQuerySwap;
   extern Memory::VmtSwap::Shared m_pClientStateSwap;
   extern Memory::VmtSwap::Shared m_pNetChannelSwap;
   extern Memory::VmtSwap::Shared m_pStudioRenderSwap;
   extern Memory::VmtSwap::Shared m_pEngineSoundSwap;
   extern Memory::VmtSwap::Shared m_pEngineSwap;
#endif

   // netvar proxies
   extern RecvPropHook::Shared m_pDidSmokeEffectSwap;
   extern RecvPropHook::Shared m_pFlAbsYawSwap;
   extern RecvPropHook::Shared m_pPlaybackRateSwap;

   bool Create( void* reserved );
   void Destroy( );

   void* CreateInterface( const std::string& image_name, const std::string& name, bool force = false );

}
