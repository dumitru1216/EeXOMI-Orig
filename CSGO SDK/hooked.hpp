#pragma once

#include "source.hpp"
#include "Protobuf.h"

struct vrect_t {
   int				x, y, width, height;
   vrect_t* pnext;
};

class CRecvProxyData;

namespace Hooked
{
   struct PlayerHook {
	  Memory::VmtSwap clientHook;
	  Memory::VmtSwap renderableHook;
	  Memory::VmtSwap networkableHook;

	  ~PlayerHook( );
	  void SetHooks( );
   };
   extern std::map< int, Hooked::PlayerHook > player_hooks;

   using CreateMoveFn = bool( __stdcall* )( float, CUserCmd* );
   inline CreateMoveFn oCreateMove;
   bool __stdcall CreateMove( float, CUserCmd* );

   //void __fastcall CreateMoveInput( void* _this, int, int sequence_number, float input_sample_frametime, bool active );

   using DoPostScreenEffectsFn = int( __thiscall* )( IClientMode*, int );
   inline DoPostScreenEffectsFn oDoPostScreenEffects;
   int __stdcall DoPostScreenEffects( int a1 );

   using FrameStageNotifyFn = void( __thiscall* )( void*, ClientFrameStage_t );
   inline FrameStageNotifyFn oFrameStageNotify;
   void __fastcall FrameStageNotify( void* ecx, void* edx, ClientFrameStage_t stage );

   using RunCommandFn = void( __thiscall* )( void*, C_CSPlayer*, CUserCmd*, IMoveHelper* );
   inline RunCommandFn oRunCommand;
   void __fastcall RunCommand( void* ecx, void* edx, C_CSPlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper );

   using PaintTraverseFn = void( __thiscall* )( void*, unsigned int, bool, bool );
   inline PaintTraverseFn oPaintTraverse;
   void __fastcall PaintTraverse( void* ecx, void* edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce );

   using EndSceneFn = HRESULT( __stdcall* )( IDirect3DDevice9* );
   inline EndSceneFn oEndScene;
   HRESULT __stdcall EndScene( IDirect3DDevice9* pDevice );

   using ResetFn = HRESULT( __stdcall* )( IDirect3DDevice9*, D3DPRESENT_PARAMETERS* );
   inline ResetFn oReset;
   HRESULT __stdcall Reset( IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters );

   using PresentFn = HRESULT( __stdcall* )( IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA* );
   inline PresentFn oPresent;
   HRESULT __stdcall Present( LPDIRECT3DDEVICE9 pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion );

   using LockCursorFn = void( __thiscall* )( void* );
   inline LockCursorFn oLockCursor;
   void __stdcall LockCursor( );

   using OverrideViewFn = void( __thiscall* )( void*, CViewSetup* );
   inline OverrideViewFn oOverrideView;
   void __fastcall OverrideView( void* ECX, int EDX, CViewSetup* vsView );

   using BeginFrameFn = void( __thiscall* )( void* ECX, float ft );
   inline BeginFrameFn oBeginFrame;
   void __fastcall BeginFrame( void* ECX, void* EDX, float ft );

   void __fastcall SetReservationCookie( void* ECX, void* EDX, int a2, int a3 );

   using PacketStartFn = void( __thiscall* )( void*, int, int );
   inline PacketStartFn oPacketStart;
   void __fastcall PacketStart( void* ECX, void* EDX, int incoming_sequence, int outgoing_acknowledged );

   using PacketEndFn = void( __thiscall* )( void* );
   inline PacketEndFn oPacketEnd;
   void __fastcall PacketEnd( void* ECX, void* EDX );

   using ListLeavesInBoxFn = int( __thiscall* )( void*, Vector& mins, Vector& maxs, unsigned short* pList, int listMax );
   inline ListLeavesInBoxFn oListLeavesInBox;
   int __fastcall ListLeavesInBox( void* ECX, void* EDX, Vector& mins, Vector& maxs, unsigned short* pList, int listMax );

   using SendNetMsgFn = bool( __thiscall* )( INetChannel* pNetChan, INetMessage& msg, bool bForceReliable, bool bVoice );
   inline SendNetMsgFn oSendNetMsg;
   bool __fastcall SendNetMsg( INetChannel* pNetChan, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice );

   using SendDatagramFn = int( __thiscall* )( INetChannel* pNetChan, void* buf );
   inline SendDatagramFn oSendDatagram;
   int __fastcall SendDatagram( INetChannel* pNetChan, void* edx, void* buf );

   using ProcessPacketFn = int( __thiscall* )( INetChannel* pNetChan, void* packet, bool header );
   inline ProcessPacketFn oProcessPacket;
   void __fastcall ProcessPacket( INetChannel* pNetChan, void* edx, void* packet, bool header );

   using ShutdownFn = void( __thiscall* )( INetChannel*, const char* );
   inline ShutdownFn oShutdown;
   void __fastcall Shutdown( INetChannel* pNetChan, void* EDX, const char* reason );

   using DrawModelFn = void( __thiscall* )( void*, void*, DrawModelInfo_t*, const matrix3x4_t*, float*, float*, Vector&, int );
   inline DrawModelFn oDrawModel;
   void __fastcall DrawModel( void* ECX, void* EDX, void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags );

   using DrawModelExecuteFn = void( __thiscall* )( void*, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pCustomBoneToWorld );
   inline DrawModelExecuteFn oDrawModelExecute;
   void __fastcall DrawModelExecute( void* ECX, void* EDX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pCustomBoneToWorld );

