#pragma once
#include "../ExGui.hpp"

class c_combo : public c_element {
private:
	std::vector<std::string> items;
	int* m_iVar;

public:
	c_combo( const std::string &label, int* var, const std::vector<std::string> &items = { "None" } );

	void think( ) override;
	void draw( ) override;

};

