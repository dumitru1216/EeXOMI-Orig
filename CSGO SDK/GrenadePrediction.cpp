#include "GrenadePrediction.hpp"
#include "player.hpp"
#include "CBaseHandle.hpp"
#include "entity.hpp"
#include "weapon.hpp"
#include "Render.hpp"
#include "Autowall.h"

class CGrenadePrediction : public IGrenadePrediction {
public:
   void Tick( int buttons ) override;
   void View( ) override;
   void Paint( ) override;
private:
   void Setup( C_BasePlayer* pl, Vector& vecSrc, Vector& vecThrow, const QAngle& angEyeAngles );
   void Simulate( QAngle& Angles, C_BasePlayer* pLocal );
   int Step( Vector& vecSrc, Vector& vecThrow, int tick, float interval );
   bool CheckDetonate( const Vector& vecThrow, const CGameTrace& tr, int tick, float interval );
   void TraceHull( Vector& src, Vector& end, CGameTrace& tr );
   void AddGravityMove( Vector& move, Vector& vel, float frametime, bool onground );
   void PushEntity( Vector& src, const Vector& move, CGameTrace& tr );
   void ResolveFlyCollisionCustom( CGameTrace& tr, Vector& vecVelocity, float interval );
   int PhysicsClipVelocity( const Vector& in, const Vector& normal, Vector& out, float overbounce );

   enum {
	  ACT_NONE,
	  ACT_LOB,
	  ACT_DROP,
	  ACT_THROW
   };

   int act = 0;
   int type = 0;
   std::vector<Vector> path;
   std::vector<std::pair<Vector, QAngle>> OtherCollisions;
   std::vector< C_BaseEntity* > ignore_entities;
   Color TracerColor = Color( 255, 255, 0, 255 );
   bool firegrenade_didnt_hit = false;
};

class CPredTraceFilter : public ITraceFilter {
public:
   CPredTraceFilter( ) = default;

   bool ShouldHitEntity( IHandleEntity* pEntityHandle, int /*contentsMask*/ ) {
	  auto it = std::find( entities.begin( ), entities.end( ), pEntityHandle );
	  if ( it != entities.end( ) )
		 return false;

	  ClientClass* pEntCC = ( ( IClientEntity* ) pEntityHandle )->GetClientClass( );
	  if ( pEntCC && strcmp( ccIgnore, "" ) ) {
		 if ( pEntCC->m_pNetworkName == ccIgnore )
			return false;
	  }

	  return true;
   }

   virtual TraceType GetTraceType( ) const { return TraceType::TRACE_EVERYTHING; }

   inline void SetIgnoreClass( const char* Class ) { ccIgnore = Class; }

   std::vector< C_BaseEntity* > entities;
   const char* ccIgnore = "";
};

IGrenadePrediction* IGrenadePrediction::Get( ) {
   static CGrenadePrediction instance;
   return &instance;
}

void CGrenadePrediction::Tick( int buttons ) {
   if ( !g_Vars.esp.NadePred )
	  return;

   bool in_attack = ( buttons & IN_ATTACK );
   bool in_attack2 = ( buttons & IN_ATTACK2 );

   act = ( in_attack && in_attack2 ) ? ACT_DROP :
	  ( in_attack2 ) ? ACT_THROW :
	  ( in_attack ) ? ACT_LOB :
	  ACT_NONE;
}

void CGrenadePrediction::View( ) {
   if ( !g_Vars.esp.NadePred )
	  return;

   C_CSPlayer* g_LocalPlayer = C_CSPlayer::GetLocalPlayer( );

   if ( !g_LocalPlayer || g_LocalPlayer->IsDead( ) )
	  return;

   auto weapon = ( C_WeaponCSBaseGun* ) g_LocalPlayer->m_hActiveWeapon( ).Get( );
   if ( !weapon )
	  return;

   auto weapon_data = weapon->GetCSWeaponData( ).Xor( );

   if ( !weapon_data )
	  return;

   if ( ( weapon_data->m_iWeaponType == WEAPONTYPE_GRENADE ) && act != ACT_NONE ) {
	  QAngle angThrow;
	  Source::m_pEngine->GetViewAngles( angThrow );

	  ClientClass* pWeaponClass = weapon->GetClientClass( );
	  if ( !pWeaponClass ) {
		 type = -1;
		 Simulate( angThrow, g_LocalPlayer );
	  } else {
		 type = pWeaponClass->m_ClassID;
		 Simulate( angThrow, g_LocalPlayer );
	  }
   } else {
	  type = -1;
   }
}

