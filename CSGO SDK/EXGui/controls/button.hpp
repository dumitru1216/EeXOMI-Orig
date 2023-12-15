#pragma once
#include "../ExGui.hpp"

class c_button : public c_element {
private:
	std::function< void( ) > callback;
	bool active;

public:
	c_button( const std::string &label, std::function< void( ) > callback, const Vector2 &size = Vector2( 0, 20 ) );

	void think( ) override;
	void draw( ) override;

};
