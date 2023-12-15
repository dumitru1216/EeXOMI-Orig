#pragma once
#include "../ExGui.hpp"

class c_tab : public c_element {
private:
	Vector2 cursor_pos;

public:
	c_tab( const std::string &label );

	void think( ) override;
	void draw( ) override;

	void add_child( c_element *child ) override;
	Vector2 get_child_draw_position( ) override;

	Vector2 get_cursor_pos( );
	void set_cursor_pos( const Vector2 &pos );

};
