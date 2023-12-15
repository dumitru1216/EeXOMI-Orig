#pragma once
#include "../ExGui.hpp"

class c_multicombo : public c_element {
private:
  std::vector<std::string> items;
  std::vector<bool*> m_vecVar;

public:
  c_multicombo( const std::string &label, std::vector<bool*> var, const std::vector<std::string> &items = { "None" } );

  void think( ) override;
  void draw( ) override;

};

