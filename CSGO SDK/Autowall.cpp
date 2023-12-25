#include "Autowall.h"
#include "displacement.hpp"
#include "player.hpp"
#include "weapon.hpp"

//#define DEBUG_AUTOWALL

#ifdef DEBUG_AUTOWALL
#include XorStr( "InputSys.hpp" )
#define DEBUG_DURATION Source::m_pCvar->FindVar(XorStr( "sv_showimpacts_time" ))->GetFloat()
#endif

// IsBreakableEntity
// https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/game/shared/obstacle_pushaway.cpp
bool Autowall::IsBreakable( C_BaseEntity* entity, int contents ) {
   if ( !entity || entity->m_entIndex == 0 )
	  return false;

   auto m_take_damage = *( uintptr_t* ) ( ( uintptr_t ) Engine::Displacement.Function.m_uIsBreakable + 38 );
   auto backup = *( uint8_t* ) ( ( uintptr_t ) entity + m_take_damage );

   // fix world desync between server and client ds
   auto client_class = entity->GetClientClass( );
   auto hash_name = hash_32_fnv1a_const( client_class->m_pNetworkName );
   if ( hash_name == hash_32_fnv1a_const( "CBreakableSurface" ) )
	  *( uint8_t* ) ( ( uintptr_t ) entity + m_take_damage ) = 2; // DAMAGE_YES
   else if ( hash_name == hash_32_fnv1a_const( "CBaseDoor" ) || hash_name == hash_32_fnv1a_const( ( "CDynamicProp" ) ) )
	  *( uint8_t* ) ( ( uintptr_t ) entity + m_take_damage ) = 0; // DAMAGE_NO	

   using fn_t = bool( __thiscall* )( C_BaseEntity* );
   auto result = ( ( fn_t ) Engine::Displacement.Function.m_uIsBreakable )( entity );
   *( uint8_t* ) ( ( uintptr_t ) entity + m_take_damage ) = backup;

   return result;
}

bool Autowall::IsArmored( C_CSPlayer* player, int hitgroup ) {
   auto has_helmet = player->m_bHasHelmet( );
   auto armor_value = static_cast< float >( player->m_ArmorValue( ) );

   if ( armor_value > 0.f ) {
	  switch ( hitgroup ) {
		 case Hitgroup_Generic:
		 case Hitgroup_Chest:
		 case Hitgroup_Stomach:
		 case Hitgroup_LeftArm:
		 case Hitgroup_RightArm:
		 case Hitgroup_LeftLeg:
		 case Hitgroup_RightLeg:
		 case Hitgroup_Gear:
		 return true;
		 break;
		 case Hitgroup_Head:
		 return has_helmet || player->m_bHasHeavyArmor( );
		 break;
		 default:
		 return player->m_bHasHeavyArmor( );
		 break;
	  }
   }

   return false;
}

