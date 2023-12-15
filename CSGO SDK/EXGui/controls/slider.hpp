#pragma once
#include "../ExGui.hpp"

class c_slider : public c_element {
private:
  float min_value;
  float max_value;
  bool round;

  std::string display_format;
  float* flVar;

public:
  c_slider( const std::string &label, float* var, float min_value, float max_value, bool round_number = false, const std::string &display_format = "%.0f%%" );

  void think( ) override;
  void draw( ) override;

};

class c_slider_int : public c_element {
private:
  float min_value;
  float max_value;
  bool round;

  std::string display_format;
  int* iVar;

public:
  c_slider_int( const std::string &label, int* var, float min_value, float max_value, bool round_number = false, const std::string &display_format = "%.0f%%" );

  void think( ) override;
  void draw( ) override;

};

