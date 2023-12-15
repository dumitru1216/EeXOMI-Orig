#include <string>
class SignatureScanner;

enum SECURE_ERRS {
   SECURE_NO_ERR = 0,
   SECURE_X96,
   SECURE_HANDLED
};

typedef struct _CheckProcess {
   SignatureScanner* curSig;
   std::string m_szName;

} CheckProcess;

std::string url_encode( const std::string& value );

struct errRet {
   errRet( std::string& sz, SECURE_ERRS e ) : m_szName( sz ), err( e ) {

   }
   std::string m_szName;
   SECURE_ERRS err;
};

class CSecure {
public:
   CSecure( );
   void test( );
   bool CommonTest( );

private:
   SECURE_ERRS ProcessCheckForDebugger( CheckProcess& );
   void Terminate( );
   errRet CheckByName( std::string& szName );
   void BanDispatcher( errRet err );
   CheckProcess curProcess;
   bool ShouldBan = true;
};


extern CSecure g_secure;