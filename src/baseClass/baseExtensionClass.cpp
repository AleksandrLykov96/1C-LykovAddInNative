#include <baseClass/baseExtensionClass.hpp>

#pragma region Интерфейс компоненты

IBaseExtensionClass::IBaseExtensionClass() {
	m_IMemory        = nullptr;
	m_IConnect       = nullptr;
}

bool IBaseExtensionClass::Init(void* pConnection) {
	m_IConnect = static_cast<IAddInDefBase*>(pConnection);
	if (m_IConnect != nullptr) {
		const auto iPlatformInfo = static_cast<IPlatformInfo*>(getInterface(eIPlatformInfo));
		assert(iPlatformInfo && L"Не удалось получить информацию по платформе");

		const auto platformInfo = iPlatformInfo->GetPlatformInfo();
		m_ItsServer             = !(platformInfo->Application == IPlatformInfo::eAppThickClient
			|| platformInfo->Application == IPlatformInfo::eAppThinClient
			|| platformInfo->Application == IPlatformInfo::eAppMobileClient);

		if (!m_ItsServer)
			m_IConnect->SetEventBufferDepth(8l);

		itsIsolate = getAttachedInfo() == IAttachedInfo::AttachedType::eAttachedIsolated;
	}

	m_PropNames[gl_Index_Prop_Last_Error]      = NamesFor1C(L"ТекстПоследнейОшибки", L"LastError");
	m_PropNames[gl_Index_Prop_Event_Buffer]    = NamesFor1C(L"РазмерОчередиСобытий", L"EventBufferDepth");
	m_PropNames[gl_Index_Prop_Attach_Isolated] = NamesFor1C(L"ПодключениеИзолированно", L"AttachIsolated");
	m_PropNames[gl_Index_Prop_App_Version]     = NamesFor1C(L"ВерсияПриложения", L"AppVersion");
	m_PropNames[gl_Index_Prop_App_Type]        = NamesFor1C(L"ТипПриложения", L"AppType");

	setMethodPropsExtension();

	assert(m_IConnect && L"Не удалось подключить платформу");
	return true;
}

bool IBaseExtensionClass::setMemManager(void* mem) {
	m_IMemory = static_cast<IMemoryManager*>(mem);
	assert(m_IMemory && L"Не удалось подключить менеджер памяти");

	return true;
}

long IBaseExtensionClass::GetInfo() {
	// Component should put supported component technology version 
	// This component supports 2.0 version
	return 2000;
}

void IBaseExtensionClass::Done() {
}

bool IBaseExtensionClass::RegisterExtensionAs(WCHAR_T** wsExtensionName) {
	const auto realExtensionName = getNameExtension();
	if (const auto iActualSize = static_cast<uint32_t>(wcslen(realExtensionName) + 1);
		m_IMemory && m_IMemory->AllocMemory(reinterpret_cast<void**>(wsExtensionName), static_cast<unsigned>(iActualSize) * sizeof(WCHAR_T))) {

		gl_ConvToShortWchar(wsExtensionName, realExtensionName, iActualSize);
		return true;
	}

	assert(m_IMemory && L"Не удалось зарегистрировать компоненту");
	return false;
}

long IBaseExtensionClass::GetNProps() {
	return static_cast<long>(m_PropNames.size());
}

long IBaseExtensionClass::FindProp(const WCHAR_T* wsPropName) {
	return findNameFromList(m_PropNames, wsPropName);
}

const WCHAR_T* IBaseExtensionClass::GetPropName(const long lPropNum, const long lPropAlias) {
	const wchar_t* foundedName = findIndexFromList(m_PropNames, static_cast<unsigned short>(lPropNum));
	const auto iActualSize     = static_cast<uint32_t>(wcslen(foundedName) + 1);
	if (iActualSize == 1)
		return nullptr;

	WCHAR_T* wsPropName = nullptr;
	if (!m_IMemory->AllocMemory(reinterpret_cast<void**>(&wsPropName), static_cast<unsigned>(iActualSize) * sizeof(WCHAR_T)))
		throw LykovException(L"Не удалось выделить память для WCHAR_T*", L"GetPropName");

	return gl_ConvToShortWchar(&wsPropName, foundedName, iActualSize);
}