inline float CSGO_Armor( float flDamage, int ArmorValue ) {
   float flArmorRatio = 0.5f;
   float flArmorBonus = 0.5f;
   if ( ArmorValue > 0 ) {
	  float flNew = flDamage * flArmorRatio;
	  float flArmor = ( flDamage - flNew ) * flArmorBonus;

	  if ( flArmor > static_cast< float >( ArmorValue ) ) {
		 flArmor = static_cast< float >( ArmorValue ) * ( 1.f / flArmorBonus );
		 flNew = flDamage - flArmor;
	  }

	  flDamage = flNew;
   }
   return flDamage;
}


void CGrenadePrediction::Paint( ) {
   if ( !g_Vars.esp.NadePred )
	  return;

   C_CSPlayer* g_LocalPlayer = C_CSPlayer::GetLocalPlayer( );

   if ( !g_LocalPlayer || g_LocalPlayer->IsDead( ) )
	  return;

   auto weapon = ( C_WeaponCSBaseGun* ) g_LocalPlayer->m_hActiveWeapon( ).Get( );
   if ( !weapon )
	  return;

   auto weapon_data = weapon->GetCSWeaponData( ).Xor( );

   if ( !weapon_data )
	  return;

   if ( ( type ) && path.size( ) > 1 && act != ACT_NONE && weapon_data->m_iWeaponType == WEAPONTYPE_GRENADE ) {
	  Vector2D ab, cd;
	  Vector prev = path[ 0 ];
	  for ( auto it = path.begin( ), end = path.end( ); it != end; ++it ) {
		 if ( WorldToScreen( prev, ab ) && WorldToScreen( *it, cd ) ) {
			Render::Get( )->AddLine( ab, cd, g_Vars.esp.nade_pred_color );
		 }
		 prev = *it;
	  }

	  Vector2D temp_point;
	  for ( auto it = OtherCollisions.begin( ), end = OtherCollisions.end( ); it != end; ++it ) {
		 if ( WorldToScreen( it->first, temp_point ) ) {
			Render::Get( )->AddCircleFilled( { temp_point.x, temp_point.y }, 3.0f, Color( 0, 255, 0, 200 ).GetD3DColor( ) );
			Render::Get( )->AddCircle( { temp_point.x, temp_point.y }, 3.5f, FloatColor( 0.05f, 0.05f, 0.05f, 1.0f ).Hex( ) );
		 }
	  }

	  if ( WorldToScreen( OtherCollisions.rbegin( )->first, temp_point ) ) {
		 Render::Get( )->AddCircleFilled( { temp_point.x, temp_point.y }, 3.0f, Color( 255, 0, 0, 200 ).GetD3DColor( ) );
		 Render::Get( )->AddCircle( { temp_point.x, temp_point.y }, 3.5f, FloatColor( 0.05f, 0.05f, 0.05f, 1.0f ).Hex( ) );
	  }

	  std::string EntName;
	  auto bestdmg = 0;
	  static Color redcol = { 255, 0, 0, 255 };
	  static Color greencol = { 25, 255, 25, 255 };
	  static Color yellowgreencol = { 177, 253, 2, 255 };
	  static Color yellowcol = { 255, 255, 0, 255 };
	  static Color orangecol = { 255, 128, 0, 255 };
	  Color* BestColor = &redcol;

	  Vector endpos = path[ path.size( ) - 1 ];
	  Vector absendpos = endpos;

	  float totaladded = 0.0f;

	  while ( totaladded < 30.0f ) {
		 if ( Source::m_pEngineTrace->GetPointContents( endpos ) == CONTENTS_EMPTY )
			break;

		 totaladded += 2.0f;
		 endpos.z += 2.0f;
	  }

	  int weap_id = weapon->m_iItemDefinitionIndex( );

	  if ( weapon &&
		   weap_id == WEAPON_HEGRENADE ||
		   weap_id == WEAPON_MOLOTOV ||
		   weap_id == WEAPON_INC ) {
		 for ( int i = 1; i < 64; i++ ) {
			C_CSPlayer* pEntity = ( C_CSPlayer* ) Source::m_pEntList->GetClientEntity( i );

			if ( !pEntity || pEntity->m_iTeamNum( ) == g_LocalPlayer->m_iTeamNum( ) )
			   continue;

			float dist = ( pEntity->m_vecOrigin( ) - endpos ).Length( );

			if ( dist < 350.0f ) {
			   CTraceFilter filter;
			   filter.pSkip = g_LocalPlayer;
			   Ray_t ray;
			   Vector2D NadeScreen;
			   WorldToScreen( endpos, NadeScreen );

			   auto studio_model = Source::m_pModelInfo->GetStudiomodel( pEntity->GetModel( ) );
			   auto hitboxSet = studio_model->pHitboxSet( pEntity->m_nHitboxSet( ) );

			   auto GetHitboxPos = [&] ( int id ) -> Vector {
				  if ( studio_model ) {
					 auto hitbox = hitboxSet->pHitbox( id );
					 if ( hitbox ) {
						auto
						   min = Vector{},
						   max = Vector{};

						auto pos = ( ( hitbox->bbmin + hitbox->bbmax ) * 0.5f ).Transform( pEntity->m_CachedBoneData( ).m_Memory.m_pMemory[ hitbox->bone ] );
						return pos;
					 }
				  }
			   };

			   Vector vPelvis = GetHitboxPos( HITBOX_PELVIS );
			   ray.Init( endpos, vPelvis );
			   CGameTrace ptr;
			   Source::m_pEngineTrace->TraceRay( ray, MASK_SHOT, &filter, &ptr );

			   if ( ptr.hit_entity == pEntity ) {
				  Vector2D PelvisScreen;

				  WorldToScreen( vPelvis, PelvisScreen );

				  static float a = 105.0f;
				  static float b = 25.0f;
				  static float c = 140.0f;

				  float d = ( ( ( ( pEntity->m_vecOrigin( ) ) - prev ).Length( ) - b ) / c );
				  float flDamage = a * exp( -d * d );
				  auto dmg = std::max( static_cast< int >( ceilf( CSGO_Armor( flDamage, pEntity->m_ArmorValue( ) ) ) ), 0 );

				  Color* destcolor = dmg >= 65 ? &redcol : dmg >= 40.0f ? &orangecol : dmg >= 20.0f ? &yellowgreencol : &greencol;

				  if ( dmg > bestdmg ) {
					 player_info_t info;
					 if ( Source::m_pEngine->GetPlayerInfo( pEntity->entindex( ), &info ) )
						EntName = info.szName;

					 BestColor = destcolor;
					 bestdmg = dmg;
				  }
			   }
			}
		 }
	  }

	  if ( bestdmg > 0.f && weapon->m_iItemDefinitionIndex( ) == WEAPON_HEGRENADE ) {
		 if ( WorldToScreen( *path.begin( ), cd ) ) {
			Render::Get( )->SetTextFont( FONT_VISITOR );
			Render::Get( )->AddText( Vector2D( cd[ 0 ], cd[ 1 ] - 10 ), BestColor->GetD3DColor( ), CENTER_X | CENTER_Y, ( "Most damage dealt to: " + EntName + " -" + std::to_string( bestdmg ) ).c_str( ) );
		 }
	  }
   }
}

