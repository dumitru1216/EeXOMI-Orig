#include "CVariables.hpp"
#include "source.hpp"
#include "KitParser.hpp"
std::vector<KeyBind_t*> g_keybinds;

CVariables g_Vars;

FloatColor FloatColor::Black = FloatColor( 0.0f, 0.0f, 0.0f, 1.0f );
FloatColor FloatColor::White = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
FloatColor FloatColor::Gray = FloatColor( 0.75f, 0.75f, 0.75f, 1.0f );

FloatColor::FloatColor( ImColor c ) { *this = FloatColor( c.Value.x, c.Value.y, c.Value.z, c.Value.w ); }
FloatColor::FloatColor( ImVec4 c ) { *this = FloatColor( c.x, c.y, c.z, c.w ); }

FloatColor FloatColor::Lerp( const FloatColor& dst, float t ) const {
   return FloatColor( ( dst.r - r ) * t + r, ( dst.g - g ) * t + g, ( dst.b - b ) * t + b, ( dst.a - a ) * t + a );
}

FloatColor::operator ImColor( ) const {
   return ImColor( r, g, b, a );
}

FloatColor::operator ImColor* ( ) {
   return reinterpret_cast< ImColor* >( this );
}

void CVariables::Create( ) {
   legit_weapons.SetName( XorStr( "legit_weapons" ) );
   for ( auto weapon : weapon_skins ) {
	  if ( weapon.group != -1 ) {
		 auto idx = legit_weapons.AddEntry( );
		 legit_weapons[ idx ]->item_idx = weapon.id;
	  }
   }

   // add weapons
   m_skin_changer.SetName( XorStr( "skin_changer" ) );
   for ( const auto& value : weapon_skins ) {
	  auto idx = m_skin_changer.AddEntry( );
	  auto entry = m_skin_changer[ idx ];
	  entry->m_definition_index = value.id;
	  entry->SetName( value.name );
   }

   this->AddChild( &m_skin_changer );
   this->AddChild( &legit_weapons );

   sv_accelerate = Source::m_pCvar->FindVar( XorStr( "sv_accelerate" ) );
   sv_airaccelerate = Source::m_pCvar->FindVar( XorStr( "sv_airaccelerate" ) );
   sv_gravity = Source::m_pCvar->FindVar( XorStr( "sv_gravity" ) );
   sv_jump_impulse = Source::m_pCvar->FindVar( XorStr( "sv_jump_impulse" ) );

   m_yaw = Source::m_pCvar->FindVar( XorStr( "m_yaw" ) );
   m_pitch = Source::m_pCvar->FindVar( XorStr( "m_pitch" ) );
   sensitivity = Source::m_pCvar->FindVar( XorStr( "sensitivity" ) );

   cl_sidespeed = Source::m_pCvar->FindVar( XorStr( "cl_sidespeed" ) );
   cl_forwardspeed = Source::m_pCvar->FindVar( XorStr( "cl_forwardspeed" ) );
   cl_upspeed = Source::m_pCvar->FindVar( XorStr( "cl_upspeed" ) );

   weapon_recoil_scale = Source::m_pCvar->FindVar( XorStr( "weapon_recoil_scale" ) );
   view_recoil_tracking = Source::m_pCvar->FindVar( XorStr( "view_recoil_tracking" ) );

   r_jiggle_bones = Source::m_pCvar->FindVar( XorStr( "r_jiggle_bones" ) );

   mp_friendlyfire = Source::m_pCvar->FindVar( XorStr( "mp_friendlyfire" ) );

   sv_maxunlag = Source::m_pCvar->FindVar( XorStr( "sv_maxunlag" ) );
   sv_minupdaterate = Source::m_pCvar->FindVar( XorStr( "sv_minupdaterate" ) );
   sv_maxupdaterate = Source::m_pCvar->FindVar( XorStr( "sv_maxupdaterate" ) );

   sv_client_min_interp_ratio = Source::m_pCvar->FindVar( XorStr( "sv_client_min_interp_ratio" ) );
   sv_client_max_interp_ratio = Source::m_pCvar->FindVar( XorStr( "sv_client_max_interp_ratio" ) );

   cl_interp_ratio = Source::m_pCvar->FindVar( XorStr( "cl_interp_ratio" ) );
   cl_interp = Source::m_pCvar->FindVar( XorStr( "cl_interp" ) );
   cl_updaterate = Source::m_pCvar->FindVar( XorStr( "cl_updaterate" ) );

   game_type = Source::m_pCvar->FindVar( XorStr( "game_type" ) );
   game_mode = Source::m_pCvar->FindVar( XorStr( "game_mode" ) );

   ff_damage_bullet_penetration = Source::m_pCvar->FindVar( XorStr( "ff_damage_bullet_penetration" ) );
   ff_damage_reduction_bullets = Source::m_pCvar->FindVar( XorStr( "ff_damage_reduction_bullets" ) );

   mp_damage_scale_ct_head = Source::m_pCvar->FindVar( XorStr( "mp_damage_scale_ct_head" ) );
   mp_damage_scale_t_head = Source::m_pCvar->FindVar( XorStr( "mp_damage_scale_t_head" ) );
   mp_damage_scale_ct_body = Source::m_pCvar->FindVar( XorStr( "mp_damage_scale_ct_body" ) );
   mp_damage_scale_t_body = Source::m_pCvar->FindVar( XorStr( "mp_damage_scale_t_body" ) );

   viewmodel_fov = Source::m_pCvar->FindVar( XorStr( "viewmodel_fov" ) );
   viewmodel_offset_x = Source::m_pCvar->FindVar( XorStr( "viewmodel_offset_x" ) );
   viewmodel_offset_y = Source::m_pCvar->FindVar( XorStr( "viewmodel_offset_y" ) );
   viewmodel_offset_z = Source::m_pCvar->FindVar( XorStr( "viewmodel_offset_z" ) );

   sv_show_impacts = Source::m_pCvar->FindVar( XorStr( "sv_showimpacts" ) );

   molotov_throw_detonate_time = Source::m_pCvar->FindVar( XorStr( "molotov_throw_detonate_time" ) );
   weapon_molotov_maxdetonateslope = Source::m_pCvar->FindVar( XorStr( "weapon_molotov_maxdetonateslope" ) );
   net_client_steamdatagram_enable_override = Source::m_pCvar->FindVar( XorStr( "net_client_steamdatagram_enable_override" ) );
   mm_dedicated_search_maxping = Source::m_pCvar->FindVar( XorStr( "mm_dedicated_search_maxping" ) );


   sv_clockcorrection_msecs = Source::m_pCvar->FindVar( XorStr( "sv_clockcorrection_msecs" ) );
   /*developer = Source::m_pCvar->FindVar( XorStr( "developer" ) );
   con_enable = Source::m_pCvar->FindVar( XorStr( "con_enable" ) );
   con_filter_enable = Source::m_pCvar->FindVar( XorStr( "con_filter_enable" ) );
   con_filter_text = Source::m_pCvar->FindVar( XorStr( "con_filter_text" ) );
   con_filter_text_out = Source::m_pCvar->FindVar( XorStr( "con_filter_text_out" ) );
   contimes = Source::m_pCvar->FindVar( XorStr( "contimes" ) );*/
}

