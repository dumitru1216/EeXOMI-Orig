#pragma once

#include "horizon.hpp"

#include <d3d9.h>

#include <stdint.h>

#include "vector2d.hpp"
#include "vector.hpp"
#include "vector4d.hpp"
#include "qangle.hpp"
#include "matrix.hpp"
#include "Math.h"
#include "CColor.hpp"
#include "Definitions.hpp"
#include "CVariables.hpp"
#include "CStudioRender.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_dx9.h"
#include <minwindef.h>

using namespace Horizon;
#define M_PI 3.1415926535f
#define TIME_TO_TICKS(dt) ((int)( 0.5f + (float)(dt) / Source::m_pGlobalVars->interval_per_tick))
#define TICKS_TO_TIME(t) (Source::m_pGlobalVars->interval_per_tick * (t) )
#define ROUND_TO_TICKS( t ) ( Source::m_pGlobalVars->interval_per_tick * TIME_TO_TICKS( t ) )

#define NUM_ENT_ENTRY_BITS         (11 + 2)
#define NUM_ENT_ENTRIES            (1 << NUM_ENT_ENTRY_BITS)
#define INVALID_EHANDLE_INDEX       0xFFFFFFFF
#define NUM_SERIAL_NUM_BITS        16 // (32 - NUM_ENT_ENTRY_BITS)
#define NUM_SERIAL_NUM_SHIFT_BITS (32 - NUM_SERIAL_NUM_BITS)
#define ENT_ENTRY_MASK             (( 1 << NUM_SERIAL_NUM_BITS) - 1)

#define MAX_COORD_FLOAT ( 16384.0f )
#define MIN_COORD_FLOAT ( -MAX_COORD_FLOAT )

#define DECLARE_POINTER_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#define MAXSTUDIOSKINS		32

#define FRUSTUM_NUMPLANES    6

#ifndef max
#define MIN(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define MAX(a,b)            (((a) < (b)) ? (a) : (b))
#endif

// 
// macros
// 

#define TELEPORT_DISTANCE 4096.f

#pragma region decl_macros
#define MULTIPLAYER_BACKUP 150

#define FL_ONGROUND ( 1 << 0 )
#define FL_DUCKING ( 1 << 1 )

#define IN_ATTACK ( 1 << 0 )
#define IN_JUMP ( 1 << 1 )
#define IN_DUCK ( 1 << 2 )
#define IN_FORWARD ( 1 << 3 )
#define IN_BACK ( 1 << 4 )
#define IN_USE ( 1 << 5 )
#define IN_CANCEL ( 1 << 6 )
#define IN_LEFT ( 1 << 7 )
#define IN_RIGHT ( 1 << 8 )
#define IN_MOVELEFT ( 1 << 9 )
#define IN_MOVERIGHT ( 1 << 10 )
#define IN_ATTACK2 ( 1 << 11 )
#define IN_RUN ( 1 << 12 )
#define IN_RELOAD ( 1 << 13 )
#define IN_ALT1 ( 1 << 14 )
#define IN_ALT2 ( 1 << 15 )
#define IN_SCORE ( 1 << 16 )
#define IN_SPEED ( 1 << 17 )
#define IN_WALK ( 1 << 18 )
#define IN_ZOOM ( 1 << 19 )
#define IN_WEAPON1 ( 1 << 20 )
#define IN_WEAPON2 ( 1 << 21 )
#define IN_BULLRUSH ( 1 << 22 )
#define IN_GRENADE1 ( 1 << 23 )
#define IN_GRENADE2 ( 1 << 24 )
#pragma endregion

#pragma region masks

#define   DISPSURF_FLAG_SURFACE           (1<<0)
#define   DISPSURF_FLAG_WALKABLE          (1<<1)
#define   DISPSURF_FLAG_BUILDABLE         (1<<2)
#define   DISPSURF_FLAG_SURFPROP1         (1<<3)
#define   DISPSURF_FLAG_SURFPROP2         (1<<4)

#define   CONTENTS_EMPTY                0

#define   CONTENTS_SOLID                0x1       
#define   CONTENTS_WINDOW               0x2
#define   CONTENTS_AUX                  0x4
#define   CONTENTS_GRATE                0x8
#define   CONTENTS_SLIME                0x10
#define   CONTENTS_WATER                0x20
#define   CONTENTS_BLOCKLOS             0x40 
#define   CONTENTS_OPAQUE               0x80 
#define   LAST_VISIBLE_CONTENTS         CONTENTS_OPAQUE

#define   ALL_VISIBLE_CONTENTS            (LAST_VISIBLE_CONTENTS | (LAST_VISIBLE_CONTENTS-1))

#define   CONTENTS_TESTFOGVOLUME        0x100
#define   CONTENTS_UNUSED               0x200     
#define   CONTENTS_BLOCKLIGHT           0x400
#define   CONTENTS_TEAM1                0x800 
#define   CONTENTS_TEAM2                0x1000 
#define   CONTENTS_IGNORE_NODRAW_OPAQUE 0x2000
#define   CONTENTS_MOVEABLE             0x4000
#define   CONTENTS_AREAPORTAL           0x8000
#define   CONTENTS_PLAYERCLIP           0x10000
#define   CONTENTS_MONSTERCLIP          0x20000
#define   CONTENTS_CURRENT_0            0x40000
#define   CONTENTS_CURRENT_90           0x80000
#define   CONTENTS_CURRENT_180          0x100000
#define   CONTENTS_CURRENT_270          0x200000
#define   CONTENTS_CURRENT_UP           0x400000
#define   CONTENTS_CURRENT_DOWN         0x800000

#define   CONTENTS_ORIGIN               0x1000000 

#define   CONTENTS_MONSTER              0x2000000 
#define   CONTENTS_DEBRIS               0x4000000
#define   CONTENTS_DETAIL               0x8000000 
#define   CONTENTS_TRANSLUCENT          0x10000000
#define   CONTENTS_LADDER               0x20000000
#define   CONTENTS_HITBOX               0x40000000

#define   SURF_LIGHT                    0x0001 
#define   SURF_SKY2D                    0x0002 
#define   SURF_SKY                      0x0004 
#define   SURF_WARP                     0x0008 
#define   SURF_TRANS                    0x0010
#define   SURF_NOPORTAL                 0x0020 
#define   SURF_TRIGGER                  0x0040 
#define   SURF_NODRAW                   0x0080 

#define   SURF_HINT                     0x0100 

#define   SURF_SKIP                     0x0200   
#define   SURF_NOLIGHT                  0x0400   
#define   SURF_BUMPLIGHT                0x0800   
#define   SURF_NOSHADOWS                0x1000   
#define   SURF_NODECALS                 0x2000   
#define   SURF_NOPAINT                  SURF_NODECALS
#define   SURF_NOCHOP                   0x4000   
#define   SURF_HITBOX                   0x8000   

