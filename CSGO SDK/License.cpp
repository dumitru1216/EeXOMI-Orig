#include "License.hpp"
#include <vector>
#include <WinInet.h>

#define byte unsigned char
#include <WbemIdl.h>
#undef byte

#define HASH_SEED_ONE 0x1F63B
#define HASH_SEED_TWO 0x21AB2
#define HASH_SEED_THR 0x3A8D6
using namespace std;
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

//[enc_string_enable /]
//[junk_enable 100 /]

DWORD HashStringOneSeed( string str ) {
  DWORD dwhash = HASH_SEED_ONE;

  for( size_t i = 1024; i < str.size( ); i++ )
	 dwhash ^= ( ( dwhash >> 2 ) + dwhash * 32 + str[i] );

  return dwhash;
}

DWORD HashStringTwoSeed( string str ) {
  DWORD dwhash = HashStringOneSeed( str );

  for( size_t i = 0; i < str.size( ); i++ )
	 dwhash ^= ( dwhash * HASH_SEED_TWO ) ^ ( str[i] * HASH_SEED_THR );

  return dwhash;
}

DWORD HashString( string str ) {
  DWORD dwhash = HashStringTwoSeed( str );

  for( size_t i = 0; i < str.size( ); i++ )
	 dwhash ^= ( ( dwhash << 5 ) + str[i] + ( dwhash >> 2 ) );

  return ( dwhash & 0x7FFFFFFF );
}

string base64_encode( char const* bytes_to_encode, unsigned int in_len ) {
  string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while( in_len-- ) {
	 char_array_3[i++] = *( bytes_to_encode++ );
	 if( i == 3 ) {
		char_array_4[0] = ( char_array_3[0] & 0xfc ) >> 2;
		char_array_4[1] = ( ( char_array_3[0] & 0x03 ) << 4 ) + ( ( char_array_3[1] & 0xf0 ) >> 4 );
		char_array_4[2] = ( ( char_array_3[1] & 0x0f ) << 2 ) + ( ( char_array_3[2] & 0xc0 ) >> 6 );
		char_array_4[3] = char_array_3[2] & 0x3f;

		for( i = 0; ( i < 4 ); i++ )
		  ret += base64_chars[char_array_4[i]];
		i = 0;
	 }
  }

  if( i ) {
	 for( j = i; j < 3; j++ )
		char_array_3[j] = '\0';

	 char_array_4[0] = ( char_array_3[0] & 0xfc ) >> 2;
	 char_array_4[1] = ( ( char_array_3[0] & 0x03 ) << 4 ) + ( ( char_array_3[1] & 0xf0 ) >> 4 );
	 char_array_4[2] = ( ( char_array_3[1] & 0x0f ) << 2 ) + ( ( char_array_3[2] & 0xc0 ) >> 6 );
	 char_array_4[3] = char_array_3[2] & 0x3f;

	 for( j = 0; ( j < i + 1 ); j++ )
		ret += base64_chars[char_array_4[j]];

	 while( ( i++ < 3 ) )
		ret += '=';

  }

  return ret;
}