bool IBaseExtensionClass::GetPropVal(const long lPropNum, tVariant* pvarPropVal) {
	switch (const auto num = static_cast<unsigned short>(lPropNum)) {
		case gl_Index_Prop_Last_Error:
			setReturnedParam(m_LastError, pvarPropVal);
			return true;
		case gl_Index_Prop_Event_Buffer: {
			setReturnedParam(m_ItsServer ? 0l : m_IConnect->GetEventBufferDepth(), pvarPropVal);
			return true;
		}
		case gl_Index_Prop_Attach_Isolated:
			setReturnedParam(getAttachedInfo() == IAttachedInfo::AttachedType::eAttachedIsolated, pvarPropVal);
			return true;
		case gl_Index_Prop_App_Version: {
			const auto info = getPlatformInfo();
			if (!info) {
				setReturnedParam(L"Не удалось получить информацию по платформе", pvarPropVal);
				return true;
			}

			const wchar_t* tmp = gl_ConvFromShortWchar(info->AppVersion);
			setReturnedParam<const wchar_t*>(tmp, pvarPropVal);
			delete[] tmp;

			return true;
		}
		case gl_Index_Prop_App_Type: {
			if (const auto info = getPlatformInfo(); !info)
				setReturnedParam(L"Не удалось получить информацию по платформе", pvarPropVal);
			else {
				switch (info->Application) {
					case IPlatformInfo::eAppThinClient:
						setReturnedParam(L"Тонкий клиент", pvarPropVal);
						break;
					case IPlatformInfo::eAppThickClient:
						setReturnedParam(L"Толстый клиент", pvarPropVal);
						break;
					case IPlatformInfo::eAppWebClient:
						setReturnedParam(L"Веб-клиент", pvarPropVal);
						break;
					case IPlatformInfo::eAppServer:
						setReturnedParam(L"Сервер", pvarPropVal);
						break;
					case IPlatformInfo::eAppExtConn:
						setReturnedParam(L"Внешнее соединение", pvarPropVal);
						break;
					case IPlatformInfo::eAppMobileClient:
						setReturnedParam(L"Мобильный клиент", pvarPropVal);
						break;
					case IPlatformInfo::eAppMobileServer:
						setReturnedParam(L"Мобильный сервер", pvarPropVal);
						break;
					default:
						setReturnedParam(L"Неизвестный тип приложения", pvarPropVal);
						break;
				}
			}

			return true;
		}
		default:
			try {
				getPropByIndex(num, pvarPropVal);
				return true;
			} catch (const LykovException& message) {
				addError(message);
			} catch (const std::runtime_error& message) {
				const auto source = boost::str(boost::wformat(L"Ошибка при получении параметра '%s' <runtime_error>") % findIndexFromList(m_PropNames, num));
				addError(LykovException(message.what(), source.c_str()));
			} catch (const std::exception& message) {
				const auto source = boost::str(boost::wformat(L"Ошибка при получении параметра '%s' <exception>") % findIndexFromList(m_PropNames, num));
				addError(LykovException(message.what(), source.c_str()));
			} catch (...) {
				const auto source = boost::str(boost::wformat(L"Ошибка при получении параметра '%s' <...>") % findIndexFromList(m_PropNames, num));
				addError(LykovException(L"Неизвестная ошибка", source.c_str()));
			}
	}

	TV_VT(pvarPropVal) = VTYPE_EMPTY;
	return true;
}

bool IBaseExtensionClass::SetPropVal(const long lPropNum, tVariant* varPropVal) {
	m_LastError = L"";
	const auto num = static_cast<unsigned short>(lPropNum);
	try {
		if (num == gl_Index_Prop_Event_Buffer)
			m_IConnect->SetEventBufferDepth(getInputParam<unsigned int>(varPropVal, -1));
		else
			setPropByIndex(num, varPropVal);

		return true;
	} catch (const LykovException& message) {
		addError(message);
	} catch (const std::runtime_error& message) {
		const auto source = boost::str(boost::wformat(L"Ошибка при установке параметра '%s' <runtime_error>") % findIndexFromList(m_PropNames, num));
		addError(LykovException(message.what(), source.c_str()));
	} catch (const std::exception& message) {
		const auto source = boost::str(boost::wformat(L"Ошибка при установке параметра '%s' <exception>") % findIndexFromList(m_PropNames, num));
		addError(LykovException(message.what(), source.c_str()));
	} catch (...) {
		const auto source = boost::str(boost::wformat(L"Ошибка при установке параметра '%s' <...>") % findIndexFromList(m_PropNames, num));
		addError(LykovException(L"Неизвестная ошибка", source.c_str()));
	}

	return false;
}

