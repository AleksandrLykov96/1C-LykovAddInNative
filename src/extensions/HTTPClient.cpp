#include <extensions/HTTPClient.hpp>

#pragma region Переопределение методов

const wchar_t* HttpClient::getNameExtension() {
	return L"HttpClient";
}

const wchar_t* HttpClient::getVersion() {
	return L"1.0.0.18";
}

void HttpClient::setMethodPropsExtension() {
	// Свойства
	m_PropNames[gl_Index_Prop_Max_Connects]          = NamesFor1C(L"МаксимальноеКоличествоСоединенийВКэше", L"MaxConnects");
	m_PropNames[gl_Index_Prop_Max_Concurrent_Stream] = NamesFor1C(L"МаксимальноеКоличествоПотоковДляHTTP2", L"MaxConcurrentStreamHTTP2");
	m_PropNames[gl_Index_Prop_Max_Host_Connection]   = NamesFor1C(L"МаксимальноеКоличествоСоединенийКОдномуХосту", L"MaxHostConnection");
	m_PropNames[gl_Index_Prop_Max_Total_Connection]  = NamesFor1C(L"МаксимальноеОбщееКоличествоСоединений", L"MaxTotalConnection");
	m_PropNames[gl_Index_Prop_Enable_Debug]          = NamesFor1C(L"РежимОтладки", L"EnableDebug");
	m_PropNames[gl_Index_Prop_Enable_Headers]        = NamesFor1C(L"ВозвращатьЗаголовки", L"EnableHeaders");
	m_PropNames[gl_Index_Prop_Enable_Cookies]        = NamesFor1C(L"ИспользоватьCookie", L"EnableCookies");
	m_PropNames[gl_Index_Prop_TimeoutMultiWait]      = NamesFor1C(L"ТаймаутМультиДескриптора", L"TimeoutMultiDescriptor");
	
	setAllEasyParameters();
	
	// Методы
	m_MethodNames[gl_Index_Method_Send_Requests_Async]             = NamesFor1C(L"ОтправитьЗапросыАсинхронно", L"SendRequestsAsync");
	m_MethodNames[gl_Index_Method_Send_Requests_Sync]              = NamesFor1C(L"ОтправитьЗапросыПоследовательно", L"SendRequestsSync");
	m_MethodNames[gl_Index_Method_Register_Requests_For_Execution] = NamesFor1C(L"ЗарегистрироватьЗапросыКВыполнению", L"RegisterRequestsForExecution");
	m_MethodNames[gl_Index_Method_Get_Results_Registered_Requests] = NamesFor1C(L"ПолучитьРезультатыЗарегистрированныхЗапросов", L"GetResultsRegisteredRequests");
}

void HttpClient::getPropByIndex(const unsigned short indexProp, tVariant* pvarPropVal) {
	switch (indexProp) {
		case gl_Index_Prop_Max_Connects:
			setReturnedParam<long>(m_MaxConnects, pvarPropVal);
			return;
		case gl_Index_Prop_Max_Concurrent_Stream:
			setReturnedParam<long>(m_MaxConcurrentStream, pvarPropVal);
			return;
		case gl_Index_Prop_Max_Host_Connection:
			setReturnedParam<long>(m_MaxHostConnection, pvarPropVal);
			return;
		case gl_Index_Prop_Max_Total_Connection:
			setReturnedParam<long>(m_MaxTotalConnection, pvarPropVal);
			return;
		case gl_Index_Prop_Enable_Debug:
			setReturnedParam(m_EnableDebug, pvarPropVal);
			return;
		case gl_Index_Prop_Enable_Headers:
			setReturnedParam(m_EnableHeaders, pvarPropVal);
			return;
		case gl_Index_Prop_Enable_Cookies:
			setReturnedParam(m_EnableCookies, pvarPropVal);
			return;
		case gl_Index_Prop_TimeoutMultiWait:
			setReturnedParam<long>(m_TimeoutMultiWait, pvarPropVal);
			return;
		default: {
			if (const auto result = m_CurrentEasyParams.find(indexProp); result != m_CurrentEasyParams.end()) {
				if (!result->second->setThisValue) {
					TV_VT(pvarPropVal) = VTYPE_NULL;
					return;
				}

				switch (result->second->type) {
				case CURLOT_LONG: {
					setReturnedParam(std::get<long>(result->second->value), pvarPropVal);
					return;
				}
				case CURLOT_STRING: {
					const auto res = std::get<std::string>(result->second->value);
					setReturnedParam<const std::wstring&>(gl_StringToWString(res), pvarPropVal);
					return;
				}
				default:
					return;
				}
			}
		}
	}
}

