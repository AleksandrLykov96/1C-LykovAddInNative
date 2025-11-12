
#include "extensions/postgreSQL.hpp"

using namespace std;
using namespace native1C;
using namespace pqxx;

#pragma region Методы для переопределения

wstring PostgreSQL::getNameExtension() {
	return L"PostgreSQL";
}

void PostgreSQL::initializeComponent() {
	addProperty(L"СоединениеУстановлено",
	            L"connectionEstablished",
	            [this] {
		            return connectionEstablished();
	            });

	addProperty(L"Хост",
	            L"host",
	            [this] {
		            if (connectionEstablished())
			            return p_Conn->hostname();

		            return "";
	            });

	addProperty(L"НаименованиеБазы",
	            L"DBName",
	            [this] {
		            if (connectionEstablished())
			            return p_Conn->dbname();

		            return "";
	            });

	addProperty(L"ИмяПользователя",
	            L"username",
	            [this] {
		            if (connectionEstablished())
			            return p_Conn->username();

		            return "";
	            });

	addProperty(L"Порт",
	            L"port",
	            [this] {
		            if (connectionEstablished())
			            return p_Conn->port();

		            return "";
	            });

	addProperty(L"ИДПроцесса",
	            L"backendPID",
	            [this] {
		            if (connectionEstablished())
			            return p_Conn->backendpid();

		            return 0;
	            });

	addProperty(L"Сокет",
	            L"socket",
	            [this] {
		            if (connectionEstablished())
			            return p_Conn->sock();

		            return 0;
	            });

	addProperty(L"ВерсияПротокола",
	            L"protocolVersion",
	            [this] {
		            if (connectionEstablished())
			            return p_Conn->protocol_version();

		            return 0;
	            });

	addProperty(L"ВерсияСервера",
	            L"serverVersion",
	            [this] {
		            if (connectionEstablished())
			            return p_Conn->server_version();

		            return 0;
	            });

	addProperty(L"Кодировка",
	            L"encoding",
	            [this] {
		            if (connectionEstablished())
			            return p_Conn->get_client_encoding();

		            return string("");
	            });

	addMethod(L"УстановитьСоединение",
	          L"connectDB",
	          this,
	          &PostgreSQL::connectToDb,
	          {{1, false}, {2, L""}});

	addMethod(L"ВыполнитьЗапрос",
	          L"exec",
	          this,
	          &PostgreSQL::exec,
	          {{1, -1}, {2, L""} });

	addMethod(L"НачатьВыполнениеЗапроса",
	          L"initExec",
	          this,
	          &PostgreSQL::initExec,
	          {{1, L"default"}, {2, -1}, {3, L""}, {4, false}});

	addMethod(L"ПолучитьРезультатыЗарегистрированныхЗапросов",
	          L"getResultsInitializedQueries",
	          this,
	          &PostgreSQL::getResultsInitializedQueries,
	          {{0, L""}});

	addMethod(L"ВставитьДанныеВТаблицу",
	          L"putDataInTable",
	          this,
	          &PostgreSQL::putDataInTable,
	          {{3, false}});

	addMethod(L"СброситьСоединение",
	          L"reloadConnection",
	          this,
	          &PostgreSQL::reloadConnection);
}

#pragma endregion

#pragma region Методы компоненты