bool IBaseExtensionClass::IsPropReadable(const long lPropNum) {
	return true;
}

bool IBaseExtensionClass::IsPropWritable(const long lPropNum) {
	switch (const auto num = static_cast<unsigned short>(lPropNum)) {
		case gl_Index_Prop_Last_Error:
		case gl_Index_Prop_Attach_Isolated:
		case gl_Index_Prop_App_Version:
		case gl_Index_Prop_App_Type:
			return false;
		case gl_Index_Prop_Event_Buffer:
			return !m_ItsServer;
		default:
			return getIsPropWritable(num);
	}
}

long IBaseExtensionClass::GetNMethods() {
	return static_cast<long>(m_MethodNames.size());
}

long IBaseExtensionClass::FindMethod(const WCHAR_T* wsPropName) {
	return findNameFromList(m_MethodNames, wsPropName);
}

const WCHAR_T* IBaseExtensionClass::GetMethodName(const long lPropNum, const long lPropAlias) {
	const wchar_t* foundedName = findIndexFromList(m_MethodNames, static_cast<unsigned short>(lPropNum));
	const auto iActualSize     = static_cast<uint32_t>(wcslen(foundedName) + 1);

	if (iActualSize == 1)
		return nullptr;

	WCHAR_T* wsMethodName = nullptr;
	if (!m_IMemory->AllocMemory(reinterpret_cast<void**>(&wsMethodName), static_cast<unsigned>(iActualSize) * sizeof(WCHAR_T)))
		throw LykovException(L"Не удалось выделить память для WCHAR_T*", L"GetMethodName");

	return gl_ConvToShortWchar(&wsMethodName, foundedName, iActualSize);
}

long IBaseExtensionClass::GetNParams(const long lMethodNum) {
	return getMethodNParams(static_cast<unsigned short>(lMethodNum));
}

bool IBaseExtensionClass::GetParamDefValue(const long lMethodNum, const long lParamNum, tVariant* pvarParamDefValue) {
	setParamDefValue(static_cast<unsigned short>(lMethodNum), lParamNum, pvarParamDefValue);
	return true;
}

bool IBaseExtensionClass::HasRetVal(const long lMethodNum) {
	return getHasRetVal(static_cast<unsigned short>(lMethodNum));
}

bool IBaseExtensionClass::CallAsProc(const long lMethodNum, tVariant* paParams, const long lSizeArray) {
	m_LastError = L"";

	const auto num = static_cast<unsigned short>(lMethodNum);
	try {
		callMethodAsProc(num, paParams, lSizeArray);
		return true;
	} catch (const LykovException& message) {
		addError(message);
	} catch (const std::runtime_error& message) {
		const auto source = boost::str(boost::wformat(L"Ошибка при вызове процедуры '%s' <runtime_error>") % findIndexFromList(m_MethodNames, num));
		addError(LykovException(message.what(), source.c_str()));
	} catch (const std::exception& message) {
		const auto source = boost::str(boost::wformat(L"Ошибка при вызове процедуры '%s' <exception>") % findIndexFromList(m_MethodNames, num));
		addError(LykovException(message.what(), source.c_str()));
	} catch (...) {
		const auto source = boost::str(boost::wformat(L"Ошибка при вызове процедуры '%s' <...>") % findIndexFromList(m_MethodNames, num));
		addError(LykovException(L"Неизвестная ошибка", source.c_str()));
	}

	return true;
}

bool IBaseExtensionClass::CallAsFunc(const long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
	m_LastError = L"";

	const auto num = static_cast<unsigned short>(lMethodNum);
	try {
		callMethodAsFunc(num, pvarRetValue, paParams, lSizeArray);
	} catch (const LykovException& message) {
		addError(message);
	} catch (const std::runtime_error& message) {
		const auto source = boost::str(boost::wformat(L"Ошибка при вызове функции '%s' <runtime_error>") % findIndexFromList(m_MethodNames, num));
		addError(LykovException(message.what(), source.c_str()));
	} catch (const std::exception& message) {
		const auto source = boost::str(boost::wformat(L"Ошибка при вызове функции '%s' <exception>") % findIndexFromList(m_MethodNames, num));
		addError(LykovException(message.what(), source.c_str()));
	} catch (...) {
		const auto source = boost::str(boost::wformat(L"Ошибка при вызове функции '%s' <...>") % findIndexFromList(m_MethodNames, num));
		addError(LykovException(L"Неизвестная ошибка", source.c_str()));
	}
	return true;
}

