#include "SkinChanger.hpp"
#include "CVariables.hpp"
#include "weapon.hpp"
#include "player.hpp"
#include "CBaseHandle.hpp"
#include "KitParser.hpp"
#include "PropManager.hpp"
#include <algorithm>
#include <memory.h>
#include "recv_swap.hpp"
#include "FnvHash.hpp"
#include "displacement.hpp"

static auto is_knife( const int i ) -> bool {
   return ( i >= WEAPON_KNIFE_BAYONET && i < GLOVE_STUDDED_BLOODHOUND ) || i == WEAPON_KNIFE_T || i == WEAPON_KNIFE;
}

static CHandle< C_BaseCombatWeapon > glove_handle{};

class CSkinChanger : public ISkinChanger {
public:
   void Create( ) override;
   void Destroy( ) override;
   void OnNetworkUpdate( bool start = true ) override;
private:
   void PostDataUpdateStart( C_CSPlayer* local );
   void EraseOverrideIfExistsByIndex( const int definition_index );
   void ApplyConfigOnAttributableItem( C_BaseAttributableItem* item, CVariables::skin_changer_data* config, const unsigned xuid_low );
   void GloveChanger( C_CSPlayer* local );
   static void SequenceProxyFn( const CRecvProxyData* proxy_data_const, void* entity, void* output );
   void DoSequenceRemapping( CRecvProxyData* data, C_BaseViewModel* entity );
   int GetNewAnimation( const uint32_t model, const int sequence, C_BaseViewModel* viewModel );
   CVariables::skin_changer_data* GetDataFromIndex( int idx );
   void ForceItemUpdate( C_CSPlayer* local );
   void UpdateHud( );

   std::unordered_map< std::string_view, std::string_view > m_icon_overrides;
   RecvPropHook::Shared m_sequence_hook = nullptr;

   float lastSkinUpdate = 0.0f;
   float lastGloveUpdate = 0.0f;
};

ISkinChanger* ISkinChanger::Get( ) {
   static CSkinChanger instance;
   return &instance;
}

void CSkinChanger::Create( ) {
   auto& pPropManager = Engine::PropManager::Instance( );

   RecvProp* prop = nullptr;
   pPropManager->GetProp( XorStr( "DT_BaseViewModel" ), XorStr( "m_nSequence" ), &prop );
   m_sequence_hook = std::make_shared<RecvPropHook>( prop, &SequenceProxyFn );
}

void CSkinChanger::Destroy( ) {
   m_sequence_hook->Unhook( );
   m_sequence_hook.reset( );
}

void CSkinChanger::OnNetworkUpdate( bool start ) {
   auto& global = g_Vars.m_global_skin_changer;

   auto local = C_CSPlayer::GetLocalPlayer( );
   if ( !local )
	  return;

   if ( local->IsDead( ) )
	  return;

   if ( !start ) {
	  if ( global.m_update_skins && !global.m_update_gloves ) {
		 float meme = Source::m_pGlobalVars->realtime - lastSkinUpdate;
		 if ( meme >= 0.65f ) {
			lastSkinUpdate = Source::m_pGlobalVars->realtime - 0.125f;
		 } else if ( meme >= 0.2f ) {
			ForceItemUpdate( local );
			global.m_update_skins = false;
			lastSkinUpdate = Source::m_pGlobalVars->realtime;
		 }
	  }

	  if ( ( !global.m_active || !global.m_glove_changer ) || global.m_update_gloves ) {
		 auto glove = glove_handle.Get( );
		 if ( glove ) {
			glove->GetClientNetworkable( )->SetDestroyedOnRecreateEntities( );
			glove->GetClientNetworkable( )->Release( );
			glove_handle.Set( nullptr );
		 }

		 const auto glove_config = GetDataFromIndex( global.m_gloves_idx );
		 if ( ( global.m_update_gloves && Source::m_pGlobalVars->realtime - lastGloveUpdate >= 0.5f ) || ( glove_config && !glove_config->m_enabled && glove_config->m_executed ) ) {
			Source::m_pClientState->m_nDeltaTick( ) = -1;
			if ( global.m_update_gloves )
			   lastGloveUpdate = Source::m_pGlobalVars->realtime;

			global.m_update_gloves = false;

			if ( glove_config )
			   glove_config->m_executed = false;
		 }
	  } else if ( local ) {
		 GloveChanger( local );
	  }
	  return;
   }

   PostDataUpdateStart( local );
}