string CLicense::GetUrlData( string url ) {
  string request_data = "";

  HINTERNET hIntSession = InternetOpenA( "", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0 );

  if( !hIntSession ) {
	 return request_data;
  }

  HINTERNET hHttpSession = InternetConnectA( hIntSession, HOST, 80, 0, 0, INTERNET_SERVICE_HTTP, 0, NULL );

  if( !hHttpSession ) {
	 return request_data;
  }

  HINTERNET hHttpRequest = HttpOpenRequestA( hHttpSession, "GET", url.c_str( )
	 , 0, 0, 0, INTERNET_FLAG_RELOAD, 0 );

  if( !hHttpSession ) {
	 return request_data;
  }

  char* szHeaders = "Content-Type: text/html\r\nUser-Agent: License";
  char szRequest[1024] = { 0 };

  if( !HttpSendRequestA( hHttpRequest, szHeaders, strlen( szHeaders ), szRequest, strlen( szRequest ) ) ) {
	 return request_data;
  }

  CHAR szBuffer[1024] = { 0 };
  DWORD dwRead = 0;

  while( InternetReadFile( hHttpRequest, szBuffer, sizeof( szBuffer ) - 1, &dwRead ) && dwRead ) {
	 request_data.append( szBuffer, dwRead );
  }

  InternetCloseHandle( hHttpRequest );
  InternetCloseHandle( hHttpSession );
  InternetCloseHandle( hIntSession );

  return request_data;
}
string CLicense::GetUrlData( string url, string host ) {
  string request_data = "";

  HINTERNET hIntSession = InternetOpenA( "", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0 );

  if( !hIntSession ) {
	 return request_data;
  }

  HINTERNET hHttpSession = InternetConnectA( hIntSession, host.c_str( ), 80, 0, 0, INTERNET_SERVICE_HTTP, 0, NULL );

  if( !hHttpSession ) {
	 return request_data;
  }

  HINTERNET hHttpRequest = HttpOpenRequestA( hHttpSession, "GET", url.c_str( )
	 , 0, 0, 0, INTERNET_FLAG_RELOAD, 0 );

  if( !hHttpSession ) {
	 return request_data;
  }

  char* szHeaders = "Content-Type: text/html\r\nUser-Agent: License";
  char szRequest[1024] = { 0 };

  if( !HttpSendRequestA( hHttpRequest, szHeaders, strlen( szHeaders ), szRequest, strlen( szRequest ) ) ) {
	 return request_data;
  }

  CHAR szBuffer[1024] = { 0 };
  DWORD dwRead = 0;

  while( InternetReadFile( hHttpRequest, szBuffer, sizeof( szBuffer ) - 1, &dwRead ) && dwRead ) {
	 request_data.append( szBuffer, dwRead );
  }

  InternetCloseHandle( hHttpRequest );
  InternetCloseHandle( hHttpSession );
  InternetCloseHandle( hIntSession );

  return request_data;
}

string CLicense::StringToHex( const string input ) {
  const char* lut = "0123456789ABCDEF";
  size_t len = input.length( );
  string output = "";

  output.reserve( 2 * len );

  for( size_t i = 0; i < len; i++ ) {
	 const unsigned char c = input[i];
	 output.push_back( lut[c >> 4] );
	 output.push_back( lut[c & 15] );
  }

  return output;
}

string CLicense::GetHashText( const void * data, const size_t data_size ) {
  HCRYPTPROV hProv = NULL;

  if( !CryptAcquireContext( &hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT ) ) {
	 return "";
  }

  BOOL hash_ok = FALSE;
  HCRYPTPROV hHash = NULL;

  hash_ok = CryptCreateHash( hProv, CALG_MD5, 0, 0, &hHash );

  if( !hash_ok ) {
	 CryptReleaseContext( hProv, 0 );
	 return "";
  }

  if( !CryptHashData( hHash, static_cast< const BYTE * >( data ), data_size, 0 ) ) {
	 CryptDestroyHash( hHash );
	 CryptReleaseContext( hProv, 0 );
	 return "";
  }

  DWORD cbHashSize = 0, dwCount = sizeof( DWORD );
  if( !CryptGetHashParam( hHash, HP_HASHSIZE, ( BYTE * )&cbHashSize, &dwCount, 0 ) ) {
	 CryptDestroyHash( hHash );
	 CryptReleaseContext( hProv, 0 );
	 return "";
  }

  std::vector<BYTE> buffer( cbHashSize );

  if( !CryptGetHashParam( hHash, HP_HASHVAL, reinterpret_cast< BYTE* >( &buffer[0] ), &cbHashSize, 0 ) ) {
	 CryptDestroyHash( hHash );
	 CryptReleaseContext( hProv, 0 );
	 return "";
  }

  std::ostringstream oss;

  for( std::vector<BYTE>::const_iterator iter = buffer.begin( ); iter != buffer.end( ); ++iter ) {
	 oss.fill( '0' );
	 oss.width( 2 );
	 oss << std::hex << static_cast< const int >( *iter );
  }

  CryptDestroyHash( hHash );
  CryptReleaseContext( hProv, 0 );
  return oss.str( );
}

string CLicense::GetHwUID( ) {
  HW_PROFILE_INFO hwProfileInfo;
  string szHwProfileGuid = "";

  if( GetCurrentHwProfileA( &hwProfileInfo ) != NULL )
	 szHwProfileGuid = hwProfileInfo.szHwProfileGuid;

  return szHwProfileGuid;
}