void PostgreSQL::connectToDb(const all_input_types& lineInput, const all_input_types& keyDecryptInput) {
	if (!holds_alternative<wstring>(lineInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка' (строка подключения к PostgreSQL)"));
	if (!holds_alternative<wstring>(keyDecryptInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №2 должен быть 'Строка' (ключ для дешифровки строки подключения)"));

	auto line = get<wstring>(lineInput);
	if (const auto keyForDecrypt = get<wstring>(keyDecryptInput); !keyForDecrypt.empty()) {
		line = kuz::Kuznechik(keyForDecrypt).decrypt(line);
	}

	closeAllConnection(false);

	p_ConnectionLine = gl_conv_wstring_to_string(line);
	setupConnection();
}

wstring PostgreSQL::exec(const all_input_types& requestInput, const all_input_types& countRowsInput, const all_input_types& fileNameResultInput) {
	if (!connectionEstablished())
		BOOST_THROW_EXCEPTION(runtime_error("Не установлено подключение к базе!"));
	if (!holds_alternative<wstring>(requestInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка' (текст запроса)"));
	if (!holds_alternative<int32_t>(countRowsInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №2 должен быть 'Число' (количество строк в результате запроса)"));
	if (!holds_alternative<wstring>(fileNameResultInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №3 должен быть 'Строка' (имя файла для записи результата)"));

	const auto wRequest = get<wstring>(requestInput);
	const auto request = gl_conv_wstring_to_string(wRequest);
	const auto countRows = get<int32_t>(countRowsInput);
	const auto fileNameResult = get<wstring>(fileNameResultInput);

	const auto result = p_NonTransaction->exec(request);
	return writeResultToJson(result, fileNameResult, countRows);
}

void PostgreSQL::initExec(const all_input_types& requestInput, const all_input_types& requestIdInput, const all_input_types& countRowsInput,
                          const all_input_types& fileNameResultInput, const all_input_types& initExternalInput) {
	if (!connectionEstablished())
		BOOST_THROW_EXCEPTION(runtime_error("Не установлено подключение к базе!"));
	if (!holds_alternative<wstring>(requestInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка' (текст запроса)"));
	if (!holds_alternative<wstring>(requestIdInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №2 должен быть 'Строка' (идентификатор запроса)"));
	if (!holds_alternative<int32_t>(countRowsInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №3 должен быть 'Число' (количество строк в результате запроса)"));
	if (!holds_alternative<wstring>(fileNameResultInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №4 должен быть 'Строка' (имя файла для записи результата)"));

	bool initExternal = false;
	if (!itsServer) {
		if (!holds_alternative<bool>(initExternalInput))
			BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №5 должен быть 'Булево' (инициализировать внешнее событие)"));

		initExternal = get<bool>(initExternalInput);
	}

	const auto wRequest = get<wstring>(requestInput);
	const auto request = gl_conv_wstring_to_string(wRequest);
	const auto idRequest = get<wstring>(requestIdInput);
	const auto countRows = get<int32_t>(countRowsInput);
	const auto fileNameResult = get<wstring>(fileNameResultInput);

	p_Mutex.lock();
	p_ActiveThreads[idRequest] = std::async(std::launch::async, [this, request, idRequest, initExternal, countRows, fileNameResult]() -> wstring {
		connection localConn(p_ConnectionLine);

		const auto result = nontransaction(localConn).exec(request);
		auto forReturn = writeResultToJson(result, fileNameResult, countRows);

		localConn.close();
		if (initExternal) {
			externalEvent(L"PostgreSQLRequest", idRequest, forReturn);
			return L"";
		}

		return forReturn;
	});
	p_Mutex.unlock();
}

wstring PostgreSQL::getResultsInitializedQueries(const all_input_types& requestIdInput) {
	if (!connectionEstablished())
		BOOST_THROW_EXCEPTION(runtime_error("Не установлено подключение к базе!"));
	if (!holds_alternative<wstring>(requestIdInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка' (идентификатор запроса для получения)"));

	if (p_ActiveThreads.empty())
		return L"[]";

	json_string_buffer json;
	json_writer writer(json);

	const auto idRequest = get<wstring>(requestIdInput);
	if (idRequest.empty()) {
		writer.StartArray();

		for (auto it = p_ActiveThreads.begin(); it != p_ActiveThreads.end();) {
			writer.Key(it->first.c_str(), static_cast<rapidjson::SizeType>(it->first.size()));
			
			const auto tmpJsonStr = it->second.get();

			if (const auto tmpJson = getInputJSON(tmpJsonStr); tmpJson.IsArray())
				writer.RawValue(tmpJsonStr.c_str(), static_cast<rapidjson::SizeType>(tmpJsonStr.size()), rapidjson::kArrayType);
			else if (tmpJson.IsObject())
				writer.RawValue(tmpJsonStr.c_str(), static_cast<rapidjson::SizeType>(tmpJsonStr.size()), rapidjson::kObjectType);
			else
				writer.RawValue(tmpJsonStr.c_str(), static_cast<rapidjson::SizeType>(tmpJsonStr.size()), rapidjson::kStringType);

			p_ActiveThreads.erase(it++);
		}

		writer.EndArray();
		return json.GetString();
	}

	const auto futureIt = p_ActiveThreads.find(idRequest);
	if (futureIt == p_ActiveThreads.end())
		BOOST_THROW_EXCEPTION(invalid_argument("Не найден запрос с указанным идентификатором"));

	const auto result = futureIt->second.get();
	p_ActiveThreads.erase(futureIt);

	return result;
}

void PostgreSQL::putDataInTable(const all_input_types& tableNameInput, const all_input_types& dataInput, const all_input_types& columnsNameInput,
                                const all_input_types& fromFileInput) {
	if (!connectionEstablished())
		BOOST_THROW_EXCEPTION(runtime_error("Не установлено подключение к базе!"));
	if (!holds_alternative<wstring>(tableNameInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка' (наименование таблицы для вставки данных)"));
	if (!holds_alternative<wstring>(dataInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №2 должен быть 'Строка' (данные для вставки JSON / имя файла)"));
	if (!holds_alternative<wstring>(columnsNameInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №3 должен быть 'Строка' (имена колонок)"));
	if (!holds_alternative<bool>(fromFileInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №4 должен быть 'Булево' (читать данные из файла)"));

	const auto wTableName = get<wstring>(tableNameInput);
	const auto tableName = gl_conv_wstring_to_string(wTableName);
	const auto data = get<wstring>(dataInput);
	const auto wColumnsNames = get<wstring>(columnsNameInput);
	const auto columnsNames = gl_conv_wstring_to_string(wColumnsNames);
	const auto fromFile = get<bool>(fromFileInput);

	json_document json;

	if (fromFile)
		json = readJSONFromFile(data);
	else
		json = getInputJSON(data);

	if (!json.IsArray())
		BOOST_THROW_EXCEPTION(invalid_argument("Пришёл некорректный JSON для передачи в PostgreSQL (должен быть массив массивов)"));

	stream_to stream = stream_to::raw_table(*p_NonTransaction, tableName, columnsNames);
	size_t countColumns = ranges::count(columnsNames, ',');

	std::vector<std::string> rowForStream;
	rowForStream.reserve(countColumns);

	for (const auto& arrayJson = json.GetArray(); const auto& row : arrayJson) {
		if (!row.IsArray())
			BOOST_THROW_EXCEPTION(invalid_argument("Пришёл некорректный JSON для передачи в PostgreSQL (строка массива не массив)"));

		for (const auto& field : row.GetArray()) {
			if (!field.IsString())
				BOOST_THROW_EXCEPTION(
				invalid_argument("Пришёл некорректный JSON для передачи в PostgreSQL (колонка строки не массив)! Должны быть только строковые значения."));

			const auto str = gl_conv_wstring_to_string(field.GetString());
			rowForStream.emplace_back(str);
		}

		stream.write_row(rowForStream);
		rowForStream.clear();
	}

	stream.complete();
}

void PostgreSQL::reloadConnection() {
	closeAllConnection(true);
	setupConnection();
}

#pragma endregion

#pragma region Вспомогательные методы

PostgreSQL::PostgreSQL() {
	p_Conn = nullptr;
	p_NonTransaction = nullptr;

#ifdef __linux__
	signal(SIGPIPE, SIG_IGN);
#endif
}

PostgreSQL::~PostgreSQL() {
	closeAllConnection(false);
}

bool PostgreSQL::connectionEstablished() const {
	return p_Conn && p_Conn->is_open();
}

void PostgreSQL::setColumnTypes() {
	
	const auto resBasicTypes = p_NonTransaction->exec(
		"SELECT oid AS oid,\n"
		"	typcategory AS typcategory,\n"
		"	typlen AS typLen,\n"
		"	typname AS typname\n"
		"FROM pg_type\n"
		"WHERE typcategory IN('B', 'N')");
	
	for (const auto& row : resBasicTypes) {
		const auto [oidValue, typCategory, typLen, nameType] = row.as<oid, string, int, string>();
		if (gl_str_iequal(typCategory, "B"))
			p_ColumnsTypes[oidValue] = P_ColumnsTypes { p_column_types_for_json::t_bool, false };
		else if (gl_str_iequal(typCategory, "N")) {
			if (gl_str_iequal(nameType, "oid"))
				p_ColumnsTypes[oidValue] = P_ColumnsTypes{ p_column_types_for_json::t_uint, false };
			else if (nameType.starts_with("int") || nameType.starts_with("_int")) {
				switch (typLen) {
					case 2:
					case 4:
						p_ColumnsTypes[oidValue] = P_ColumnsTypes{ p_column_types_for_json::t_int, false };
						break;
					default:
						p_ColumnsTypes[oidValue] = P_ColumnsTypes{ p_column_types_for_json::t_int64, false };
						break;
				}
			}
			else
				p_ColumnsTypes[oidValue] = P_ColumnsTypes{ p_column_types_for_json::t_double, false };
		}
	}

	// Массивы
	const auto resArrayTypes = p_NonTransaction->exec(
		"SELECT oid AS oid,\n"
		"	typelem AS typelem\n"
		"FROM pg_type\n"
		"WHERE typcategory = 'A'");

	for (const auto& row : resArrayTypes) {
		const auto [oidValue, typElem] = row.as<oid, oid>();

		if (const auto res = p_ColumnsTypes.find(typElem); res != p_ColumnsTypes.end())
			p_ColumnsTypes[oidValue] = P_ColumnsTypes{ res->second.basicType, true };
		else
			p_ColumnsTypes[oidValue] = P_ColumnsTypes{ p_column_types_for_json::t_string, true };
	}
}

wstring PostgreSQL::writeResultToJson(const result& resultPG, const wstring& fileName, const int countOfRows) {
	if (countOfRows == 0) {
		resultPG.no_rows();
		return L"";
	}

	map<string, P_ColumnsTypes> columnsTypes;
	for (row_size_type i = 0; i < resultPG.columns(); i++) {
		const auto columnOid = resultPG.column_type(i);
		const auto columnName = string(resultPG.column_name(i));
		
		if (const auto type = p_ColumnsTypes.find(columnOid); type != p_ColumnsTypes.end())
			columnsTypes[columnName] = type->second;
		else
			columnsTypes[columnName] = P_ColumnsTypes{ p_column_types_for_json::t_string, false };
	}

	if (fileName.empty()) {
		json_string_buffer json;
		json_writer writer(json);

		if (countOfRows == 1) {
			const auto row = resultPG.one_row();

			writeRowResultToWriter(writer, row, columnsTypes);
			return json.GetString();
		}

		writer.StartArray();
		for (const auto& row : resultPG)
			writeRowResultToWriter(writer, row, columnsTypes);

		writer.EndArray();
		return json.GetString();
	}

	wofstream ofs;

	const filesystem::path inputPath(fileName);
	ofs.open(inputPath);

	if (!ofs || !ofs.is_open())
		BOOST_THROW_EXCEPTION(invalid_argument("Не удалось открыть поток для записи результата JSON"));

	json_ofile_stream wOfs(ofs);
	json_writer_file writer(wOfs);

	if (countOfRows == 1) {
		const auto row = resultPG.one_row();
		writeRowResultToWriter(writer, row, columnsTypes);
	}
	else {
		writer.StartArray();
		for (const auto& row : resultPG)
			writeRowResultToWriter(writer, row, columnsTypes);

		writer.EndArray();
	}

	ofs.close();
	return L"";
}

template <typename Type>
void PostgreSQL::writeRowResultToWriter(Type& writer, const row& result, const map<string, P_ColumnsTypes>& columnsTypes) {
	writer.StartObject();

	for (const auto& field : result) {
		const auto name = string(field.name());
		const auto wName = gl_conv_string_to_wstring(name);
		
		writer.Key(wName.c_str(), static_cast<rapidjson::SizeType>(wName.size()));
		if (result[name].is_null() || field.is_null()) {
			writer.Null();
			continue;
		}

		if (const auto type = columnsTypes.find(name)->second; type.isArray) {
			writer.StartArray();

			vector<string> values;

			try {
				const auto arrayField = field.as_sql_array<string>();
				for_each(arrayField.cbegin(), arrayField.cend(), [&values](const string& value) {
					values.push_back(value);
				});
			}
			catch (...) {
				stringstream ss(pqxx::to_string(field));
				string value;

				while (getline(ss, value, ' '))
					values.push_back(value);
			}

			for (const auto& it : values) {
				switch (type.basicType) {
					case p_column_types_for_json::t_int: {
						const auto forWrite = stoi(it);
						writer.Int(forWrite);

						break;
					}
					case p_column_types_for_json::t_uint:
					case p_column_types_for_json::t_int64: {
						const auto forWrite = stoll(it);
						writer.Int64(forWrite);

						break;
					}
					case p_column_types_for_json::t_double: {
						const auto forWrite = stod(it);
						writer.Double(forWrite);

						break;
					}
					case p_column_types_for_json::t_bool: {
						writer.Bool(gl_str_iequal(it, "t"));
						break;
					}
					default: {
						const auto wStr = gl_conv_string_to_wstring(it);
						writer.String(wStr.c_str(), static_cast<rapidjson::SizeType>(wStr.size()));

						break;
					}
				}
			}

			writer.EndArray();
		} else {
			switch (type.basicType) {
				case p_column_types_for_json::t_uint: {
					writer.Uint(field.as<oid>());
					break;
				}
				case p_column_types_for_json::t_int: {
					writer.Int(field.as<int32_t>());
					break;
				}
				case p_column_types_for_json::t_int64: {
					writer.Int64(field.as<int64_t>());
					break;
				}
				case p_column_types_for_json::t_double: {
					writer.Double(field.as<double>());
					break;
				}
				case p_column_types_for_json::t_bool: {
					writer.Bool(field.as<bool>());
					break;
				}
				default: {
					const auto str = pqxx::to_string(field);
					const auto wStr = gl_conv_string_to_wstring(str);

					writer.String(wStr.c_str(), static_cast<rapidjson::SizeType>(wStr.size()));
					break;
				}
			}
		}
			
	}

	writer.EndObject();
}

template <typename Type>
void PostgreSQL::writeFieldToWriter(Type& writer, const field& field, const P_ColumnsTypes& columnType) {
	
}

void PostgreSQL::setupConnection() {
	if (p_ConnectionLine.empty())
		return;

	p_Conn = new connection(p_ConnectionLine);
	if (!connectionEstablished())
		BOOST_THROW_EXCEPTION(runtime_error("Не удалось подключиться к PostgreSQL базе!"));

	p_NonTransaction = new nontransaction(*p_Conn);
	setColumnTypes();
}

void PostgreSQL::closeAllConnection(const bool doNotTouchLineConnection) {
	if (!p_Conn)
		return;

	p_ActiveThreads.clear();
	p_NonTransaction = nullptr;

	p_ColumnsTypes.clear();

	p_Conn->close();
	p_Conn = nullptr;

	if (!doNotTouchLineConnection)
		p_ConnectionLine.clear();
}

#pragma endregion