float Autowall::ScaleDamage( C_CSPlayer* player, float damage, float weapon_armor_ratio, int hitgroup ) {
   if ( !player )
	   return -1.f;

   C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );

   if ( !pLocal )
	   return -1.f;

   C_WeaponCSBaseGun* pWeapon = ( C_WeaponCSBaseGun* )pLocal->m_hActiveWeapon( ).Get( );

   if ( !pWeapon )
	   return -1.f;

   auto team = player->m_iTeamNum( );
   auto head_scale = team == TEAM_CT ? g_Vars.mp_damage_scale_ct_head->GetFloat( ) : g_Vars.mp_damage_scale_t_head->GetFloat( );
   auto body_scale = team == TEAM_CT ? g_Vars.mp_damage_scale_ct_body->GetFloat( ) : g_Vars.mp_damage_scale_t_body->GetFloat( );

   auto is_armored = IsArmored( player, hitgroup );
   auto armor_heavy = player->m_bHasHeavyArmor( );
   auto armor_value = static_cast< float >( player->m_ArmorValue( ) );

   if ( armor_heavy )
	  head_scale *= 0.5f;

   // ref: CCSPlayer::TraceAttack
   switch ( hitgroup ) {
	  case Hitgroup_Head:
	  damage = ( damage * 4.f ) * head_scale;
	  break;
	  case Hitgroup_Chest:
	  damage *= body_scale;
	  case Hitgroup_Gear:
	  // notice, sometimes body_scale = -4.0, look at CTakeDamageInfo::m_bitsDamageType:
	  // armor_value should <= 0, DMG_BLAST, not DMG_SHOCK, not teamdamage?
	  // well i guess, we using DMG_BULLET, so dont care
	  damage *= body_scale;
	  break;
	  case Hitgroup_Stomach:
	  damage = ( damage * 1.25f ) * body_scale;
	  break;
	  case Hitgroup_LeftArm:
	  case Hitgroup_RightArm:
	  damage *= body_scale;
	  break;
	  case Hitgroup_LeftLeg:
	  case Hitgroup_RightLeg:
	  damage = ( damage * 0.75f ) * body_scale;
	  break;
	  default:
	  break;
   }

   // ref: CCSPlayer::OnTakeDamage ref: "%f: Player %s at [%0.2f %0.2f %0.2f] took %f damage from %s, type %s\n" server.dll

   // Deal with Armour
   if ( is_armored ) {
	  auto armor_scale = 1.f;
	  auto armor_ratio = ( weapon_armor_ratio * 0.5f );
	  auto armor_bonus_ratio = 0.5f;

	  if ( armor_heavy ) {
		 armor_ratio *= 0.2f;
		 armor_bonus_ratio = 0.33f;
		 armor_scale = 0.25f;
	  }

	  float new_damage = damage * armor_ratio;
	  /* if ( armor_heavy )
		  new_damage *= 0.85f;*/ // removed?

	  float estiminated_damage = ( damage - ( damage * armor_ratio ) ) * ( armor_scale * armor_bonus_ratio );
	  if ( estiminated_damage > armor_value )
		 new_damage = ( damage - ( armor_value / armor_bonus_ratio ) );

	  damage = new_damage;
   }

   return damage;
}

void Autowall::TraceLine( const Vector& start, const Vector& end, uint32_t mask, ITraceFilter* ignore, CGameTrace* ptr ) {
   Ray_t ray;
   ray.Init( start, end );
   Source::m_pEngineTrace->TraceRay( ray, mask, ignore, ptr );
}

/* src engine rebuld */
__forceinline float DistanceToRay( const Vector& vecPosition, const Vector& vecRayStart, const Vector& vecRayEnd, float* flAlong = NULL, Vector* vecPointOnRay = NULL ) {
	Vector vecTo = vecPosition - vecRayStart;
	Vector vecDir = vecRayEnd - vecRayStart;
	float flLength = vecDir.Normalize( );

	float flRangeAlong = DotProduct( vecDir, vecTo );
	if ( flAlong ) {
		*flAlong = flRangeAlong;
	}

	float flRange;

	if ( flRangeAlong < 0.0f ) {
		// off start point
		flRange = -vecTo.Length( );

		if ( vecPointOnRay ) {
			*vecPointOnRay = vecRayStart;
		}
	} else if ( flRangeAlong > flLength ) {
		// off end point
		flRange = -( vecPosition - vecRayEnd ).Length( );

		if ( vecPointOnRay ) {
			*vecPointOnRay = vecRayEnd;
		}
	} else { // within ray bounds
		Vector vecOnRay = vecRayStart + vecDir * flRangeAlong;
		flRange = ( vecPosition - vecOnRay ).Length( );

		if ( vecPointOnRay ) {
			*vecPointOnRay = vecOnRay;
		}
	}

	return flRange;
}

