#include "source.hpp"
#include "InputSys.hpp"
#include "defs.hpp"

#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma disable(warning:4099)

#include "md5.hpp"
#include "aes_encryption.hpp"
#include "License.hpp"

#ifndef _DEBUG
#pragma comment(lib, "cryptlib.lib")
#else
#pragma comment(lib, "cryptlib_debug.lib")
#endif

#include "Utils/threading.h"
#include "Utils/shared_mutex.h"
#include "displacement.hpp"
#include "CVariables.hpp"
#include "lazy_importer.hpp"
#include "CSecure.hpp"
#include "VMP.hpp"
#include "CrashHandler.hpp"
#include <thread>

// #define BEEP_DEBUG

#include <iomanip> 
#include "syscall.hpp"

std::string url_encode( const  std::string& value ) {
   std::ostringstream escaped;
   escaped.fill( '0' );
   escaped << hex;

   for ( std::string::const_iterator i = value.begin( ), n = value.end( ); i != n; ++i ) {
	  std::string::value_type c = ( *i );

	  // Keep alphanumeric and other accepted characters intact
	  if ( isalnum( c ) || c == '-' || c == '_' || c == '.' || c == '~' ) {
		 escaped << c;
		 continue;
	  }

	  // Any other characters are percent-encoded
	  escaped << uppercase;
	  escaped << '%' << std::setw( 2 ) << int( ( unsigned char ) c );
	  escaped << nouppercase;
   }

   return escaped.str( );
}

static Semaphore dispatchSem;
static SharedMutex smtx;

using ThreadIDFn = int( _cdecl* )( );

ThreadIDFn AllocateThreadID;
ThreadIDFn FreeThreadID;

int AllocateThreadIDWrapper( ) {
   return AllocateThreadID( );
}

int FreeThreadIDWrapper( ) {
   return FreeThreadID( );
}

template<typename T, T& Fn>
static void AllThreadsStub( void* ) {
   dispatchSem.Post( );
   smtx.rlock( );
   smtx.runlock( );
   Fn( );
}

// TODO: Build this into the threading library
template<typename T, T& Fn>
static void DispatchToAllThreads( void* data ) {
   smtx.wlock( );

   for ( size_t i = 0; i < Threading::numThreads; i++ )
	  Threading::QueueJobRef( AllThreadsStub<T, Fn>, data );

   for ( size_t i = 0; i < Threading::numThreads; i++ )
	  dispatchSem.Wait( );

   smtx.wunlock( );

   Threading::FinishQueue( false );
}

struct DllArguments {
   HMODULE hModule;
   LPVOID lpReserved;
};

DWORD WINAPI Entry( DllArguments* pArgs ) {
#ifdef DevelopMode
   AllocConsole( );
   freopen_s( ( FILE * * ) stdin, XorStr( "CONIN$" ), XorStr( "r" ), stdin );
   freopen_s( ( FILE * * ) stdout, XorStr( "CONOUT$" ), XorStr( "w" ), stdout );
   SetConsoleTitleA( XorStr( " " ) );
#endif

#ifdef BEEP_DEBUG
   LI_FN( Beep ).forwarded_safe_cached( )( 500, 500 );
#endif

   //LI_FN( Beep ).forwarded_safe_cached( )( 500, 100 );

   auto prior = GetPriorityClass( GetCurrentProcess( ) );
   if ( prior != REALTIME_PRIORITY_CLASS && prior != HIGH_PRIORITY_CLASS )
	  SetPriorityClass( GetCurrentProcess( ), HIGH_PRIORITY_CLASS );

   int maxIterations = 0;
   while ( !GetModuleHandleA( XorStr( "serverbrowser.dll" ) ) ) {
	  Sleep( 500 );
	  maxIterations++;
	  if ( maxIterations >= 10 )
		 break;
   }

   auto tier0 = GetModuleHandleA( XorStr( "tier0.dll" ) );

   AllocateThreadID = ( ThreadIDFn ) GetProcAddress( tier0, XorStr( "AllocateThreadID" ) );
   FreeThreadID = ( ThreadIDFn ) GetProcAddress( tier0, XorStr( "FreeThreadID" ) );

   Threading::InitThreads( );

   DispatchToAllThreads<decltype( AllocateThreadIDWrapper ), AllocateThreadIDWrapper>( nullptr );

   if ( Source::Create( pArgs->lpReserved ) ) {
   #ifdef BEEP_DEBUG
	  LI_FN( Beep ).forwarded_safe_cached( )( 500, 500 );
   #endif
	  LI_FN( Beep ).forwarded_safe_cached( )( 500, 100 );

	  printf( XorStr( "Create\n" ) );

	  while ( !g_Vars.globals.hackUnload )
		 Sleep( 100 );

	  g_Vars.globals.d3dinitialize = true;
	  Source::Destroy( );
	  Sleep( 500 );
	  Source::m_pClientState->m_nDeltaTick( ) = -1;
	  Sleep( 500 );
   }

#ifdef BEEP_DEBUG
   LI_FN( Beep ).forwarded_safe_cached( )( 1000, 2000 );
#endif

   Threading::FinishQueue( );

   DispatchToAllThreads<decltype( FreeThreadIDWrapper ), FreeThreadIDWrapper>( nullptr );

   Threading::EndThreads( );

   FreeLibraryAndExitThread( pArgs->hModule, EXIT_SUCCESS );

   delete pArgs;
}

