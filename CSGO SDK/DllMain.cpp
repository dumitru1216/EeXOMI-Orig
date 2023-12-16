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
	AllocConsole( );
	freopen_s( ( FILE** )stdin, XorStr( "CONIN$" ), XorStr( "r" ), stdin );
	freopen_s( ( FILE** )stdout, XorStr( "CONOUT$" ), XorStr( "w" ), stdout );
#ifdef DevelopMode
   SetConsoleTitleA( XorStr( "dev" ) );
#elif
	SetConsoleTitleA( XorStr( "release" ) );
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
   }

   return TRUE;
}