void HttpClient::setPropByIndex(const unsigned short indexProp, tVariant* varPropVal) {
	switch (indexProp) {
		// Multi
		case gl_Index_Prop_Max_Connects:
			m_MaxConnects = setMultiParam<unsigned int>(CURLMOPT_MAXCONNECTS, L"МаксимальноеКоличествоСоединенийВКэше (CURLMOPT_MAXCONNECTS)", varPropVal);
			return;
		case gl_Index_Prop_Max_Concurrent_Stream:
			m_MaxConcurrentStream = setMultiParam<unsigned int>(CURLMOPT_MAX_CONCURRENT_STREAMS, L"МаксимальноеКоличествоПотоковДля (CURLMOPT_MAX_CONCURRENT_STREAMS)", varPropVal);
			return;
		case gl_Index_Prop_Max_Host_Connection:
			m_MaxConcurrentStream = setMultiParam<unsigned int>(CURLMOPT_MAX_HOST_CONNECTIONS, L"МаксимальноеКоличествоСоединенийКОдномуХосту (CURLMOPT_MAX_HOST_CONNECTIONS)", varPropVal);
			return;
		case gl_Index_Prop_Max_Total_Connection:
			m_MaxTotalConnection = setMultiParam<unsigned int>(CURLMOPT_MAX_TOTAL_CONNECTIONS, L"МаксимальноеОбщееКоличествоСоединений (CURLMOPT_MAX_TOTAL_CONNECTIONS)", varPropVal);
			return;
		case gl_Index_Prop_Enable_Debug:
			m_EnableDebug = getInputParam<bool>(varPropVal, -1l);
			return;
		case gl_Index_Prop_Enable_Headers:
			m_EnableHeaders = getInputParam<bool>(varPropVal, -1l);
			return;
		case gl_Index_Prop_Enable_Cookies: {
			const auto needValue    = getInputParam<bool>(varPropVal, -1l);
			const CURLSHcode result = needValue
				                          ? curl_share_setopt(m_Sh, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE)
				                          : curl_share_setopt(m_Sh, CURLSHOPT_UNSHARE, CURL_LOCK_DATA_COOKIE);

			if (result != CURLSHE_OK)
				throw LykovException(curl_share_strerror(result), L"Cookie", ADDIN_E_MSGBOX_FAIL, result);

			m_EnableCookies = needValue;
			return;
		}
		case gl_Index_Prop_TimeoutMultiWait:
			m_TimeoutMultiWait = getInputParam<unsigned int>(varPropVal, -1l);
			return;
		default: {
			const auto result = m_CurrentEasyParams.find(indexProp);
			if (result == m_CurrentEasyParams.end())
				return;

			// Если пришёл null (неопределено), то отключаем данный параметр
			if (TV_VT(varPropVal) == VTYPE_NULL || TV_VT(varPropVal) == VTYPE_EMPTY) {
				result->second->setThisValue = false;
				m_ParametersForEasy.erase(result->first);

				return;
			}

			const auto tmp = createEasyCurl();
			switch (result->second->type) {
				case CURLOT_LONG: {
					const auto value = getInputParam<int>(varPropVal, -1l);
					createEasyCurl_SetParam(tmp, result->second->id, value, result->second->name);

					result->second->value        = value;
					result->second->setThisValue = true;
					m_ParametersForEasy.insert(result->first);

					break;
				}
				case CURLOT_STRING: {
					const auto value = getInputParam<std::string>(varPropVal, -1l);
					createEasyCurl_SetParam(tmp, result->second->id, value.c_str(), result->second->name);

					result->second->value        = value;
					result->second->setThisValue = true;
					m_ParametersForEasy.insert(result->first);

					break;
				}
				default:
					throw LykovException(L"Для данного параметра не удалось подобрать нужный тип значения!", L"setPropByIndex");
			}
			curl_easy_cleanup(tmp);
		}
	}
}

bool HttpClient::getIsPropWritable(const unsigned short indexProp) {
	return true;
}

long HttpClient::getMethodNParams(const unsigned short indexMethod) {
	switch (indexMethod) {
		case gl_Index_Method_Send_Requests_Async:
			return 1l;
		case gl_Index_Method_Send_Requests_Sync:
			return 2l;
		case gl_Index_Method_Register_Requests_For_Execution:
			return m_ItsServer ? 1l : 2l;
		default:
			return 0l;
	}
}

void HttpClient::setParamDefValue(const unsigned short indexMethod, const long indexParam, tVariant* pvarParamDefValue) {
	switch (indexMethod) {
		case gl_Index_Method_Send_Requests_Sync: {
			switch (indexParam) {
				case 0l:
					setReturnedParam(L"{}", pvarParamDefValue);
					return;
				case 1l:
					setReturnedParam(0, pvarParamDefValue);
					return;
				default:
					break;
			}
			break;
		}
		case gl_Index_Method_Send_Requests_Async: {
			if (indexParam == 0l) {
				setReturnedParam(L"{}", pvarParamDefValue);
				return;
			}
			break;
		}
		case gl_Index_Method_Register_Requests_For_Execution: {
			switch (indexParam) {
				case 0l:
					setReturnedParam(L"{}", pvarParamDefValue);
					return;
				case 1l:
					setReturnedParam(false, pvarParamDefValue);
					return;
				default:
					break;
			}

			break;
		}
		default:
			break;
	}

	TV_VT(pvarParamDefValue) = VTYPE_EMPTY;
}

bool HttpClient::getHasRetVal(const unsigned short indexMethod) {
	switch (indexMethod) {
		case gl_Index_Method_Send_Requests_Async:
		case gl_Index_Method_Send_Requests_Sync:
		case gl_Index_Method_Get_Results_Registered_Requests:
			return true;
		default:
			return false;
	}
}

void HttpClient::callMethodAsProc(const unsigned short indexMethod, const tVariant* paParams, const long paParamsSize) {
	if (indexMethod == gl_Index_Method_Register_Requests_For_Execution)
		registerRequestsForExecution(paParams);
}

void HttpClient::callMethodAsFunc(const unsigned short indexMethod, tVariant* pvarRetValue, const tVariant* paParams, const long paParamsSize) {
	switch (indexMethod) {
		case gl_Index_Method_Send_Requests_Async:
			sendRequestsAsync(paParams, pvarRetValue);
			return;
		case gl_Index_Method_Send_Requests_Sync:
			sendRequestsSync(paParams, pvarRetValue);
			return;
		case gl_Index_Method_Get_Results_Registered_Requests:
			getResultsRegisteredRequests(pvarRetValue);
			return;
		default:
			return;
	}
}

#pragma endregion

#pragma region Методы компоненты

