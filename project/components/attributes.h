#pragma region Attributes
#if defined(GEN_SYSTEM_WINDOWS) || defined( __CYGWIN__ )
#	define GEN_API_
#	define GEN_API_Export_Code   __declspec(dllexport)
#	define GEN_API_Import_Code   __declspec(dllimport)
#	define GEN_Attribute_Keyword __declspec

	#define API_Export stringize( GEN_API_Export_Code  );
	#define API_Import stringize( GEN_API_Import_Code  );
	#define Keyword    stringize( GEN_Attribute_Keyword);

#elif GEN_HAS_ATTRIBUTE( visibility ) || GEN_GCC_VERSION_CHECK( 3, 3, 0 ) || GEN_INTEL_VERSION_CHECK( 13, 0, 0 )
#	define GEN_API_Export_Code   __attribute__ ((visibility ("default")))
#	define GEN_API_Import_Code   __attribute__ ((visibility ("default")))
#	define GEN_Attribute_Keyword __attribute__

	#define API_Export stringize( GEN_API_Export_Code );
	#define API_Import stringize( GEN_API_Import_Code );
	#define Keyword    stringize( GEN_Attribute_Keyword);

#else
#	define GEN_API_Export_Code
#	define GEN_API_Import_Code
#	define GEN_Attribute_Keyword

	#define API_Export ""
	#define API_Import ""
	#define Keyword    ""
#endif
#pragma endregion Attributes
