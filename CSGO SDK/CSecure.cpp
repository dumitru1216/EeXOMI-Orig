#include "CSecure.hpp"
#include <Windows.h>
#include <TlHelp32.h>
#include "XorStr.hpp"
#include "License.hpp"
#include <cryptopp/aes.h>
#include "aes_encryption.hpp"
#include <mutex>
#include "lazy_importer.hpp"
#include "ExternalSigScaner.hpp"
//#define SECURE_OFF

vector<string> vecBlackProcessExe; 

CLicense sLicense;

CSecure::CSecure( ) {
   auto add_process = [] ( string szName ) {
	  return szName + ".exe";
   };

#define add(s)  add_process( XorStr( s ) )

   vecBlackProcessExe.push_back( add( "x64dbg" ) );
   vecBlackProcessExe.push_back( add( "x32dbg" ) );
   vecBlackProcessExe.push_back( add( "idaq" ) );
   vecBlackProcessExe.push_back( add( "ida" ) );
   vecBlackProcessExe.push_back( add( "ollydbg" ) );
   vecBlackProcessExe.push_back( add( "OLLYDBG" ) );
   vecBlackProcessExe.push_back( add( "windbg" ) );
   vecBlackProcessExe.push_back( add( "idaPlus" ) );
   vecBlackProcessExe.push_back( add( "wireshark" ) );
   vecBlackProcessExe.push_back( add( "lordpe" ) );
   vecBlackProcessExe.push_back( add( "hookshark" ) );
   vecBlackProcessExe.push_back( add( "idag" ) );
   vecBlackProcessExe.push_back( add( "MPGH Virus Scan Tool v6" ) );
   vecBlackProcessExe.push_back( add( "procexp" ) );
   vecBlackProcessExe.push_back( add( "procmon" ) );
   vecBlackProcessExe.push_back( add( "filemon" ) );
   vecBlackProcessExe.push_back( add( "ollyice" ) );
   vecBlackProcessExe.push_back( add( "processspy" ) );
   vecBlackProcessExe.push_back( add( "spyxx" ) );
   vecBlackProcessExe.push_back( add( "cv" ) );
   //vecBlackProcessExe.push_back( add( "ProcessHacker" ) );
   vecBlackProcessExe.push_back( add( "tcpview" ) );
   vecBlackProcessExe.push_back( add( "autoruns" ) );
   vecBlackProcessExe.push_back( add( "autorunsc" ) );
   vecBlackProcessExe.push_back( add( "ImmunityDebugger" ) );
   vecBlackProcessExe.push_back( add( "idaq64" ) );
   vecBlackProcessExe.push_back( add( "dumpcap" ) );
   vecBlackProcessExe.push_back( add( "HookExplorer" ) );
   vecBlackProcessExe.push_back( add( "ImportREC" ) );
   vecBlackProcessExe.push_back( add( "PETools" ) );
   vecBlackProcessExe.push_back( add( "LordPE" ) );
   vecBlackProcessExe.push_back( add( "SysInspector" ) );
   vecBlackProcessExe.push_back( add( "proc_analyzer" ) );
   vecBlackProcessExe.push_back( add( "sysAnalyzer" ) );
   vecBlackProcessExe.push_back( add( "sniff_hit" ) );
   vecBlackProcessExe.push_back( add( "joeboxserver" ) );
   vecBlackProcessExe.push_back( add( "joeboxcontrol" ) );
}

void TestThread( ) {
#ifndef SECURE_OFF
   while ( 1 ) {
	  g_secure.CommonTest( );
	  std::this_thread::sleep_for( 3s );
   }
#endif
}

void CSecure::test( ) {
   this->ShouldBan = true;
   thread ther( TestThread );
   ther.detach( );
}

mutex Commonmtx;

bool CSecure::CommonTest( ) {
#ifndef SECURE_OFF
   Commonmtx.lock( );
   //  return;

   auto h_snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
   if ( h_snapshot == INVALID_HANDLE_VALUE ) this->Terminate( );

   PROCESSENTRY32 pe;
   pe.dwSize = sizeof( pe );

   if ( Process32First( h_snapshot, &pe ) ) {
	  while ( Process32Next( h_snapshot, &pe ) ) {
		 auto h_process = OpenProcess( PROCESS_ALL_ACCESS, 0, pe.th32ProcessID );
		 if ( h_process == 0 || h_process == INVALID_HANDLE_VALUE ) goto ret;
		 {
			CheckProcess checkProc = CheckProcess( );
			SignatureScanner scanner = SignatureScanner( );
			if ( !scanner.GetProcess( pe.th32ProcessID ) ) this->Terminate( );

			checkProc.curSig = &scanner;
			checkProc.m_szName = pe.szExeFile;

			if ( this->ProcessCheckForDebugger( checkProc ) == SECURE_HANDLED ) {
			   CloseHandle( h_snapshot );
			   Commonmtx.unlock( );
			   return true;
			}
		 }
	  ret:
		 CloseHandle( h_process );
	  }
   }
full_ret:
   CloseHandle( h_snapshot );
   Commonmtx.unlock( );
   return false;
#endif  
}

int UnloadTryCnt = 0;

SECURE_ERRS CSecure::ProcessCheckForDebugger( CheckProcess& curProc ) {
   auto [errName, status] = CheckByName( curProc.m_szName );

   if ( status != SECURE_ERRS::SECURE_NO_ERR ) {
   #ifndef SECURE_OFF
	  if ( this->ShouldBan ) {
		 this->BanDispatcher( errRet( errName, status ) );
	  } else {
		 UnloadTryCnt++;
		 if ( UnloadTryCnt > 3 ) this->BanDispatcher( errRet( errName, status ) );
		 LI_FN( MessageBoxA ).forwarded_safe_cached( )( 0, ( string( XorStr( "Unload " ) ) + errName ).c_str( ), XorStr( "Error" ), MB_OK | MB_ICONSTOP );
		 return SECURE_HANDLED;
	  }
   #else
	  printf( "Detected: %s\n", errName.c_str( ) );
   #endif
   }
}

void CSecure::Terminate( ) {
   //    ExitProcess( 1337 );
}

errRet CSecure::CheckByName( string& szName ) {
   for ( auto& sz : vecBlackProcessExe ) {
	  if ( szName == sz ) {
		 return errRet( szName, SECURE_ERRS::SECURE_X96 );
	  }
   }

   return errRet( std::string( "ne" ), SECURE_ERRS::SECURE_NO_ERR );
}

[[noreturn]] void CSecure::BanDispatcher( errRet err ) {
#if 1
   std::string serverDataPassword = XorStr( "X828Gw2JbGkBBbbdum8MKV@YVZSjHqg1#pErAXGlBH352SPZ1b" );
   AesEncryption aes( XorStr( "cbc" ), 256 );

   string banReason = err.m_szName;

   CryptoPP::SecByteBlock ppBanReason = aes.encrypt( banReason.c_str( ), serverDataPassword );
   std::string encBanReason = std::string( ppBanReason.begin( ), ppBanReason.end( ) );

   string ntd = XorStr( "/license/ntd/?binding=" ) + sLicense.GetSerial( );
   ntd += XorStr( "&reason=" );
   ntd += url_encode( encBanReason.c_str( ) ).c_str( );

   string ServerRequest = sLicense.GetUrlData( ntd );
#else
   printf( "Ban %s\n", err.m_szName );
#endif
   ExitProcess( 1337 );
}

CSecure g_secure;