void CSkinChanger::PostDataUpdateStart( C_CSPlayer* local ) {
   const auto local_index = local->entindex( );

   player_info_t player_info;
   if ( !Source::m_pEngine->GetPlayerInfo( local_index, &player_info ) )
	  return;

   auto& global = g_Vars.m_global_skin_changer;

   if ( g_Vars.misc.skins_model ) {
	  const char* models_to_change[] = {
	 XorStr( "models/player/custom_player/legacy/tm_phoenix.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_sas.mdl" ),
     XorStr( "models/player/custom_player/legacy/tm_balkan_variantj.mdl" ),
     XorStr( "models/player/custom_player/legacy/tm_balkan_variantg.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_balkan_varianti.mdl" ),
     XorStr( "models/player/custom_player/legacy/tm_balkan_variantf.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_st6_varianti.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_st6_variantm.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_st6_variantg.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_st6_variante.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_st6_variantk.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_balkan_varianth.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_fbi_varianth.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_fbi_variantg.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_fbi_variantf.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_phoenix_variantg.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_phoenix_variantf.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_phoenix_varianth.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_leet_variantf.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_leet_varianti.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_leet_varianth.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_leet_variantg.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_fbi_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_sas_variantf.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_anarchist.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_anarchist_varianta.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_anarchist_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_anarchist_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_anarchist_variantd.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_pirate.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_pirate_varianta.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_pirate_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_pirate_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_pirate_variantd.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_professional.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_professional_var1.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_professional_var2.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_professional_var3.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_professional_var4.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_separatist.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_separatist_varianta.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_separatist_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_separatist_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_separatist_variantd.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_gign.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_gign_varianta.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_gign_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_gign_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_gign_variantd.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_gsg9.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_gsg9_varianta.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_gsg9_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_gsg9_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_gsg9_variantd.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_idf.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_idf_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_idf_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_idf_variantd.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_idf_variante.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_idf_variantf.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_swat.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_swat_varianta.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_swat_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_swat_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_swat_variantd.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_sas_varianta.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_sas_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_sas_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_sas_variantd.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_st6.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_st6_varianta.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_st6_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_st6_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_st6_variantd.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_balkan_variante.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_balkan_varianta.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_balkan_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_balkan_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_balkan_variantd.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_jumpsuit_varianta.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_jumpsuit_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_jumpsuit_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_phoenix_heavy.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_heavy.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_leet_varianta.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_leet_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_leet_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_leet_variantd.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_leet_variante.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_phoenix.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_phoenix_varianta.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_phoenix_variantb.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_phoenix_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/tm_phoenix_variantd.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_fbi.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_fbi_varianta.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_fbi_variantc.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_fbi_variantd.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_fbi_variante.mdl" ),
	 XorStr( "models/player/custom_player/legacy/ctm_sas.mdl" )
	  };

	  for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; i++ ) {
		 auto entity = ( C_BaseEntity* ) Source::m_pEntList->GetClientEntity( i );

		 if ( !entity )
			continue;

		 if ( entity->IsPlayer( ) ) {
			auto TeroristModelIndex = Source::m_pModelInfo->GetModelIndex( models_to_change[ std::max( 0, g_Vars.misc.tt_model_type - 1 ) ] );
			auto CTeroristModelIndex = Source::m_pModelInfo->GetModelIndex( models_to_change[ std::max( 0, g_Vars.misc.ct_model_type - 1 ) ] );
			int iTeam = entity->m_iTeamNum( );

			if ( entity == local ) {
			   if ( iTeam == TEAM_TT && g_Vars.misc.tt_model_type != 0 ) {
				  if ( TeroristModelIndex )
					 entity->SetModelIndex( TeroristModelIndex );
			   } else if ( iTeam == TEAM_CT && g_Vars.misc.ct_model_type != 0 ) {
				  if ( CTeroristModelIndex )
					 entity->SetModelIndex( CTeroristModelIndex );
			   }
			}
		 }
	  }
   }
   // Handle weapon configs
   {
	  auto weapons = local->m_hMyWeapons( );
	  for ( int i = 0; i < 48; ++i ) {
		 auto weapon = ( C_BaseAttributableItem* ) weapons[ i ].Get( );
		 if ( !weapon )
			continue;

		 auto& definition_index = weapon->m_Item( ).m_iItemDefinitionIndex( );

		 auto idx = is_knife( definition_index ) ? global.m_knife_idx : definition_index;
		 const auto active_conf = GetDataFromIndex( idx );
		 if ( active_conf ) {
			if ( ( !active_conf->m_enabled || !global.m_active ) && active_conf->m_executed )
			   global.m_update_skins = true;

			ApplyConfigOnAttributableItem( weapon, active_conf, player_info.xuid_low );
		 } else {
			EraseOverrideIfExistsByIndex( definition_index );
		 }
	  }
   }
   const auto view_model = ( C_BaseViewModel* ) local->m_hViewModel( ).Get( );
   if ( !view_model )
	  return;

   const auto view_model_weapon = ( C_BaseAttributableItem* ) view_model->m_hWeapon( ).Get( );
   if ( !view_model_weapon )
	  return;

   auto idx = view_model_weapon->m_Item( ).m_iItemDefinitionIndex( );
   if ( k_weapon_info.count( idx ) > 0 ) {
	  const auto override_info = k_weapon_info.at( idx );
	  const auto override_model_index = Source::m_pModelInfo->GetModelIndex( override_info.model );
	  view_model->m_nModelIndex( ) = override_model_index;

	  const auto world_model = view_model_weapon->m_hWeaponWorldModel( ).Get( );
	  if ( world_model )
		 world_model->m_nModelIndex( ) = override_model_index + 1;
   }
}