#ifndef DevelopMode
void Setup( DllArguments* pArgs ) {
   VMP_BEGIN_ULTRA( );
#ifdef BEEP_DEBUG
   LI_FN( Beep ).forwarded_safe_cached( )( 500, 500 );
#endif
   g_secure.test( );
#if 0
   LI_FN( MessageBoxA ).forwarded_safe_cached( )( 0,
	  string( string( "Login: " ) + g_Vars.globals.c_login + string( " | Password: " ) + g_Vars.globals.c_password ).c_str( ),
	  XorStr( "Data Check" ), MB_OK | MB_ICONQUESTION );
#endif

   std::string ServerRequest;
   //MessageBoxA( 0, std::string( pArgs->lpReserved ).c_str( ), "", MB_OK );
   // MessageBoxA( 0, ( g_Vars.globals.c_login + std::string( XorStr( " " ) ) + g_Vars.globals.c_password ).c_str( ), "", MB_OK );

   CLicense License;

   string NewSerial = License.GetSerial( );
   string path = ( XorStr( "/license/?login=" ) ) + g_Vars.globals.c_login + ( XorStr( "&password=" ) ) + g_Vars.globals.c_password + ( XorStr( "&binding=" ) );
   string timephp = ( XorStr( "/license/time/" ) );
   string timetoleft = ( XorStr( "/license/usertime/?login=" ) ) + g_Vars.globals.c_login + ( XorStr( "&password=" ) ) + g_Vars.globals.c_password;
   string Time = License.GetUrlData( timephp );
   string Token = License.Token( g_Vars.globals.c_login, g_Vars.globals.c_password, NewSerial, Time );
   string ReLicenseUrl = path + NewSerial + ( XorStr( "&token=" ) ) + Token;

   std::string password = md5( Token );
   std::string serverDataPassword = XorStr( "X828Gw2JbGkBBbbdum8MKV@YVZSjHqg1#pErAXGlBH352SPZ1b" );
   AesEncryption aes( XorStr( "cbc" ), 256 );
   CryptoPP::SecByteBlock ValidCryptoStr = aes.encrypt( NewSerial, password );
   bool LoginState = false;

   ServerRequest = License.GetUrlData( ReLicenseUrl );

   CryptoPP::SecByteBlock ppServerDec = aes.decrypt( ServerRequest, serverDataPassword );
   std::string serverDec = std::string( ppServerDec.begin( ), ppServerDec.end( ) );

   CryptoPP::SecByteBlock ValidServerDec = aes.decrypt( serverDec, password );
   std::string ifValidDecServer = std::string( ValidServerDec.begin( ), ValidServerDec.end( ) );

#if 0
   MessageBoxA( 0, serverDec.c_str( ), "", MB_OK );
   MessageBoxA( 0, ifValidDecServer.c_str( ), "", MB_OK );
#endif

   if ( serverDec == ( XorStr( "error1440" ) ) ) {
	  LoginState = false;
   } else if ( serverDec == ( XorStr( "error1444" ) ) ) {
	  LoginState = false;
   } else if ( serverDec == ( XorStr( "error1403" ) ) ) {
	  LoginState = false;
   } else if ( serverDec == ( XorStr( "error0" ) ) ) {
	  LoginState = false;
   } else if ( serverDec == ( XorStr( "error1404" ) ) ) {
	  LoginState = false;
   } else if ( serverDec == ( XorStr( "errorHWID" ) ) ) {
	  LoginState = false;
   } else if ( serverDec == ( XorStr( "errorTIME" ) ) ) {
	  LoginState = false;
   } else if ( serverDec == ( XorStr( "gg" ) ) ) {
	  LoginState = false;
   } else if ( ifValidDecServer == NewSerial ) {
   #if 0
	  LI_FN( MessageBoxA ).forwarded_safe_cached( )( 0,
		 "Logined",
		 XorStr( "HWID Check" ), MB_OK | MB_ICONQUESTION );
   #endif
	  LoginState = true;
   } else {
	  LoginState = false;
   }

   if ( LoginState ) {
	  if ( ifValidDecServer == NewSerial ) {
		 //MessageBoxA( 0, XorStr( "thread start" ), "", MB_OK );
		 //CreateThread( nullptr, 0u, ( LPTHREAD_START_ROUTINE ) Entry, pArgs, 0u, nullptr );

         HANDLE thread;

         syscall( NtCreateThreadEx )( &thread, THREAD_ALL_ACCESS, nullptr, current_process,
                                      nullptr, pArgs, THREAD_CREATE_FLAGS_CREATE_SUSPENDED | THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER, NULL, NULL, NULL, nullptr );
         CONTEXT context;
         context.ContextFlags = CONTEXT_FULL;
         syscall( NtGetContextThread )( thread, &context );
         context.Eax = reinterpret_cast< uint32_t >( &Entry );
         syscall( NtSetContextThread )( thread, &context );
         syscall( NtResumeThread )( thread, nullptr );
	  } else { // ban naxoy
		 string banReason = XorStr( "CheatCantLoginWithLoaderData" );

		 CryptoPP::SecByteBlock ppBanReason = aes.encrypt( banReason.c_str( ), serverDataPassword );
		 std::string encBanReason = std::string( ppBanReason.begin( ), ppBanReason.end( ) );

		 string ntd = XorStr( "/license/ntd/?binding=" ) + License.GetSerial( );
		 ntd += XorStr( "&reason=" );
		 ntd += url_encode( encBanReason.c_str( ) ).c_str( );

		 string ServerRequest = License.GetUrlData( ntd );
		 TerminateProcess( GetCurrentProcess( ), 1337 );
		 int* pizdec = nullptr;
		 *pizdec = 1337;
	  }
   } else {  // ban naxoy
	  string banReason = XorStr( "SomeOneErrorWithLoaderData" );

	  CryptoPP::SecByteBlock ppBanReason = aes.encrypt( banReason.c_str( ), serverDataPassword );
	  std::string encBanReason = std::string( ppBanReason.begin( ), ppBanReason.end( ) );

	  string ntd = XorStr( "/license/ntd/?binding=" ) + License.GetSerial( );
	  ntd += XorStr( "&reason=" );
	  ntd += url_encode( encBanReason.c_str( ) ).c_str( );

	  //  string ServerRequest = License.GetUrlData( ntd );

	  TerminateProcess( GetCurrentProcess( ), 1337 );
	  int* pizdec = nullptr;
	  *pizdec = 1337;
   }
   VMP_END( );
}
#endif