void Autowall::ClipTraceToPlayers( const Vector& vecAbsStart, const Vector& vecAbsEnd, uint32_t mask, ITraceFilter* filter, CGameTrace* tr ) {
   float smallestFraction = tr->fraction;
   constexpr float maxRange = 60.0f;

   Vector delta( vecAbsEnd - vecAbsStart );
   const float delta_length = delta.Normalize( );

   Ray_t ray;
   ray.Init( vecAbsStart, vecAbsEnd );

   for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; ++i ) {
	  auto ent = C_CSPlayer::GetPlayerByIndex( i );
	  if ( !ent || ent->IsDormant( ) || ent->IsDead( ) )
		 continue;

	  if ( filter && !filter->ShouldHitEntity( ent, mask ) )
		 continue;

	  auto collideble = ent->GetCollideable( );
	  auto mins = collideble->OBBMins( );
	  auto maxs = collideble->OBBMaxs( );

	  auto obb_center = ( maxs + mins ) * 0.5f;

	  auto extend = ( obb_center - vecAbsStart );
	  auto rangeAlong = delta.Dot( extend );

	 /* should be better now */
	  const Vector vecPosition = obb_center + ent->m_vecOrigin( );
	  const float flRange = DistanceToRay( vecPosition, vecAbsStart, vecAbsEnd );
	  if ( flRange < 0.0f || flRange > 60.0f )
		  return;

	  CGameTrace playerTrace;
	  Source::m_pEngineTrace->ClipRayToEntity( ray, MASK_SHOT_HULL | CONTENTS_HITBOX, ent, &playerTrace );
	  if ( playerTrace.fraction < smallestFraction ) {
		  // we shortened the ray - save off the trace
		  *tr = playerTrace;
		  smallestFraction = playerTrace.fraction;
	  }
   }
}

// game rebuilded
bool Autowall::TraceToExit( CGameTrace* pEnterTrace, Vector vecStartPos, Vector vecDirection, CGameTrace* pExitTrace ) { 
	constexpr float flMaxDistance = 90.f, flStepSize = 4.f;
	float flCurrentDistance = 0.f;

	int iFirstContents = 0;

	bool bIsWindow = 0;
	auto v23 = 0;

	do {
		// Add extra distance to our ray
		flCurrentDistance += flStepSize;

		// Multiply the direction vector to the distance so we go outwards, add our position to it.
		Vector vecEnd = vecStartPos + ( vecDirection * flCurrentDistance );

		int iPointContents = Source::m_pEngineTrace->GetPointContents( vecEnd, MASK_SHOT_PLAYER );

		if ( !iFirstContents )
			iFirstContents = iPointContents;

		if ( !( iPointContents & MASK_SHOT_HULL ) || ( ( iPointContents & CONTENTS_HITBOX ) && iPointContents != iFirstContents ) ) {
			//Let's setup our end position by deducting the direction by the extra added distance
			Vector vecStart = vecEnd - ( vecDirection * flStepSize );

			// this gets a bit more complicated and expensive when we have to deal with displacements
			TraceLine( vecEnd, vecStart, MASK_SHOT_PLAYER, nullptr, pExitTrace );

			// we hit an ent's hitbox, do another trace.
			if ( pExitTrace->startsolid && pExitTrace->surface.flags & SURF_HITBOX ) {
				uint32_t filter_[ 4 ] = { *reinterpret_cast< uint32_t* > ( Engine::Displacement.Function.m_TraceFilterSimple ), uint32_t( C_CSPlayer::GetLocalPlayer( ) ), 0, 0 };
				filter_[ 1 ] = reinterpret_cast< uint32_t >( pExitTrace->hit_entity );

				// do another trace, but skip the player to get the actual exit surface 
				TraceLine( vecStartPos, vecStart, MASK_SHOT_HULL, reinterpret_cast< CTraceFilter* >( filter_ ), pExitTrace );

				if ( pExitTrace->DidHit( ) && !pExitTrace->startsolid ) {
					vecEnd = pExitTrace->endpos;
					return true;
				}

				continue;
			} else {

				if ( !pExitTrace->DidHit( ) || pExitTrace->startsolid ) {
					//if (pEnterTrace->DidHitNonWorldEntity() && IsBreakable((C_BaseEntity*)pEnterTrace->hit_entity)) {
					//	// if we hit a breakable, make the assumption that we broke it if we can't find an exit (hopefully..)
					//	// fake the end pos
					//	pExitTrace = pEnterTrace;
					//	pExitTrace->endpos = vecStartPos + vecDirection;
					//	return true;
					//}
					if ( pExitTrace->hit_entity ) {
						if ( pEnterTrace->DidHitNonWorldEntity( ) ) {
							if ( IsBreakable( ( C_BaseEntity* )pEnterTrace->hit_entity ) ) {
								pExitTrace = pEnterTrace;
								pExitTrace->endpos = vecStartPos + vecDirection;
								return true;
							}
						}
					}
	} else {
					if ( IsBreakable( ( C_BaseEntity* )pEnterTrace->hit_entity ) && IsBreakable( ( C_BaseEntity* )pExitTrace->hit_entity ) )
						return true;

					if ( pEnterTrace->surface.flags & SURF_NODRAW ||
						 ( !( pExitTrace->surface.flags & SURF_NODRAW ) && pExitTrace->plane.normal.Dot( vecDirection ) <= 1.f ) ) {
						const float flMultAmount = pExitTrace->fraction * 4.f;

						// get the real end pos
						vecStart -= vecDirection * flMultAmount;
						return true;
					}

					continue;
				}
}
		}
		// max pen distance is 90 units.
	} while ( flCurrentDistance <= flMaxDistance );

	return false;
}