void CSkinChanger::EraseOverrideIfExistsByIndex( const int definition_index ) {
   if ( k_weapon_info.count( definition_index ) <= 0 )
	  return;

   // We have info about the item not needed to be overridden
   const auto& original_item = k_weapon_info.at( definition_index );
   auto& icon_override_map = m_icon_overrides;

   if ( !original_item.icon )
	  return;

   const auto override_entry = icon_override_map.find( original_item.icon );

   // We are overriding its icon when not needed
   if ( override_entry != end( icon_override_map ) )
	  icon_override_map.erase( override_entry ); // Remove the leftover override
}

void CSkinChanger::ApplyConfigOnAttributableItem( C_BaseAttributableItem* attribute, CVariables::skin_changer_data* config, const unsigned xuid_low ) {
   auto& item = attribute->m_Item( );
   auto& global = g_Vars.m_global_skin_changer;

   // Force fallback values to be used.
   item.m_iItemIDHigh( ) = -1;

   // Set the owner of the weapon to our lower XUID. (fixes StatTrak)
   item.m_iAccountID( ) = xuid_low;

   item.m_nFallbackPaintKit( ) = config->m_paint_kit;

   item.m_nFallbackSeed( ) = config->m_seed;

   if ( config->m_stat_trak )
	  item.m_nFallbackStatTrak( ) = config->m_stat_trak;

   item.m_iEntityQuality( ) = 0;

   item.m_flFallbackWear( ) = config->m_wear;

   //if (!config->m_custom_name.empty() && config->m_custom_name[0])
   //  strcpy_s(item.m_szCustomName(), 32u, config->m_custom_name.c_str());

   auto& definition_index = item.m_iItemDefinitionIndex( );

   auto& icon_override_map = m_icon_overrides;

   bool knife = is_knife( definition_index );

   int definition_override = 0;

   if ( knife )
	  definition_override = global.m_knife_idx;
   else if ( config->m_definition_index >= GLOVE_STUDDED_BLOODHOUND && config->m_definition_index <= GLOVE_HYDRA )
	  definition_override = global.m_gloves_idx;
   /*} else {
   if ( knife ) {
   auto local = C_CSPlayer::GetLocalPlayer( );

   if ( local && local->m_iTeamNum( ) == 3 )
   definition_override = WEAPON_KNIFE;
   else
   definition_override = WEAPON_KNIFE_T;
   }
   }*/

   if ( definition_override && definition_override != definition_index ) // We need to override defindex
   {
	  // We have info about what we gonna override it to
	  if ( k_weapon_info.count( definition_override ) > 0 ) {
		 const auto replacement_item = &k_weapon_info.at( definition_override );

		 const auto old_definition_index = definition_index;

		 item.m_iItemDefinitionIndex( ) = definition_override;

		 // Set the weapon model index -- required for paint kits to work on replacement items after the 29/11/2016 update.
		 auto idx = Source::m_pModelInfo->GetModelIndex( replacement_item->model );
		 attribute->SetModelIndex( idx );
		 attribute->GetClientNetworkable( )->PreDataUpdate( 0 );

		 // We didn't override 0, but some actual weapon, that we have data for
		 if ( old_definition_index ) {
			if ( k_weapon_info.count( old_definition_index ) > 0 ) {
			   const auto original_item = &k_weapon_info.at( old_definition_index );
			   if ( original_item->icon && replacement_item->icon )
				  icon_override_map[ original_item->icon ] = replacement_item->icon;
			}
		 }
	  }
   } else {
	  EraseOverrideIfExistsByIndex( definition_index );
   }

   config->m_executed = false;
}