void HttpClient::sendRequestsAsync(const tVariant* paParams, tVariant* pvarRetValue) {
	auto inputJson = getInputParam<rapidjson::Document>(paParams);
	if (!inputJson.IsArray() || inputJson.GetArray().Size() <= 1) {
		sendRequestsSync_DoIt(pvarRetValue, inputJson);
		return;
	}

	registerRequests(inputJson, m_Cm, m_RegisteredRequests);
	setReturnedParam<const std::wstring&>(getResultsRequests(m_Cm, m_RegisteredRequests), pvarRetValue);
}

void HttpClient::sendRequestsSync(const tVariant* paParams, tVariant* pvarRetValue) {
	const auto inputJson = getInputParam<rapidjson::Document>(paParams, 0l);
	const unsigned delay = getInputParam<unsigned int>(paParams, 1l);

	sendRequestsSync_DoIt(pvarRetValue, inputJson, delay);
}

void HttpClient::registerRequestsForExecution(const tVariant* paParams) {
	bool initExternal = false;
	if (!m_ItsServer)
		initExternal = getInputParam<bool>(paParams, 1l);

	if (!initExternal) {
		const auto inputJson = getInputParam<rapidjson::Document>(paParams, 0l);
		if (!inputJson.IsObject() && !inputJson.IsArray())
			throw LykovException(L"Непонятный JSON на входе.", L"Зарегистрировать запросы к выполнению");

		registerRequests(inputJson, m_Cm, m_RegisteredRequests);

		auto tmp = -1;
		curl_multi_perform(m_Cm, &tmp);
	}
	else {
		const auto inputString = getInputParam<std::string>(paParams, 0l);
		auto* newThread        = new std::thread([this, inputString] {
			sendRequestsSync_Thread(inputString);
		});

		m_Mutex.lock();
		m_ThreadsMap[newThread->get_id()] = newThread;
		m_Mutex.unlock();
	}
}

void HttpClient::getResultsRegisteredRequests(tVariant* pvarRetValue) {
	setReturnedParam<const std::wstring&>(getResultsRequests(m_Cm, m_RegisteredRequests), pvarRetValue);
	clearAllThreads();
}

#pragma endregion

#pragma region Вспомогательные функции

