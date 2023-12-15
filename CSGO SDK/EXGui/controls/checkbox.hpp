#pragma once
#include "../ExGui.hpp"

class c_checkbox : public c_element {
  bool* m_bVar;
public:
  c_checkbox( const std::string &label, bool* var );

  void think( ) override;
  void draw( ) override;

};

