
#ifndef __HTTP_CLIENT_HPP__
#define __HTTP_CLIENT_HPP__

#include "baseClass/baseClassForExtension.hpp"
#include "baseClass/encrypt.hpp"

#include <ranges>
#include <shared_mutex>

#include <curl/curl.h>
#include <magic_enum/magic_enum_all.hpp>

constexpr curl_off_t limit_large_body = 2147483648L;
typedef std::variant<std::wstring, long, std::monostate> curl_all_types;

class CurlRequestInfo final {
public:
	CurlRequestInfo(const native1C::json_value&, bool, bool);
	~CurlRequestInfo();
	BOOST_DELETED_FUNCTION(CurlRequestInfo(CurlRequestInfo&&) BOOST_NOEXCEPT)
	BOOST_DELETED_FUNCTION(CurlRequestInfo(const CurlRequestInfo&))
	BOOST_DELETED_FUNCTION(CurlRequestInfo& operator=(const CurlRequestInfo&))
	BOOST_DELETED_FUNCTION(CurlRequestInfo& operator=(CurlRequestInfo&&))

	CURL* eh;
	std::wstring id; // ID запроса

	size_t writeResult(const char*, size_t size);
	size_t writeHeaders(const char*, size_t size);
	int writeDebugInfo(curl_infotype, const char*, size_t);
	size_t readDataFromFile(char*, size_t size);

	native1C::json_value createResultJSON(CURLcode, rapidjson::MemoryPoolAllocator<>&);


private:
	std::wstring url; // URL запроса
	std::wstring type; // Метод запроса

	std::wstring inputFileName; // Имя файла тела запроса
	std::string body; // Тело запроса
	std::ifstream streamIf; // Файл тела запроса
	bool fromFile = false;

	std::wstring outputFileName; // Имя файла тела результата
	std::string bodyResult; // Тело результат
	std::ofstream streamOf; // Файл результат
	bool toFile = false;

	std::unordered_map<std::wstring, std::wstring> headersResult; // Заголовки ответа

	std::string userName; // Имя пользователя
	std::string password; // Пароль пользователя

	// debug
	bool itsDebug = false;

	std::string debugInfo_Text;
	std::string debugInfo_HeaderIn;
	std::string debugInfo_HeaderOut;
	std::string debugInfo_DataIn;
	std::string debugInfo_DataOut;
	std::string debugInfo_SSLDataIn;
	std::string debugInfo_SSLDataOut;

	static void copyValueStringFromSrc(std::wstring&, const native1C::json_value&, const wchar_t*, const wchar_t* = L"");
};

class HttpClient final : public native1C::IBaseExtensionClass {
public:
	HttpClient();
	~HttpClient() BOOST_OVERRIDE;
	BOOST_DELETED_FUNCTION(HttpClient(HttpClient&&) BOOST_NOEXCEPT)
	BOOST_DELETED_FUNCTION(HttpClient(const HttpClient&))
	BOOST_DELETED_FUNCTION(HttpClient& operator=(const HttpClient&))
	BOOST_DELETED_FUNCTION(HttpClient& operator=(HttpClient&&))


protected:
	virtual std::wstring getNameExtension() BOOST_OVERRIDE;
	virtual void initializeComponent() BOOST_OVERRIDE;


private:
	std::mutex p_Mutex;
	CURLM* p_Cm; // multi
	CURLSH* p_Sh; // Общий ресурс
	bool p_ReturnHeaders = false;

	std::map<CURLoption, curl_all_types> g_SingleOptions;
	std::map<CURLMoption, curl_all_types> p_MultiOptions;
	std::unordered_map<CURL*, CurlRequestInfo*> p_ActiveRequests;
	std::map<std::jthread::id, std::jthread*> p_ActiveThreads;

	// Методы компоненты
	native1C::all_returned_types sendRequestsAsync(const native1C::all_input_types&);
	native1C::all_returned_types sendRequestsSync(const native1C::all_input_types&, const native1C::all_input_types&);
	void registerRequestsForExecution(const native1C::all_input_types&, const native1C::all_input_types&);
	native1C::all_returned_types getResultsRegisteredRequests();

	// Вспомогательные функции
	void setOptMulti(CURLM*);
	void setupEasyCurl(CURL*);

	bool registerRequests(const native1C::json_document&, CURLM*, std::unordered_map<CURL*, CurlRequestInfo*>&);
	native1C::json_document getResultsRequests(CURLM*, std::unordered_map<CURL*, CurlRequestInfo*>&, bool = false);
	native1C::json_document sendRequestSync(const native1C::json_value&);
	void sendRequestSync(const native1C::json_value&, native1C::json_value&, rapidjson::MemoryPoolAllocator<>&);
	void sendRequestsSync_Thread(const native1C::json_document&);
	void initExternalEvent(const native1C::json_value&);

	static void clearAllActiveRequests(std::unordered_map<CURL*, CurlRequestInfo*>&);
};

#endif