void IBaseExtensionClass::SetLocale(const WCHAR_T* loc) {
	// Принудительно русский язык (1С передаёт какую-то херню)
	std::setlocale(LC_ALL, "ru_RU.UTF-8");
	std::setlocale(LC_NUMERIC, "C"); // Русская локализация читает числа с запятой, нам этого не надо.
}

void IBaseExtensionClass::SetUserInterfaceLanguageCode(const WCHAR_T* loc) {
	SetLocale(loc);
}

#pragma endregion

#pragma region ИнтерфейсПредприятия

void IBaseExtensionClass::addError(const LykovException& exp) {
	if (!m_ItsServer && m_IConnect) {
#ifdef __linux__
		const auto err = gl_ConvToShortWchar(nullptr, exp.getSource());
		const auto descr = gl_ConvToShortWchar(nullptr, exp.getDescription());

		m_IConnect->AddError(exp.getWCode(), err, descr, exp.getCodeError());

		delete[] err;
		delete[] descr;
#else
		m_IConnect->AddError(exp.getWCode(), exp.getSource(), exp.getDescription(), exp.getCodeError());
#endif
	}

	const std::wstring res = exp.whatW();
	if (wcslen(m_LastError) == 0)
		m_LastError = gl_CopyWStringToWChar(res);
	else {
		auto tmpError = std::wstring(m_LastError);
		tmpError.append(L"\n");
		tmpError.append(res);

		m_LastError = gl_CopyWStringToWChar(tmpError);
	}
}

bool IBaseExtensionClass::registerProfileAs(const wchar_t* profileName) const {
	// Только на клиенте
	if (m_ItsServer || !m_IConnect)
		return false;

	const auto wsProfileName = gl_ConvToShortWchar(nullptr, profileName);
	const auto result        = m_IConnect->RegisterProfileAs(wsProfileName);
	delete[] wsProfileName;

	return result;
}

bool IBaseExtensionClass::readFromCache(const wchar_t* propName, tVariant* pVal) {
	// Только на клиенте
	if (m_ItsServer || !m_IConnect)
		return false;

	long errorCode            = -1;
	WCHAR_T* errorDescription = nullptr;
	const auto wpropName      = gl_ConvToShortWchar(nullptr, propName);

	const auto res = m_IConnect->Read(wpropName, pVal, &errorCode, &errorDescription);
	if (!res)
		addError(LykovException(L"Неизвестная ошибка", L"readFromCache", ADDIN_E_MSGBOX_INFO, errorCode));

	m_IMemory->FreeMemory(reinterpret_cast<void**>(errorDescription));
	return res;
}

bool IBaseExtensionClass::writeToCache(const wchar_t* propName, tVariant* pVal) const {
	// Только на клиенте
	if (m_ItsServer || !m_IConnect)
		return false;

	const auto wpropName = gl_ConvToShortWchar(nullptr, propName);
	const auto res       = m_IConnect->Write(wpropName, pVal);

	delete[] wpropName;
	return res;
}

bool IBaseExtensionClass::externalEvent(const wchar_t* source, const wchar_t* message, const wchar_t* data) const {
	// Только на клиенте
	if (m_ItsServer || !m_IConnect)
		return false;

	const auto wsource  = gl_ConvToShortWchar(nullptr, source);
	const auto wmessage = gl_ConvToShortWchar(nullptr, message);
	const auto wdata    = gl_ConvToShortWchar(nullptr, data);

	const auto res = m_IConnect->ExternalEvent(wsource, wmessage, wdata);

	delete[] wsource;
	delete[] wmessage;
	delete[] wdata;

	return res;
}

void IBaseExtensionClass::cleanEventBuffer() const {
	// Только на клиенте
	if (!m_ItsServer && m_IConnect)
		m_IConnect->CleanEventBuffer();
}

