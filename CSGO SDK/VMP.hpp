#ifndef VMPROTECT
#define VMPROTECT

#ifndef DevelopMode
#include <VMProtectSDK.h>
#endif

#define VMP_STRINGIFY_(x) #x
#define VMP_STRINGIFY(x) VMP_STRINGIFY_(x)
#define VMP_DEFAULT_LABEL __FILE__ ":" __FUNCTION__ "(" VMP_STRINGIFY(__LINE__) ")"

#ifndef DevelopMode
#define VMP_BEGIN() VMProtectBegin(VMP_DEFAULT_LABEL)
#define VMP_BEGIN_VIRTUALIZATION() VMProtectBeginVirtualization(VMP_DEFAULT_LABEL)
#define VMP_BEGIN_MUTATION() VMProtectBeginMutation(VMP_DEFAULT_LABEL)
#define VMP_BEGIN_ULTRA() VMProtectBeginUltra(VMP_DEFAULT_LABEL)
#define VMP_BEGIN_VRTIUALIZATION_LOCK_BY_KEY() VMProtectBeginVirtualizationLockByKey(VMP_DEFAULT_LABEL)
#define VMP_BEGIN_ULTRA_LOCK_BY_KEY() VMProtectBeginUltraLockByKey(VMP_DEFAULT_LABEL)
#define VMP_END() VMProtectEnd()
#else
#define VMP_BEGIN() 0
#define VMP_BEGIN_VIRTUALIZATION() 0
#define VMP_BEGIN_MUTATION() 0
#define VMP_BEGIN_ULTRA() 0
#define VMP_BEGIN_VRTIUALIZATION_LOCK_BY_KEY() 0
#define VMP_BEGIN_ULTRA_LOCK_BY_KEY() 0
#define VMP_END() 0
#endif

#ifdef _MSC_VER
#if (_MSC_VER >= 1200)
#define VMP_FORCEINLINE __forceinline
#else
#define VMP_FORCEINLINE __inline
#endif
#elif defined(__GNUC__)
#define VMP_FORCEINLINE inline __attribute__((__always_inline__))
#elif defined(__CLANG__)
#if __has_attribute(__always_inline__)
#define VMP_FORCEINLINE inline __attribute__((__always_inline__))
#else
#define VMP_FORCEINLINE inline
#endif
#else
#define VMP_FORCEINLINE inline
#endif

#ifndef DevelopMode
namespace vmp
{

   class string {
	  const char* _value;

   public:
	  VMP_FORCEINLINE string( const char* value ) noexcept
		 : _value( VMProtectDecryptStringA( value ) ) {
	  }

	  ~string( ) noexcept {
		 VMProtectFreeString( _value );
	  }

	  operator const char* ( ) const noexcept {
		 return _value;
	  }
   };

   class wstring {
	  const wchar_t* _value;

   public:
	  VMP_FORCEINLINE wstring( const wchar_t* value ) noexcept
		 : _value( VMProtectDecryptStringW( value ) ) {
	  }

	  ~wstring( ) noexcept {
		 VMProtectFreeString( _value );
	  }

	  operator const wchar_t* ( ) const noexcept {
		 return _value;
	  }
   };


   namespace literals
   {
	  static VMP_FORCEINLINE auto operator"" _vmp( const char* value, size_t size ) noexcept {
	  #ifdef VMPROTECT
		 return vmp::string( value );
	  #else
		 return value;
	  #endif
	  }

	  static VMP_FORCEINLINE auto operator"" _vmp( const wchar_t* value, size_t size ) noexcept {
	  #ifdef VMPROTECT
		 return vmp::wstring( value );
	  #else
		 return value;
	  #endif
	  }
   }
}
#endif
#endif