// -----------------------------------------------------
// spatial content masks - used for spatial queries (traceline,etc.)
// -----------------------------------------------------
#define   MASK_ALL                      (0xFFFFFFFF)
#define   MASK_SOLID                    (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define   MASK_PLAYERSOLID              (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define   MASK_NPCSOLID                 (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define   MASK_NPCFLUID                 (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define   MASK_WATER                    (CONTENTS_WATER|CONTENTS_MOVEABLE|CONTENTS_SLIME)
#define   MASK_OPAQUE                   (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_OPAQUE)
#define   MASK_OPAQUE_AND_NPCS          (MASK_OPAQUE|CONTENTS_MONSTER)
#define   MASK_BLOCKLOS                 (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_BLOCKLOS)
#define   MASK_BLOCKLOS_AND_NPCS        (MASK_BLOCKLOS|CONTENTS_MONSTER)
#define   MASK_VISIBLE                  (MASK_OPAQUE|CONTENTS_IGNORE_NODRAW_OPAQUE)
#define   MASK_VISIBLE_AND_NPCS         (MASK_OPAQUE_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE)
#define   MASK_SHOT                     (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_HITBOX)
#define   MASK_SHOT_BRUSHONLY           (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_DEBRIS)
#define   MASK_SHOT_HULL                (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_GRATE)
#define   MASK_SHOT_PORTAL              (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define   MASK_SOLID_BRUSHONLY          (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_GRATE)
#define   MASK_PLAYERSOLID_BRUSHONLY    (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_PLAYERCLIP|CONTENTS_GRATE)
#define   MASK_NPCSOLID_BRUSHONLY       (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE)
#define   MASK_NPCWORLDSTATIC           (CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE)
#define   MASK_NPCWORLDSTATIC_FLUID     (CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP)
#define   MASK_SPLITAREAPORTAL          (CONTENTS_WATER|CONTENTS_SLIME)
#define   MASK_CURRENT                  (CONTENTS_CURRENT_0|CONTENTS_CURRENT_90|CONTENTS_CURRENT_180|CONTENTS_CURRENT_270|CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN)
#define   MASK_DEADSOLID                (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_GRATE)
#define   MASK_SHOT_PLAYER              ( MASK_SHOT_HULL | CONTENTS_HITBOX )
#pragma endregion

#pragma region tex_groups
// These are given to FindMaterial to reference the texture groups that Show up on the 
#define TEXTURE_GROUP_LIGHTMAP						        "Lightmaps"
#define TEXTURE_GROUP_WORLD							          "World textures"
#define TEXTURE_GROUP_MODEL							          "Model textures"
#define TEXTURE_GROUP_VGUI							          "VGUI textures"
#define TEXTURE_GROUP_PARTICLE						        "Particle textures"
#define TEXTURE_GROUP_DECAL							          "Decal textures"
#define TEXTURE_GROUP_SKYBOX						          "SkyBox textures"
#define TEXTURE_GROUP_CLIENT_EFFECTS				      "ClientEffect textures"
#define TEXTURE_GROUP_OTHER							          "Other textures"
#define TEXTURE_GROUP_PRECACHED						        "Precached"
#define TEXTURE_GROUP_CUBE_MAP						        "CubeMap textures"
#define TEXTURE_GROUP_RENDER_TARGET					      "RenderTargets"
#define TEXTURE_GROUP_UNACCOUNTED					        "Unaccounted textures"
//#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER		  "Static Vertex"
#define TEXTURE_GROUP_STATIC_INDEX_BUFFER			    "Static Indices"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_DISP		"Displacement Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_COLOR	"Lighting Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_WORLD	"World Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS	"Model Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_OTHER	"Other Verts"
#define TEXTURE_GROUP_DYNAMIC_INDEX_BUFFER			  "Dynamic Indices"
#define TEXTURE_GROUP_DYNAMIC_VERTEX_BUFFER			  "Dynamic Verts"
#define TEXTURE_GROUP_DEPTH_BUFFER					      "DepthBuffer"
#define TEXTURE_GROUP_VIEW_MODEL					        "ViewModel"
#define TEXTURE_GROUP_PIXEL_SHADERS					      "Pixel Shaders"
#define TEXTURE_GROUP_VERTEX_SHADERS				      "Vertex Shaders"
#define TEXTURE_GROUP_RENDER_TARGET_SURFACE			  "RenderTarget Surfaces"
#define TEXTURE_GROUP_MORPH_TARGETS					      "Morph Targets"
#pragma endregion

#define NOISE_DIVISIONS		128
#define MAX_BEAM_ENTS		10

#define MAX_VIS_LEAVES    32
#define MAX_AREA_STATE_BYTES        32
#define MAX_AREA_PORTAL_STATE_BYTES 24

__forceinline auto DotProduct( const Vector & a, const Vector & b ) -> float {
   return ( a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ] );
}

