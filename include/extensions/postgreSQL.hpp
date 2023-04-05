#ifndef __LYKOV__POSTGRE_SQL_HPP__
#define __LYKOV__POSTGRE_SQL_HPP__

#include <baseClass/baseExtensionClass.hpp>

#include <functional>
#include <mutex>
#include <curl/curl.h>

#include <pqxx/pqxx>

#ifdef __linux__
	#include <csignal>
#endif

constexpr unsigned short gl_Type_Column_PostgreSQL_OID        = 0;
constexpr unsigned short gl_Type_Column_PostgreSQL_Int16      = 1;
constexpr unsigned short gl_Type_Column_PostgreSQL_Int32      = 2;
constexpr unsigned short gl_Type_Column_PostgreSQL_Int64      = 3;
constexpr unsigned short gl_Type_Column_PostgreSQL_Float      = 4;
constexpr unsigned short gl_Type_Column_PostgreSQL_Double     = 5;
constexpr unsigned short gl_Type_Column_PostgreSQL_LongDouble = 6;
constexpr unsigned short gl_Type_Column_PostgreSQL_Boolean    = 7;

constexpr unsigned short gl_Index_Prop_Connection_Established      = gl_Index_Last_Prop + 1;
constexpr unsigned short gl_Index_Prop_Connection_Host             = gl_Index_Last_Prop + 2;
constexpr unsigned short gl_Index_Prop_Connection_Db_Name          = gl_Index_Last_Prop + 3;
constexpr unsigned short gl_Index_Prop_Connection_Username         = gl_Index_Last_Prop + 4;
constexpr unsigned short gl_Index_Prop_Connection_Port             = gl_Index_Last_Prop + 5;
constexpr unsigned short gl_Index_Prop_Connection_Backend_Pid      = gl_Index_Last_Prop + 6;
constexpr unsigned short gl_Index_Prop_Connection_Sock             = gl_Index_Last_Prop + 7;
constexpr unsigned short gl_Index_Prop_Connection_Protocol_Version = gl_Index_Last_Prop + 8;
constexpr unsigned short gl_Index_Prop_Connection_Server_Version   = gl_Index_Last_Prop + 9;
constexpr unsigned short gl_Index_Prop_Connection_Client_Encoding  = gl_Index_Last_Prop + 10;

constexpr unsigned short gl_Index_Method_Connect_Db                   = gl_Index_Last_Method + 1;
constexpr unsigned short gl_Index_Method_Exec                         = gl_Index_Last_Method + 2;
constexpr unsigned short gl_Index_Method_Init_Query                   = gl_Index_Last_Method + 3;
constexpr unsigned short gl_Index_Method_Get_Result_Initialized_Query = gl_Index_Last_Method + 4;
constexpr unsigned short gl_Index_Method_Initialize_1c                = gl_Index_Last_Method + 5;
constexpr unsigned short gl_Index_Method_Put_Data                     = gl_Index_Last_Method + 6;
constexpr unsigned short gl_Index_Method_Read_Data_From_Table         = gl_Index_Last_Method + 7;
constexpr unsigned short gl_Index_Method_Read_Data_From_Query         = gl_Index_Last_Method + 8;
constexpr unsigned short gl_Index_Method_Reload_Connection            = gl_Index_Last_Method + 9;

class PostgreSQL final : public IBaseExtensionClass
{
	std::mutex m_Mutex;

public:
	PostgreSQL();
	~PostgreSQL() override;

	// Переопределяемые методы
	const wchar_t* getNameExtension() override; // Должен возвращать фактическое наименование расширения
	const wchar_t* getVersion() override; // Должен возвращать номер версии компоненты
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
	struct CmpByCompareChar
	{
		bool operator()(const wchar_t* a, const wchar_t* b) const {
#ifdef __linux__
			return wcscasecmp(a, b) < 0;
#else
			return _wcsicmp(a, b) < 0;
#endif
		}
	};

	using ColumnTypesType = std::unordered_map<pqxx::oid, unsigned short>;

	pqxx::connection* m_Conn;
	pqxx::nontransaction* m_NonTransaction;
	ColumnTypesType m_ColumnsTypes;
	std::string m_ConnectionLine;

	std::map<const wchar_t*, std::thread*, CmpByCompareChar> m_ThreadsMap;
	std::map<const wchar_t*, std::string, CmpByCompareChar> m_ThreadResults;

	void connectToDb(const tVariant*); // Подключиться к БД
	void exec(const tVariant*, tVariant*) const; // Выполнить запрос
	void initQuery(const tVariant*); // Начать выполнение запроса
	void getResultsInitializedQueries(const tVariant*, tVariant*); // Получить результаты зарегистрированных запросов
	void initialize1C() const; // Инициализировать функции для 1С
	void putCopyIn(const tVariant*) const; // Прочитать данные Copy In
	void readDataFromTable(const tVariant*, tVariant*) const; // Прочитать данные из таблицы
	void readDataFromQuery(const tVariant*, tVariant*) const; // Прочитать данные из запроса
	void reloadConnection(); // Перезапустить соединение

	// Вспомогательные функции
	[[nodiscard]] bool connectionEstablished() const; // Соединение стабильно
	void closeConnection(); // Закрыть все соединения
	void initQuery_Thread(const char*, const wchar_t*, bool, int, const wchar_t*); // Поток для отправки запроса

	static std::unordered_map<const char*, short> getTypesOfColumns(const pqxx::row&, const ColumnTypesType&);
	static std::wstring arrayResultToJson(const pqxx::result&, const ColumnTypesType&, const std::wstring&, bool = false, size_t = -1);
	static std::wstring rowResultToJson(const pqxx::row&, const ColumnTypesType&, const std::wstring&, bool = false);

	template <typename type>
	static void arrayResultToJson_WriteRow(const pqxx::row&, const std::unordered_map<const char*, short>&, rapidjson::Writer<type>&);
	template <typename type>
	static void streamResultToJson(pqxx::stream_from&, rapidjson::Writer<type>&);

	void clearAllThreads();
};

#endif