void CSkinChanger::GloveChanger( C_CSPlayer* local ) {
   const auto local_index = local->entindex( );

   player_info_t player_info;
   if ( !Source::m_pEngine->GetPlayerInfo( local_index, &player_info ) )
	  return;

   auto& global = g_Vars.m_global_skin_changer;

   // Handle glove config
   {
	  const auto wearables = local->m_hMyWearables( );

	  const auto glove_config = GetDataFromIndex( global.m_gloves_idx );

	  auto glove = ( C_BaseAttributableItem* ) wearables[ 0 ].Get( );

	  if ( !glove_config || global.m_gloves_idx == 0 ) {
		 return;
	  }

	  if ( !glove ) {
		 // Try to get our last created glove
		 const auto our_glove = ( C_BaseAttributableItem* ) glove_handle.Get( );
		 if ( our_glove ) // Our glove still exists
		 {
			wearables[ 0 ] = glove_handle;
			glove = our_glove;
		 }
	  }

	  if ( local->IsDead( ) ) {
		 // We are dead but we have a glove, destroy it
		 if ( glove ) {
			glove->GetClientNetworkable( )->SetDestroyedOnRecreateEntities( );
			glove->GetClientNetworkable( )->Release( );
		 }

		 return;
	  }

	  // We don't have a glove, but we should
	  bool just_created = false;
	  if ( !glove ) {
		 auto get_wearable_create_fn = [] ( ) -> CreateClientClassFn {
			auto clazz = Source::m_pClient->GetAllClasses( );
			while ( clazz->m_ClassID != CEconWearable )
			   clazz = clazz->m_pNext;

			return ( CreateClientClassFn ) clazz->m_pCreateFn;
		 };

		 static auto create_wearable_fn = get_wearable_create_fn( );

		 const auto entry = Source::m_pEntList->GetHighestEntityIndex( ) + 1;
		 const auto serial = rand( ) % 0x1000;
		 create_wearable_fn( entry, serial );

		 glove = static_cast< C_BaseAttributableItem* >( Source::m_pEntList->GetClientEntity( entry ) );
		 assert( glove );

		 // He he
		 {
			Vector new_pos = Vector{ 10000.0f, 10000.0f, 10000.f };
			glove->SetAbsOrigin( new_pos );
		 }

		 wearables[ 0 ] = CBaseHandle( entry | ( serial << 16 ) );

		 // Let's store it in case we somehow lose it.
		 glove_handle = wearables[ 0 ];

		 just_created = true;
	  }

	  glove_config->m_executed = true;
	  if ( glove ) {
		 // Thanks, Beakers
		 *( int* ) ( ( uintptr_t ) glove + 0x64 ) = -1;
		 // *( int* ) ( ( uintptr_t ) local + 0xA20 ) = 1; // remove default arms in 3th person mode dword_15268230 = (int)"m_nBody";
		 ApplyConfigOnAttributableItem( glove, glove_config, player_info.xuid_low );

	  #if 0
		 static auto fnEquip
			= reinterpret_cast< int( __thiscall* )( void*, void* ) >(
			Memory::Scan( XorStr( "client.dll" ), XorStr( "55 8B EC 83 EC 10 53 8B 5D 08 57 8B F9" ) )
			);

		 fnEquip( glove, local );

		 auto CreateRenderableHandle = [&] ( ) -> void {
			static auto adr = Memory::Scan( ( std::uintptr_t )GetModuleHandleA( XorStr( "client.dll" ) ), XorStr( "55 8B EC 51 53 8B C1 56 57 51" ) );
			static auto fnCreateRenderableHandle = ( void( __thiscall* )( void*, int, int, char, signed int, char ) ) adr;
			fnCreateRenderableHandle( Source::m_pClientLeafSystem, reinterpret_cast< uintptr_t >( glove->GetClientRenderable( ) ), 0, 0, 0xFFFFFFFF, 0xFFFFFFFF );
		 };

		 CreateRenderableHandle( );
	  #endif 
	  }
   }
}