bool Autowall::HandleBulletPenetration( Encrypted_t<C_FireBulletData> data ) {
   // can we penetrate?  
   if ( data->m_WeaponData->m_flPenetration <= 0.f )
	  return false;

   if ( data->m_iPenetrationCount <= 0 )
	  return false;

   auto contents_grate = data->m_EnterTrace.contents & CONTENTS_GRATE;
   auto surf_nodraw = data->m_EnterTrace.surface.flags & SURF_NODRAW;
   auto enter_material = data->m_EnterSurfaceData->game.material;

   const bool is_solid_surf = data->m_EnterTrace.contents >> 3 & CONTENTS_SOLID;
   const bool is_light_surf = data->m_EnterTrace.surface.flags >> 7 & SURF_LIGHT;

   CGameTrace exit_trace = {};
   if ( !TraceToExit( &data->m_EnterTrace, data->m_EnterTrace.endpos, data->m_vecDirection, &exit_trace ) ) {
	  if ( !( Source::m_pEngineTrace->GetPointContents( data->m_EnterTrace.endpos, MASK_SHOT_HULL, 0 ) & MASK_SHOT_HULL ) )
		 return false;
   }

   auto enter_penetration_modifier = data->m_EnterSurfaceData->game.flPenetrationModifier;

   auto exit_surface_data = Source::m_pPhysSurface->GetSurfaceData( exit_trace.surface.surfaceProps );

   if ( !exit_surface_data )
	  return false;

   auto exit_material = exit_surface_data->game.material;
   auto exit_penetration_modifier = exit_surface_data->game.flPenetrationModifier;
   auto combined_damage_modifier = 0.16f;
   auto combined_penetration_modifier = ( enter_penetration_modifier + exit_penetration_modifier ) * 0.5f;
   if ( enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE ) {
	  combined_penetration_modifier = 3.f;
	  combined_damage_modifier = 0.05f;
   } else if ( contents_grate || surf_nodraw ) {
	  combined_penetration_modifier = 1.f;
   } else if ( enter_material == CHAR_TEX_FLESH && ( data->m_Player->IsTeammate( ( C_CSPlayer* ) ( data->m_EnterTrace.hit_entity ) ) ) &&
			   g_Vars.ff_damage_reduction_bullets->GetFloat( ) == 0.f ) {
	  if ( g_Vars.ff_damage_bullet_penetration->GetFloat( ) == 0.f )
		 return false;

	  combined_penetration_modifier = g_Vars.ff_damage_bullet_penetration->GetFloat( );
	  combined_damage_modifier = 0.16f;
   }

   if ( enter_material == exit_material ) {
	  if ( exit_material == CHAR_TEX_WOOD || exit_material == CHAR_TEX_CARDBOARD )
		 combined_penetration_modifier = 3.f;
	  else if ( exit_material == CHAR_TEX_PLASTIC )
		 combined_penetration_modifier = 2.f;
   }

  /* if ( enter_material == 71 || enter_material == 89 ) {
	  combined_damage_modifier = 0.050000001;
	  combined_penetration_modifier = 3.0;
   } else if ( is_solid_surf || is_light_surf ) {
	  combined_damage_modifier = 0.16;
	  combined_penetration_modifier = 1.0;
   } else {
	  combined_damage_modifier = 0.16f;
	  combined_penetration_modifier = ( enter_penetration_modifier + exit_penetration_modifier ) * 0.5f;
   }

   if ( enter_material == exit_material ) {
	  if ( exit_material == CHAR_TEX_WOOD || exit_material == CHAR_TEX_CARDBOARD )
		 combined_penetration_modifier = 3.f;
	  else if ( exit_material == CHAR_TEX_PLASTIC )
		 combined_penetration_modifier = 2.f;
   }*/

   // Correct damage calculation
   auto penetration_modifier = std::fmaxf( 0.f, 1.f / combined_penetration_modifier );
   auto penetration_distance = ( exit_trace.endpos - data->m_EnterTrace.endpos ).Length( );
   penetration_distance = ( ( penetration_distance * penetration_distance ) * penetration_modifier ) * 0.041666668f;

   auto damage_modifier = std::fmaxf( 0.f, ( 3.f / data->m_WeaponData->m_flPenetration ) * 1.25f ) * penetration_modifier * 3.f
	  + ( data->m_flCurrentDamage * combined_damage_modifier ) + penetration_distance;

   auto damage_lost = std::fmaxf( 0.f, damage_modifier );
   if ( damage_lost > data->m_flCurrentDamage )
	  return false;

   data->m_flCurrentDamage -= damage_lost;
   if ( data->m_flCurrentDamage < 1.0f )
	  return false;

   data->m_vecStart = exit_trace.endpos;
   --data->m_iPenetrationCount;
   return true;

}

