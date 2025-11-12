
#ifndef __POSTGRESQL_HPP__
#define __POSTGRESQL_HPP__

#ifdef __linux__
	#include <signal.h>
#endif

#include "baseClass/baseClassForExtension.hpp"
#include "baseClass/encrypt.hpp"

#include <future>
#include <pqxx/pqxx>

class PostgreSQL final : public native1C::IBaseExtensionClass {
	
public:
	PostgreSQL();
	~PostgreSQL() BOOST_OVERRIDE;
	BOOST_DELETED_FUNCTION(PostgreSQL(PostgreSQL&&) BOOST_NOEXCEPT)
	BOOST_DELETED_FUNCTION(PostgreSQL(const PostgreSQL&))
	BOOST_DELETED_FUNCTION(PostgreSQL& operator=(const PostgreSQL&))
	BOOST_DELETED_FUNCTION(PostgreSQL& operator=(PostgreSQL&&))

protected:
	virtual std::wstring getNameExtension() BOOST_OVERRIDE; // Возвращает фактическое наименование расширения.
	virtual void initializeComponent() BOOST_OVERRIDE; // Возвращает фактическое наименование расширения.

private:
	std::mutex p_Mutex;

	enum class p_column_types_for_json {
		t_uint,
		t_int,
		t_int64,
		t_double,
		t_bool,
		t_string
	};

	struct P_ColumnsTypes {
		p_column_types_for_json basicType;
		bool isArray;

		BOOST_DEFAULTED_FUNCTION(P_ColumnsTypes(), {})
		BOOST_DEFAULTED_FUNCTION(~P_ColumnsTypes(), {})
		BOOST_DEFAULTED_FUNCTION(P_ColumnsTypes(const P_ColumnsTypes&), {})
		BOOST_DEFAULTED_FUNCTION(P_ColumnsTypes(P_ColumnsTypes&&) BOOST_NOEXCEPT, {})
		BOOST_DEFAULTED_FUNCTION(P_ColumnsTypes& operator=(const P_ColumnsTypes&), {})
		BOOST_DEFAULTED_FUNCTION(P_ColumnsTypes& operator=(P_ColumnsTypes&&), {})

		P_ColumnsTypes(const p_column_types_for_json basicTypeInput, const bool isArrayInput) : basicType(basicTypeInput), isArray(isArrayInput) { }
	};

	pqxx::connection* p_Conn;
	pqxx::nontransaction* p_NonTransaction;
	std::string p_ConnectionLine;
	std::map<pqxx::oid, P_ColumnsTypes> p_ColumnsTypes;
	std::map<std::wstring, std::future<std::wstring>> p_ActiveThreads;

	// Методы компоненты
	void connectToDb(const native1C::all_input_types&, const native1C::all_input_types&);
	std::wstring exec(const native1C::all_input_types&, const native1C::all_input_types&, const native1C::all_input_types&);
	void initExec(const native1C::all_input_types&, const native1C::all_input_types&, const native1C::all_input_types&, const native1C::all_input_types&,
	              const native1C::all_input_types&);
	std::wstring getResultsInitializedQueries(const native1C::all_input_types&);
	void putDataInTable(const native1C::all_input_types&, const native1C::all_input_types&, const native1C::all_input_types&, const native1C::all_input_types&);
	void reloadConnection();

	// Вспомогательные методы
	BOOST_ATTRIBUTE_NODISCARD bool connectionEstablished() const;
	void setupConnection();
	void closeAllConnection(bool);
	void setColumnTypes();

	std::wstring writeResultToJson(const pqxx::result&, const std::wstring&, int);

	template <typename Type>
	static void writeRowResultToWriter(Type&, const pqxx::row&, const std::map<std::string, P_ColumnsTypes>&);

	template <typename Type>
	static void writeFieldToWriter(Type&, const pqxx::field&, const P_ColumnsTypes&);
};

#endif
