#include "source.hpp"
#include "ExGui.hpp"

namespace Menu
{
  c_ui UI;

  void c_ui::update_ui( ) {
	 // Let's see if the menu was toggled.	

	 if( InputSys::Get( )->WasKeyPressed( VirtualKeys::Insert ) ) {
		g_Vars.globals.menuOpen = !g_Vars.globals.menuOpen;
	 }

	 // Update menu opacity.
	 constexpr float fade_ratio = 1.0f / 0.70f;
	 const     float  cur_ratio = fade_ratio * Source::m_pGlobalVars->absoluteframetime;

	 float new_alpha = g_Vars.globals.MenuAlpha;
	 new_alpha += g_Vars.globals.menuOpen ? cur_ratio : -cur_ratio;
	 new_alpha = Math::Clamp( new_alpha, 0.0f, 1.0f );

	 g_Vars.globals.MenuAlpha = new_alpha;
  }

  void c_ui::initialize( ) {
	 // Implement an object container.
	 m_container = std::make_shared< object::c_container >( );

	 auto form = std::make_shared< object::c_form >( 50.0f, 50.0f, 650.0f, 450.0f ); {
		auto tab_control = std::make_shared< object::c_tab_control >( ); {
		  auto aimbot_tab = std::make_shared< object::c_tab >( "aimbot" ); {
			 auto container = std::make_shared< object::c_tab_container >( ); {
				container->push( std::make_shared<object::c_checkbox>( "enable", &g_Vars.rage.active ) );
				container->push( std::make_shared<object::c_checkbox>( "silent aim", &g_Vars.rage.silent_aim ) );
				container->push( std::make_shared<object::c_checkbox>( "resolver", &g_Vars.rage.resolver ) );
				container->push( std::make_shared<object::c_checkbox>( "lag fix", &g_Vars.rage.lagfix ) );

				container->push( std::make_shared< object::c_dropdown >( "resolver type", &g_Vars.rage.resolver_type, std::vector<std::string>{"experimental", "bruteforce"} ) );
				container->push( std::make_shared< object::c_dropdown >( "position adjustment", &g_Vars.rage.backtrack, std::vector<std::string>{"off", "low", "medium", "high"} ) );
				container->push( std::make_shared< object::c_dropdown >( "delay shot", &g_Vars.rage.delayshot, std::vector<std::string>{"off", "unlag"} ) );

				std::vector< object::multi_item_t > accuracy{
				  {"no recoil", &g_Vars.rage.no_recoil },
				  {"no spread", &g_Vars.rage.no_spread },
				};

				std::vector< object::multi_item_t > automatic{
				  {"autowall",&g_Vars.rage.autowall },
				  {"auto revolver",&g_Vars.rage.autorevolver },
				  {"auto scope",&g_Vars.rage.autoscope }
				};

				container->push( std::make_shared<object::c_multi_dropdown>( "accuracy", accuracy ) );
				container->push( std::make_shared<object::c_multi_dropdown>( "automatic", automatic ) );

			 }
			 container->split( ); {
				container->push( std::make_shared<object::c_slider<float> >( "taser hitchacne", 1.0f, 100.0f, &g_Vars.rage.taser_hitchance ) );
				container->push( std::make_shared<object::c_slider<float> >( "hitchacne", 1.0f, 100.0f, &g_Vars.rage.hitchance ) );
				container->push( std::make_shared<object::c_slider<float> >( "minimal damage", 1.0f, 100.0f, &g_Vars.rage.min_damage ) );
			 }
			 aimbot_tab->push( container );
		  }
		  tab_control->push( aimbot_tab );

		  auto hitscan_tab = std::make_shared< object::c_tab >( "hitscan" ); {
			 auto container = std::make_shared< object::c_tab_container >( ); {\

				std::vector< object::multi_item_t > multipoint_hitscan{
				{"neck",&g_Vars.rage.neck_hitbox },
				{"body",&g_Vars.rage.pelvis_hitbox },
				{"stomach",&g_Vars.rage.stomach_hitbox },
				{"chest",&g_Vars.rage.chest_hitbox },
				{"arms",&g_Vars.rage.arms_hitbox },
				{"legs",&g_Vars.rage.legs_hitbox },
			 };

			 std::vector< object::multi_item_t > backtrack_multipoint_hitscan{
				{"head",&g_Vars.rage.bt_head_hitbox },
			   {"neck",&g_Vars.rage.bt_neck_hitbox },
				{"body",&g_Vars.rage.bt_pelvis_hitbox },
				{"stomach",&g_Vars.rage.bt_stomach_hitbox },
				{"chest",&g_Vars.rage.bt_chest_hitbox },
				{"arms",&g_Vars.rage.bt_arms_hitbox },
				{"legs",&g_Vars.rage.bt_legs_hitbox },
			 }; 

			 container->push( std::make_shared<object::c_multi_dropdown>( "multipoints", multipoint_hitscan ) );
			 container->push( std::make_shared<object::c_multi_dropdown>( "backtrack override", backtrack_multipoint_hitscan ) );
			 container->push( std::make_shared<object::c_dropdown >( "priority hitbox", &g_Vars.rage.target_hitbox, std::vector<std::string>{"off", "head", "neck", "chest", "stomach", "pelvis"} ) );
			
			 container->push( std::make_shared<object::c_checkbox>( "priority if in air", &g_Vars.rage.targetbox_inair ) );
			 container->push( std::make_shared<object::c_checkbox>( "priority if lethal", &g_Vars.rage.targetbox_lethal ) );
			 container->push( std::make_shared<object::c_checkbox>( "stop lag comp if lethal", &g_Vars.rage.stop_backtrack_if_lethal ) );

			 container->push( std::make_shared<object::c_checkbox>( "ignore limbs on move", &g_Vars.rage.ignorelimbs_ifwalking ) );
			 container->push( std::make_shared<object::c_checkbox>( "priority on hitbox center", &g_Vars.rage.center_priority ) );
	
			 }
			 container->split( ); {
				container->push( std::make_shared<object::c_slider<float> >( "pointscale head", 1.0f, 100.0f, &g_Vars.rage.head_ps ) );
				container->push( std::make_shared<object::c_slider<float> >( "pointscale neck", 1.0f, 100.0f, &g_Vars.rage.neck_ps ) );
				container->push( std::make_shared<object::c_slider<float> >( "pointscale body", 1.0f, 100.0f, &g_Vars.rage.pelvis_ps ) );
				container->push( std::make_shared<object::c_slider<float> >( "pointscale stomach", 1.0f, 100.0f, &g_Vars.rage.stomach_ps ) );
				container->push( std::make_shared<object::c_slider<float> >( "pointscale chest", 1.0f, 100.0f, &g_Vars.rage.chest_ps ) );
				container->push( std::make_shared<object::c_slider<float> >( "pointscale arms", 1.0f, 100.0f, &g_Vars.rage.arms_ps ) );
				container->push( std::make_shared<object::c_slider<float> >( "pointscale legs", 1.0f, 100.0f, &g_Vars.rage.legs_ps ) );
				container->push( std::make_shared<object::c_slider<int> >( "priority after shoots", 0, 20, &g_Vars.rage.target_after_xshots ) );
			 }
			 hitscan_tab->push( container );
		  }
		  tab_control->push( hitscan_tab );

		  auto misc_tab = std::make_shared< object::c_tab >( "misc" ); {
			 auto container = std::make_shared< object::c_tab_container >( ); {\

			 }
			 container->split( ); {
			 }
			 misc_tab->push( container );
		  }
		  tab_control->push( misc_tab );

		 /* auto test_tab = std::make_shared< object::c_tab >( "test" ); {
			 auto container = std::make_shared< object::c_tab_container >( ); {
				static bool var1,var2,var3;
				std::vector< object::multi_item_t > test_multi{
				{ "apple", &var1 },
				{ "pasta", &var2},
				{ "polak", &var3},

				};

				container->push( std::make_shared<object::c_multi_dropdown>( "ya multi combo", test_multi ) );

				container->push( std::make_shared< object::c_colorpicker >( "menu color", &g_Vars.menu.main_color ) );
			 }
			 container->split( ); {
			 }
			 misc_tab->push( container );
		  }
		  tab_control->push( test_tab );*/
		}
		form->push( tab_control );
	 }
	 m_container->push( form );

	 m_initialized = true;
  }

  void c_ui::on_frame( ) {
	 if( !m_initialized )
		return;

	 // Make sure we update the UI every frame.
	 update_ui( );

	 // Draw the UI.
	 if( g_Vars.globals.MenuAlpha >= 0.02f )
		m_container->draw( );
  }
}
