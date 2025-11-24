
#include "baseClass/from1C/ComponentBase.h"

#if ENABLE_BASE_FUNCTION
	#include "extensions/extensionBaseFunctions.hpp"
#elif ENABLE_HTTP_CLIENT
	#include "extensions/HTTPClient.hpp"
#elif ENABLE_POSTGRE_SQL
	#include "extensions/postgreSQL.hpp"
#elif ENABLE_PDF_EDITOR
	#include "extensions/pdfEditor.hpp"
#endif

#ifdef _WINDOWS

	#pragma warning (disable : 4311) // reinterpret_cast: усечение указателя из "IComponentBase *" в "long", требуется для GetClassObject
	#define GET_CLASS_NAME(src) L##src

#else
	
	#define GET_CLASS_NAME(src) u##src
	
#endif

AppCapabilities gl_capabilities = eAppCapabilitiesInvalid;

const WCHAR_T* GetClassNames() {
#if ENABLE_BASE_FUNCTION
	return GET_CLASS_NAME("BaseFunctions");
#elif ENABLE_HTTP_CLIENT
	return GET_CLASS_NAME("HttpClient");
#elif ENABLE_POSTGRE_SQL
	return GET_CLASS_NAME("PostgreSQL");
#elif ENABLE_PDF_EDITOR
	return GET_CLASS_NAME("PdfEditor");
#else
	return GET_CLASS_NAME("");
#endif
}

long GetClassObject(const WCHAR_T* exceptionName, IComponentBase** pIntf) {
	if (*pIntf)
		return 0;

	// Обязательно до сравнения строк
	std::setlocale(LC_ALL, "ru_RU.UTF-8"); // NOLINT(cert-err33-c, concurrency-mt-unsafe)
	std::setlocale(LC_NUMERIC, "C"); // Русская локализация читает числа с запятой, нам этого не надо NOLINT(cert-err33-c, concurrency-mt-unsafe)

#if ENABLE_BASE_FUNCTION
	if (gl_str_iequal(exceptionName, GET_CLASS_NAME("BaseFunctions")))
		*pIntf = new BaseFunctions();
#elif ENABLE_HTTP_CLIENT
	if (gl_str_iequal(exceptionName, GET_CLASS_NAME("HttpClient")))
		*pIntf = new HttpClient();
#elif ENABLE_POSTGRE_SQL
	if (gl_str_iequal(exceptionName, GET_CLASS_NAME("PostgreSQL")))
		*pIntf = new PostgreSQL();
#elif ENABLE_PDF_EDITOR
	if (gl_str_iequal(exceptionName, GET_CLASS_NAME("PdfEditor")))
		*pIntf = new PDFEditor();
#endif
	
	return *pIntf == nullptr ? 0 : reinterpret_cast<long>(*pIntf);
}

AttachType GetAttachType() {
	return eCanAttachAny;
}

long DestroyObject(IComponentBase** pIntf) {
	if (!*pIntf)
		return -1;

	delete *pIntf;
	*pIntf = nullptr;

	return 0;
}

AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities) {
	gl_capabilities = capabilities;
	return eAppCapabilitiesLast;
}
