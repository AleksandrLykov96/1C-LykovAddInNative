
#include "extensions/HTTPClient.hpp"

using namespace std;
using namespace native1C;

#pragma region Методы для переопределения

wstring HttpClient::getNameExtension() {
	return L"HttpClient";
}

void HttpClient::initializeComponent() {
	addProperty(L"ВозвращатьЗаголовки",
	            L"returnHeaders",
	            [this] { return p_ReturnHeaders; },
	            [&](const all_input_types& res) {
		            if (!holds_alternative<bool>(res))
			            BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра должен быть 'Булево'"));

		            p_ReturnHeaders = get<bool>(res);
	            }
	);

	// Параметры multi
	for (const auto& key : p_MultiOptions | views::keys) {
		const auto name = string(magic_enum::enum_name(key));
		const auto wName = gl_conv_string_to_wstring(name);

		addProperty(wName,
		            wName,
		            [key, this] {
			            return visit(Overloaded{
				                         [](const wstring& val) {
					                         return static_cast<all_returned_types>(val);
				                         },
				                         [](const long val) {
					                         return static_cast<all_returned_types>(val);
				                         },
				                         [](auto) {
					                         return static_cast<all_returned_types>(monostate());
				                         },
			                         }, p_MultiOptions[key]);
		            },
		            [&](const all_input_types& res) {
			            visit(Overloaded{
				                  [&](const wstring& val) {
					                  p_MultiOptions[key] = val;
				                  },

				                  [&](const int32_t& val) {
					                  p_MultiOptions[key] = val;
				                  },

				                  [&](const double& val) {
					                  p_MultiOptions[key] = lround(val);
				                  },

				                  [&](const monostate& val) {
					                  p_MultiOptions[key] = monostate();
				                  },
				                  [](auto) {
					                  BOOST_THROW_EXCEPTION(invalid_argument("Неизвестный тип параметра! Может быть только 'Строка', 'Число' или 'Неопределено / NULL'"));
				                  }
			                  }, res);

			            setOptMulti(p_Cm);
		            });
	}

	// Параметры single
	for (const auto& key : g_SingleOptions | views::keys) {
		const auto name = string(curl_easy_option_by_id(key)->name);
		const auto wName = gl_conv_string_to_wstring(name);

		addProperty(wName,
		            wName,
		            [key, this] {
			            return visit(Overloaded{
				                         [](const wstring& val) {
					                         return static_cast<all_returned_types>(val);
				                         },
				                         [](const long val) {
					                         return static_cast<all_returned_types>(val);
				                         },
				                         [](auto) {
					                         return static_cast<all_returned_types>(monostate());
				                         },
			                         }, g_SingleOptions[key]);
		            },
		            [&](const all_input_types& res) {
			            visit(Overloaded{
				                  [&](const wstring& val) {
									  g_SingleOptions[key] = val;
				                  },

				                  [&](const int32_t& val) {
									  g_SingleOptions[key] = val;
				                  },

				                  [&](const double& val) {
									  g_SingleOptions[key] = lround(val);
				                  },

				                  [&](const monostate& val) {
									  g_SingleOptions[key] = monostate();
				                  },
				                  [](auto) {
					                  BOOST_THROW_EXCEPTION(invalid_argument("Неизвестный тип параметра! Может быть только 'Строка', 'Число' или 'Неопределено / NULL'"));
				                  }
			                  }, res);
		            });
	}

	// Методы компоненты
	addMethod(L"ОтправитьЗапросыАсинхронно",
	          L"sendRequestsAsync",
	          this,
	          &HttpClient::sendRequestsAsync);

	addMethod(L"ОтправитьЗапросыСинхронно",
	          L"sendRequestsSync",
	          this,
	          &HttpClient::sendRequestsSync,
	          {{1, 0}});

	addMethod(L"ЗарегистрироватьЗапросыКВыполнению",
	          L"registerRequestsForExecution",
	          this,
	          &HttpClient::registerRequestsForExecution,
	          {{1, false}});

	addMethod(L"ПолучитьРезультатыЗарегистрированныхЗапросов",
	          L"getResultsRegisteredRequests",
	          this,
	          &HttpClient::getResultsRegisteredRequests);
}

