#pragma once
#include <windows.h>

#include <iphlpapi.h>
#include <string>
#include <sstream>
#include <string>
#include "md5.hpp"
//#include "Decrypt.h"

using namespace std;

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "iphlpapi")

//[enc_string_enable /]
#define HOST			"panel.eexomi.host"
#define VERSION			"1"
#define VERSION_FREE	"3.1"

class CLicense
{
private:
  //[swap_lines]
  string	StringToHex(const string input);
  string	GetHashText(const void * data, const size_t data_size);
  string	GetHwUID();
  string	GetMacID();
  DWORD	GetVolumeID();
  string	GetCompUserName(bool User);
  string	GetSerialKey();
  string	GetHashSerialKey();
  //[/swap_lines]
public:
  //[swap_lines]
  string	GetUrlData(string url);
  string GetUrlData(string url, string host);
  string	GetSerial();
  string	GetSerial64();
  string  GetIP(string hostname);
  string  Token(string login, string password, string binding, string time);
  //[/swap_lines]
};

string base64_encode(char const* bytes_to_encode, unsigned int in_len);