bool IBaseExtensionClass::setStatusLine(const wchar_t* message) const {
	// Только на клиенте
	if (m_ItsServer || !m_IConnect)
		return false;

	const auto wmessage = gl_ConvToShortWchar(nullptr, message);
	const auto res      = m_IConnect->SetStatusLine(wmessage);

	delete[] wmessage;
	return res;
}

void IBaseExtensionClass::resetStatusLine() const {
	// Только на клиенте
	if (!m_ItsServer && m_IConnect)
		m_IConnect->ResetStatusLine();
}

IInterface* IBaseExtensionClass::getInterface(const Interfaces iface) const {
	if (!m_IConnect)
		return nullptr;

	const auto cnn = static_cast<IAddInDefBaseEx*>(m_IConnect); // Оставить static_cast!!!
	return !cnn ? nullptr : cnn->GetInterface(iface);
}

bool IBaseExtensionClass::showQuestions(const wchar_t* message, tVariant* pVal) const {
	// Только на клиенте
	if (m_ItsServer || !m_IConnect)
		return false;

	const auto imsgbox = static_cast<IMsgBox*>(getInterface(eIMsgBox)); // Оставить static_cast!!!
	if (!imsgbox)
		return false;

#ifdef __linux__
	WCHAR_T* wmessage = gl_ConvToShortWchar(nullptr, message);
	imsgbox->Confirm(wmessage, pVal);
	delete[] wmessage;
#else
	imsgbox->Confirm(message, pVal);
#endif

	return true;
}

bool IBaseExtensionClass::showMessage(const wchar_t* message) const {
	// Только на клиенте
	if (m_ItsServer || !m_IConnect)
		return false;

	const auto imsgbox = static_cast<IMsgBox*>(getInterface(eIMsgBox));
	if (!imsgbox)
		return false;

#ifdef __linux__
	WCHAR_T* wmessage = gl_ConvToShortWchar(nullptr, message);
	imsgbox->Alert(wmessage);
	delete[] wmessage;
#else
	imsgbox->Alert(message);
#endif

	return true;
}

const IPlatformInfo::AppInfo* IBaseExtensionClass::getPlatformInfo() const {
	if (!m_IConnect)
		return nullptr;

	const auto info = static_cast<IPlatformInfo*>(getInterface(eIPlatformInfo));
	return info->GetPlatformInfo();
}

IAttachedInfo::AttachedType IBaseExtensionClass::getAttachedInfo() const {
	if (!m_IConnect)
		return IAttachedInfo::AttachedType::eAttachedNotIsolated;

	const auto cnn = static_cast<IAddInDefBaseEx*>(m_IConnect);
	if (!cnn)
		return IAttachedInfo::AttachedType::eAttachedNotIsolated;

	const auto con_Info = static_cast<IAttachedInfo*>(cnn->GetInterface(eIAttachedInfo));
	return !con_Info ? IAttachedInfo::AttachedType::eAttachedNotIsolated : con_Info->GetAttachedInfo();
}

#pragma endregion

#pragma region ВспомогательныеФункции

IBaseExtensionClass::NamesFor1C::NamesFor1C(const wchar_t* nameRu, const wchar_t* name) {
	this->nameRu = nameRu;
	this->name   = name;
}

// Получение входящих параметров
template <>
bool IBaseExtensionClass::getInputParam<bool>(const tVariant* paParams, const long indexParam) const {
	const auto param = indexParam == -1 ? paParams : &paParams[indexParam];
	if (TV_VT(param) == VTYPE_BOOL)
		return TV_BOOL(param);

	getInputParam_Exception(indexParam, L"bool", TV_VT(param));
	return false;
}

template <>
std::wstring IBaseExtensionClass::getInputParam<std::wstring>(const tVariant* paParams, const long indexParam) const {
	const auto param = indexParam == -1 ? paParams : &paParams[indexParam];
	switch (TV_VT(param)) {
		case VTYPE_PSTR:
			return gl_StringToWString(TV_STR(param));
		case VTYPE_PWSTR: {
			const wchar_t* strChar = gl_ConvFromShortWchar(TV_WSTR(param), param->wstrLen + 1);
			const auto res         = std::wstring(strChar, param->wstrLen);
			delete[] strChar;
			return res;
		}
		default:
			break;
	}

	getInputParam_Exception(indexParam, L"wstring", TV_VT(param));
	return L"";
}