// 
// indices
// 
#pragma region decl_indices
namespace Index
{
   namespace IBaseClientDLL
   {
	  enum {
		 GetAllClasses = 8,
		 CreateMove = 22,
		 FrameStageNotify = 36,
		 IsChatRaised = 90,
	  };
   }
   namespace ModelDraw
   {
	  enum {
		 DrawModelExecute = 21,
	  };
   }
   namespace GameEvent
   {
	  enum {
		 GameEvent = 9,
	  };
   }
   namespace StudioRender
   {
	  enum {
		 DrawModel = 29,
	  };
   }
   namespace NetChannel
   {
	  enum {
		 Shutdown = 27,
		 SendNetMsg = 42,
	  };
   }
   namespace EngineClient
   {
	  enum {
		 GetNetChannelInfo = 78,
		 IsPlayingDemo = 82,
         IsHltv = 93,
		 GetScreenAspectRatio = 101,
	  };
   }
   namespace BSPTreeQuery
   {
	  enum {
		 ListLeavesInBox = 6,
	  };
   }
   namespace CClientModeShared
   {
	  enum {
		 CreateMove = 24,
		 DoPostScreenSpaceEffects = 44,
		 OverrideView = 18,
	  };
   }
   namespace DirectX
   {
	  enum {
		 EndScene = 42,
		 Reset = 16,
		 Present = 17,
	  };
   };
   namespace IClientEntityList
   {
	  enum {
		 GetClientEntity = 3,
		 GetClientEntityFromHandle = 4,
		 GetHighestEntityIndex = 6,
	  };
   }
   namespace IGameMovement
   {
	  enum {
		 ProcessMovement = 1,
		 Reset = 2,
		 StartTrackPredictionErrors = 3,
		 FinishTrackPredictionErrors = 4,
		 GetPlayerMins = 6,
		 GetPlayerMaxs = 7,
		 GetPlayerViewOffset = 8,
	  };
   }
   namespace IPrediction
   {
	  enum {
		 Update = 3,
		 CheckMovingGround = 18,
		 RunCommand = 19,
		 SetupMove = 20,
		 FinishMove = 21,
	  };
   }
   namespace IMoveHelper
   {
	  enum {
		 SetHost = 1,
		 ProccesImpacts = 4,
	  };
   }
   namespace IInput
   {
	  enum {
		 GetUserCmd = 8,
	  };
   }
   namespace IVEngineClient
   {
	  enum {
		 GetScreenSize = 5,
		 GetPlayerInfo = 8,
		 GetLocalPlayer = 12,
		 Time = 14,
		 GetViewAngles = 18,
		 SetViewAngles = 19,
		 GetMaxClients = 20,
		 IsInGame = 26,
		 IsConnected = 27,
		 WorldToScreenMatrix = 37,
		 GetNetChannelInfo = 78,
		 ClientCmd_Unrestricted = 114,
	  };
   }
   namespace IPanel
   {
	  enum {
		 GetName = 36,
		 PaintTraverse = 41,
	  };
   }
   namespace VguiSurface
   {
	  enum {
		 UnLockCursor = 66,
		 LockCursor = 67,
		 OnScreenSizeChanged = 116,
	  };
   }
   namespace MatSystem
   {
	  enum {
		 BeginFrame = 42,
	  };
   }
}
#pragma endregion

// 
// enums
// 
#pragma region decl_enums
enum MDLCacheDataType_t {
   // Callbacks to Get called when data is loaded or unloaded for these:
   MDLCACHE_STUDIOHDR = 0,
   MDLCACHE_STUDIOHWDATA,
   MDLCACHE_VCOLLIDE,

   // Callbacks NOT called when data is loaded or unloaded for these:
   MDLCACHE_ANIMBLOCK,
   MDLCACHE_VIRTUALMODEL,
   MDLCACHE_VERTEXES,
   MDLCACHE_DECODEDANIMBLOCK
};

enum MDLCacheFlush_t {
   MDLCACHE_FLUSH_STUDIOHDR = 0x01,
   MDLCACHE_FLUSH_STUDIOHWDATA = 0x02,
   MDLCACHE_FLUSH_VCOLLIDE = 0x04,
   MDLCACHE_FLUSH_ANIMBLOCK = 0x08,
   MDLCACHE_FLUSH_VIRTUALMODEL = 0x10,
   MDLCACHE_FLUSH_AUTOPLAY = 0x20,
   MDLCACHE_FLUSH_VERTEXES = 0x40,

   MDLCACHE_FLUSH_IGNORELOCK = 0x80000000,
   MDLCACHE_FLUSH_ALL = 0xFFFFFFFF
};

enum SendPropType {
   DPT_Int = 0,
   DPT_Float,
   DPT_Vector,
   DPT_VectorXY,
   DPT_String,
   DPT_Array,
   DPT_DataTable,
   DPT_NUMSendPropTypes,
};

enum MapLoadType_t {
   MapLoad_NewGame = 0,
   MapLoad_LoadGame,
   MapLoad_Transition,
   MapLoad_Background,
};

enum ClientFrameStage_t {
   FRAME_UNDEFINED = -1,
   FRAME_START,
   FRAME_NET_UPDATE_START,
   FRAME_NET_UPDATE_POSTDATAUPDATE_START,
   FRAME_NET_UPDATE_POSTDATAUPDATE_END,
   FRAME_NET_UPDATE_END,
   FRAME_RENDER_START,
   FRAME_RENDER_END,
};
#pragma endregion

// 
// unimplemented
// 
#pragma region decl_unimplemented
struct model_t;

class SendProp;
#pragma endregion

// 
// types
// 
#pragma region decl_types
using CRC32_t = unsigned int;
using VPANEL = unsigned int;
using ModelInstanceHandle_t = unsigned short;
using ImageFormat = int;
using VertexFormat_t = int;
using MaterialPropertyTypes_t = int;
using OverrideType_t = int;
using LightCacheHandle_t = void*;
using ChangeCallback_t = void( * )( ConVar* var, const char* pOldValue, float flOldValue );
using StudioDecalHandle_t = void*;
using pixelvis_handle_t = int;
using str_32 = char[ 32 ];
using CreateClientClassFn = void* ( * )( int entnum, int serialNum );
using LeafIndex_t = unsigned short;
#pragma endregion

// 
// structs
// 
#pragma region decl_structs
struct string_t;
struct model_t;
struct mstudioanimdesc_t;
struct mstudioseqdesc_t;
struct Ray_t;
struct DrawModelInfo_t;
struct studiohwdata_t;
struct MaterialLightingState_t;
struct ColorMeshInfo_t;
struct MaterialSystem_Config_t;
struct MaterialSystemHWID_t;
struct AspectRatioInfo_t;
struct studiohwdata_t;
struct vcollide_t;
struct virtualmodel_t;
struct vertexFileHeader_t;
class CPhysCollide;
class ISpatialQuery;
class CUtlBuffer;
class CStudioHdr;
struct virtualmodel_t;
struct client_textmessage_t;
class CSentence;
class CAudioSource;
class SurfInfo;
struct Frustum_t;
class IAchievementMgr;
class AudioState_t;
class ISPSharedMemory;
class C_Beam;

struct hud_weapons_t {
   std::int32_t* get_weapon_count( ) {
	  return reinterpret_cast< std::int32_t* >( std::uintptr_t( this ) + 0x80 );
   }
};

struct IntRect {
   int x0;
   int y0;
   int x1;
   int y1;
};

struct Vertex_t {
   Vertex_t( ) { }
   Vertex_t( const Vector2D& pos, const Vector2D& coord = Vector2D( 0, 0 ) ) {
	  m_Position = pos;
	  m_TexCoord = coord;
   }
   void Init( const Vector2D& pos, const Vector2D& coord = Vector2D( 0, 0 ) ) {
	  m_Position = pos;
	  m_TexCoord = coord;
   }

   Vector2D m_Position;
   Vector2D m_TexCoord;
};

struct DrawModelState_t {
   studiohdr_t* m_pStudioHdr;
   studiohwdata_t* m_pStudioHWData;
   void* m_pRenderable;
   const matrix3x4_t* m_pModelToWorld;
   StudioDecalHandle_t     m_decals;
   int                     m_drawFlags;
   int                     m_lod;
};