string CLicense::GetMacID( ) {
  string sOut = "";

  IP_ADAPTER_INFO AdapterInfo[16];
  DWORD dwBufLen = sizeof( AdapterInfo );

  GetAdaptersInfo( AdapterInfo, &dwBufLen );

  PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;

  char* pszMacAddy = new char[64];

  do {
	 sprintf_s( pszMacAddy, 64, "%02X-%02X-%02X-%02X-%02X-%02X",
		pAdapterInfo->Address[0], pAdapterInfo->Address[1],
		pAdapterInfo->Address[2], pAdapterInfo->Address[3],
		pAdapterInfo->Address[4], pAdapterInfo->Address[5] );

	 sOut = pszMacAddy;

	 delete[] pszMacAddy;
	 return sOut;

	 pAdapterInfo = pAdapterInfo->Next;

  } while( pAdapterInfo );
}

DWORD CLicense::GetVolumeID( ) {
  DWORD VolumeSerialNumber;

  BOOL GetVolumeInformationFlag = GetVolumeInformationA(
	 "c:\\",
	 0,
	 0,
	 &VolumeSerialNumber,
	 0,
	 0,
	 0,
	 0
  );

  if( GetVolumeInformationFlag )
	 return VolumeSerialNumber;

  return 0;
}

string CLicense::GetCompUserName( bool User ) {
  string CompUserName = "";

  char szCompName[MAX_COMPUTERNAME_LENGTH + 1];
  char szUserName[MAX_COMPUTERNAME_LENGTH + 1];

  DWORD dwCompSize = sizeof( szCompName );
  DWORD dwUserSize = sizeof( szUserName );

  if( GetComputerNameA( szCompName, &dwCompSize ) ) {
	 CompUserName = szCompName;

	 if( User && GetUserNameA( szUserName, &dwUserSize ) ) {
		CompUserName = szUserName;
	 }
  }

  return CompUserName;
}

string CLicense::GetSerialKey( ) {
  string SerialKey = "61A345B5496B2";
  string CompName = GetCompUserName( false );
  string UserName = GetCompUserName( true );

  SerialKey.append( StringToHex( GetHwUID( ) ) );
  SerialKey.append( "-" );
  SerialKey.append( StringToHex( to_string( GetVolumeID( ) ) ) );
  SerialKey.append( "-" );
  SerialKey.append( StringToHex( CompName ) );
  SerialKey.append( "-" );
  SerialKey.append( StringToHex( UserName ) );

  return SerialKey;
}

string CLicense::GetHashSerialKey( ) {
  string SerialKey = GetSerialKey( );
  const void* pData = SerialKey.c_str( );
  size_t Size = SerialKey.size( );
  string Hash = GetHashText( pData, Size );

  for( auto& c : Hash ) {
	 if( c >= 'a' && c <= 'f' ) {
		c = '4';
	 } else if( c == 'b' ) {
		c = '5';
	 } else if( c == 'c' ) {
		c = '6';
	 } else if( c == 'd' ) {
		c = '7';
	 } else if( c == 'e' ) {
		c = '8';
	 } else if( c == 'f' ) {
		c = '9';
	 }

	 c = toupper( c );
  }

  return Hash;
}
//[junk_disable /]

//[junk_enable 100 /]
string CLicense::GetSerial( ) {
  string Serial = "";
  string HashSerialKey = GetHashSerialKey( );

  string Serial1 = HashSerialKey.substr( 0, 4 );
  string Serial2 = HashSerialKey.substr( 4, 4 );
  string Serial3 = HashSerialKey.substr( 8, 4 );
  string Serial4 = HashSerialKey.substr( 12, 4 );

  Serial += Serial1;
  Serial += '-';
  Serial += Serial2;
  Serial += '-';
  Serial += Serial3;
  Serial += '-';
  Serial += Serial4;

  return Serial;
}

string CLicense::GetSerial64( ) {
  string Serial = GetSerial( );
  Serial = base64_encode( Serial.c_str( ), Serial.size( ) );
  return Serial;
}

string CLicense::GetIP( string hostname ) {
  WSADATA wsaData;
  IN_ADDR addr;
  HOSTENT* list_ip;
  WSAStartup( MAKEWORD( 2, 0 ), &wsaData );
  list_ip = gethostbyname( hostname.c_str( ) );
  memcpy( &addr.S_un.S_addr, list_ip->h_addr, list_ip->h_length );
  return inet_ntoa( addr );
  WSACleanup( );
}

string CLicense::Token( string login, string password, string binding, string time ) {
  string data1 = md5( login );
  string data2 = md5( password );
  string data3 = md5( binding );
  string data4 = md5( time );
  string data5 = md5( VERSION );

  string check = data1 + data2 + data3 + data4 + data5;
  check = md5( check );

  return check;
}