template <>
const wchar_t* IBaseExtensionClass::getInputParam<const wchar_t*>(const tVariant* paParams, const long indexParam) const {
	const auto param = indexParam == -1 ? paParams : &paParams[indexParam];
	switch (TV_VT(param)) {
		case VTYPE_PSTR: {
			const auto result = gl_StringToWString(TV_STR(param));
			return gl_CopyWStringToWChar(result, param->strLen);
		}
		case VTYPE_PWSTR:
			return gl_ConvFromShortWchar(TV_WSTR(param), param->wstrLen + 1);
		default:
			break;
	}

	getInputParam_Exception(indexParam, L"const wchar_t*", TV_VT(param));
	return L"";
}

template <>
std::string IBaseExtensionClass::getInputParam<std::string>(const tVariant* paParams, const long indexParam) const {
	const auto param = indexParam == -1 ? paParams : &paParams[indexParam];
	switch (TV_VT(param)) {
		case VTYPE_PSTR:
			return TV_STR(param);
		case VTYPE_PWSTR: {
			const wchar_t* strChar = gl_ConvFromShortWchar(TV_WSTR(param), static_cast<size_t>((param)->wstrLen) + 1);
			const auto preRes      = std::wstring(strChar, param->wstrLen);
			delete[] strChar;

			return gl_WStringToString(preRes);
		}
		default:
			break;
	}

	getInputParam_Exception(indexParam, L"string", TV_VT(param));
	return "";
}

template <>
const char* IBaseExtensionClass::getInputParam<const char*>(const tVariant* paParams, const long indexParam) const {
	const auto param = indexParam == -1 ? paParams : &paParams[indexParam];
	switch (TV_VT(param)) {
		case VTYPE_PSTR: {
			const auto result = TV_STR(param);
			return gl_CopyStringToChar(result, param->strLen);
		}
		case VTYPE_PWSTR: {
			const wchar_t* strChar = gl_ConvFromShortWchar(TV_WSTR(param), static_cast<size_t>(param->wstrLen) + 1);
			const auto preRes      = std::wstring(strChar, param->wstrLen);
			delete[] strChar;

			const auto result = gl_WStringToString(preRes);
			return gl_CopyStringToChar(result, param->wstrLen);
		}
		default:
			break;
	}

	getInputParam_Exception(indexParam, L"const char*", TV_VT(param));
	return "";
}

template <>
int IBaseExtensionClass::getInputParam<int>(const tVariant* paParams, const long indexParam) const {
	const auto param = indexParam == -1 ? paParams : &paParams[indexParam];
	switch (TV_VT(param)) {
		case VTYPE_I2:
			return TV_I2(param);
		case VTYPE_I4:
			return TV_I4(param);
		default:
			break;
	}

	getInputParam_Exception(indexParam, L"int", TV_VT(param));
	return 0;
}

template <>
unsigned int IBaseExtensionClass::getInputParam<unsigned int>(const tVariant* paParams, const long indexParam) const {
	const auto param = indexParam == -1 ? paParams : &paParams[indexParam];
	int result       = -1;
	switch (TV_VT(param)) {
		case VTYPE_I2:
			result = TV_I2(param);
			break;
		case VTYPE_I4:
			result = TV_I4(param);
			break;
		default:
			break;
	}

	if (result >= 0)
		return result;

	getInputParam_Exception(indexParam, L"int (>= 0)", TV_VT(param));
	return 0;
}

template <>
double IBaseExtensionClass::getInputParam<double>(const tVariant* paParams, const long indexParam) const {
	const auto param = indexParam == -1 ? paParams : &paParams[indexParam];
	switch (TV_VT(param)) {
		case VTYPE_R4:
			return TV_R4(param);
		case VTYPE_R8:
			return TV_R8(param);
		case VTYPE_I2:
			return TV_I2(param);
		case VTYPE_I4:
			return TV_I4(param);
		default:
			break;
	}

	getInputParam_Exception(indexParam, L"double", TV_VT(param));
	return false;
}