void CSkinChanger::SequenceProxyFn( const CRecvProxyData* proxy_data_const, void* entity, void* output ) {
   auto skins = ( CSkinChanger* ) Get( );
   if ( skins->m_sequence_hook ) {
	  auto original_fn = skins->m_sequence_hook->GetOriginalFunction( );

	  // Remove the constness from the proxy data allowing us to make changes.
	  const auto proxy_data = const_cast< CRecvProxyData* >( proxy_data_const );

	  const auto view_model = static_cast< C_BaseViewModel* >( entity );

	  skins->DoSequenceRemapping( proxy_data, view_model );

	  // Call the original function with our edited data.
	  original_fn( proxy_data_const, entity, output );
   }
}

void CSkinChanger::DoSequenceRemapping( CRecvProxyData* data, C_BaseViewModel* entity ) {
   auto local = C_CSPlayer::GetLocalPlayer( );
   if ( !local || local->IsDead( ) )
	  return;

   const auto owner = entity->m_hOwner( ).Get( );
   if ( owner != local )
	  return;

   const auto view_model_weapon = entity->m_hWeapon( ).Get( );
   if ( !view_model_weapon )
	  return;

   auto idx = view_model_weapon->m_Item( ).m_iItemDefinitionIndex( );
   if ( k_weapon_info.count( idx ) <= 0 )
	  return;

   const auto weapon_info = &k_weapon_info.at( idx );

   const auto override_model = weapon_info->model;

   auto& sequence = data->m_Value.m_Int;
   sequence = GetNewAnimation( hash_32_fnv1a_const( override_model ), sequence, entity );
}