void CGrenadePrediction::Setup( C_BasePlayer* pl, Vector& vecSrc, Vector& vecThrow, const QAngle& angEyeAngles ) {
   QAngle angThrow = angEyeAngles;
   float pitch = angThrow.pitch;

   if ( pitch <= 90.0f ) {
	  if ( pitch < -90.0f ) {
		 pitch += 360.0f;
	  }
   } else {
	  pitch -= 360.0f;
   }
   float a = pitch - ( 90.0f - fabs( pitch ) ) * 10.0f / 90.0f;
   angThrow.pitch = a;

   // Gets ThrowVelocity from weapon files
   // Clamped to [15,750]
   float flVel = 750.0f * 0.9f;

   // Do magic on member of grenade object [esi+9E4h]
   // m1=1  m1+m2=0.5  m2=0
   static const float power[] = { 1.0f, 1.0f, 0.5f, 0.0f };
   float b = power[ act ];
   // Clamped to [0,1]
   b = b * 0.7f;
   b = b + 0.3f;
   flVel *= b;

   Vector vForward, vRight, vUp;
   vForward = angThrow.ToVectors( &vRight, &vUp );

   vecSrc = pl->m_vecOrigin( );
   vecSrc += pl->m_vecViewOffset( );
   float off = ( power[ act ] * 12.0f ) - 12.0f;
   vecSrc.z += off;

   // Game calls UTIL_TraceHull here with hull and assigns vecSrc tr.endpos
   CGameTrace tr;
   Vector vecDest = vecSrc;
   vecDest = ( vecDest + vForward * 22.0f );
   TraceHull( vecSrc, vecDest, tr );

   // After the hull trace it moves 6 units back along vForward
   // vecSrc = tr.endpos - vForward * 6
   Vector vecBack = vForward; vecBack *= 6.0f;
   vecSrc = tr.endpos;
   vecSrc -= vecBack;

   // Finally calculate velocity
   vecThrow = pl->m_vecVelocity( ); vecThrow *= 1.25f;
   vecThrow = vecThrow + vForward * flVel;
}