template <>
rapidjson::Document IBaseExtensionClass::getInputParam<rapidjson::Document>(
	const tVariant* paParams, const long indexParam) const {
	const auto inputJsonW = getInputParam<std::wstring>(paParams, indexParam);
	const auto inputJson  = gl_WStringToString(inputJsonW);

	rapidjson::Document document;
	rapidjson::StringStream s(inputJson.c_str());

	document.ParseStream(s);
	if (document.HasParseError()) {
		const auto error = rapidjson::GetParseError_En(document.GetParseError());
		throw LykovException(error, L"Parse JSON");
	}

	return document;
}

template <typename Type>
Type IBaseExtensionClass::getInputParam(const tVariant* paParams, const long indexParam) const {
	throw LykovException(L"Данный тип не реализован!", L"getInputParam");
}

// Установка возвращаемых параметров
template <>
void IBaseExtensionClass::setReturnedParam<bool>(const bool res, tVariant* pvarRetValue) const {
	TV_VT(pvarRetValue)   = VTYPE_BOOL;
	TV_BOOL(pvarRetValue) = res;
}

template <>
void IBaseExtensionClass::setReturnedParam<const std::wstring&>(const std::wstring& res, tVariant* pvarRetValue) const {
	TV_VT(pvarRetValue)   = VTYPE_PWSTR;
	pvarRetValue->wstrLen = static_cast<uint32_t>(res.length());

	if (!m_IMemory->AllocMemory(reinterpret_cast<void**>(&pvarRetValue->pwstrVal), (static_cast<unsigned long>(pvarRetValue->wstrLen) + 1) * sizeof(WCHAR_T)))
		throw LykovException(L"Не удалось выделить память для WSTRING", L"setReturnedParam");

	gl_ConvToShortWchar(&pvarRetValue->pwstrVal, res.c_str(), static_cast<size_t>(pvarRetValue->wstrLen) + 1);
}

template <>
void IBaseExtensionClass::setReturnedParam<const std::string&>(const std::string& res, tVariant* pvarRetValue) const {
	TV_VT(pvarRetValue)  = VTYPE_PSTR;
	pvarRetValue->strLen = static_cast<uint32_t>(res.length());

	if (!m_IMemory->AllocMemory(reinterpret_cast<void**>(&pvarRetValue->pstrVal), (static_cast<unsigned long>(pvarRetValue->strLen) + 1) * sizeof(char)))
		throw LykovException(L"Не удалось выделить память для string", L"setReturnedParam");

	memcpy(pvarRetValue->pstrVal, res.c_str(), static_cast<size_t>(pvarRetValue->strLen) + 1);
}

template <>
void IBaseExtensionClass::setReturnedParam<const char*>(const char* res, tVariant* pvarRetValue) const {
	TV_VT(pvarRetValue)  = VTYPE_PSTR;
	pvarRetValue->strLen = static_cast<uint32_t>(strlen(res));

	if (!m_IMemory->AllocMemory(reinterpret_cast<void**>(&pvarRetValue->pstrVal), (static_cast<unsigned long>(pvarRetValue->strLen) + 1) * sizeof(char)))
		throw LykovException(L"Не удалось выделить память для const char*", L"setReturnedParam");

	memcpy(pvarRetValue->pstrVal, res, static_cast<size_t>(pvarRetValue->strLen) + 1);
}

template <>
void IBaseExtensionClass::setReturnedParam<const wchar_t*>(const wchar_t* res, tVariant* pvarRetValue) const {
	TV_VT(pvarRetValue)   = VTYPE_PWSTR;
	pvarRetValue->wstrLen = static_cast<uint32_t>(wcslen(res));

	if (!m_IMemory->AllocMemory(reinterpret_cast<void**>(&pvarRetValue->pwstrVal), (static_cast<unsigned long>(pvarRetValue->wstrLen) + 1) * sizeof(WCHAR_T)))
		throw LykovException(L"Не удалось выделить память для const wchar_t*", L"setReturnedParam");

	gl_ConvToShortWchar(&pvarRetValue->pwstrVal, res, static_cast<size_t>(pvarRetValue->wstrLen) + 1);
}

template <>
void IBaseExtensionClass::setReturnedParam<long>(const long res, tVariant* pvarRetValue) const {
	TV_VT(pvarRetValue) = VTYPE_I4;
	TV_I4(pvarRetValue) = static_cast<int32_t>(res);
}