HttpClient::HttpClient() {
	curl_global_sslset(CURLSSLBACKEND_OPENSSL, nullptr, nullptr);
	curl_global_init(CURL_GLOBAL_ALL);

	// Инициализация подключений
	m_Cm = curl_multi_init();
	curl_multi_setopt(m_Cm, CURLMOPT_MAXCONNECTS, m_MaxConnects);
	curl_multi_setopt(m_Cm, CURLMOPT_MAX_CONCURRENT_STREAMS, m_MaxConcurrentStream);
	curl_multi_setopt(m_Cm, CURLMOPT_MAX_HOST_CONNECTIONS, m_MaxHostConnection);
	curl_multi_setopt(m_Cm, CURLMOPT_MAX_TOTAL_CONNECTIONS, m_MaxTotalConnection);

	m_Sh = curl_share_init();
	curl_share_setopt(m_Sh, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);

	if (m_EnableCookies)
		curl_share_setopt(m_Sh, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
}

HttpClient::~HttpClient() {
	clearAllThreads();

	curl_share_cleanup(m_Sh);
	curl_multi_cleanup(m_Cm);

	curl_global_cleanup();
}

HttpClient::EasyParamStruct::EasyParamStruct(const CURLoption opt, const char* name, const curl_easytype type, const bool setThisValue) {
	this->id           = opt;
	this->name         = gl_CharToWChar(name);
	this->type         = type;
	this->setThisValue = setThisValue;
}

HttpClient::EasyParamStruct::EasyParamStruct(const EasyParamStruct& forCopy) {
	this->id           = forCopy.id;
	this->name         = gl_CopyWStringToWChar(forCopy.name);
	this->type         = forCopy.type;
	this->value        = forCopy.value;
	this->setThisValue = forCopy.setThisValue;
}

HttpClient::EasyParamStruct::~EasyParamStruct() {
	delete[] name;
}

HttpClient::RequestsStruct::RequestsStruct(CURL* curl, const char* id, const char* url, const char* type, const char* fileName, const char* fileResultName, const bool debug) {
	this->eh   = curl;
	this->id   = gl_CopyStringToChar(id);
	this->url  = gl_CopyStringToChar(url);
	this->type = gl_CopyStringToChar(type);

	if (const auto sizeFileName = strlen(fileName); sizeFileName > 0) {
		const auto fileNameW = gl_CharToWChar(fileName, sizeFileName);
		this->fromFileSize   = static_cast<curl_off_t>(file_size(std::filesystem::path(fileNameW)));

		this->fromFile = true;
		this->streamIf.open(std::filesystem::path(fileNameW));
		delete[] fileNameW;

		if (!this->streamIf.good())
			throw LykovException(L"Не удалось открыть файл для чтения запроса!", L"Формирование запроса");
	}

	this->fileNameResultSize = static_cast<rapidjson::SizeType>(strlen(fileResultName));
	if (this->fileNameResultSize > 0) {
		const auto fileResultNameW = gl_CharToWChar(fileResultName, this->fileNameResultSize);
		this->toFile               = true;
		this->streamOf.open(std::filesystem::path(fileResultNameW));
		delete[] fileResultNameW;

		if (!this->streamOf.good())
			throw LykovException(L"Не удалось открыть файл для записи результата запроса!", L"Формирование запроса");

		this->fileNameResult = gl_CopyStringToChar(fileResultName, this->fileNameResultSize);
	}

	this->itsDebug = debug;
}

HttpClient::RequestsStruct::~RequestsStruct() {
	curl_easy_cleanup(this->eh);
	if (this->fromFile)
		this->streamIf.close();

	if (this->toFile)
		this->streamOf.close();

	delete[] id;
	delete[] url;
	delete[] type;

	if (toFile)
		delete[] fileNameResult;
}

void HttpClient::setAllEasyParameters() {
	auto currentIndex = static_cast<unsigned short>(m_PropNames.size());

	auto* opt = curl_easy_option_next(nullptr);
	while (opt) {

		switch (opt->id) {
			// Исключения (Ставим руками).
			case CURLOPT_VERBOSE:
				break;
			// Значения по умолчанию.
			case CURLOPT_BUFFERSIZE:
			case CURLOPT_UPLOAD_BUFFERSIZE: {
				m_CurrentEasyParams[currentIndex] = new EasyParamStruct(opt->id, opt->name, opt->type, true);
				m_CurrentEasyParams[currentIndex]->value = static_cast<long>(gl_Chunk_Size);

				m_PropNames[currentIndex] = NamesFor1C(m_CurrentEasyParams[currentIndex]->name
				                                       , m_CurrentEasyParams[currentIndex]->name);
				m_ParametersForEasy.insert(currentIndex);

				currentIndex++;
				break;
			}
			case CURLOPT_SSL_VERIFYHOST:
			case CURLOPT_SSL_VERIFYPEER: {
				m_CurrentEasyParams[currentIndex] = new EasyParamStruct(opt->id, opt->name, opt->type, true);
				m_CurrentEasyParams[currentIndex]->value = 0l;

				m_PropNames[currentIndex] = NamesFor1C(m_CurrentEasyParams[currentIndex]->name
					, m_CurrentEasyParams[currentIndex]->name);
				m_ParametersForEasy.insert(currentIndex);

				currentIndex++;
				break;
			}
			case CURLOPT_ACCEPT_ENCODING: {
				m_CurrentEasyParams[currentIndex] = new EasyParamStruct(opt->id, opt->name, opt->type, true);
				m_CurrentEasyParams[currentIndex]->value = "";

				m_PropNames[currentIndex] = NamesFor1C(m_CurrentEasyParams[currentIndex]->name
				                                       , m_CurrentEasyParams[currentIndex]->name);
				m_ParametersForEasy.insert(currentIndex);

				currentIndex++;
				break;
			}
			case CURLOPT_TIMEOUT_MS: {
				m_CurrentEasyParams[currentIndex] = new EasyParamStruct(opt->id, opt->name, opt->type, true);
				m_CurrentEasyParams[currentIndex]->value = 30000l;

				m_PropNames[currentIndex] = NamesFor1C(m_CurrentEasyParams[currentIndex]->name
				                                       , m_CurrentEasyParams[currentIndex]->name);
				m_ParametersForEasy.insert(currentIndex);

				currentIndex++;
				break;
			}
			case CURLOPT_FOLLOWLOCATION: {
				m_CurrentEasyParams[currentIndex] = new EasyParamStruct(opt->id, opt->name, opt->type, true);
				m_CurrentEasyParams[currentIndex]->value = 1l;

				m_PropNames[currentIndex] = NamesFor1C(m_CurrentEasyParams[currentIndex]->name
				                                       , m_CurrentEasyParams[currentIndex]->name);
				m_ParametersForEasy.insert(currentIndex);

				currentIndex++;
				break;
			}
			case CURLOPT_USERAGENT: {
				m_CurrentEasyParams[currentIndex] = new EasyParamStruct(opt->id, opt->name, opt->type, true);
				m_CurrentEasyParams[currentIndex]->value = "Lykov_AddInNativeFor1C";

				m_PropNames[currentIndex] = NamesFor1C(m_CurrentEasyParams[currentIndex]->name
				                                       , m_CurrentEasyParams[currentIndex]->name);
				m_ParametersForEasy.insert(currentIndex);

				currentIndex++;
				break;
			}
			case CURLOPT_NOSIGNAL: {
				m_CurrentEasyParams[currentIndex] = new EasyParamStruct(opt->id, opt->name, opt->type, true);
				m_CurrentEasyParams[currentIndex]->value = 1l;

				m_PropNames[currentIndex] = NamesFor1C(m_CurrentEasyParams[currentIndex]->name, m_CurrentEasyParams[currentIndex]->name);
				m_ParametersForEasy.insert(currentIndex);

				currentIndex++;
				break;
			}
			// Остальные по умолчанию отключаем
			default: {
				switch (opt->type) {
					case CURLOT_LONG:
					case CURLOT_STRING: {
						m_CurrentEasyParams[currentIndex] = new EasyParamStruct(opt->id, opt->name, opt->type, false);
						m_PropNames[currentIndex] = NamesFor1C(m_CurrentEasyParams[currentIndex]->name, m_CurrentEasyParams[currentIndex]->name);

						currentIndex++;
						break;
					}
					case CURLOT_VALUES: {
						m_CurrentEasyParams[currentIndex] = new EasyParamStruct(opt->id, opt->name, CURLOT_LONG, false);
						m_PropNames[currentIndex] = NamesFor1C(m_CurrentEasyParams[currentIndex]->name, m_CurrentEasyParams[currentIndex]->name);

						currentIndex++;
						break;
					}
					default:
						break;
				}

				break;
			}
		}
		
		opt = curl_easy_option_next(opt);
	}
}

template <typename type>
type HttpClient::setMultiParam(const CURLMoption opt, const wchar_t* name, tVariant* prop) {
	const auto needValue = getInputParam<type>(prop, -1l);
	if (const auto result = curl_multi_setopt(m_Cm, opt, needValue); result != CURLM_OK)
		throw LykovException(curl_multi_strerror(result), name, ADDIN_E_MSGBOX_FAIL, result);

	return needValue;
}

CURL* HttpClient::createEasyCurl() {
	CURL* curl = curl_easy_init();
	if (!curl)
		throw LykovException(L"Не удалось инициализировать CURL запрос", L"createEasyCurl");

	for (const auto& key : m_ParametersForEasy) {
		if (const auto result = m_CurrentEasyParams.find(key); result != m_CurrentEasyParams.end()) {
			switch (result->second->type) {
				case CURLOT_LONG:
					createEasyCurl_SetParam(curl, result->second->id, std::get<long>(result->second->value), result->second->name);
					continue;
				case CURLOT_STRING:
					createEasyCurl_SetParam<const char*>(curl, result->second->id, std::get<std::string>(result->second->value).c_str(), result->second->name);
					continue;
				default:
					break;
			}
		}
	}

	if (m_EnableCookies) {
		createEasyCurl_SetParam(curl, CURLOPT_SHARE, m_Sh, L"CURLOPT_SHARE");
		createEasyCurl_SetParam(curl, CURLOPT_COOKIEFILE, "", L"CURLOPT_SHARE");
	}
		

	return curl;
}

template <typename type>
void HttpClient::createEasyCurl_SetParam(CURL* c, const CURLoption opt, const type value, const wchar_t* nameParam, RequestsStruct* request) {
	if (const auto result = curl_easy_setopt(c, opt, value); result != CURLE_OK) {
		request != nullptr ? delete[] request : curl_easy_cleanup(c);
		throw LykovException(curl_easy_strerror(result), nameParam, ADDIN_E_MSGBOX_FAIL, result);
	}
}

HttpClient::RequestsStruct* HttpClient::createRequest(const rapidjson::Value::Object& json) {
	const auto curl = createEasyCurl();
	const auto id   = json.HasMember("ИдентификаторЗапроса") && json["ИдентификаторЗапроса"].IsString()
		                  ? json["ИдентификаторЗапроса"].GetString()
		                  : "";
	const auto body = json.HasMember("ТелоЗапроса") && json["ТелоЗапроса"].IsString()
		                  ? json["ТелоЗапроса"].GetString()
		                  : "";
	const auto url  = json.HasMember("URL") && json["URL"].IsString() ? json["URL"].GetString() : "";
	const auto type = json.HasMember("МетодЗапроса") && json["МетодЗапроса"].IsString()
		                  ? json["МетодЗапроса"].GetString()
		                  : "";
	const auto fileName = json.HasMember("ИмяФайлаЧтения") && json["ИмяФайлаЧтения"].IsString()
		                      ? json["ИмяФайлаЧтения"].GetString()
		                      : "";
	const auto fileResultName = json.HasMember("ИмяФайлаРезультат") && json["ИмяФайлаРезультат"].IsString()
		                            ? json["ИмяФайлаРезультат"].GetString()
		                            : "";
	const auto itsPost       = gl_IEqualsCaseInsensitive(type, "POST");
	const auto requestResult = new RequestsStruct(curl, id, url, type, fileName, fileResultName, m_EnableDebug);

	// Тело запроса
	createEasyCurl_SetParam(curl, CURLOPT_WRITEFUNCTION, &gl_WriteCallback_Body, L"CURLOPT_WRITEFUNCTION", requestResult);
	createEasyCurl_SetParam(curl, CURLOPT_WRITEDATA, static_cast<void*>(requestResult), L"CURLOPT_WRITEDATA", requestResult);

	if (requestResult->fromFile) {
		createEasyCurl_SetParam(curl, CURLOPT_READFUNCTION, &gl_ReadCallback, L"CURLOPT_READFUNCTION", requestResult);
		createEasyCurl_SetParam(curl, CURLOPT_READDATA, static_cast<void*>(requestResult), L"CURLOPT_READDATA", requestResult);

		if (!itsPost) {
			createEasyCurl_SetParam(curl, CURLOPT_UPLOAD, 1l, L"CURLOPT_UPLOAD", requestResult);
			createEasyCurl_SetParam(curl,
					requestResult->fromFileSize > gl_Limit_Large_Body ? CURLOPT_INFILESIZE_LARGE : CURLOPT_INFILESIZE,
					requestResult->fromFileSize,
					L"CURLOPT_INFILESIZE",
					requestResult);
		}

	} else if (const auto size = strlen(body); size > 0) {
		createEasyCurl_SetParam(curl, CURLOPT_POSTFIELDS, body, L"CURLOPT_POSTFIELDS", requestResult);
		createEasyCurl_SetParam(curl,
				size > gl_Limit_Large_Body ? CURLOPT_POSTFIELDSIZE_LARGE : CURLOPT_POSTFIELDSIZE,
				size,
				L"CURLOPT_POSTFIELDSIZE",
				requestResult);
	}

	// Заголовки
	if (json.HasMember("Заголовки") && json["Заголовки"].IsObject()) {
		curl_slist* headers = nullptr;
		for (const auto& it : json["Заголовки"].GetObj()) {
			if (!it.value.IsString())
				continue;

			std::string headerResult = boost::str(boost::format("%s: %s") % it.name.GetString() % it.value.GetString());
			headers = curl_slist_append(headers, headerResult.c_str());
		}

		createEasyCurl_SetParam(curl, CURLOPT_HTTPHEADER, headers, L"CURLOPT_HTTPHEADER", requestResult);
	}

	// Возврат заголовков
	if (m_EnableHeaders || m_EnableDebug) {
		createEasyCurl_SetParam(curl, CURLOPT_HEADERFUNCTION, &gl_WriteCallback_Headers, L"CURLOPT_HEADERFUNCTION", requestResult);
		createEasyCurl_SetParam(curl, CURLOPT_HEADERDATA, static_cast<void*>(requestResult), L"CURLOPT_HEADERDATA", requestResult);
	}

	// URL запроса
	if (strlen(url) > 0)
		createEasyCurl_SetParam(curl, CURLOPT_URL, url, L"CURLOPT_URL", requestResult);

	// Тип запроса
	if (itsPost)
		createEasyCurl_SetParam(curl, CURLOPT_POST, 1l, L"CURLOPT_POST", requestResult);
	else if (gl_IEqualsCaseInsensitive(type, "GET"))
		createEasyCurl_SetParam(curl, CURLOPT_HTTPGET, 1l, L"CURLOPT_HTTPGET", requestResult);
	else if (strlen(type) > 0)
		createEasyCurl_SetParam(curl, CURLOPT_CUSTOMREQUEST, type, L"CURLOPT_CUSTOMREQUEST", requestResult);

	if (m_EnableDebug) {
		createEasyCurl_SetParam(curl, CURLOPT_VERBOSE, 1l, L"CURLOPT_VERBOSE", requestResult);
		createEasyCurl_SetParam(curl, CURLOPT_DEBUGFUNCTION, &gl_DebugCallback, L"CURLOPT_DEBUGFUNCTION", requestResult);
		createEasyCurl_SetParam(curl, CURLOPT_DEBUGDATA, static_cast<void*>(requestResult), L"CURLOPT_DEBUGFUNCTION", requestResult);
	}
	
	return requestResult;
}

template <typename type>
void HttpClient::writeResultToJson(const CURLcode codeResult, RequestsStruct* request, rapidjson::Writer<type>& writer) {
	writer.StartObject();

	writer.Key("UnixTimestampОтвета", static_cast<rapidjson::SizeType>(25));
	writer.Double(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000000.);

	writer.Key("ИдентификаторЗапроса", static_cast<rapidjson::SizeType>(40));
	writer.String(request->id, static_cast<rapidjson::SizeType>(strlen(request->id)));

	writer.Key("URL", static_cast<rapidjson::SizeType>(3));
	writer.String(request->url, static_cast<rapidjson::SizeType>(strlen(request->url)));

	writer.Key("ТипЗапроса", static_cast<rapidjson::SizeType>(20));
	writer.String(request->type, static_cast<rapidjson::SizeType>(strlen(request->type)));

	double total = 0;
	long code    = 0;

	curl_easy_getinfo(request->eh, CURLINFO_TOTAL_TIME, &total);
	curl_easy_getinfo(request->eh, CURLINFO_RESPONSE_CODE, &code);

	writer.Key("КодОтвета", static_cast<rapidjson::SizeType>(18));
	writer.Int(code);

	writer.Key("ВремяОтвета", static_cast<rapidjson::SizeType>(22));
	writer.Double(total);

	writer.Key("Успешно", static_cast<rapidjson::SizeType>(14));
	writer.Bool(code >= 200 && code < 300);

	if (codeResult != CURLE_OK) {
		if (request->toFile) {
			gl_WriteStringToFile(request->fileNameResult, curl_easy_strerror(codeResult));
			writer.Key("ФайлРезультат", static_cast<rapidjson::SizeType>(26));
			writer.String(request->fileNameResult, request->fileNameResultSize);
		} else {
			writer.Key("ТелоРезультат", static_cast<rapidjson::SizeType>(26));
			writer.String(curl_easy_strerror(codeResult));
		}
	} else if (request->toFile) {
		writer.Key("ФайлРезультат", static_cast<rapidjson::SizeType>(26));
		writer.String(request->fileNameResult, request->fileNameResultSize);
	} else {
		writer.Key("ТелоРезультат", static_cast<rapidjson::SizeType>(26));
		writer.String(request->bodyResult.c_str(), request->bodyResultSize, true);
	}

	if (request->itsDebug) {
		writer.Key("ОтладочнаяИнформация", static_cast<rapidjson::SizeType>(40));

		writer.StartObject();

		writer.Key("ОбщаяИнформация", static_cast<rapidjson::SizeType>(30));
		writer.String(request->debugInfo_Text.c_str(), request->debugInfo_TextSize, true);

		writer.Key("ПолученныеДанные", static_cast<rapidjson::SizeType>(32));
		writer.String(request->debugInfo_DataIn.c_str(), request->debugInfo_DataInSize, true);

		writer.Key("ОтправленныеДанные", static_cast<rapidjson::SizeType>(36));
		writer.String(request->debugInfo_DataOut.c_str(), request->debugInfo_DataOutSize, true);

		writer.Key("ПолученныеЗаголовки", static_cast<rapidjson::SizeType>(38));
		writer.String(request->debugInfo_HeaderIn.c_str(), request->debugInfo_HeaderInSize, true);

		writer.Key("ОтправленныеЗаголовки", static_cast<rapidjson::SizeType>(42));
		writer.String(request->debugInfo_HeaderOut.c_str(), request->debugInfo_HeaderOutSize, true);

		writer.Key("ОтправленныеДанныеПоSSL", static_cast<rapidjson::SizeType>(43));
		writer.String(request->debugInfo_SSLDataOut.c_str(), request->debugInfo_SSLDataOutSize, true);

		writer.Key("ПолученныеДанныеПоSSL", static_cast<rapidjson::SizeType>(39));
		writer.String(request->debugInfo_SSLDataIn.c_str(), request->debugInfo_SSLDataInSize, true);

		writer.EndObject();
	}

	if (!request->headersResult.empty()) {
		writer.Key("ЗаголовкиРезультат", static_cast<rapidjson::SizeType>(36));

		writer.StartObject();
		for (const auto& it : request->headersResult) {
			writer.Key(it.first.c_str(), static_cast<rapidjson::SizeType>(it.first.size()), true);
			writer.String(it.second.c_str(), static_cast<rapidjson::SizeType>(it.second.size()), true);
		}
		writer.EndObject();
	}

	writer.EndObject();
}

void HttpClient::registerRequests(const rapidjson::Document& json, CURLM* currentCM, std::unordered_map<CURL*, RequestsStruct*>& localRegisteredRequests) {
	std::list<RequestsStruct*> localRequests;
	if (json.IsArray()) {
		for (const auto& it : json.GetArray()) {
			const auto& row    = it.GetObj();
			const auto request = createRequest(reinterpret_cast<const rapidjson::Value::Object&>(row));

			if (const auto code = curl_multi_add_handle(currentCM, request->eh); code != CURLM_OK) {
				for (const auto& it : localRequests) {
					curl_multi_remove_handle(currentCM, it);
					delete it;
				}

				delete request;
				throw LykovException(curl_multi_strerror(code), L"Добавление запроса к мульти-дескриптору", ADDIN_E_MSGBOX_FAIL, code);
			}

			localRequests.push_back(request);
		}
	}
	else if (json.IsObject()) {
		const auto& row    = json.GetObj();
		const auto request = createRequest(reinterpret_cast<const rapidjson::Value::Object&>(row));

		if (const auto code = curl_multi_add_handle(currentCM, request->eh); code != CURLM_OK) {
			delete request;
			throw LykovException(curl_multi_strerror(code), L"Добавление запроса к мульти-дескриптору", ADDIN_E_MSGBOX_FAIL, code);
		}

		localRequests.push_back(request);
	}

	for (const auto& it : localRequests)
		localRegisteredRequests[it->eh] = it;
}

std::wstring HttpClient::getResultsRequests(CURLM* currentCM, std::unordered_map<CURL*, RequestsStruct*>& currentRegisteredRequests) const {
	CURLMsg* msg;
	auto stillAlive = 1;
	auto tmp        = -1;

	rapidjson::StringBuffer json;
	rapidjson::Writer resultJson(json);

	resultJson.StartArray();

	do {
		if (const auto code = curl_multi_perform(currentCM, &stillAlive); code != CURLM_OK)
			throw LykovException(curl_multi_strerror(code), L"curl_multi_perform(...)", ADDIN_E_MSGBOX_FAIL, code);

		while ((msg = curl_multi_info_read(currentCM, &tmp))) {
			if (msg->msg != CURLMSG_DONE)
				continue;

			const auto currentRequest = currentRegisteredRequests.find(msg->easy_handle)->second;
			if (currentRequest->eh == nullptr)
				continue;

			writeResultToJson(msg->data.result, currentRequest, resultJson);

			if (const auto code = curl_multi_remove_handle(currentCM, msg->easy_handle); code != CURLM_OK)
				throw LykovException(curl_multi_strerror(code), L"curl_multi_remove_handle(...)", ADDIN_E_MSGBOX_FAIL, code);

			currentRegisteredRequests.erase(msg->easy_handle);
			delete currentRequest;
		}

		if (stillAlive)
			if (const auto code = curl_multi_wait(currentCM, nullptr, 0, m_TimeoutMultiWait, nullptr);
					code != CURLM_OK && code != CURLM_UNRECOVERABLE_POLL) // CURLM_UNRECOVERABLE_POLL - Странная ошибка.
				throw LykovException(curl_multi_strerror(code), L"curl_multi_perform(...)", ADDIN_E_MSGBOX_FAIL, code);
	} while (stillAlive);

	resultJson.EndArray();
	return gl_StringToWString(json.GetString());
}

void HttpClient::sendRequestsSync_DoIt(tVariant* pvarRetValue, const rapidjson::Document& inputJson, const unsigned delay) {
	if (!inputJson.IsObject() && !inputJson.IsArray())
		throw LykovException(L"Непонятный JSON на входе.", L"ОтправитьЗапросыПоследовательно");

	rapidjson::StringBuffer json;
	rapidjson::Writer resultJson(json);

	if (inputJson.IsObject()) {
		const auto& row = inputJson.GetObj();
		const auto request = createRequest(reinterpret_cast<const rapidjson::Value::Object&>(row));

		const auto resultRequest = curl_easy_perform(request->eh);
		writeResultToJson(resultRequest, request, resultJson);

		delete request;
	}
	else {
		bool itsFirst = true;

		resultJson.StartArray();
		for (const auto& it : inputJson.GetArray()) {
			if (!it.IsObject())
				throw LykovException(L"Некорректный JSON для запросов", L"ОтправитьЗапросыПоследовательно");

			if (!itsFirst) {
				if (delay > 0)
					std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			}
			else
				itsFirst = false;

			const auto& row = it.GetObj();
			const auto request = createRequest(reinterpret_cast<const rapidjson::Value::Object&>(row));

			const auto resultRequest = curl_easy_perform(request->eh);
			writeResultToJson(resultRequest, request, resultJson);

			delete request;
		}
		resultJson.EndArray();
	}

	setReturnedParam<const std::wstring&>(gl_StringToWString(json.GetString()), pvarRetValue);
}

void HttpClient::sendRequestsSync_Thread(std::string inputString) {
	try {
		if (const auto json = gl_GetInputJson(inputString); json.IsObject()) {
			const auto& row    = json.GetObj();
			const auto request = createRequest(reinterpret_cast<const rapidjson::Value::Object&>(row));

			const auto resultRequest = curl_easy_perform(request->eh);

			rapidjson::StringBuffer jsonBuffer;
			rapidjson::Writer resultJson(jsonBuffer);

			const auto idW = gl_CharToWChar(request->id);
			writeResultToJson(resultRequest, request, resultJson);

			externalEvent(L"HTTPRequestsResult", idW, gl_StringToWString(jsonBuffer.GetString()).c_str());
			delete[] idW;

			delete request;
		}
		else {
			CURLM* localCm = curl_multi_init();
			curl_multi_setopt(localCm, CURLMOPT_MAXCONNECTS, m_MaxConnects);
			curl_multi_setopt(localCm, CURLMOPT_MAX_CONCURRENT_STREAMS, m_MaxConcurrentStream);
			curl_multi_setopt(localCm, CURLMOPT_MAX_HOST_CONNECTIONS, m_MaxHostConnection);
			curl_multi_setopt(localCm, CURLMOPT_MAX_TOTAL_CONNECTIONS, m_MaxTotalConnection);

			std::unordered_map<CURL*, RequestsStruct*> localRegisteredRequests;

			registerRequests(json, localCm, localRegisteredRequests);

			std::wstring ids;
			for (const auto& it : localRegisteredRequests) {
				if (!ids.empty())
					ids.append(L";");
				ids.append(gl_StringToWString(it.second->id));
			}

			const auto source         = gl_CopyWStringToWChar(ids);
			const std::wstring result = getResultsRequests(localCm, localRegisteredRequests);

			externalEvent(L"HTTPRequestsResult", source, result.c_str());
			delete[] source;

			curl_multi_cleanup(localCm);
		}
	} catch (const LykovException& message) {
		externalEvent(L"HTTPRequestsResult", message.getSource(), message.whatW());
	} catch (const std::runtime_error& message) {
		const auto source = boost::str(boost::wformat(L"HTTP запрос (runtime_error) - Ошибка в потоке %d") % std::this_thread::get_id());
		const auto myExp = LykovException(message.what(), source.c_str());
		externalEvent(L"HTTPRequestsResult", myExp.getSource(), myExp.whatW());
	} catch (const std::exception& message) {
		const auto source = boost::str(boost::wformat(L"HTTP запрос (exception) - Ошибка в потоке %d") % std::this_thread::get_id());
		const auto myExp = LykovException(message.what(), source.c_str());
		externalEvent(L"HTTPRequestsResult", myExp.getSource(), myExp.whatW());
	} catch (...) {
		const auto source = boost::str(boost::wformat(L"HTTP запрос (...) - Ошибка в потоке %d") % std::this_thread::get_id());
		const auto myExp = LykovException(L"Неизвестная ошибка", source.c_str());
		externalEvent(L"HTTPRequestsResult", myExp.getSource(), myExp.whatW());
	}

	m_Mutex.lock();
	const std::thread::id thisId = std::this_thread::get_id();
	m_ThreadsMap.erase(thisId);
	m_Mutex.unlock();
}

void HttpClient::clearAllThreads() {
	for (unsigned short i = 0; i < 100; i++) {
		if (m_ThreadsMap.empty())
			break;

		if (const auto first = m_ThreadsMap.begin()->second; first->joinable())
			first->join();
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	m_ThreadsMap.clear();
}

static size_t gl_WriteCallback_Body(const char* contents, size_t size, size_t nmemb, HttpClient::RequestsStruct* request) {
	const auto resultSize = size * nmemb;
	if (request->toFile)
		request->streamOf.write(contents, resultSize);
	else {
		request->bodyResult.append(contents, resultSize);
		request->bodyResultSize += static_cast<rapidjson::SizeType>(resultSize);
	}

	return resultSize;
}

static size_t gl_WriteCallback_Headers(const char* contents, const size_t size, const size_t nmemb, HttpClient::RequestsStruct* request) {
	const auto result = size * nmemb;
	auto str          = std::string(contents, result);
	str               = str.substr(0, str.size() - 2);

	if (!str.empty()) {
		if (const auto delim = str.find(':'); delim != std::string::npos)
			request->headersResult[str.substr(0, delim)] = str.substr(delim + 2);
	}

	return result;
}

static size_t gl_ReadCallback(char* contents, const size_t size, const size_t nmemb, HttpClient::RequestsStruct* request) {
	return static_cast<size_t>(request->streamIf.read(contents, size * nmemb).gcount());
}

static int gl_DebugCallback(CURL* handle, const curl_infotype type, const char* data, const size_t size, HttpClient::RequestsStruct* request) {
	switch (type) {
		case CURLINFO_TEXT: {
			request->debugInfo_Text.append(data, size);
			request->debugInfo_TextSize += static_cast<rapidjson::SizeType>(size);

			return 0;
		}
		case CURLINFO_HEADER_IN: {
			request->debugInfo_HeaderIn.append(data, size);
			request->debugInfo_HeaderInSize += static_cast<rapidjson::SizeType>(size);

			return 0;
		}
		case CURLINFO_HEADER_OUT: {
			request->debugInfo_HeaderOut.append(data, size);
			request->debugInfo_HeaderOutSize += static_cast<rapidjson::SizeType>(size);

			return 0;
		}
		case CURLINFO_SSL_DATA_IN: {
			request->debugInfo_SSLDataIn.append(data, size);
			request->debugInfo_SSLDataInSize += static_cast<rapidjson::SizeType>(size);

			return 0;
		}
		case CURLINFO_SSL_DATA_OUT: {
			request->debugInfo_SSLDataOut.append(data, size);
			request->debugInfo_SSLDataOutSize += static_cast<rapidjson::SizeType>(size);

			return 0;
		}
		case CURLINFO_DATA_IN: {
			request->debugInfo_DataIn.append(data, size);
			request->debugInfo_DataInSize += static_cast<rapidjson::SizeType>(size);

			return 0;
		}
		case CURLINFO_DATA_OUT: {
			request->debugInfo_DataOut.append(data, size);
			request->debugInfo_DataOutSize += static_cast<rapidjson::SizeType>(size);

			return 0;
		}
		default:
			return 0;
	}
}

#pragma endregion