void CGrenadePrediction::Simulate( QAngle& Angles, C_BasePlayer* pLocal ) {
   Vector vecSrc, vecThrow;
   Setup( pLocal, vecSrc, vecThrow, Angles );

   float interval = Source::m_pGlobalVars->interval_per_tick;

   // Log positions 20 times per sec
   int logstep = static_cast< int >( 0.05f / interval );
   int logtimer = 0;

   ignore_entities.clear( );
   path.clear( );
   OtherCollisions.clear( );

   ignore_entities.push_back( pLocal );

   TracerColor = Color( 255, 255, 0, 255 );
   for ( unsigned int i = 0; i < path.max_size( ) - 1; ++i ) {
	  if ( !logtimer )
		 path.push_back( vecSrc );

	  int s = Step( vecSrc, vecThrow, i, interval );
	  if ( ( s & 1 ) || vecThrow == Vector( 0, 0, 0 ) )
		 break;

	  // Reset the log timer every logstep OR we bounced
	  if ( ( s & 2 ) || logtimer >= logstep ) logtimer = 0;
	  else ++logtimer;
   }
   path.push_back( vecSrc );
}

int CGrenadePrediction::Step( Vector& vecSrc, Vector& vecThrow, int tick, float interval ) {
   // Apply gravity
   Vector move;
   AddGravityMove( move, vecThrow, interval, false );

   // Push entity
   CGameTrace tr;
   PushEntity( vecSrc, move, tr );

   int result = 0;
   // Check ending conditions
   if ( CheckDetonate( vecThrow, tr, tick, interval ) ) {
	  result |= 1;
   }

   // Resolve collisions
   if ( tr.fraction != 1.0f ) {
	  result |= 2; // Collision!
	  ResolveFlyCollisionCustom( tr, vecThrow, interval );
   }

   if ( tr.hit_entity && ( ( C_BasePlayer* ) tr.hit_entity )->IsPlayer( ) ) {
	  TracerColor = Color( 255, 0, 0, 255 );
   }

   if ( ( result & 1 ) || vecThrow == Vector( 0, 0, 0 ) || tr.fraction != 1.0f ) {
	  QAngle angles;
	  Math::AngleVectors( angles, ( tr.endpos - tr.startpos ).Normalized( ) );
	  OtherCollisions.push_back( std::make_pair( tr.endpos, angles ) );
   }

   // Set new position
   vecSrc = tr.endpos;

   return result;
}

bool CGrenadePrediction::CheckDetonate( const Vector& vecThrow, const CGameTrace& tr, int tick, float interval ) {
   firegrenade_didnt_hit = false;
   switch ( type ) {
	  case CSmokeGrenade:
	  case CDecoyGrenade:
	  // Velocity must be <0.1, this is only checked every 0.2s
	  if ( vecThrow.Length( ) < 0.1f ) {
		 int det_tick_mod = static_cast< int >( 0.2f / interval );
		 return !( tick % det_tick_mod );
	  }
	  return false;

	  /* TIMES AREN'T COMPLETELY RIGHT FROM WHAT I'VE SEEN ! ! ! */
	  case CMolotovGrenade:
	  case CIncendiaryGrenade:
	  // Detonate when hitting the floor
	  if ( tr.fraction != 1.0f && tr.plane.normal.z > 0.7f )
		 return true;
	  // OR we've been flying for too long

	  case CFlashbang:
	  case CHEGrenade:
	  {
		 // Pure timer based, detonate at 1.5s, checked every 0.2s
		 firegrenade_didnt_hit = static_cast< float >( tick ) * interval > 1.5f && !( tick % static_cast< int >( 0.2f / interval ) );
		 return firegrenade_didnt_hit;
	  }
	  default:
	  Assert( false );
	  return false;
   }
}

void CGrenadePrediction::TraceHull( Vector& src, Vector& end, CGameTrace& tr ) {
   // Setup grenade hull
   static const Vector hull[ 2 ] = { Vector( -2.0f, -2.0f, -2.0f ), Vector( 2.0f, 2.0f, 2.0f ) };

   CPredTraceFilter filter;
   filter.SetIgnoreClass( XorStr( "CBaseCSGrenadeProjectile" ) );
   filter.entities = ignore_entities;

   Ray_t ray;
   ray.Init( src, end, hull[ 0 ], hull[ 1 ] );

   const unsigned int mask = 0x200400B;
   Source::m_pEngineTrace->TraceRay( ray, mask, &filter, &tr );
}