struct LightingQuery_t {
   Vector                  m_LightingOrigin;
   ModelInstanceHandle_t   m_InstanceHandle;
   bool                    m_bAmbientBoost;
};

typedef struct InputContextHandle_t__* InputContextHandle_t;

struct BeamTrail_t {
   // NOTE:  Don't add user defined fields except after these four fields.
   BeamTrail_t* next;
   float			die;
   Vector			org;
   Vector			vel;
};

struct cplane_t {
   Vector normal;
   float dist;
   uint8_t type;   // for fast side tests
   uint8_t signbits;  // signx + (signy<<1) + (signz<<1)
   uint8_t pad[ 2 ];

};
struct MaterialVideoMode_t {
   int m_Width;			// if width and height are 0 and you select 
   int m_Height;			// windowed mode, it'll use the window size
   ImageFormat m_Format;	// use ImageFormats (ignored for windowed mode)
   int m_RefreshRate;		// 0 == default (ignored for windowed mode)
};
//-----------------------------------------------------------------------------
// Enumeration interface for EnumerateLinkEntities
//-----------------------------------------------------------------------------
class IEntityEnumerator {
public:
   // This gets called with each handle
   virtual bool EnumEntity( void* pHandleEntity ) = 0;
};

class C_CSPlayer;
struct BeamInfo_t {
   //Beam
   int				m_nType;
   C_CSPlayer* m_pStartEnt;
   int				m_nStartAttachment;
   C_CSPlayer* m_pEndEnt;
   int				m_nEndAttachment;
   Vector			m_vecStart;
   Vector			m_vecEnd;
   int				m_nModelIndex;
   const char* m_pszModelName;
   int				m_nHaloIndex;
   const char* m_pszHaloName;
   float			m_flHaloScale;
   float			m_flLife;
   float			m_flWidth;
   float			m_flEndWidth;
   float			m_flFadeLength;
   float			m_flAmplitude;
   float			m_flBrightness;
   float			m_flSpeed;
   int				m_nStartFrame;
   float			m_flFrameRate;
   float			m_flRed;
   float			m_flGreen;
   float			m_flBlue;
   bool			m_bRenderable;
   int				m_nSegments;
   int				m_nFlags;
   // Rings
   Vector			m_vecCenter;
   float			m_flStartRadius;
   float			m_flEndRadius;

   BeamInfo_t( ) {
	  m_nType = TE_BEAMPOINTS;
	  m_nSegments = -1;
	  m_pszModelName = NULL;
	  m_pszHaloName = NULL;
	  m_nModelIndex = -1;
	  m_nHaloIndex = -1;
	  m_bRenderable = true;
	  m_nFlags = 0;
   }
};
struct BrushSideInfo_t {
   Vector4D plane;               // The plane of the brush side
   unsigned short bevel;    // Bevel plane?
   unsigned short thin;     // Thin?
};

class CPhysCollide;


struct vcollide_t {
   unsigned short solidCount : 15;
   unsigned short isPacked : 1;
   unsigned short descSize;
   // VPhysicsSolids
   CPhysCollide** solids;
   char* pKeyValues;
   void* pUserData;
};

struct cmodel_t {
   Vector         mins, maxs;
   Vector         origin;        // for sounds or lights
   int            headnode;
   vcollide_t     vcollisionData;
};

struct csurface_t {
   const char* name;
   short          surfaceProps;
   unsigned short flags;         // BUGBUG: These are declared per surface, not per material, but this database is per-material now
};
struct ApplicationInstantCountersInfo_t {
   uint32_t m_nCpuActivityMask;
   uint32_t m_nDeferredWordsAllocated;
};
struct MaterialAdapterInfo_t {
   char m_pDriverName[ MATERIAL_ADAPTER_NAME_LENGTH ];
   unsigned int m_VendorID;
   unsigned int m_DeviceID;
   unsigned int m_SubSysID;
   unsigned int m_Revision;
   int m_nDXSupportLevel;			// This is the *preferred* dx support level
   int m_nMinDXSupportLevel;
   int m_nMaxDXSupportLevel;
   unsigned int m_nDriverVersionHigh;
   unsigned int m_nDriverVersionLow;
};
struct StaticLightingQuery_t : public LightingQuery_t {
   void* m_pRenderable;
};
struct StaticPropRenderInfo_t {
   const matrix3x4_t* pModelToWorld;
   const model_t* pModel;
   void* pRenderable;
   Vector* pLightingOrigin;
   short                   skin;
   ModelInstanceHandle_t   instance;
};

struct VPlane {
   Vector        m_Normal;
   float         m_Dist;
};

#pragma endregion

//typedefs
typedef VPlane Frustum[ FRUSTUM_NUMPLANES ];

// 
// classes
// 
#pragma region decl_classes
class IHandleEntity;
class IClientUnknown;
class ICollideable;
class IClientNetworkable;
class IClientRenderable;
class IClientEntity;
class C_BaseEntity;
class C_BaseAnimating;
class C_BaseCombatCharacter;
class C_BasePlayer;
class C_CSPlayer;
class C_BaseCombatWeapon;
class C_WeaponCSBaseGun;
class bf_write;
class bf_read;
class DVariant;
class CRecvProxyData;
class RecvProp;
class RecvTable;

class ClientClass;
class CUserCmd;
class CVerifiedUserCmd;
class CGlobalVarsBase;
class CGlobalVars;
class CMoveData;
class CGamestatsData;
class ITexture;
class CEnvTonemapContorller {
public:
	bool* m_bUseCustomAutoExposureMax( ) {
		return ( bool* )( ( DWORD )this + ( DWORD )0x9D9 );
	}

	bool* m_bUseCustomAutoExposureMin( ) {
		return ( bool* )( ( DWORD )this + ( DWORD )0x9D8 );
	}

	float* m_flCustomAutoExposureMax( ) {
		return ( float* )( ( DWORD )this + ( DWORD )0x9E0 );
	}