int CSkinChanger::GetNewAnimation( const uint32_t model, const int sequence, C_BaseViewModel* viewModel ) {

   // This only fixes if the original knife was a default knife.
   // The best would be having a function that converts original knife's sequence
   // into some generic enum, then another function that generates a sequence
   // from the sequences of the new knife. I won't write that.
   enum ESequence {
	  SEQUENCE_DEFAULT_DRAW = 0,
	  SEQUENCE_DEFAULT_IDLE1 = 1,
	  SEQUENCE_DEFAULT_IDLE2 = 2,
	  SEQUENCE_DEFAULT_LIGHT_MISS1 = 3,
	  SEQUENCE_DEFAULT_LIGHT_MISS2 = 4,
	  SEQUENCE_DEFAULT_HEAVY_MISS1 = 9,
	  SEQUENCE_DEFAULT_HEAVY_HIT1 = 10,
	  SEQUENCE_DEFAULT_HEAVY_BACKSTAB = 11,
	  SEQUENCE_DEFAULT_LOOKAT01 = 12,

	  SEQUENCE_BUTTERFLY_DRAW = 0,
	  SEQUENCE_BUTTERFLY_DRAW2 = 1,
	  SEQUENCE_BUTTERFLY_LOOKAT01 = 13,
	  SEQUENCE_BUTTERFLY_LOOKAT03 = 15,

	  SEQUENCE_FALCHION_IDLE1 = 1,
	  SEQUENCE_FALCHION_HEAVY_MISS1 = 8,
	  SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP = 9,
	  SEQUENCE_FALCHION_LOOKAT01 = 12,
	  SEQUENCE_FALCHION_LOOKAT02 = 13,

	  SEQUENCE_DAGGERS_IDLE1 = 1,
	  SEQUENCE_DAGGERS_LIGHT_MISS1 = 2,
	  SEQUENCE_DAGGERS_LIGHT_MISS5 = 6,
	  SEQUENCE_DAGGERS_HEAVY_MISS2 = 11,
	  SEQUENCE_DAGGERS_HEAVY_MISS1 = 12,

	  SEQUENCE_BOWIE_IDLE1 = 1,
   };

   auto random_sequence = [] ( const int low, const int high ) -> int {
	  return rand( ) % ( high - low + 1 ) + low;
   };

   // Hashes for best performance.
   switch ( model ) {
	  case hash_32_fnv1a_const( ( "models/weapons/v_knife_butterfly.mdl" ) ):
	  {
		 switch ( sequence ) {
			case SEQUENCE_DEFAULT_DRAW:
			return random_sequence( SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2 );
			case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence( SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03 );
			default:
			return sequence + 1;
		 }
	  }
	  case hash_32_fnv1a_const( ( "models/weapons/v_knife_falchion_advanced.mdl" ) ):
	  {
		 switch ( sequence ) {
			case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_FALCHION_IDLE1;
			case SEQUENCE_DEFAULT_HEAVY_MISS1:
			return random_sequence( SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP );
			case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence( SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02 );
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
			return sequence;
			default:
			return sequence - 1;
		 }
	  }
	  case hash_32_fnv1a_const( ( "models/weapons/v_knife_push.mdl" ) ):
	  {
		 switch ( sequence ) {
			case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_DAGGERS_IDLE1;
			case SEQUENCE_DEFAULT_LIGHT_MISS1:
			case SEQUENCE_DEFAULT_LIGHT_MISS2:
			return random_sequence( SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5 );
			case SEQUENCE_DEFAULT_HEAVY_MISS1:
			return random_sequence( SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1 );
			case SEQUENCE_DEFAULT_HEAVY_HIT1:
			case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
			case SEQUENCE_DEFAULT_LOOKAT01:
			return sequence + 3;
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
			return sequence;
			default:
			return sequence + 2;
		 }
	  }
	  case hash_32_fnv1a_const( ( "models/weapons/v_knife_survival_bowie.mdl" ) ):
	  {
		 switch ( sequence ) {
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
			return sequence;
			case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_BOWIE_IDLE1;
			default:
			return sequence - 1;
		 }
	  }
	  case hash_32_fnv1a_const( ( "models/weapons/v_knife_ursus.mdl" ) ):
	  case hash_32_fnv1a_const( ( "models/weapons/v_knife_skeleton.mdl" ) ):
	  case hash_32_fnv1a_const( ( "models/weapons/v_knife_outdoor.mdl" ) ):
	  case hash_32_fnv1a_const( ( "models/weapons/v_knife_canis.mdl" ) ):
	  case hash_32_fnv1a_const( ( "models/weapons/v_knife_cord.mdl" ) ):
	  {
		 switch ( sequence ) {
			case SEQUENCE_DEFAULT_DRAW:
			return random_sequence( SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2 );
			case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence( SEQUENCE_BUTTERFLY_LOOKAT01, 14 );
			default:
			return sequence + 1;
		 }
	  }
	  case hash_32_fnv1a_const( ( "models/weapons/v_knife_stiletto.mdl" ) ):
	  {
		 switch ( sequence ) {
			case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence( 12, 13 );
		 }
	  }
	  case hash_32_fnv1a_const( ( "models/weapons/v_knife_widowmaker.mdl" ) ):
	  {
		 switch ( sequence ) {
			case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence( 14, 15 );
		 }
	  }

	  default:
	  return sequence;
   }
}

