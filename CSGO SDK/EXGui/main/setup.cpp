
#include "../controls/tab.hpp"
#include "../controls/group.hpp"
#include "../controls/checkbox.hpp"
#include "../controls/slider.hpp"
#include "../controls/combo.hpp"
#include "../controls/colorpicker.hpp"
#include "../controls/button.hpp"
#include "../controls/multicombo.hpp"
#include "../ExGui.hpp"
#include "setup.hpp"

#include "../../CVariables.hpp"

void menu_setup::setup( ) {

  //Vector2 size = Vector2( 550, 600 );

  //menu->set_position( Vector2( ( g_Vars.globals.screen_width / 2 ) - size.x / 2, ( g_Vars.globals.screen_height / 2 ) - size.y / 2 ) );
  //menu->set_size( size );

  //size -= Vector2D( 50, 100 );

  //c_tab *rage_tab = new c_tab( "Aimbot" );
  //{
	 //menu->add_tab( rage_tab );

	 //c_group *general_group = new c_group( "General", size * Vector2( 0.5f, 0.5f ) );
	 //{
		//general_group->add_child( new c_checkbox( "Enable", &g_Vars.rage.enabled ) );
		//general_group->add_child( new c_checkbox( "Resolver", &g_Vars.rage.resolver ) );
		//general_group->add_child( new c_checkbox( "Lag Fix", &g_Vars.rage.lagfix ) );
		//general_group->add_child( new c_combo( "Resolver Type", &g_Vars.rage.resolver_type, { "Expiremental", "Brutforce" } ) );
		//general_group->add_child( new c_combo( "Position Adjustment", &g_Vars.rage.backtrack, { "Off", "Low", "Medium", "High" } ) );
		//general_group->add_child( new c_combo( "Delay Shot", &g_Vars.rage.delayshot, { "Off", "Unlag" } ) );
		//general_group->add_child( new c_multicombo( "Accuracy", { &g_Vars.rage.no_recoil,&g_Vars.rage.no_spread, },
		//  { "No recoil", "No Spread" } ) );
		//general_group->add_child( new c_multicombo( "Automatic", { &g_Vars.rage.autowall,&g_Vars.rage.autoscope, &g_Vars.rage.autorevolver },
		//  { "Autowall", "Auto Scope", "Auto revolver" } ) );
	 //}
	 //rage_tab->add_child( general_group );

	 //c_group *hitscan_group = new c_group( "Hitscan", size * Vector2( 0.5f, 0.5f ) );
	 //{
		//hitscan_group->add_child( new c_multicombo( "Hitboxes", { &g_Vars.rage.head_hitbox , &g_Vars.rage.neck_hitbox	,  &g_Vars.rage.chest_hitbox, &g_Vars.rage.stomach_hitbox	, &g_Vars.rage.pelvis_hitbox	, &g_Vars.rage.arms_hitbox	, &g_Vars.rage.legs_hitbox	, },{ "Head", "Neck", "Chest", "Stomach", "Pelvis", "Arms", "Legs" } ) );
		//hitscan_group->add_child( new c_multicombo( "Backtrack Hitboxes", { &g_Vars.rage.bt_head_hitbox , &g_Vars.rage.bt_neck_hitbox	,  &g_Vars.rage.bt_chest_hitbox, &g_Vars.rage.bt_stomach_hitbox	, &g_Vars.rage.bt_pelvis_hitbox	, &g_Vars.rage.bt_arms_hitbox	, &g_Vars.rage.bt_legs_hitbox	, },{ "Head", "Neck", "Chest", "Stomach", "Pelvis", "Arms", "Legs" } ) );
		//
		//hitscan_group->add_child( new c_slider( "Max Head Scale", &g_Vars.rage.head_ps, 0, 1.f, false, "%.1f%%" ) );
		//hitscan_group->add_child( new c_slider( "Max Neck Scale", &g_Vars.rage.neck_ps, 0, 1.f, false, "%.1f%%" ) );
		//hitscan_group->add_child( new c_slider( "Max Stomach Scale", &g_Vars.rage.stomach_ps, 0, 1.f, false, "%.1f%%" ) );
		//hitscan_group->add_child( new c_slider( "Max Pelvis Scale", &g_Vars.rage.pelvis_ps, 0, 1.f, false, "%.1f%%" ) );
		//hitscan_group->add_child( new c_slider( "Max Chest Scale", &g_Vars.rage.chest_ps, 0, 1.f, false, "%.1f%%" ) );
		//hitscan_group->add_child( new c_slider( "Max Legs Scale", &g_Vars.rage.arms_ps, 0, 1.f, false, "%.1f%%" ) );
		//hitscan_group->add_child( new c_slider( "Max Neck Scale", &g_Vars.rage.legs_ps, 0, 1.f, false, "%.1f%%" ) );
		//
		////hitscan_group->add_child( new c_hotkey("Test bind ", &g_Vars.antiaim.manual_back_bind ) );
	 //}
	 //rage_tab->add_child( hitscan_group );

	 //c_group *accuracy_group = new c_group( "Accuracy", size * Vector2( 0.5f, 0.5f ) );
	 //{
		//accuracy_group->add_child( new c_slider( "Hitchance", &g_Vars.rage.hitchance, 0, 100.f, true, "%.0f%%" ) );
		//accuracy_group->add_child( new c_slider( "Taser Hitchance", &g_Vars.rage.taser_hitchance, 0, 100.f, true, "%.0f%%" ) );
		//accuracy_group->add_child( new c_slider( "Min Damage", &g_Vars.rage.min_damage, 0, 100.f, true, "%.0f%%" ) );
		////accuracy_group->add_child( new c_colorpicker("Menu Color",&g_Vars.menu.main_color,true,false ) );
	 //}
	 //rage_tab->add_child( accuracy_group );

	 //c_group *hitscan1_group = new c_group( "Adjustment", size * Vector2( 0.5f, 0.5f ) );
	 //{
		//hitscan1_group->add_child( new c_combo( "Priority Hitbox", &g_Vars.rage.target_hitbox, { "Off", "Head", "Neck", "Chest", "Stomach", "Pelvis" } ) );
		//hitscan1_group->add_child( new c_checkbox( "Priority if In air", &g_Vars.rage.targetbox_inair ) );
		//hitscan1_group->add_child( new c_checkbox( "Priority if Lethal", &g_Vars.rage.targetbox_lethal ) );
		//hitscan1_group->add_child( new c_checkbox( "Priority On Hitbox Center", &g_Vars.rage.center_priority ) );
		//hitscan1_group->add_child( new c_checkbox( "Ignore limbs on move", &g_Vars.rage.ignorelimbs_ifwalking ) );
		//hitscan1_group->add_child( new c_checkbox( "Stop Backtrack if Lethal", &g_Vars.rage.stop_backtrack_if_lethal ) );
		//hitscan1_group->add_child( new c_combo( "Auto Stop", &g_Vars.rage.autostop, { "Off", "Full Stop", "Minimal speed" } ) );
		//hitscan1_group->add_child( new c_checkbox( "Disable auto stop in air", &g_Vars.rage.autostop_disable_inair ) );
		//hitscan1_group->add_child( new c_slider_int( "Priority after shoots", &g_Vars.rage.target_after_xshots, 0, 20, true, "%i Shots" ) );
	 //}
	 //rage_tab->add_child( hitscan1_group );

  //}

  //c_tab *antiaim_tab = new c_tab( "Anti-Aim" );
  //{
	 //menu->add_tab( antiaim_tab );
	 //c_group *antiaim_group = new c_group( "Main", size * Vector2( 0.5f, 0.95f ) );
	 //{
		//antiaim_group->add_child( new c_combo( "Pitch", &g_Vars.antiaim.antiaim_x, { "Down", "Up", "Zero", "Secret" } ) );
		//antiaim_group->add_child( new c_combo( "Yaw", &g_Vars.antiaim.antiaim_y, { "Backward", "Forward", "Manual", "Spinbot" } ) );
		//antiaim_group->add_child( new c_combo( "Shift Pitch", &g_Vars.antiaim.antiaim_shift_x, { "Off", "Inverse", "Forward" } ) );
		//antiaim_group->add_child( new c_combo( "Shift Yaw", &g_Vars.antiaim.antiaim_shift_x, { "Off", "Inverse", "Static", "Spinbot"} ) );
		//antiaim_group->add_child( new c_combo( "Pitch Base", &g_Vars.antiaim.angle_base_pitch, { "Static", "View", "At targets" } ) );
		//antiaim_group->add_child( new c_combo( "Yaw Base", &g_Vars.antiaim.angle_base_yaw, { "Static", "View", "At targets"} ) );
		//antiaim_group->add_child( new c_combo( "Direction", &g_Vars.antiaim.autodirection, { "Off", "On", "Auto Desync" } ) );
		//antiaim_group->add_child( new c_combo( "Desync", &g_Vars.antiaim.desync, { "Off", "Static", "Balance", "Jitter", "Jitter Test"} ) );
		//
		//antiaim_group->add_child( new c_hotkey("Desync Flip ", &g_Vars.antiaim.desync_flip_bind ) );

		//antiaim_group->add_child( new c_hotkey("Manual Left ", &g_Vars.antiaim.manual_left_bind ) );
		//antiaim_group->add_child( new c_hotkey("Manual Right ", &g_Vars.antiaim.manual_right_bind ) );
		//antiaim_group->add_child( new c_hotkey("Manual Back ", &g_Vars.antiaim.manual_back_bind ) );

  c_tab *antiaim_tab = new c_tab( "Anti-Aim" );
  {
	 menu->add_tab( antiaim_tab );
	 c_group *antiaim_group = new c_group( "Main", size * Vector2( 0.5f, 0.95f ) );
	 {
		antiaim_group->add_child( new c_combo( "Pitch", &g_Vars.antiaim.antiaim_x, { "Down", "Up", "Zero", "Secret" } ) );
		antiaim_group->add_child( new c_combo( "Yaw", &g_Vars.antiaim.antiaim_y, { "Backward", "Forward", "Manual", "Spinbot" } ) );
		antiaim_group->add_child( new c_combo( "Shift Pitch", &g_Vars.antiaim.antiaim_shift_x, { "Off", "Inverse", "Forward" } ) );
		antiaim_group->add_child( new c_combo( "Shift Yaw", &g_Vars.antiaim.antiaim_shift_x, { "Off", "Inverse", "Static", "Spinbot"} ) );
		antiaim_group->add_child( new c_combo( "Pitch Base", &g_Vars.antiaim.angle_base_pitch, { "Static", "View", "At targets" } ) );
		antiaim_group->add_child( new c_combo( "Yaw Base", &g_Vars.antiaim.angle_base_yaw, { "Static", "View", "At targets"} ) );
		antiaim_group->add_child( new c_combo( "Direction", &g_Vars.antiaim.autodirection, { "Off", "On", "Auto Desync" } ) );
		antiaim_group->add_child( new c_combo( "Desync", &g_Vars.antiaim.desync, { "Off", "Static", "Balance", "Jitter", "Jitter Test"} ) );
		
	//	antiaim_group->add_child( new c_hotkey("Desync Flip ", &g_Vars.antiaim.desync_flip_bind ) );

	//	antiaim_group->add_child( new c_hotkey("Manual Left ", &g_Vars.antiaim.manual_left_bind ) );
	//	antiaim_group->add_child( new c_hotkey("Manual Right ", &g_Vars.antiaim.manual_right_bind ) );
	//	antiaim_group->add_child( new c_hotkey("Manual Back ", &g_Vars.antiaim.manual_back_bind ) );

		antiaim_group->add_child( new c_slider( "Spin Speed", &g_Vars.antiaim.spin_speed, 0, 20.f, false, "%.0f%%" ) );

		//antiaim_group->add_child( new c_checkbox( "Hide Shoots", &g_Vars.rage.hide_shots) );

	 //}
	 //antiaim_tab->add_child( antiaim_group );
	 //c_group *fakelag_group = new c_group( "Fakelag", size * Vector2( 0.5f, 0.95f ) );
	 //{

	 //}
	 //antiaim_tab->add_child( fakelag_group );
	 ///*ImGui::PushItemWidth( 175.0f );
	 //ImGui::Combo( "AntiAim Pitch", &g_Vars.antiaim.antiaim_x, std::vector<std::string>{ "Down", "Up", "Zero", "Secret"} );
	 //ImGui::Combo( "AntiAim Yaw", &g_Vars.antiaim.antiaim_y, std::vector<std::string>{ "Backward", "Forward", "Manual", "Spinbot"} );
	 //ImGui::Combo( "AntiAim Shift Pitch", &g_Vars.antiaim.antiaim_shift_x, std::vector<std::string>{ "Off", "Inverse", "Forward" } );
	 //ImGui::Combo( "AntiAim Shift Yaw", &g_Vars.antiaim.antiaim_shift_y, std::vector<std::string>{  "Off", "Inverse", "Static", "Spinbot"} );
	 //ImGui::Combo( "Pitch Angle Base", &g_Vars.antiaim.angle_base_pitch, std::vector<std::string>{ "Static", "View", "At targets" } );
	 //ImGui::Combo( "Yaw Angle Base", &g_Vars.antiaim.angle_base_yaw, std::vector<std::string>{ "Static", "View", "At targets" } );
	 //ImGui::Combo( "Auto direction", &g_Vars.antiaim.autodirection, std::vector<std::string>{ "Off", "On", "Auto Desync" } );
	 //ImGui::Combo( "Desync", &g_Vars.antiaim.desync, std::vector<std::string>{ "Off", "Static", "Balance", "Jitter", "Jitter Test" } );
	 //ImGui::PopItemWidth( );

	 //ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
	 //ImGui::KeyBox( &g_Vars.antiaim.desync_flip_bind );

	 //ImGui::Text( "Bind Left" );
	 //ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
	 //ImGui::KeyBox( &g_Vars.antiaim.manual_left_bind );
	 //ImGui::Text( "Bind Right" );
	 //ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
	 //ImGui::KeyBox( &g_Vars.antiaim.manual_right_bind );
	 //ImGui::Text( "Bind Back" );
	 //ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
	 //ImGui::KeyBox( &g_Vars.antiaim.manual_back_bind );

	 //ImGui::SliderFloatA( "Spin speed", &g_Vars.antiaim.spin_speed, -20.0f, 20.0f, "%.1f degrees" );

	 //ImGui::Checkbox( "Hide shots", &g_Vars.rage.hide_shots );*/

  //}

  //c_tab *visuals_tab = new c_tab( "Visuals" );
  //{
	 //menu->add_tab( visuals_tab );

	 //c_group *player_group = new c_group( "Player ESP", size * Vector2( 0.5f, 0.5f ) );
	 //{

	 //}

	 //c_group *models_group = new c_group( "Colored models", size * Vector2( 0.5f, 0.5f ) );
	 //{

	 //}
	 //visuals_tab->add_child( models_group );


	 //c_group *other_group = new c_group( "Other esp", size *  Vector2( 0.5f, 0.5f ) );
	 //{

	 //}
	 //visuals_tab->add_child( other_group );

	 //c_group *effects_group = new c_group( "Effects", size * Vector2( 0.5f, 0.5f ) );
	 //{

	 //}
	 //visuals_tab->add_child( effects_group );

  //}

  //c_tab *misc_tab = new c_tab( "Miscellaneous" );
  //{
	 //menu->add_tab( misc_tab );

	 //c_group *general_group = new c_group( "General", size * Vector2( 0.5f, 0.90f ) );
	 //{


	 //}
	 //misc_tab->add_child( general_group );

	 //c_group *settings_group = new c_group( "Settings", size * Vector2( 0.5f, 0.90f ) );
	 //{


	 //}
	 //misc_tab->add_child( settings_group );
  //}

  //menu->set_active_tab( visuals_tab );
}