	float* m_flCustomAutoExposureMin( ) {
		return ( float* )( ( DWORD )this + ( DWORD )0x9DC );
	}
	bool* m_bUseCustomBloomScale( ) {
		return ( bool* )( ( DWORD )this + ( DWORD )0x9DA );
	}
	float* m_flCustomBloomScale( ) {
		return ( float* )( ( DWORD )this + ( DWORD )0x9E4 );
	}
	float* m_flCustomBloomScaleMinimum( ) {
		return ( float* )( ( DWORD )this + ( DWORD )0x9E8 );
	}
	float* m_flBloomExponent( ) {
		return ( float* )( ( DWORD )this + ( DWORD )0x9EC );
	}
	float* m_flBloomSaturation( ) {
		return ( float* )( ( DWORD )this + ( DWORD )0x9F0 );
	}
	float* m_flTonemapPercentTarget( ) {
		return ( float* )( ( DWORD )this + ( DWORD )0x9F4 );
	}
	float* m_flTonemapPercentBrightPixels( ) {
		return ( float* )( ( DWORD )this + ( DWORD )0x9F8 );
	}
	float* m_flTonemapMinAvgLum( ) {
		return ( float* )( ( DWORD )this + ( DWORD )0x9FC );
	}
	float* m_flTonemapRate( ) {
		return ( float* )( ( DWORD )this + ( DWORD )0xA00 );
	}
};
class IMaterialVar {
public:
   ITexture* GetTextureValue( ) {
	  return Memory::VCall< ITexture * ( __thiscall* )( void* ) >( this, 1 )( this );
   }

   void SetFloatValue( float value ) {
	  Memory::VCall< void( __thiscall* )( void*, float ) >( this, 4 )( this, value );
   }

   void SetIntValue( int value ) {
	  Memory::VCall< void( __thiscall* )( void*, int ) >( this, 5 )( this, value );
   }

   void SetStringValue( char const* value ) {
	  Memory::VCall< void( __thiscall* )( void*, char const* ) >( this, 6 )( this, value );
   }

   void SetVecValue( float value1, float value2, float value3 ) {
	  Memory::VCall< void( __thiscall* )( void*, float, float, float ) >( this, 11 )( this, value1, value2, value3 );
   }

   void SetVecComponent( const float value1, const int component ) {
      Memory::VCall< void( __thiscall* )( void*, float, int ) >( this, 26 )( this, value1, component );
   }

   void SetTextureValue( ITexture* tex ) {
	  // Memory::VCall< void( __thiscall* )( void*, ITexture* ) >( this, 15 )( this, tex );
   }
};
class KeyValues;
class IMaterialProxyFactory;
class IMaterialSystemHardwareConfig;
class CShadowMgr;
class IShader;
class IMaterial;
class CStudioHdr;
class IMatRenderContext;
class DataCacheHandle_t;
class CSteamAPIContext;
class KeyValues;
class CBaseHandle;
class IMatRenderContext {
public:
	auto release( void ) -> int {
		typedef int( __thiscall* original_fn )( void* );
		return Memory::VCall<original_fn>( this, 1 )( this );
	}

	auto set_render_target( ITexture* pTexture ) -> void {
		typedef void( __thiscall* original_fn )( void*, ITexture* );
		return Memory::VCall<original_fn>( this, 6 )( this, pTexture );
	}

	auto draw_screen_space_rect( IMaterial* pMaterial, int destX, int destY, int width, int height, float srcTextureX0, float srcTextureY0, float srcTextureX1, float srcTextureY1, int srcTextureWidth, int srcTextureHeight, void* pClientRenderable, int nXDice, int nYDice ) -> void {
		typedef void( __thiscall* original_fn )( void*, IMaterial*, int, int, int, int, float, float, float, float, int, int, void*, int, int );
		return Memory::VCall<original_fn>( this, 114 )( this, pMaterial, destX, destY, width, height, srcTextureX0, srcTextureY0, srcTextureX1, srcTextureY1, srcTextureWidth, srcTextureHeight, pClientRenderable, nXDice, nYDice );
	}

	auto push_render_target_and_viewport( void ) -> void {
		typedef void( __thiscall* original_fn )( void* );
		return Memory::VCall<original_fn>( this, 119 )( this );
	}

	auto pop_render_target_and_viewport( void ) -> void {
		typedef void( __thiscall* original_fn )( void* );
		return Memory::VCall<original_fn>( this, 120 )( this );
	}
};
#pragma endregion

// 
// interfaces
// 
#pragma region decl_interfaces
class IBaseClientDLL;
class IClientEntityList;
class IGameMovement;
class IPrediction;
class IMoveHelper;
class IInput;
class ICVar;
class IVEngineClient;
class IGameEventManager;
class IPanel;
class IMDLCache;
#pragma endregion


// 
// function types
// 
#pragma region decl_function_types
using CreateInterfaceFn = void* ( * )( const char*, int* );
using RecvVarProxyFn = void( * )( const CRecvProxyData*, void*, void* );
using pfnDemoCustomDataCallback = void( * )( uint8_t* pData, size_t iSize );
#pragma endregion


// 
// implementation
// 

#pragma region impl_structs
struct string_t {
public:
   const char* ToCStr( ) const;
protected:
   const char* pszValue;
};


#pragma endregion

#pragma region impl_classes


class DVariant {
public:
   union {
	  float m_Float;
	  int m_Int;
	  const char* m_pString;
	  void* m_pData;
	  float m_Vector[ 3 ];
   };
   SendPropType m_Type;
};

class CRecvProxyData {
public:
   const RecvProp* m_pRecvProp;
private:
   std::uint8_t __pad[ 4 ];
public:
   DVariant m_Value;
   int m_iElement;
   int m_ObjectID;
};

class RecvProp {
public:
   const char* m_pVarName;
   SendPropType m_RecvType;
   int m_Flags;
   int m_StringBufferSize;
   bool m_bInsideArray;
   const void* m_pExtraData;
   RecvProp* m_pArrayProp;
   void* m_ArrayLengthProxy;
   RecvVarProxyFn m_ProxyFn;
   void* m_DataTableProxyFn;
   RecvTable* m_pDataTable;
   int m_Offset;
   int m_ElementStride;
   int m_nElements;
   const char* m_pParentArrayPropName;
};


enum class MotionBlurMode_t {
   MOTION_BLUR_DISABLE = 1,
   MOTION_BLUR_GAME = 2,
   MOTION_BLUR_SFM = 3
};

class CViewSetup {
public:
	int x;
	int oldX;
	int y;
	int oldY;
	int width;
	int oldWidth;
	int height;
	int oldHeight;

	bool m_bOrtho;
	float m_OrthoLeft;
	float m_OrthoTop;
	float m_OrthoRight;
	float m_OrthoBottom;

private:
	char pad1[ 0x7C ];

public:
	float fov;
	float fovViewmodel;
	Vector origin;
	QAngle angles;

	float zNear;
	float zFar;
	float zNearViewmodel;
	float zFarViewmodel;

	float m_flAspectRatio;
	float m_flNearBlurDepth;
	float m_flNearFocusDepth;
	float m_flFarFocusDepth;
	float m_flFarBlurDepth;
	float m_flNearBlurRadius;
	float m_flFarBlurRadius;
	int m_nDoFQuality;
	MotionBlurMode_t m_nMotionBlurMode;

