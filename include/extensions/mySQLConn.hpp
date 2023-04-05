#ifndef __MY_SQL_CONN_HPP__
#define __MY_SQL_CONN_HPP__

#include <baseClass/baseExtensionClass.hpp>

#include <mysql/mysql.h>
#include <map>

constexpr unsigned short gl_Type_Column_MySQL_Int    = 0;
constexpr unsigned short gl_Type_Column_MySQL_Long   = 1;
constexpr unsigned short gl_Type_Column_MySQL_Double = 2;
constexpr unsigned short gl_Type_Column_MySQL_String = 3;

constexpr unsigned short gl_Index_Prop_MySQL_Connection_Info = gl_Index_Last_Prop + 1;

constexpr unsigned short gl_Index_Method_MySQL_Connect_Db = gl_Index_Last_Method + 1;
constexpr unsigned short gl_Index_Method_MySQL_Exec       = gl_Index_Last_Method + 2;

class MySqlConn final : public IBaseExtensionClass {
public:
	MySqlConn();
	~MySqlConn() override;

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
	struct ColumnInfo {
		const char* name;
		unsigned short type;
		unsigned nameLength;

		ColumnInfo() = default;
		ColumnInfo(const char* name, unsigned short type, unsigned nameLength) {
			this->name = name;
			this->type = type;
			this->nameLength = nameLength;
		}

		~ColumnInfo() = default;
	};

	MYSQL* conn;

	void connectToDb(const tVariant*) const; // Подключиться к БД
	void exec(const tVariant*, tVariant*) const; // Выполнить запрос
};

#endif
 