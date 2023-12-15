#pragma once
#include "../ExGui.hpp"

class c_hotkey : public c_element {
private:
	bool is_inline;
	bool is_picking = false;
	int* m_iKey = 0;
	bool context_menu_open = false;
	Vector2 context_menu_pos;

	std::vector< std::pair< std::string, std::function< void( c_hotkey* ) > > > context_menu_actions;

	static void context_menu_always_on( c_hotkey *self );
	static void context_menu_on_hotkey( c_hotkey *self );
	static void context_menu_toggle( c_hotkey *self );
	static void context_menu_off_hotkey( c_hotkey *self );

public:
	c_hotkey( const std::string &label, int* key, bool is_inline = false );

	void think( ) override;
	void draw( ) override;

};