	float m_flShutterTime;
	Vector m_vShutterOpenPosition;
	QAngle m_shutterOpenAngles;
	Vector m_vShutterClosePosition;
	QAngle m_shutterCloseAngles;

	float m_flOffCenterTop;
	float m_flOffCenterBottom;
	float m_flOffCenterLeft;
	float m_flOffCenterRight;

	bool m_bOffCenter : 1;
	bool m_bRenderToSubrectOfLargerScreen : 1;
	bool m_bDoBloomAndToneMapping : 1;
	bool m_bDoDepthOfField : 1;
	bool m_bHDRTarget : 1;
	bool m_bDrawWorldNormal : 1;
	bool m_bCullFrontFaces : 1;
	bool m_bCacheFullSceneState : 1;
	bool m_bRenderFlashlightDepthTranslucents : 1;
private:
	char pad2[ 0x40 ];
};
struct ModelRenderInfo_t {
   Vector                  origin;
   QAngle                  angles;
  //  char					pad[ 0x4 ];
   IClientRenderable* pRenderable;
   const model_t* pModel;
   const matrix3x4_t* pModelToWorld;
   const matrix3x4_t* pLightingOffset;
   const Vector* pLightingOrigin;
   int                     flags;
   int                     entity_index;
   int                     skin;
   int                     body;
   int                     hitboxset;
   ModelInstanceHandle_t   instance;

   ModelRenderInfo_t( ) {
	  pModelToWorld = NULL;
	  pLightingOffset = NULL;
	  pLightingOrigin = NULL;
   }
};
class LightDesc_t {
public:
   LightType_t m_Type;                 //0x0000
   Vector m_Color;                     //0x0004
   Vector m_Position;                  //0x0010
   Vector m_Direction;                 //0x001C
   float m_Range;                      //0x0028
   float m_Falloff;                    //0x002C
   float m_Attenuation0;               //0x0030
   float m_Attenuation1;               //0x0034
   float m_Attenuation2;               //0x0038
   float m_Theta;                      //0x003C
   float m_Phi;                        //0x0040
   float m_ThetaDot;                   //0x0044
   float m_PhiDot;                     //0x0048
   float m_OneOverThetaDotMinusPhiDot; //0x004C
   __int32 m_Flags;                    //0x0050
   float m_RangeSquared;               //0x0054

}; //Size=0x0058

class lightpos_t {
public:
   Vector delta;  //0x0000
   float falloff; //0x000C
   float dot;     //0x0010

}; //Size=0x0014

struct MaterialLightingState_t {
   Vector m_vecAmbientCube[ 6 ]; // ambient, and lights that aren't in locallight[]
   Vector m_vecLightingOrigin; // The position from which lighting state was computed
   int m_nLocalLightCount;
   LightDesc_t m_pLocalLightDesc[ 4 ];
};
struct DrawModelInfo_t {
   studiohdr_t* m_pStudioHdr;
   studiohwdata_t* m_pHardwareData;
   StudioDecalHandle_t m_Decals;
   int m_Skin;
   int m_Body;
   int m_HitboxSet;
   IClientRenderable* m_pClientEntity;
   int m_Lod;
   ColorMeshInfo_t* m_pColorMeshes;
   bool m_bStaticLighting;
   MaterialLightingState_t m_LightingState;
};
class RecvTable {
public:
   RecvProp* m_pProps;
   int m_nProps;
   void* m_pDecoder;
   const char* m_pNetTableName;
   bool m_bInitialized;
   bool m_bInMainList;
};

class ClientClass {
public:
   void* m_pCreateFn;
   void* m_pCreateEventFn;
   const char* m_pNetworkName;
   RecvTable* m_pRecvTable;
   ClientClass* m_pNext;
   int m_ClassID;
};

//struct characterset_t
//{
//  char set[256];
//};

#include "IMemAlloc.hpp"

#include "IBaseClientDll.hpp"
#include "CUserCmd.hpp"
#include "IClientEntityList.hpp"
#include "IGameMovement.hpp"
#include "IPanel.hpp"
#include "IPrediction.hpp"
#include "IMoveHelper.hpp"
#include "IVEngineClient.hpp"
#include "IInput.hpp"
#include "ISurface.hpp"
#include "IConVar.hpp"
#include "IGameEventManager.hpp"
#include "IVModelRender.hpp"
#include "IMaterialSystem.hpp"
#include "ISteamClient.hpp"
#include "IPhysics.hpp"
#include "WeaponInfo.hpp"
#include "GlowOutlineEffect.hpp"
#include "IVModelInfo.hpp"
#include "CClientState.hpp"
#include "IVDebugOverlay.hpp"
#include "IEngineSound.hpp"
#include "IViewRenderBeams.hpp"
#include "ILocalize.hpp"
#include "INetChannel.hpp"
#include "INetMessage.hpp"
#include "IStudioRender.hpp"
#include "CPlayerResource.hpp"
#include "ICenterPrint.hpp"
#include "IVRenderView.hpp"
#include "IClientLeafSystem.hpp"
#include "IViewRender.hpp"

class NonCopyable {
protected:
   NonCopyable( ) { }
   ~NonCopyable( ) { }

private:
   NonCopyable( const NonCopyable& ) = delete;
   NonCopyable& operator=( const NonCopyable& ) = delete;

   NonCopyable( NonCopyable&& ) = delete;
   NonCopyable& operator=( NonCopyable&& ) = delete;
};
struct MaterialSystem_SortInfo_t {
   IMaterial* material;
   int			lightmapPageID;
};


class CGlobalVarsBase {
public:
   float realtime = 0.0f;
   int framecount = 0;
   float absoluteframetime = 0.0f;
   float absoluteframestarttimestddev = 0.0f;
   float curtime = 0.0f;
   float frametime = 0.0f;
   int maxClients = 0;
   int tickcount = 0;
   float interval_per_tick = 0.0f;
   float interpolation_amount = 0.0f;
   int simTicksThisFrame = 0;
   int network_protocol = 0;
   void* pSaveData = nullptr;
   bool m_bClient = false;
   int nTimestampNetworkingBase = 0;
   int nTimestampRandomizeWindow = 0;
};

class CGlobalVars : public CGlobalVarsBase {
public:
   string_t mapname = { };
   string_t mapGroupName = { };
   int mapversion = 0;
   string_t startspot = { };
   MapLoadType_t eLoadType = MapLoad_NewGame;
   bool bMapLoadFailed = false;
   bool deathmatch = false;
   bool coop = false;
   bool teamplay = false;
   int maxEntities = 0;
   int serverCount = 0;
   void* pEdicts = nullptr;
};