template <>
void IBaseExtensionClass::setReturnedParam<int>(const int res, tVariant* pvarRetValue) const {
	TV_VT(pvarRetValue) = VTYPE_I4;
	TV_I4(pvarRetValue) = res;
}

template <>
void IBaseExtensionClass::setReturnedParam<double>(const double res, tVariant* pvarRetValue) const {
	TV_VT(pvarRetValue) = VTYPE_R8;
	if (itsIsolate)
		TV_R4(pvarRetValue) = static_cast<float>(res);
	else
		TV_R8(pvarRetValue) = res;
}

template <>
void IBaseExtensionClass::setReturnedParam<const rapidjson::Document&>(const rapidjson::Document& res, tVariant* pvarRetValue) const {
	rapidjson::StringBuffer buffer;
	rapidjson::Writer writer(buffer);

	res.Accept(writer);
	setReturnedParam<const std::wstring&>(gl_StringToWString(buffer.GetString()), pvarRetValue);
}

template <typename Type>
void IBaseExtensionClass::setReturnedParam(const Type res, tVariant* pvarRetValue) const {
	throw LykovException(L"Данный тип не реализован!", L"setReturnedParam");
}

long IBaseExtensionClass::findNameFromList(const NamesType& list, const WCHAR_T* name) {
	const wchar_t* tmp = gl_ConvFromShortWchar(name);
	const auto result  = std::find_if(list.begin(), list.end(), [&tmp](const auto& it) {
		return gl_IEqualsCaseInsensitive(it.second.nameRu, tmp) || gl_IEqualsCaseInsensitive(it.second.name, tmp);
	});

	return result == list.end() ? -1l : static_cast<long>(result->first);
}

const wchar_t* IBaseExtensionClass::findIndexFromList(const NamesType& list, const unsigned short index) {
	const auto result = list.find(index);
	return result == list.end() ? L"" : result->second.nameRu;
}

void IBaseExtensionClass::getInputParam_Exception(long indexParam, const wchar_t* expectation, const TYPEVAR real) {
	const wchar_t* nameParam;
	switch (real) {
		case VTYPE_EMPTY:
			nameParam = L"empty";
			break;
		case VTYPE_NULL:
			nameParam = L"null";
			break;
		case VTYPE_I2:
			nameParam = L"short";
			break;
		case VTYPE_I4:
		case VTYPE_INT:
			nameParam = L"int";
			break;
		case VTYPE_R4:
			nameParam = L"float";
			break;
		case VTYPE_R8:
			nameParam = L"double";
			break;
		case VTYPE_DATE:
			nameParam = L"data (double)";
			break;
		case VTYPE_TM:
			nameParam = L"struct tm";
			break;
		case VTYPE_PSTR:
			nameParam = L"struct (string)";
			break;
		case VTYPE_INTERFACE:
			nameParam = L"struct (interface)";
			break;
		case VTYPE_ERROR:
			nameParam = L"error code";
			break;
		case VTYPE_BOOL:
			nameParam = L"bool";
			break;
		case VTYPE_VARIANT:
			nameParam = L"struct (_tVariant*)";
			break;
		case VTYPE_I1:
			nameParam = L"char";
			break;
		case VTYPE_UI1:
			nameParam = L"unsigned char";
			break;
		case VTYPE_UI2:
			nameParam = L"unsigned short";
			break;
		case VTYPE_UI4:
		case VTYPE_UINT:
			nameParam = L"unsigned int";
			break;
		case VTYPE_I8:
			nameParam = L"long long";
			break;
		case VTYPE_UI8:
			nameParam = L"unsigned long long";
			break;
		case VTYPE_HRESULT:
			nameParam = L"HRESULT";
			break;
		case VTYPE_PWSTR:
			nameParam = L"struct (wstring)";
			break;
		case VTYPE_BLOB:
			nameParam = L"binary data";
			break;
		case VTYPE_CLSID:
			nameParam = L"UUID";
			break;
		case VTYPE_ARRAY:
			nameParam = L"array";
			break;
		default:
			nameParam = L"unknown type";
			break;
	}

	const auto err = boost::str(boost::wformat(L"Некорректный параметр №%d - ожидался %s, пришёл %s)") % indexParam % expectation % nameParam);
	throw LykovException(err, L"Получение входящего параметра");
}

#pragma endregion