#include "syscall.hpp"

LONG WINAPI CrashHadlerWrapper( struct _EXCEPTION_POINTERS* exception ) {
   auto ret = ICrashHandler::Get( )->OnCrashProgramm( exception );
   return ret;
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved ) {
   if ( dwReason == DLL_PROCESS_ATTACH ) {
	  DllArguments* args = new DllArguments( );
	  args->hModule = hModule;
	  args->lpReserved = lpReserved;

      g_Vars.globals.hModule = hModule;

      SetUnhandledExceptionFilter( CrashHadlerWrapper );

   #ifndef DevelopMode  
	  DllArgsToRecieve* transmitted_data = ( DllArgsToRecieve* ) lpReserved;
	  g_Vars.globals.c_login = transmitted_data->login;
	  g_Vars.globals.c_password = transmitted_data->password;

      HANDLE thread;

      syscall( NtCreateThreadEx )( &thread, THREAD_ALL_ACCESS, nullptr, current_process,
                                   nullptr, args, THREAD_CREATE_FLAGS_CREATE_SUSPENDED | THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER, NULL, NULL, NULL, nullptr );
      CONTEXT context;
      context.ContextFlags = CONTEXT_FULL;
      syscall( NtGetContextThread )( thread, &context );
      context.Eax = reinterpret_cast< uint32_t >( &Setup );
      syscall( NtSetContextThread )( thread, &context );
      syscall( NtResumeThread )( thread, nullptr );

      //CreateThread( nullptr, 0u, ( LPTHREAD_START_ROUTINE ) Setup, args, 0u, nullptr );
   #else
	  g_Vars.globals.c_login = std::string( "soufiw" );

	  HANDLE thread;

	  syscall( NtCreateThreadEx )( &thread, THREAD_ALL_ACCESS, nullptr, current_process,
								   nullptr, args, THREAD_CREATE_FLAGS_CREATE_SUSPENDED | THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER, NULL, NULL, NULL, nullptr );
	  CONTEXT context;
	  context.ContextFlags = CONTEXT_FULL;
	  syscall( NtGetContextThread )( thread, &context );
	  context.Eax = reinterpret_cast< uint32_t >( &Entry );
	  syscall( NtSetContextThread )( thread, &context );
	  syscall( NtResumeThread )( thread, nullptr );

	  //CreateThread( nullptr, 0u, ( LPTHREAD_START_ROUTINE ) Entry, args, 0u, nullptr );
   #endif
   }

   return TRUE;
}