float Autowall::FireBullets( Encrypted_t<C_FireBulletData> data ) {
   data->m_iPenetrationCount = 4;
   data->m_flTraceLength = 0.f;

   data->m_flCurrentDamage = static_cast< float >( data->m_WeaponData->m_iWeaponDamage );

   if ( !data->m_Weapon ) {
	  data->m_Weapon = ( C_WeaponCSBaseGun* ) ( data->m_Player->m_hActiveWeapon( ).Get( ) );
   }

   CTraceFilter filter;
   filter.pSkip = data->m_Player;

   if ( !data->m_Filter )
	  data->m_Filter = &filter;

   Vector obb_center;
   if ( data->m_TargetPlayer ) {
	  auto collideble = data->m_TargetPlayer->m_Collision( );
	  if ( collideble ) {
		 auto mins = collideble->m_vecMins;
		 auto maxs = collideble->m_vecMaxs;

		 obb_center = ( maxs + mins ) * 0.5f;
	  }
   }

   data->m_flMaxLength = data->m_WeaponData->m_flWeaponRange;

   IClientEntity* last_entity = nullptr;
   while ( data->m_flCurrentDamage > 0.f ) {
	  data->m_flMaxLength -= data->m_flTraceLength;
	  auto end = ( data->m_vecStart + data->m_vecDirection * data->m_flMaxLength );

	  TraceLine( data->m_vecStart, end, MASK_SHOT_HULL | CONTENTS_HITBOX, data->m_Filter, &data->m_EnterTrace );

	  // NOTICE: can remove valve`s hack aka bounding box fix
	  if ( data->m_TargetPlayer ) {
		 // clip trace to one player
		 Vector vecAbsStart = data->m_vecStart;
		 Vector vecAbsEnd = end + data->m_vecDirection * 40.0f;
		 Vector delta = vecAbsEnd - data->m_vecStart;
		 float delta_length = delta.Normalize( );

		 Ray_t ray;
		 ray.Init( vecAbsStart, vecAbsEnd );

		 auto extend = ( obb_center - data->m_vecStart );
		 auto rangeAlong = delta.Dot( extend );

		 float range;
		 if ( rangeAlong >= 0.0f ) {
			if ( rangeAlong <= delta_length )
			   range = Vector( obb_center - ( ( delta * rangeAlong ) + vecAbsStart ) ).Length( );
			else
			   range = -( obb_center - vecAbsEnd ).Length( );
		 } else {
			range = -extend.Length( );
		 }

		 if ( range >= 0.0f && range <= 60.0f ) {
			CGameTrace playerTrace;
			Source::m_pEngineTrace->ClipRayToEntity( ray, MASK_SHOT_HULL | CONTENTS_HITBOX, data->m_TargetPlayer, &playerTrace );
			if ( data->m_EnterTrace.fraction > playerTrace.fraction )
			   data->m_EnterTrace = playerTrace;
		 }
	  } else {
		 ClipTraceToPlayers( data->m_vecStart, end + data->m_vecDirection * 40.0f, MASK_SHOT_HULL | CONTENTS_HITBOX, data->m_Filter, &data->m_EnterTrace );
	  }

	  if ( data->m_EnterTrace.fraction == 1.f )
		 break;

	  data->m_flTraceLength += data->m_flMaxLength * data->m_EnterTrace.fraction;
	  if ( !data->m_bShouldIgnoreDistance )
		 data->m_flCurrentDamage *= powf( data->m_WeaponData->m_flRangeModifier, data->m_flTraceLength * 0.002f );
   #ifdef DEBUG_AUTOWALL
	  if ( InputSys::Get( )->IsKeyDown( VirtualKeys::H ) ) {
		 Source::m_pDebugOverlay->AddLineOverlay( data->m_vecStart, data->m_EnterTrace.endpos, 255, 255, 255, true, DEBUG_DURATION );
		 // Source::m_pDebugOverlay->AddBoxOverlay( data->m_EnterTrace.endpos, Vector( -1.0f, -1.0f, -1.0f ), Vector( 1.0f, 1.0f, 1.0f ), QAngle( 0.0f, 0.0f, 0.0f ), 255, 0, 0, 200, DEBUG_DURATION );
	  }
   #endif

	  data->m_EnterSurfaceData = Source::m_pPhysSurface->GetSurfaceData( data->m_EnterTrace.surface.surfaceProps );
	  if ( data->m_flTraceLength > 3150.0f && data->m_WeaponData->m_flPenetration > 0.f
		   || data->m_EnterSurfaceData->game.flPenetrationModifier < 0.1f ) {
		 data->m_iPenetrationCount = 0;
		 break;
	  }

	  C_CSPlayer* hit_player = ToCSPlayer( ( C_BasePlayer* ) data->m_EnterTrace.hit_entity );
	  bool can_do_damage = ( data->m_EnterTrace.hitgroup >= Hitgroup_Generic && data->m_EnterTrace.hitgroup <= Hitgroup_RightLeg );
	  bool hit_target = !data->m_TargetPlayer || hit_player == data->m_TargetPlayer;
	  if ( !data->m_EnterTrace.startsolid ) {
		 if ( can_do_damage && hit_player && hit_player->m_entIndex <= Source::m_pGlobalVars->maxClients && hit_player->m_entIndex > 0 && hit_target ) {
			if ( data->m_Weapon && data->m_Weapon->m_iItemDefinitionIndex( ) == WEAPON_ZEUS )
			   data->m_flCurrentDamage *= 0.9f;
			else
			   data->m_flCurrentDamage = ScaleDamage( hit_player, data->m_flCurrentDamage, data->m_WeaponData->m_flArmorRatio, data->m_EnterTrace.hitgroup );

			return data->m_flCurrentDamage;
		 }
	  }

	  if ( data->m_iPenetrationCount == 0 )
		 break;

	  bool penetrate = data->m_bPenetration;
	  if ( !data->m_bPenetration )
		 penetrate = data->m_EnterTrace.contents & CONTENTS_WINDOW;

	  if ( !penetrate || !HandleBulletPenetration( data ) || data->m_flCurrentDamage <= 1.0f )
		 break;

   #ifdef DEBUG_AUTOWALL
	  if ( InputSys::Get( )->IsKeyDown( VirtualKeys::H ) ) {
		 Source::m_pDebugOverlay->AddLineOverlay( data->m_vecStart, data->m_EnterTrace.endpos, 0, 0, 255, true, DEBUG_DURATION );
		 Source::m_pDebugOverlay->AddBoxOverlay( data->m_vecStart, Vector( -1.0f, -1.0f, -1.0f ), Vector( 1.0f, 1.0f, 1.0f ), QAngle( 0.0f, 0.0f, 0.0f ), 0, 255, 0, 200, DEBUG_DURATION );
		 Source::m_pDebugOverlay->AddTextOverlay( data->m_vecStart - Vector( 0.0f, 0.0f, 5.0f ), DEBUG_DURATION, XorStr( "^ %.2f damage\t %d pen count\t %.2f thickness" ), data->m_flCurrentDamage, data->m_iPenetrationCount, data->m_flPenetrationDistance );
	  }
   #endif
   }

   return -1.f;
}