#pragma endregion

#pragma region Методы компоненты

all_returned_types HttpClient::sendRequestsAsync(const all_input_types& requestsInput) {
	if (!holds_alternative<wstring>(requestsInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка (JSON)' (запросы для выполнения)"));

	const auto requests = getInputJSON(get<wstring>(requestsInput));
	if (!requests.IsObject() && !requests.IsArray())
		BOOST_THROW_EXCEPTION(invalid_argument("Некорректный JSON на входе"));

	// Можно без multi.
	if (requests.IsObject())
		return sendRequestSync(requests);

	if (registerRequests(requests, p_Cm, p_ActiveRequests))
		return getResultsRequests(p_Cm, p_ActiveRequests);

	return L"[]";
}

all_returned_types HttpClient::sendRequestsSync(const all_input_types& requestsInput, const all_input_types& delayInput) {
	if (!holds_alternative<wstring>(requestsInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка (JSON)' (запросы для выполнения)"));

	const auto requests = getInputJSON(get<wstring>(requestsInput));
	if (!requests.IsObject() && !requests.IsArray())
		BOOST_THROW_EXCEPTION(invalid_argument("Некорректный JSON на входе"));

	long long delay = -1LL;
	visit(Overloaded{
		      [&](const int32_t& milliseconds) {
			      delay = llround(milliseconds);
		      },
		      [&](const double& milliseconds) {
			      delay = llround(milliseconds);
		      },
		      [](auto) {
		      }
	      }, delayInput);

	if (delay < 0)
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №2 должен быть 'Неотрицательное число' (количество миллисекунд для паузы между запросами)"));

	if (requests.IsObject())
		return sendRequestSync(requests);

	json_document resultJson(rapidjson::kArrayType);
	auto& allocator = resultJson.GetAllocator();

	for (const auto& it : requests.GetArray()) {
		json_document requestResult(rapidjson::kObjectType);

		sendRequestSync(it.GetObj(), requestResult, allocator);
		resultJson.PushBack(requestResult, allocator);

		if (delay > 0)
			this_thread::sleep_for(chrono::milliseconds(delay));
	}

	return resultJson;
}

void HttpClient::registerRequestsForExecution(const all_input_types& requestsInput, const all_input_types& initExternalInput) {
	if (!holds_alternative<wstring>(requestsInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка (JSON)' (запросы для выполнения)"));

	const auto requests = get<wstring>(requestsInput);

	bool initExternal = false;
	if (!itsServer) {
		if (!holds_alternative<bool>(initExternalInput))
			BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №2 должен быть 'Булево' (возвращать результат как внешнее событие)"));

		initExternal = get<bool>(initExternalInput);
	}

	if (initExternal) {
		// У RapidJson конструктор копирования в private. Отправляем в новый поток строку.
		p_Mutex.lock();

		const auto thr = new jthread([this, requests] {
			const auto requestsJson = getInputJSON(requests);
			sendRequestsSync_Thread(requestsJson);
		});

		p_ActiveThreads[thr->get_id()] = thr;

		p_Mutex.unlock();
	}
	else {
		const auto requestsJson = getInputJSON(requests);
		if (!requestsJson.IsObject() && !requestsJson.IsArray())
			BOOST_THROW_EXCEPTION(invalid_argument("Некорректный JSON на входе"));

		registerRequests(requestsJson, p_Cm, p_ActiveRequests);

		auto tmp = -1;
		curl_multi_perform(p_Cm, &tmp);
	}
}

all_returned_types HttpClient::getResultsRegisteredRequests() {
	return getResultsRequests(p_Cm, p_ActiveRequests);
}

#pragma endregion

#pragma region Вспомогательные методы

#pragma region CurlRequestInfo

CurlRequestInfo::CurlRequestInfo(const json_value& json, const bool returnHeaders, const bool debug) {
	eh = curl_easy_init();
	if (!eh)
		BOOST_THROW_EXCEPTION(runtime_error("Не удалось инициализировать CURL запрос"));

	itsDebug = debug;

	copyValueStringFromSrc(id, json, L"ИдентификаторЗапроса", L"default");
	copyValueStringFromSrc(url, json, L"URL");
	copyValueStringFromSrc(type, json, L"МетодЗапроса");
	copyValueStringFromSrc(inputFileName, json, L"ИмяФайлаЧтения");
	copyValueStringFromSrc(outputFileName, json, L"ИмяФайлаРезультат");

	const bool itsPost = ranges::equal(type, L"POST");
	const auto pointer = static_cast<void*>(this);

	// Тело запроса
	if (!inputFileName.empty()) {
		const auto path = filesystem::path(inputFileName);

		streamIf.open(path);
		if (!streamIf || !streamIf.is_open())
			BOOST_THROW_EXCEPTION(invalid_argument("Не удалось открыть файловый поток для чтения тела запроса"));

		fromFile = true;

		curl_easy_setopt(eh, CURLOPT_READDATA, pointer);
		curl_easy_setopt(eh, CURLOPT_READFUNCTION, +[](char* ptr, const size_t size, const size_t nmemb, CurlRequestInfo* request) -> size_t {
			return request->readDataFromFile(ptr, size * nmemb);
		});

		if (!itsPost) {
			const auto fileSize = static_cast<curl_off_t>(file_size(path));

			curl_easy_setopt(eh, CURLOPT_UPLOAD, 1L);
			curl_easy_setopt(eh, fileSize > limit_large_body ? CURLOPT_INFILESIZE_LARGE : CURLOPT_INFILESIZE, fileSize);
		}
	}
	else {
		wstring bodyWstr;
		if (const auto bodyJson = json.FindMember(L"ТелоЗапроса"); bodyJson != json.MemberEnd() && bodyJson->value.IsString())
			bodyWstr = wstring(bodyJson->value.GetString(), bodyJson->value.GetStringLength());

		body = gl_conv_wstring_to_string(bodyWstr);
		if (const auto bodySize = static_cast<curl_off_t>(body.size()); bodySize > 0) {
			curl_easy_setopt(eh, CURLOPT_POSTFIELDS, body.c_str());
			curl_easy_setopt(eh, bodySize > limit_large_body ? CURLOPT_POSTFIELDSIZE_LARGE : CURLOPT_POSTFIELDSIZE, bodySize);
		}
	}

	// Тело результат
	if (!outputFileName.empty()) {
		streamOf.open(filesystem::path(outputFileName));
		if (!streamOf || !streamOf.is_open())
			BOOST_THROW_EXCEPTION(invalid_argument("Не удалось открыть файловый поток для записи результата запроса"));

		toFile = true;
	}

	curl_easy_setopt(eh, CURLOPT_WRITEDATA, pointer);
	curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, +[](const char* ptr, const size_t size, const size_t nmemb, CurlRequestInfo* request) -> size_t {
		return request->writeResult(ptr, size * nmemb);
	});

	// Заголовки
	if (const auto headersJson = json.FindMember(L"Заголовки"); headersJson != json.MemberEnd() && headersJson->value.IsObject()) {
		curl_slist* headers = nullptr;
		for (const auto& it : headersJson->value.GetObj()) {
			if (!it.value.IsString())
				continue;

			const wstring headerResult = format(L"{}: {}", it.name.GetString(), it.value.GetString());
			const auto tmp = gl_conv_wstring_to_string(headerResult);

			headers = curl_slist_append(headers, tmp.c_str());
		}

		curl_easy_setopt(eh, CURLOPT_HTTPHEADER, headers);
	}

	if (returnHeaders || itsDebug) {
		curl_easy_setopt(eh, CURLOPT_HEADERDATA, pointer);
		curl_easy_setopt(eh, CURLOPT_HEADERFUNCTION, +[](const char* ptr, const size_t size, const size_t nmemb, CurlRequestInfo* request) -> size_t {
			return request->writeHeaders(ptr, size * nmemb);
		});
	}

	// URL
	if (!url.empty()) {
		const auto tmp = gl_conv_wstring_to_string(url);
		curl_easy_setopt(eh, CURLOPT_URL, tmp.c_str());
	}

	// Тип запроса
	if (itsPost)
		curl_easy_setopt(eh, CURLOPT_POST, 1L);
	else if (ranges::equal(type, L"GET"))
		curl_easy_setopt(eh, CURLOPT_HTTPGET, 1L);
	else if (!type.empty()) {
		const auto tmp = gl_conv_wstring_to_string(type);
		curl_easy_setopt(eh, CURLOPT_CUSTOMREQUEST, tmp.c_str());
	}

	// Имя пользователя / пароль
	wstring wUserName;
	copyValueStringFromSrc(wUserName, json, L"ИмяПользователя");

	if (!wUserName.empty()) {
		userName = gl_conv_wstring_to_string(wUserName);
		curl_easy_setopt(eh, CURLOPT_USERNAME, userName.c_str());

		wstring wUserPassword;
		copyValueStringFromSrc(wUserName, json, L"Пароль");

		if (!wUserPassword.empty()) {
			bool needDecryptPass = false;

			if (const auto objDecryptPass = json.FindMember(L"ПарольЗашифрован"); objDecryptPass != json.MemberEnd() && objDecryptPass->value.IsBool())
				needDecryptPass = objDecryptPass->value.GetBool();

			if (needDecryptPass) {
				wstring wKeyDecrypt;
				copyValueStringFromSrc(wKeyDecrypt, json, L"КлючШифрованияПароля");

				wUserPassword = kuz::Kuznechik(wKeyDecrypt).decrypt(wUserPassword);
			}
		}

		if (!wUserPassword.empty()) {
			password = gl_conv_wstring_to_string(wUserPassword);
			curl_easy_setopt(eh, CURLOPT_PASSWORD, password.c_str());
		}
	}

	// Debug информация
	if (itsDebug) {
		curl_easy_setopt(eh, CURLOPT_VERBOSE, 1L);

		curl_easy_setopt(eh, CURLOPT_DEBUGDATA, pointer);
		curl_easy_setopt(eh, CURLOPT_DEBUGFUNCTION,
		                 +[](const CURL* handle, const curl_infotype type, const char* data, const size_t size, CurlRequestInfo* request) -> size_t {
			                 return request->writeDebugInfo(type, data, size);
		                 });
	}
}

CurlRequestInfo::~CurlRequestInfo() {
	if (fromFile && streamIf.is_open())
		streamIf.close();
	if (toFile && streamOf.is_open())
		streamOf.close();

	curl_easy_cleanup(eh);
}

size_t CurlRequestInfo::writeResult(const char* data, const size_t size) {
	if (toFile)
		streamOf.write(data, static_cast<long long>(size));
	else
		bodyResult.append(data, size);

	return size;
}

size_t CurlRequestInfo::writeHeaders(const char* data, const size_t size) {
	auto str = string(data, size);
	str = str.substr(0, str.size() - 2);

	if (!str.empty()) {
		if (const auto delim = str.find(':'); delim != string::npos) {
			const auto key = gl_conv_string_to_wstring(str.substr(0, delim));
			const auto value = gl_conv_string_to_wstring(str.substr(delim + 2));

			headersResult[key] = value;
		}
	}

	return size;
}

int CurlRequestInfo::writeDebugInfo(const curl_infotype type, const char* data, const size_t size) {
	switch (type) {
		case CURLINFO_TEXT: {
			debugInfo_Text.append(data, size);
			return 0;
		}
		case CURLINFO_HEADER_IN: {
			debugInfo_HeaderIn.append(data, size);
			return 0;
		}
		case CURLINFO_HEADER_OUT: {
			debugInfo_HeaderOut.append(data, size);
			return 0;
		}
		case CURLINFO_SSL_DATA_IN: {
			debugInfo_SSLDataIn.append(data, size);
			return 0;
		}
		case CURLINFO_SSL_DATA_OUT: {
			debugInfo_SSLDataOut.append(data, size);
			return 0;
		}
		case CURLINFO_DATA_IN: {
			debugInfo_DataIn.append(data, size);
			return 0;
		}
		case CURLINFO_DATA_OUT: {
			debugInfo_DataOut.append(data, size);
			return 0;
		}
		default:
			return 0;
	}
}

size_t CurlRequestInfo::readDataFromFile(char* data, const size_t size) {
	return static_cast<size_t>(streamIf.read(data, size).gcount());
}

json_value CurlRequestInfo::createResultJSON(const CURLcode codeResult, rapidjson::MemoryPoolAllocator<>& allocator) {
	json_value objResult(rapidjson::kObjectType);

	long code = 0;
	curl_easy_getinfo(eh, CURLINFO_RESPONSE_CODE, &code);

	if (codeResult != CURLE_OK) {
		if (toFile)
			streamOf << curl_easy_strerror(codeResult);
		else {
			const auto error = curl_easy_strerror(codeResult);
			bodyResult.append(error);
		}
	}

	objResult.AddMember(L"ИдентификаторЗапроса", id, allocator);
	objResult.AddMember(L"КодОтвета", static_cast<unsigned>(code), allocator);
	objResult.AddMember(L"Успешно", code >= 200 && code < 300 && codeResult == CURLE_OK, allocator);

	if (toFile)
		objResult.AddMember(L"ФайлРезультат", outputFileName, allocator);
	else
		objResult.AddMember(L"ТелоРезультат", gl_conv_string_to_wstring(bodyResult), allocator);

	if (!headersResult.empty()) {
		json_value objHeaders(rapidjson::kObjectType);

		for (auto const& [key, value] : headersResult) {
			json_value keyJson(key, allocator);
			json_value valueJson(value, allocator);

			objHeaders.AddMember(keyJson, valueJson, allocator);
		}

		objResult.AddMember(L"ЗаголовкиРезультат", objHeaders, allocator);
	}

	if (itsDebug) {
		json_value objDebug(rapidjson::kObjectType);

		objDebug.AddMember(L"URL", url, allocator);
		objDebug.AddMember(L"ТипЗапроса", type, allocator);
		objDebug.AddMember(L"ОбщаяИнформация", gl_conv_string_to_wstring(debugInfo_Text), allocator);
		objDebug.AddMember(L"ОтправленныеДанные", gl_conv_string_to_wstring(debugInfo_DataOut), allocator);
		objDebug.AddMember(L"ПолученныеЗаголовки", gl_conv_string_to_wstring(debugInfo_HeaderIn), allocator);
		objDebug.AddMember(L"ОтправленныеДанныеПоSSL", gl_conv_string_to_wstring(debugInfo_SSLDataOut), allocator);
		objDebug.AddMember(L"ПолученныеДанныеПоSSL", gl_conv_string_to_wstring(debugInfo_SSLDataIn), allocator);

		objResult.AddMember(L"ОтладочнаяИнформация", objDebug, allocator);
	}

	return objResult;
}

void CurlRequestInfo::copyValueStringFromSrc(wstring& dst, const json_value& src, const wchar_t* name, const wchar_t* defaultValue) {
	if (const auto obj = src.FindMember(name); obj != src.MemberEnd() && obj->value.IsString())
		dst = wstring(obj->value.GetString(), obj->value.GetStringLength());
	else if (const auto size = static_cast<rapidjson::SizeType>(wcslen(defaultValue)))
		dst = wstring(defaultValue, size);
}

#pragma endregion

#pragma region HttpClient

HttpClient::HttpClient() {
	curl_global_sslset(CURLSSLBACKEND_OPENSSL, nullptr, nullptr);
	if (const auto success = curl_global_init(CURL_GLOBAL_ALL); success != CURLE_OK)
		BOOST_THROW_EXCEPTION(runtime_error("Не удалось инициализировать CURL"));

	magic_enum::enum_for_each<CURLMoption>([&](auto val) {
		if (val != CURLMOPT_LASTENTRY)
			p_MultiOptions[val] = monostate();
	});

	auto* opt = curl_easy_option_next(nullptr);
	while (opt) {
		switch (opt->id) {
			case CURLOPT_VERBOSE:
			case CURLOPT_READDATA:
			case CURLOPT_READFUNCTION:
			case CURLOPT_UPLOAD:
			case CURLOPT_INFILESIZE_LARGE:
			case CURLOPT_INFILESIZE:
			case CURLOPT_POSTFIELDS:
			case CURLOPT_POSTFIELDSIZE_LARGE:
			case CURLOPT_POSTFIELDSIZE:
			case CURLOPT_WRITEDATA:
			case CURLOPT_WRITEFUNCTION:
			case CURLOPT_HTTPHEADER:
			case CURLOPT_HEADERFUNCTION:
			case CURLOPT_URL:
			case CURLOPT_POST:
			case CURLOPT_HTTPGET:
			case CURLOPT_CUSTOMREQUEST:
			case CURLOPT_USERNAME:
			case CURLOPT_PASSWORD:
			case CURLOPT_DEBUGDATA:
			case CURLOPT_DEBUGFUNCTION:
			case CURLOPT_NOSIGNAL:
			case CURLOPT_TIMEOUT:
				break;
			default: {
				switch (opt->type) {
					case CURLOT_LONG:
					case CURLOT_STRING:
					case CURLOT_VALUES: {
						g_SingleOptions[opt->id] = monostate();
						break;
					}
					default:
						break;
				}
			}
		}

		opt = curl_easy_option_next(opt);
	}

	// Значения по-умолчанию
	p_MultiOptions[CURLMOPT_MAXCONNECTS] = 16L;

	g_SingleOptions[CURLOPT_VERBOSE] = debuggingMode;
	g_SingleOptions[CURLOPT_BUFFERSIZE] = 262144L;
	g_SingleOptions[CURLOPT_UPLOAD_BUFFERSIZE] = 262144L;
	g_SingleOptions[CURLOPT_ACCEPT_ENCODING] = L"";
	g_SingleOptions[CURLOPT_TIMEOUT_MS] = 30000L;
	g_SingleOptions[CURLOPT_FOLLOWLOCATION] = 1L;
	g_SingleOptions[CURLOPT_USERAGENT] = L"LykovAddInNative";
	g_SingleOptions[CURLOPT_NOSIGNAL] = 1L;
	g_SingleOptions[CURLOPT_COOKIEFILE] = L"";

	p_Cm = curl_multi_init();
	setOptMulti(p_Cm);

	p_Sh = curl_share_init();
}

HttpClient::~HttpClient() {
	curl_share_cleanup(p_Sh);
	curl_multi_cleanup(p_Cm);

	clearAllActiveRequests(p_ActiveRequests);
	curl_global_cleanup();
}

void HttpClient::setOptMulti(CURLM* localCM) {
	vector<string> errors;

	for (auto const& [key, value] : p_MultiOptions) {
		visit(Overloaded{
			      [&](const wstring& val) {
				      const auto str = gl_conv_wstring_to_string(val);
				      if (const auto result = curl_multi_setopt(localCM, key, str.c_str()); result != CURLM_OK) {
					      const auto message = format("Не удалось установить значение параметра '{}' для multi: {}", magic_enum::enum_name(key),
					                                  curl_multi_strerror(result));
					      errors.push_back(message);

					      p_MultiOptions[key] = monostate();
				      }
			      },

			      [&](const long val) {
				      if (const auto result = curl_multi_setopt(localCM, key, val); result != CURLM_OK) {
					      const auto message = format("Не удалось установить значение параметра '{}' для multi: {}", magic_enum::enum_name(key),
					                                  curl_multi_strerror(result));
					      errors.push_back(message);

					      p_MultiOptions[key] = monostate();
				      }
			      },
			      [](auto) {
			      }
		      }, value);
	}

	if (!errors.empty()) {
		const string str = accumulate(errors.begin(), errors.end(), string("\n"));
		log1C(plog::error, gl_conv_string_to_wstring(str));
	}
}

void HttpClient::setupEasyCurl(CURL* curl) {
	vector<string> errors;

	for (auto const& [key, value] : g_SingleOptions) {
		visit(Overloaded{
			      [&](const wstring& val) {
				      const auto str = gl_conv_wstring_to_string(val);
				      if (const auto result = curl_easy_setopt(curl, key, str.c_str()); result != CURLE_OK) {
					      const auto message = format("Не удалось установить значение параметра '{}' для запроса CURL (параметр будет установлен по-умолчанию): {}",
					                                  curl_easy_option_by_id(key)->name, curl_easy_strerror(result));
					      errors.push_back(message);
					      g_SingleOptions[key] = monostate();
				      }
			      },

			      [&](const long val) {
				      if (const auto result = curl_easy_setopt(curl, key, val); result != CURLE_OK) {
					      const auto message = format("Не удалось установить значение параметра '{}' для запроса CURL (параметр будет установлен по-умолчанию): {}",
					                                  curl_easy_option_by_id(key)->name, curl_easy_strerror(result));
					      errors.push_back(message);
					      g_SingleOptions[key] = monostate();
				      }
			      },
			      [](auto) {
			      }
		      }, value);
	}

	if (const auto result = curl_easy_setopt(curl, CURLOPT_SHARE, p_Sh); result != CURLE_OK) {
		const auto message = format("Не удалось установить значение параметра '{}' для запроса CURL: {}", curl_easy_option_by_id(CURLOPT_SHARE)->name,
		                            curl_easy_strerror(result));
		errors.push_back(message);
	}

	if (!errors.empty()) {
		const string str = accumulate(errors.begin(), errors.end(), string("\n"));
		log1C(plog::warning, gl_conv_string_to_wstring(str));
	}
}

bool HttpClient::registerRequests(const json_document& json, CURLM* localCM, unordered_map<CURL*, CurlRequestInfo*>& localActiveRequests) {
	vector<CurlRequestInfo*> requests;

	if (json.IsArray()) {
		for (const auto& it : json.GetArray()) {
			p_Mutex.lock();

			const auto request = new CurlRequestInfo(it, p_ReturnHeaders, debuggingMode);
			setupEasyCurl(request->eh);

			p_Mutex.unlock();

			log1C(plog::info, format(L"Инициализирован запрос '{}'", request->id));
			requests.push_back(request);
		}
	}
	else if (json.IsObject()) {
		p_Mutex.lock();

		const auto request = new CurlRequestInfo(json, p_ReturnHeaders, debuggingMode);
		setupEasyCurl(request->eh);

		p_Mutex.unlock();

		log1C(plog::info, format(L"Инициализирован запрос '{}'", request->id));
		requests.push_back(request);
	}

	if (requests.empty())
		return false;

	for (const auto request : requests) {
		if (const auto code = curl_multi_add_handle(localCM, request->eh); code != CURLM_OK) {
			log1C(plog::error, format(L"Не удалось зарегистрировать запрос '{}': '{}'", request->id, gl_conv_string_to_wstring(curl_multi_strerror(code))));
			delete request;
		}
		else
			localActiveRequests[request->eh] = request;
	}

	return true;
}

json_document HttpClient::sendRequestSync(const json_value& requestJson) {
	json_document resultJson(rapidjson::kObjectType);

	sendRequestSync(requestJson, resultJson, resultJson.GetAllocator());
	return resultJson;
}

void HttpClient::sendRequestSync(const json_value& requestJson, json_value& resultJson, rapidjson::MemoryPoolAllocator<>& allocator) {
	const auto request = new CurlRequestInfo(requestJson, p_ReturnHeaders, debuggingMode);
	setupEasyCurl(request->eh);

	log1C(plog::info, format(L"Выполняется запрос '{}'", request->id));
	const auto resultRequest = curl_easy_perform(request->eh);
	log1C(plog::info, format(L"Получен результат запроса '{}'", request->id));

	const auto resultRequestJson = request->createResultJSON(resultRequest, allocator);

	resultJson.CopyFrom(resultRequestJson, allocator);
	delete request;
}

void HttpClient::sendRequestsSync_Thread(const json_document& requestJson) {
	log1C(plog::debug, L"Инициализирован отдельный поток для выполнения запросов");
	const auto basicStringError = L"Не удалось выполнить запросы в отдельном потоке";

	try {
		if (requestJson.IsObject()) {
			const auto result = sendRequestSync(requestJson);
			initExternalEvent(result);
		}
		else {
			unordered_map<CURL*, CurlRequestInfo*> localActiveRequests;

			CURLM* localCm = curl_multi_init();
			setOptMulti(localCm);

			registerRequests(requestJson, localCm, localActiveRequests);
			getResultsRequests(localCm, localActiveRequests, true);

			curl_multi_cleanup(localCm);
			clearAllActiveRequests(localActiveRequests);
		}
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const string& e) {
		log1C(plog::error, format(L"{} <string>!\n{}", basicStringError, gl_conv_string_to_wstring(e)));
	}
	catch (const char*& e) {
		log1C(plog::error, format(L"{} <string>!\n{}", basicStringError, gl_conv_string_to_wstring(e)));
	}
	catch (const wstring& e) {
		log1C(plog::error, format(L"{} <wstring>!\n{}", basicStringError, e));
	}
	catch (const wchar_t*& e) {
		log1C(plog::error, format(L"{} <wstring>!\n{}", basicStringError, e));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	p_Mutex.lock();
	p_ActiveThreads.erase(this_thread::get_id());
	p_Mutex.unlock();
}

json_document HttpClient::getResultsRequests(CURLM* localCM, unordered_map<CURL*, CurlRequestInfo*>& localActiveRequests, const bool initExternalEvents) {
	CURLMsg* msg;

	auto stillAlive = 1;
	auto tmp = -1;

	json_document resultJson(rapidjson::kArrayType);
	if (localActiveRequests.empty())
		return resultJson;

	auto& allocator = resultJson.GetAllocator();

	do {
		if (const auto code = curl_multi_perform(localCM, &stillAlive); code != CURLM_OK) {
			clearAllActiveRequests(localActiveRequests);
			BOOST_THROW_EXCEPTION(runtime_error(format("Не удалось получить результаты зарегистрированных запросов CURL: '{}'", curl_multi_strerror(code))));
		}

		while ((msg = curl_multi_info_read(localCM, &tmp))) {
			if (msg->msg != CURLMSG_DONE)
				continue;

			const auto currentRequest = localActiveRequests[msg->easy_handle];
			if (!currentRequest || currentRequest->eh == nullptr)
				continue;

			log1C(plog::info, format(L"Получен результат запроса по '{}'", currentRequest->id));

			auto resultRequestJson = currentRequest->createResultJSON(msg->data.result, allocator);
			if (initExternalEvents)
				initExternalEvent(resultRequestJson);
			else
				resultJson.PushBack(resultRequestJson, allocator);

			curl_multi_remove_handle(localCM, msg->easy_handle);
			localActiveRequests.erase(msg->easy_handle);

			delete currentRequest;
		}

		if (stillAlive)
			// CURLM_UNRECOVERABLE_POLL - Странная ошибка.
			if (const auto code = curl_multi_wait(localCM, nullptr, 0, 1000, nullptr); code != CURLM_OK && code != CURLM_UNRECOVERABLE_POLL) {
				clearAllActiveRequests(localActiveRequests);
				BOOST_THROW_EXCEPTION(
					runtime_error(format("Не удалось получить результаты зарегистрированных запросов CURL (curl_multi_perform): '{}'", curl_multi_strerror(code))));
			}
	}
	while (stillAlive);

	return resultJson;
}

void HttpClient::initExternalEvent(const json_value& resultJson) {
	const auto id = resultJson[L"ИдентификаторЗапроса"].GetString();

	json_document tmp;
	tmp.CopyFrom(resultJson, tmp.GetAllocator());

	const auto result = getStringFromJSON(tmp);

	externalEvent(L"HTTPRequestsResult", id, result);
}

void HttpClient::clearAllActiveRequests(unordered_map<CURL*, CurlRequestInfo*>& localActiveRequests) {
	for (const auto val : localActiveRequests | views::values)
		delete val;

	localActiveRequests.clear();
}

#pragma endregion

#pragma endregion