CVariables::skin_changer_data* CSkinChanger::GetDataFromIndex( int idx ) {
   auto& skin_data = g_Vars.m_skin_changer;
   for ( size_t i = 0u; i < skin_data.Size( ); ++i ) {
	  auto skin = skin_data[ i ];
	  if ( skin->m_definition_index == idx )
		 return skin;
   }
   return nullptr;
}

void CSkinChanger::ForceItemUpdate( C_CSPlayer* local ) {
   auto ForceUpdate = [] ( C_BaseCombatWeapon* item ) {
	  C_EconItemView* view = &item->m_Item( );

	  auto clearRefCountedVector = [] ( CUtlVector< IRefCounted* >& vec ) {
		 for ( int i = 0; i < vec.m_Size; ++i ) {
			auto& elem = vec.m_Memory.m_pMemory[ i ];
			if ( elem ) {
			   elem->unreference( );
			   elem = nullptr;
			}
		 }
		 vec.m_Size = 0;
	  };

	  auto clearCustomMaterials = [] ( CUtlVector< IRefCounted* >& vec ) {
		 for ( int i = 0; i < vec.m_Size; ++i ) {
			auto& element = vec.m_Memory.m_pMemory[ i ];
			// actually makes no sense
			*( int* ) ( ( ( uintptr_t ) element ) + 0x10 ) = 0;
			*( int* ) ( ( ( uintptr_t ) element ) + 0x18 ) = 0;
			*( int* ) ( ( ( uintptr_t ) element ) + 0x20 ) = 0;
			*( int* ) ( ( ( uintptr_t ) element ) + 0x24 ) = 0;
			vec.m_Memory.m_pMemory[ i ] = nullptr;
		 }

		 vec.m_Size = 0;
	  };

	  item->m_bCustomMaterialInitialized( ) = false;
	  clearCustomMaterials( item->m_CustomMaterials( ) );
	  clearCustomMaterials( view->m_CustomMaterials( ) );
	  clearRefCountedVector( view->m_VisualsDataProcessors( ) );

	  item->GetClientNetworkable( )->PostDataUpdate( 0 );
	  item->GetClientNetworkable( )->OnDataChanged( 0 );
   };

   auto& global = g_Vars.m_global_skin_changer;
   auto weapons = local->m_hMyWeapons( );
   for ( size_t i = 0; i < 48; ++i ) {
	  auto weapon_handle = weapons[ i ];
	  if ( !weapon_handle.IsValid( ) )
		 break;

	  auto weapon = ( C_BaseCombatWeapon* ) weapon_handle.Get( );
	  if ( !weapon )
		 continue;

	  auto definition_index = weapon->m_Item( ).m_iItemDefinitionIndex( );
	  if ( const auto active_conf = GetDataFromIndex( is_knife( definition_index ) ? global.m_knife_idx : definition_index ) ) {
		 UpdateHud( );
		 ForceUpdate( weapon );
	  }
   }
}

void CSkinChanger::UpdateHud( ) {
   if ( Engine::Displacement.Function.m_uClearHudWeaponIcon ) {
	  static auto clear_hud_weapon_icon_fn =
		 reinterpret_cast< std::int32_t( __thiscall* )( void*, std::int32_t ) >( ( Engine::Displacement.Function.m_uClearHudWeaponIcon ) );
	  auto element = FindHudElement<std::uintptr_t*>( ( XorStr( "CCSGO_HudWeaponSelection" ) ) );

	  if ( element && clear_hud_weapon_icon_fn ) {
		 auto hud_weapons = reinterpret_cast< hud_weapons_t* >( std::uintptr_t( element ) - 0xA0 );
		 if ( hud_weapons == nullptr )
			return;

		 if ( !*hud_weapons->get_weapon_count( ) )
			return;

		 for ( std::int32_t i = 0; i < *hud_weapons->get_weapon_count( ) - 1; i++ )
			i = clear_hud_weapon_icon_fn( hud_weapons, i );
	  }
   }
}