class CMoveData {
public:
   bool    m_bFirstRunOfFunctions : 1;
   bool    m_bGameCodeMovedPlayer : 1;
   int     m_nPlayerHandle;        // edict index on server, client entity handle on client=
   int     m_nImpulseCommand;      // Impulse command issued.
   QAngle  m_vecViewAngles;        // Command view angles (local space)
   Vector  m_vecAbsViewAngles;     // Command view angles (world space)
   int     m_nButtons;             // Attack buttons.
   int     m_nOldButtons;          // From host_client->oldbuttons;
   float   m_flForwardMove;
   float   m_flSideMove;
   float   m_flUpMove;
   float   m_flMaxSpeed;
   float   m_flClientMaxSpeed;
   Vector  m_vecVelocity;          // edict::velocity        // Current movement direction.
   QAngle  m_vecAngles;            // edict::angles
   Vector  m_vecOldAngles;
   float   m_outStepHeight;        // how much you climbed this move
   Vector  m_outWishVel;           // This is where you tried 
   Vector  m_outJumpVel;           // This is your jump velocity
   Vector  m_vecConstraintCenter;
   float   m_flConstraintRadius;
   float   m_flConstraintWidth;
   float   m_flConstraintSpeedFactor;
   float   m_flUnknown[ 5 ];
   Vector  m_vecAbsOrigin;
   char pad[ 1000 ] = {};
};

enum SoundServerFlags {
   SV_SND_STATIC_SOUND = ( 1 << 0 ),
   SV_SND_UPDATEPOS = ( 1 << 1 ),
   SV_SND_FROMSERVER = ( 1 << 2 ),
   SV_SND_STOPSOUND = ( 1 << 3 ),
};

struct StartSoundParams_t {
   StartSoundParams_t( ) :
	  staticsound( false ),
	  soundsource( 0 ),
	  entchannel( 0 ),
	  pSfx( 0 ),
	  fvol( 1.0f ),
	  soundlevel( SNDLVL_NORM ),
	  flags( 0 ),
	  pitch( PITCH_NORM ),
	  ServerFlags( 0 ),
	  delay( 0.0f ),
	  speakerentity( -1 ),
	  suppressrecording( false ),
	  initialStreamPosition( 0 ) {
	  origin.Init( );
	  direction.Init( );
   }

   bool staticsound;
   int soundsource;
   int entchannel;
   uintptr_t* pSfx;
   Vector origin;
   Vector direction;
   float fvol;
   soundlevel_t soundlevel;
   int flags;
   int pitch;
   float delay;
   int speakerentity;
   int unk;
   bool suppressrecording;
   int initialStreamPosition;
   int unk01;
   int unk02;
   const char* soundName;
   int unk03;
   int unk04;
   int ServerFlags; // SoundServerFlags
   int unk06;
};
#pragma endregion

#pragma region impl_interfaces
class IMDLCache : public IAppSystem {
public:
	virtual void                SetCacheNotify( void* pNotify ) = 0;
	virtual MDLHandle_t         FindMDL( const char* pMDLRelativePath ) = 0;
	virtual int32_t                 AddRef( MDLHandle_t handle ) = 0;
	virtual int32_t                 Release( MDLHandle_t handle ) = 0;
	virtual int32_t                 GetRef( MDLHandle_t handle ) = 0;
	virtual studiohdr_t* GetStudioHdr( MDLHandle_t handle ) = 0;
	virtual studiohwdata_t* GetHardwareData( MDLHandle_t handle ) = 0;
	virtual vcollide_t* GetVCollide( MDLHandle_t handle ) = 0;
	virtual unsigned char* GetAnimBlock( MDLHandle_t handle, int32_t nBlock ) = 0;
	virtual virtualmodel_t* GetVirtualModel( MDLHandle_t handle ) = 0;
	virtual int32_t                 GetAutoplayList( MDLHandle_t handle, unsigned short** pOut ) = 0;
	virtual vertexFileHeader_t* GetVertexData( MDLHandle_t handle ) = 0;
	virtual void                TouchAllData( MDLHandle_t handle ) = 0;
	virtual void                SetUserData( MDLHandle_t handle, void* pData ) = 0;
	virtual void* GetUserData( MDLHandle_t handle ) = 0;
	virtual bool                IsErrorModel( MDLHandle_t handle ) = 0;
	virtual void                Flush( MDLCacheFlush_t nFlushFlags = MDLCACHE_FLUSH_ALL ) = 0;
	virtual void                Flush( MDLHandle_t handle, int32_t nFlushFlags = MDLCACHE_FLUSH_ALL ) = 0;
	virtual const char* GetModelName( MDLHandle_t handle ) = 0;
	virtual virtualmodel_t* GetVirtualModelFast( const studiohdr_t* pStudioHdr, MDLHandle_t handle ) = 0;
	virtual void                BeginLock( ) = 0;
	virtual void                EndLock( ) = 0;
	virtual int32_t* GetFrameUnlockCounterPtrOLD( ) = 0;
	virtual void                FinishPendingLoads( ) = 0;
	virtual vcollide_t* GetVCollideEx( MDLHandle_t handle, bool synchronousLoad = true ) = 0;
	virtual bool                GetVCollideSize( MDLHandle_t handle, int32_t* pVCollideSize ) = 0;
	virtual bool                GetAsyncLoad( MDLCacheDataType_t type ) = 0;
	virtual bool                SetAsyncLoad( MDLCacheDataType_t type, bool bAsync ) = 0;
	virtual void                BeginMapLoad( ) = 0;
	virtual void                EndMapLoad( ) = 0;
	virtual void                MarkAsLoaded( MDLHandle_t handle ) = 0;
	virtual void                InitPreloadData( bool rebuild ) = 0;
	virtual void                ShutdownPreloadData( ) = 0;
	virtual bool                IsDataLoaded( MDLHandle_t handle, MDLCacheDataType_t type ) = 0;
	virtual int32_t* GetFrameUnlockCounterPtr( MDLCacheDataType_t type ) = 0;
	virtual studiohdr_t* LockStudioHdr( MDLHandle_t handle ) = 0;
	virtual void                UnlockStudioHdr( MDLHandle_t handle ) = 0;
	virtual bool                PreloadModel( MDLHandle_t handle ) = 0;
	virtual void                ResetErrorModelStatus( MDLHandle_t handle ) = 0;
	virtual void                MarkFrame( ) = 0;
	virtual void                BeginCoarseLock( ) = 0;
	virtual void                EndCoarseLock( ) = 0;
	virtual void                ReloadVCollide( MDLHandle_t handle ) = 0;
};

#pragma endregion