   using WriteUsercmdDeltaToBufferFn = bool( __thiscall* )( void*, int, void*, int, int, bool );
   inline WriteUsercmdDeltaToBufferFn oWriteUsercmdDeltaToBuffer;
   bool __fastcall WriteUsercmdDeltaToBuffer( void* ECX, void* EDX, int nSlot, void* buf, int from, int to, bool isnewcommand );

   IClientNetworkable* hkCreateCCSPlayer( int entnum, int serialNum );
   bool __fastcall hkSetupBones( uintptr_t ecx, void* edx, matrix3x4_t* matrix, int bone_count, int bone_mask, float time );
   void __fastcall hkEntityRelease( uintptr_t ecx, void* edx );

   using FnEmitSound = void( __thiscall* ) ( IEngineSound* thisptr, void* filter, int ent_index, int channel, const char* sound_entry, unsigned int sound_entry_hash,
	  const char* sample, float volume, float attenuation, int seed, int flags, int pitch, const Vector* origin, const Vector* direction,
	  void* vec_origins, bool update_positions, float sound_time, int speaker_entity, int test );
   inline FnEmitSound oEmitSound;

   void __fastcall hkEmitSound( IEngineSound* thisptr, uint32_t, void* filter, int ent_index, int channel, const char* sound_entry, unsigned int sound_entry_hash,
	  const char* sample, float volume, float attenuation, int seed, int flags, int pitch, const Vector* origin, const Vector* direction,
	  void* vec_origins, bool update_positions, float sound_time, int speaker_entity, int test );

   using FnGetScreenAspectRatio = float( __thiscall* )( void*, int32_t, int32_t );
   inline FnGetScreenAspectRatio oGetScreenAspectRatio;
   float __fastcall hkGetScreenAspectRatio( void* ECX, void* EDX, int32_t iWidth, int32_t iHeight );

   using FnInterpolateServerEntities = void( __cdecl* )( void );
   inline FnInterpolateServerEntities oInterpolateServerEntities;
   void __cdecl InterpolateServerEntities( void );

   using FnView_Render = void( __thiscall* )( void*, vrect_t* );
   inline FnView_Render oView_Render;
   void __fastcall View_Render( void* ecx, void* edx, vrect_t* rect );

#if 0
   using FnCL_AddSound = void( __cdecl* )( C_SoundInfo * info, int* a1, bool* a2 );
   inline FnCL_AddSound oCL_AddSound;
   void __stdcall CL_AddSound( C_SoundInfo* info, int* a1, bool* a2 );
#endif

   using FnOnSoundStarted = void( __thiscall* )( void*, int, StartSoundParams_t&, char const* );
   inline FnOnSoundStarted oOnSoundStarted;
   void __fastcall OnSoundStarted( void* ECX, void* EDX, int guid, StartSoundParams_t& params, char const* soundname );

   using FnIsBoxVisible = int( __thiscall* )( void* ECX, const Vector&, const Vector& );
   inline FnIsBoxVisible oIsBoxVisible;
   int __fastcall hkIsBoxVisible( void* ECX, uint32_t, const Vector& mins, const Vector& maxs );

   using FnIsPlayingDemo = bool( __thiscall* )( void* ECX );
   inline FnIsPlayingDemo oIsPlayingDemo;
   bool __fastcall hkIsPlayingDemo( void* ECX, void* EDX );

  //using FnCL_Move = void( __vectorcall* )( float accumulated_extra_samples, bool bFinalTick );
  //inline FnCL_Move oCL_Move;
  //void __vectorcall CL_Move( float accumulated_extra_samples, bool bFinalTick );
  //
   //void __fastcall RunSimulation( void* this_, void*, int iCommandNumber, CUserCmd* pCmd, size_t local );
   //inline Memory::DetourHook_t RunSimulationDetor( RunSimulation );
   //
   //void __fastcall PredictionUpdate( void* prediction, void*, int startframe, bool validframe, int incoming_acknowledged, int outgoing_command );
   //inline Memory::DetourHook_t PredictionUpdateDetor( PredictionUpdate );

   using FnIsConnected = bool( __thiscall* ) ( void );
   inline FnIsConnected oIsConnected;
   bool __fastcall hkIsConnected( void );

   using FnDispatchUserMessage = bool( __thiscall* ) ( void* ECX, int msg_type, int unk1, int nBytes, bf_read& msg_data );
   inline FnDispatchUserMessage oDispatchUserMessage;
   bool __fastcall hkDispatchUserMessage( void* ECX, void* EDX, int msg_type, int unk1, int nBytes, bf_read& msg_data );

   using FnRenderView = void( __thiscall* ) ( void* ECX, const CViewSetup& view, CViewSetup& hudViewSetup, int nClearFlags, int whatToDraw );
   inline FnRenderView oRenderView;
   void __fastcall hkRenderView( void* ECX, void* EDX, const CViewSetup& view, CViewSetup& hudViewSetup, int nClearFlags, int whatToDraw );

   // Recv proxy hook
   void m_nSmokeEffectTickBegin( const CRecvProxyData* pData, void* pStruct, void* pOut );
   void RecvProxy_m_flAbsYaw( const CRecvProxyData* pData, void* pStruct, void* pOut );
   void RecvProxy_PlaybackRate( const CRecvProxyData* pData, void* pStruct, void* pOut );
}
