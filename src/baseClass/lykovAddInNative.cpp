#include <baseClass/lykovAddInNative.hpp>

const WCHAR_T* GetClassNames() {
#if ENABLE_BASE_FUNCTION
	return gl_ConvToShortWchar(nullptr, L"BaseFunction");
#elif ENABLE_HTTP_CLIENT
	return gl_ConvToShortWchar(nullptr, L"HttpClient");
#elif ENABLE_POSTGRE_SQL
	return gl_ConvToShortWchar(nullptr, L"PostgreSQL");
#elif ENABLE_MY_SQL
	return gl_ConvToShortWchar(nullptr, L"MySQLConn");
#else
	return gl_ConvToShortWchar(nullptr, L"");
#endif
}

long GetClassObject(const WCHAR_T* exceptionName, IComponentBase** pInterface) {
	if (*pInterface)
		return 0;
	
	const auto tmp = gl_ConvFromShortWchar(exceptionName);
#if ENABLE_BASE_FUNCTION
	if (gl_IEqualsCaseInsensitive(L"BaseFunction", tmp))
		*pInterface = new BaseFunction();
#elif ENABLE_HTTP_CLIENT
	if (gl_IEqualsCaseInsensitive(L"HttpClient", tmp))
		*pInterface = new HttpClient();
#elif ENABLE_POSTGRE_SQL
	if (gl_IEqualsCaseInsensitive(L"PostgreSQL", tmp))
		*pInterface = new PostgreSQL();
#elif ENABLE_MY_SQL
	if (gl_IEqualsCaseInsensitive(L"MySQLConn", tmp))
		*pInterface = new MySqlConn();
#endif
	
	delete[] tmp;
	return *pInterface == nullptr ? 0 : reinterpret_cast<long>(*pInterface);
}

AttachType GetAttachType() {
	return eCanAttachAny;
}

long DestroyObject(IComponentBase** pInterface) {
	if (!*pInterface) {
		return -1;
	}

	delete*pInterface;
	*pInterface = nullptr;
	return 0;
}

AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities) {
	gl_Capabilities = capabilities;
	return eAppCapabilitiesLast;
}