void CGrenadePrediction::AddGravityMove( Vector& move, Vector& vel, float frametime, bool onground ) {
   Vector basevel( 0.0f, 0.0f, 0.0f );

   move.x = ( vel.x + basevel.x ) * frametime;
   move.y = ( vel.y + basevel.y ) * frametime;

   if ( onground ) {
	  move.z = ( vel.z + basevel.z ) * frametime;
   } else {
	  // Game calls GetActualGravity( this );
	  float gravity = 800.0f * 0.4f;

	  float newZ = vel.z - ( gravity * frametime );
	  move.z = ( ( vel.z + newZ ) / 2.0f + basevel.z ) * frametime;

	  vel.z = newZ;
   }
}

void CGrenadePrediction::PushEntity( Vector& src, const Vector& move, CGameTrace& tr ) {
   Vector vecAbsEnd = src;
   vecAbsEnd += move;

   // Trace through world
   TraceHull( src, vecAbsEnd, tr );
}

void CGrenadePrediction::ResolveFlyCollisionCustom( CGameTrace& tr, Vector& vecVelocity, float interval ) {
   // Calculate elasticity
   float flSurfaceElasticity = 1.0;  // Assume all surfaces have the same elasticity

	//Don't bounce off of players with perfect elasticity
   if ( tr.hit_entity && ( ( C_BaseEntity* ) tr.hit_entity )->IsPlayer( ) ) {
	  flSurfaceElasticity = 0.3f;
   }

   // if its breakable glass and we kill it, don't bounce.
   // give some damage to the glass, and if it breaks, pass
   // through it.
   if ( Autowall::IsBreakable( ( C_BaseEntity* ) tr.hit_entity ) ) {
	  if ( ( ( C_CSPlayer* ) tr.hit_entity )->m_iHealth( ) <= 10 ) { // todo: need scale damage and other stuff
		 ignore_entities.push_back( ( C_BaseEntity* ) tr.hit_entity );

		 // slow our flight a little bit
		 vecVelocity *= 0.4f;
		 return;
	  }
   }

   float flGrenadeElasticity = 0.45f; // GetGrenadeElasticity()
   float flTotalElasticity = flGrenadeElasticity * flSurfaceElasticity;
   if ( flTotalElasticity > 0.9f ) flTotalElasticity = 0.9f;
   if ( flTotalElasticity < 0.0f ) flTotalElasticity = 0.0f;

   // Calculate bounce
   Vector vecAbsVelocity;
   PhysicsClipVelocity( vecVelocity, tr.plane.normal, vecAbsVelocity, 2.0f );
   vecAbsVelocity *= flTotalElasticity;

   // Stop completely once we move too slow
   float flSpeedSqr = vecAbsVelocity.Length2DSquared( );
   static const float flMinSpeedSqr = 20.0f * 20.0f; // 30.0f * 30.0f in CSS
   if ( flSpeedSqr < flMinSpeedSqr ) {
	  vecAbsVelocity.x = vecAbsVelocity.y = vecAbsVelocity.z = 0;
   }

   // Stop if on ground
   if ( tr.plane.normal.z > 0.7f ) {
	  vecVelocity = vecAbsVelocity;
	  vecAbsVelocity.Mul( ( 1.0f - tr.fraction ) * interval );
	  PushEntity( tr.endpos, vecAbsVelocity, tr );
   } else {
	  vecVelocity = vecAbsVelocity;
   }
}

int CGrenadePrediction::PhysicsClipVelocity( const Vector& in, const Vector& normal, Vector& out, float overbounce ) {
   static const float STOP_EPSILON = 0.1f;

   float    backoff;
   float    change;
   float    angle;
   int        i, blocked;

   blocked = 0;

   angle = normal[ 2 ];

   if ( angle > 0 ) {
	  blocked |= 1;        // floor
   }
   if ( !angle ) {
	  blocked |= 2;        // step
   }

   backoff = in.Dot( normal ) * overbounce;

   for ( i = 0; i < 3; i++ ) {
	  change = normal[ i ] * backoff;
	  out[ i ] = in[ i ] - change;
	  if ( out[ i ] > -STOP_EPSILON && out[ i ] < STOP_EPSILON ) {
		 out[ i ] = 0;
	  }
   }

   return blocked;
}