#pragma region decl_functions
void RandomSeed( unsigned int seed );
float RandomFloat( float min, float max );
int RandomInt( int min, int max );

void CRC32_Init( CRC32_t* pulCRC );
void CRC32_ProcessBuffer( CRC32_t* pulCRC, const void* p, int len );
void CRC32_Final( CRC32_t* pulCRC );
CRC32_t CRC32_GetTableEntry( unsigned int slot );

template<class T>
static T FindHudElement( const char* name ) {
   static auto pThis = *reinterpret_cast< uintptr_t * * >( Engine::Displacement.Data.m_uHudElement );
   if ( !pThis )
	  return ( T ) 0;

   static auto find_hud_element = reinterpret_cast< uintptr_t( __thiscall* )( void*, const char* ) >( Engine::Displacement.Function.m_uFindHudElement );
   return ( T ) find_hud_element( pThis, name );
}

extern const CRC32_t pulCRCTable[ 256 ];
#pragma endregion

#define XOR_VAL xor_val
#define ADD_VAL 0x54F12F43

#ifdef DevelopMode
template<typename T>
class Encrypted_t {
public:
   T* pointer;

   __forceinline Encrypted_t( T* ptr ) {
	  pointer = ptr;
   }

   __forceinline  T* Xor( ) const {
	  return  pointer;
   }

   __forceinline  T* operator-> ( ) {
	  return Xor( );
   }

   __forceinline bool IsValid( ) const {
	  return pointer != nullptr;
   }
};
#else
#pragma  optimize( "", off ) 
template<typename T>
class Encrypted_t {
   __forceinline uintptr_t rotate_dec( uintptr_t c ) const {
	  return c;
	  //return ( ( c & 0xFFFF ) << 16 | ( c & 0xFFFF0000 ) >> 16 );
   #if 0
	  return ( c & 0xF ) << 28 | ( c & 0xF0000000 ) >> 28
		 | ( c & 0xF0 ) << 20 | ( c & 0x0F000000 ) >> 20
		 | ( c & 0xF00 ) << 12 | ( c & 0x00F00000 ) >> 12
		 | ( c & 0xF000 ) << 4 | ( c & 0x000F0000 ) >> 4;
   #endif
   }
public:
   uintptr_t np;
   uintptr_t xor_val;

   __forceinline Encrypted_t( T* ptr ) {
	  auto p = &ptr;
	  xor_val = rotate_dec( pulCRCTable[ *( ( uint8_t* ) p + 1 ) + ( ( ( uintptr_t( ptr ) >> 16 ) & 7 ) << 8 ) ] );
	  np = rotate_dec( rotate_dec( xor_val ) ^ ( uintptr_t( ptr ) + ADD_VAL ) );
   }

   __forceinline  T* Xor( ) const {
	  return ( T* ) ( ( uintptr_t ) ( rotate_dec( np ) ^ rotate_dec( xor_val ) ) - ADD_VAL );
   }

   __forceinline  T* operator-> ( ) {
	  return Xor( );
   }

   __forceinline bool IsValid( ) const {
	  return ( ( uintptr_t ) ( rotate_dec( np ) ^ rotate_dec( xor_val ) ) - ADD_VAL ) != 0;
   }
};
#pragma  optimize( "", on )

#endif

// example: 
#if 0
struct TestStructure : public Encrypted_t< TestStructure >, public Base {
   int memes;

   __declspec( noinline ) virtual void Test( ) {
	  Xor( )->memes = 1337;
	  printf( "%d\n", Xor( )->memes );
	  Xor( )->memes = 322;
	  printf( "%d\n", Xor( )->memes );
   }
};

TestStructure str;
int main( ) {
   str->memes = 69;
   printf( "%d\n", str.memes );
   str.Test( );
   return 0;
}

#endif

class NoticeText_t {
public:
	wchar_t m_nString[ 512 ]; //0x0000 
	char pad_0x0400[ 0xC ]; //0x0400
	float set; //0x040C
	float m_flStartTime; //0x0410 
	float m_flStartTime2; //0x0414 
	float m_flLifeTimeModifier; //0x0418 
	char pad_0x041C[ 0x4 ]; //0x041C
}; //Size=0x420


class SFHudDeathNoticeAndBotStatus {
public:
	char pad_0000[ 28 ]; //0x0000
	char* m_szHudName; //0x001C
	char pad_0020[ 64 ]; //0x0020
	float m_flTime; //0x0060
	char pad_0064[ 4 ]; //0x0064
	CUtlVector< NoticeText_t > m_vecDeathNotices;
};

class CHud {
public:
	template< class T >
	T FindHudElement( const char* name ) {
		static auto FindHudElement_t = reinterpret_cast< uintptr_t( __thiscall* )( void*, const char* ) >( Memory::Scan( XorStr( "client.dll" ), XorStr( "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28" ) ) );

		return reinterpret_cast< T >( FindHudElement_t( this, name ) );
	}
};

/* game effects */
class IPredictionSystem {
public:
	virtual ~IPredictionSystem( ) { }

	IPredictionSystem* pNextSystem; // 0x04
	bool bSuppressEvent; // 0x08
	C_BaseEntity* pSuppressHost; // 0x0C // i guess
	int nStatusPushed; // 0x10
};
static_assert( sizeof( IPredictionSystem ) == 0x14 );

class IEffects : public IPredictionSystem {
public:
	virtual ~IEffects( ) { };
	virtual void Beam( const Vector& Start, const Vector& End, int nModelIndex,
					   int nHaloIndex, unsigned char frameStart, unsigned char frameRate,
					   float flLife, unsigned char width, unsigned char endWidth, unsigned char fadeLength,
					   unsigned char noise, unsigned char red, unsigned char green,
					   unsigned char blue, unsigned char brightness, unsigned char speed ) = 0;

	virtual void Smoke( const Vector& origin, int modelIndex, float scale, float framerate ) = 0;
	virtual void Sparks( const Vector& position, int nMagnitude = 1, int nTrailLength = 1, const Vector* pvecDir = NULL ) = 0;
	virtual void Dust( const Vector& pos, const Vector& dir, float size, float speed ) = 0;
	virtual void MuzzleFlash( const Vector& vecOrigin, const Vector& vecAngles, float flScale, int iType ) = 0;
	virtual void MetalSparks( const Vector& position, const Vector& direction ) = 0;
	virtual void EnergySplash( const Vector& position, const Vector& direction, bool bExplosive = false ) = 0;
	virtual void Ricochet( const Vector& position, const Vector& direction ) = 0;

	virtual float Time( ) = 0;
	virtual bool IsServer( ) = 0;

	virtual void SuppressEffectsSounds( bool bSuppress ) = 0;
};