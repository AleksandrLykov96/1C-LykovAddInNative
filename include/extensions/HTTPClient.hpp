#ifndef __HTTP_CLIENT_HPP__
#define __HTTP_CLIENT_HPP__

#include <baseClass/baseExtensionClass.hpp>

#include <set>
#include <list>
#include <curl/curl.h>

constexpr unsigned short gl_Index_Prop_Max_Connects          = 5;
constexpr unsigned short gl_Index_Prop_Max_Concurrent_Stream = 6;
constexpr unsigned short gl_Index_Prop_Max_Host_Connection   = 7;
constexpr unsigned short gl_Index_Prop_Max_Total_Connection  = 8;
constexpr unsigned short gl_Index_Prop_Enable_Debug          = 9;
constexpr unsigned short gl_Index_Prop_Enable_Headers        = 10;
constexpr unsigned short gl_Index_Prop_Enable_Cookies        = 11;
constexpr unsigned short gl_Index_Prop_TimeoutMultiWait      = 12;

constexpr unsigned short gl_Index_Method_Send_Requests_Async             = 0;
constexpr unsigned short gl_Index_Method_Send_Requests_Sync              = 1;
constexpr unsigned short gl_Index_Method_Register_Requests_For_Execution = 2;
constexpr unsigned short gl_Index_Method_Get_Results_Registered_Requests = 3;

constexpr unsigned int gl_Limit_Multi_Iterations = 65536;
constexpr curl_off_t gl_Limit_Large_Body = 2147483648;

class HttpClient final : public IBaseExtensionClass {
	std::mutex m_Mutex;

public:
	struct RequestsStruct {
		CURL* eh; // Запрос
		const char* id; // Идентификатор запроса
		const char* url; // URL запроса
		const char* type; // Метод запроса

		bool fromFile = false;
		curl_off_t fromFileSize = 0;
		std::ifstream streamIf; // Файл тела запроса

		bool toFile = false;
		std::ofstream streamOf; // Файл результат
		std::string bodyResult = ""; // Тело результат

		rapidjson::SizeType bodyResultSize = 0;

		const char* fileNameResult = "";
		rapidjson::SizeType fileNameResultSize = 0;

		std::unordered_map<std::string, std::string> headersResult; // Заголовки ответа

		// debug
		bool itsDebug = false;
		
		std::string debugInfo_Text = "", debugInfo_HeaderIn = "", debugInfo_HeaderOut = "", debugInfo_DataIn = "", debugInfo_DataOut = "", debugInfo_SSLDataIn = "", debugInfo_SSLDataOut = ""; // Debug информация
		rapidjson::SizeType debugInfo_TextSize = 0, debugInfo_HeaderInSize = 0, debugInfo_HeaderOutSize = 0, debugInfo_DataInSize = 0, debugInfo_DataOutSize = 0, debugInfo_SSLDataInSize = 0, debugInfo_SSLDataOutSize = 0;

		RequestsStruct(CURL*, const char*, const char*, const char*, const char*, const char*, bool);
		~RequestsStruct();
	};

	HttpClient();
	~HttpClient() override;

	// Переопределяемые методы
	const wchar_t* getNameExtension() override; // Должен возвращать фактическое наименование расширения
	void setMethodPropsExtension() override; // Инициализация компоненты (дополнение методов и свойств компоненты)
	void getPropByIndex(unsigned short, tVariant*) override; // Должен положить значение параметра по индексу в tVariant или вызвать исключение (например, если свойство не найдено)
	void setPropByIndex(unsigned short, tVariant*) override; // Должен установить значение параметра по индексу в tVariant
	bool getIsPropWritable(unsigned short) override; // Должен вернуть доступность записи параметра по индексу
	long getMethodNParams(unsigned short) override; // Должен вернуть количество параметров метода
	void setParamDefValue(unsigned short, long, tVariant*) override; // Должен установить значения параметров по умолчанию
	bool getHasRetVal(unsigned short) override; // Должен вернуть признак наличия возвращаемого значения
	void callMethodAsProc(unsigned short, const tVariant*, long) override; // Вызвать нужную процедуру
	void callMethodAsFunc(unsigned short, tVariant*, const tVariant*, long) override; // Вызвать нужную функцию

private:
	CURLM* m_Cm; // multi
	CURLSH* m_Sh; // Общий ресурс

	// Работа с параметрами
	unsigned int m_MaxConnects = 64, m_MaxConcurrentStream = 256, m_MaxHostConnection = 0, m_MaxTotalConnection = 0, m_TimeoutMultiWait = 1000;
	bool m_EnableDebug = false, m_EnableHeaders = false, m_EnableCookies = false;

	struct EasyParamStruct {
		CURLoption id;
		const wchar_t* name;
		curl_easytype type;
		std::variant<std::string, long> value;
		bool setThisValue;

		EasyParamStruct(CURLoption, const char*, curl_easytype, bool);
		EasyParamStruct(const EasyParamStruct&);
		~EasyParamStruct();
	};

	std::map<unsigned short, EasyParamStruct*> m_CurrentEasyParams;
	std::set<unsigned short> m_ParametersForEasy;

	void setAllEasyParameters();

	// Работа с запросами
	std::unordered_map<CURL*, RequestsStruct*> m_RegisteredRequests;

	void sendRequestsSync(const tVariant*, tVariant*);
	void sendRequestsAsync(const tVariant*, tVariant*);
	void registerRequestsForExecution(const tVariant*);
	void getResultsRegisteredRequests(tVariant*);

	// Вспомогательные функции
	std::unordered_map<std::thread::id, std::thread*> m_ThreadsMap;

	CURL* createEasyCurl(); // Подготовить базовый запрос по параметрам
	RequestsStruct* createRequest(const rapidjson::Value::Object&);

	template <typename type>
	static void writeResultToJson(CURLcode, RequestsStruct*, rapidjson::Writer<type>&);
	template <typename type>
	type setMultiParam(CURLMoption, const wchar_t*, tVariant*);
	template <typename type>
	void createEasyCurl_SetParam(CURL*, CURLoption, type, const wchar_t*, RequestsStruct* = nullptr);

	void sendRequestsSync_DoIt(tVariant*, const rapidjson::Document&, unsigned = 0);
	void registerRequests(const rapidjson::Document&, CURLM*, std::unordered_map<CURL*, RequestsStruct*>&);
	std::wstring getResultsRequests(CURLM*, std::unordered_map<CURL*, RequestsStruct*>&) const;
	void sendRequestsSync_Thread(std::string inputString);
	void clearAllThreads();
};

static size_t gl_WriteCallback_Body(const char*, size_t, size_t, HttpClient::RequestsStruct*);
static size_t gl_WriteCallback_Headers(const char*, size_t, size_t, HttpClient::RequestsStruct*);
static size_t gl_ReadCallback(char*, size_t, size_t, HttpClient::RequestsStruct*);
static int gl_DebugCallback(CURL*, curl_infotype, const char*, size_t, HttpClient::RequestsStruct*);

#endif
