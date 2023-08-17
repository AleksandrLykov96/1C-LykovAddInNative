#include <extensions/postgreSQL.hpp>

#pragma region Переопределение методов

const wchar_t* PostgreSQL::getNameExtension() {
	return L"PostgreSQL";
}

const wchar_t* PostgreSQL::getVersion() {
	return L"1.0.0.17";
}

void PostgreSQL::setMethodPropsExtension() {
	// Свойства
	m_PropNames[gl_Index_Prop_Connection_Established]      = NamesFor1C(L"СоединениеУстановлено", L"ConnectionEstablished");
	m_PropNames[gl_Index_Prop_Connection_Host]             = NamesFor1C(L"Хост", L"Host");
	m_PropNames[gl_Index_Prop_Connection_Db_Name]          = NamesFor1C(L"НаименованиеБазы", L"DBName");
	m_PropNames[gl_Index_Prop_Connection_Username]         = NamesFor1C(L"ИмяПользователя", L"Username");
	m_PropNames[gl_Index_Prop_Connection_Port]             = NamesFor1C(L"Порт", L"Port");
	m_PropNames[gl_Index_Prop_Connection_Backend_Pid]      = NamesFor1C(L"ИДПроцесса", L"BackendPID");
	m_PropNames[gl_Index_Prop_Connection_Sock]             = NamesFor1C(L"Сокет", L"Socket");
	m_PropNames[gl_Index_Prop_Connection_Protocol_Version] = NamesFor1C(L"ВерсияПротокола", L"ProtocolVersion");
	m_PropNames[gl_Index_Prop_Connection_Server_Version]   = NamesFor1C(L"ВерсияСервера", L"ServerVersion");
	m_PropNames[gl_Index_Prop_Connection_Client_Encoding]  = NamesFor1C(L"Кодировка", L"Encoding");

	// Методы
	m_MethodNames[gl_Index_Method_Connect_Db]                   = NamesFor1C(L"УстановитьСоединение", L"ConnectDB");
	m_MethodNames[gl_Index_Method_Exec]                         = NamesFor1C(L"ВыполнитьЗапрос", L"Exec");
	m_MethodNames[gl_Index_Method_Init_Query]                   = NamesFor1C(L"НачатьВыполнениеЗапроса", L"InitQuery");
	m_MethodNames[gl_Index_Method_Get_Result_Initialized_Query] = NamesFor1C(L"ПолучитьРезультатыЗарегистрированныхЗапросов", L"GetResultsInitializedQueries");
	m_MethodNames[gl_Index_Method_Put_Data]                     = NamesFor1C(L"ВставитьДанныеВТаблицу", L"PutDataInTable");
	m_MethodNames[gl_Index_Method_Read_Data_From_Table]         = NamesFor1C(L"ПрочитатьДанныеИзТаблицы", L"ReadDataFromTable");
	m_MethodNames[gl_Index_Method_Read_Data_From_Query]         = NamesFor1C(L"ПрочитатьДанныеИзЗапроса", L"ReadDataFromQuery");
	m_MethodNames[gl_Index_Method_Initialize_1c]                = NamesFor1C(L"ИнициализироватьФункцииДля1С", L"Initialize1C");
	m_MethodNames[gl_Index_Method_Reload_Connection]            = NamesFor1C(L"СброситьСоединение", L"ReloadConnection");
}

void PostgreSQL::getPropByIndex(const unsigned short indexProp, tVariant* pvarPropVal) {
	switch (indexProp) {
		case gl_Index_Prop_Connection_Established:
			setReturnedParam(connectionEstablished(), pvarPropVal);
			return;
		case gl_Index_Prop_Connection_Host: {
			if (connectionEstablished())
				setReturnedParam(m_Conn->hostname(), pvarPropVal);
			else
				setReturnedParam("", pvarPropVal);

			return;
		}
		case gl_Index_Prop_Connection_Db_Name: {
			if (connectionEstablished())
				setReturnedParam(m_Conn->dbname(), pvarPropVal);
			else
				setReturnedParam("", pvarPropVal);

			return;
		}
		case gl_Index_Prop_Connection_Username: {
			if (connectionEstablished())
				setReturnedParam(m_Conn->username(), pvarPropVal);
			else
				setReturnedParam("", pvarPropVal);

			return;
		}
		case gl_Index_Prop_Connection_Port: {
			if (connectionEstablished())
				setReturnedParam(m_Conn->port(), pvarPropVal);
			else
				setReturnedParam(0, pvarPropVal);

			return;
		}
		case gl_Index_Prop_Connection_Backend_Pid: {
			if (connectionEstablished())
				setReturnedParam(m_Conn->backendpid(), pvarPropVal);
			else
				setReturnedParam(0, pvarPropVal);

			return;
		}
		case gl_Index_Prop_Connection_Sock: {
			if (connectionEstablished())
				setReturnedParam(m_Conn->sock(), pvarPropVal);
			else
				setReturnedParam(0, pvarPropVal);

			return;
		}
		case gl_Index_Prop_Connection_Protocol_Version: {
			if (connectionEstablished())
				setReturnedParam(m_Conn->protocol_version(), pvarPropVal);
			else
				setReturnedParam(0, pvarPropVal);

			return;
		}
		case gl_Index_Prop_Connection_Server_Version: {
			if (connectionEstablished())
				setReturnedParam(m_Conn->server_version(), pvarPropVal);
			else
				setReturnedParam(0, pvarPropVal);

			return;
		}
		case gl_Index_Prop_Connection_Client_Encoding: {
			if (connectionEstablished())
				setReturnedParam<const std::string&>(m_Conn->get_client_encoding(), pvarPropVal);
			else
				setReturnedParam("", pvarPropVal);

			return;
		}
		default:
			break;
	}
}

void PostgreSQL::setPropByIndex(const unsigned short indexProp, tVariant* varPropVal) {
	if (indexProp == gl_Index_Prop_Connection_Client_Encoding) {
		if (!connectionEstablished())
			throw LykovException(L"Необходимо установить соединение с БД", L"Кодировка клиента");

		const auto encoding = getInputParam<std::string>(varPropVal, -1l);
		m_Conn->set_client_encoding(encoding);
	}
}

bool PostgreSQL::getIsPropWritable(const unsigned short indexProp) {
	return indexProp == gl_Index_Prop_Connection_Client_Encoding;
}

long PostgreSQL::getMethodNParams(const unsigned short indexMethod) {
	switch (indexMethod) {
		case gl_Index_Method_Connect_Db:
		case gl_Index_Method_Get_Result_Initialized_Query:
			return 1l;
		case gl_Index_Method_Read_Data_From_Query:
			return 2l;
		case gl_Index_Method_Exec:
		case gl_Index_Method_Read_Data_From_Table:
			return 3l;
		case gl_Index_Method_Put_Data:
			return 4l;
		case gl_Index_Method_Init_Query:
			return 5l;
		default:
			return 0l;
	}
}

void PostgreSQL::setParamDefValue(const unsigned short indexMethod, const long indexParam, tVariant* pvarParamDefValue) {
	switch (indexMethod) {
		case gl_Index_Method_Exec: {
			switch (indexParam) {
				case 1l:
					setReturnedParam(-1, pvarParamDefValue);
					return;
				case 2l:
					setReturnedParam("", pvarParamDefValue);
					return;
				default:
					break;
			}
			break;
		}
		case gl_Index_Method_Init_Query: {
			switch (indexParam) {
				case 2l:
					setReturnedParam(false, pvarParamDefValue);
					return;
				case 3l:
					setReturnedParam(-1, pvarParamDefValue);
					return;
				case 4l:
					setReturnedParam("", pvarParamDefValue);
					return;
				default:
					break;
			}
			break;
		}
		case gl_Index_Method_Get_Result_Initialized_Query: {
			if (indexParam == 0l) {
				setReturnedParam(L"", pvarParamDefValue);
				return;
			}
			break;
		}
		case gl_Index_Method_Read_Data_From_Table: {
			switch (indexParam) {
				case 1l:
				case 2l:
					setReturnedParam(L"", pvarParamDefValue);
					return;
				default:
					break;
			}

			break;
		}
		case gl_Index_Method_Read_Data_From_Query: {
			if (indexParam == 1l) {
				setReturnedParam("", pvarParamDefValue);
				return;
			}
			break;
		}
		case gl_Index_Method_Put_Data: {
			switch (indexParam) {
				case 2l:
					setReturnedParam(L"", pvarParamDefValue);
					return;
				case 3l:
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

bool PostgreSQL::getHasRetVal(const unsigned short indexMethod) {
	switch (indexMethod) {
		case gl_Index_Method_Exec:
		case gl_Index_Method_Get_Result_Initialized_Query:
		case gl_Index_Method_Read_Data_From_Table:
		case gl_Index_Method_Read_Data_From_Query:
			return true;
		default:
			return false;
	}
}

void PostgreSQL::callMethodAsProc(const unsigned short indexMethod, const tVariant* paParams, const long paParamsSize) {
	switch (indexMethod) {
		case gl_Index_Method_Connect_Db:
			connectToDb(paParams);
			return;
		case gl_Index_Method_Init_Query:
			initQuery(paParams);
			return;
		case gl_Index_Method_Initialize_1c:
			initialize1C();
			return;
		case gl_Index_Method_Put_Data:
			putCopyIn(paParams);
			return;
		case gl_Index_Method_Reload_Connection:
			reloadConnection();
			return;
		default:
			return;
	}
}

void PostgreSQL::callMethodAsFunc(const unsigned short indexMethod, tVariant* pvarRetValue, const tVariant* paParams, const long paParamsSize) {
	switch (indexMethod) {
		case gl_Index_Method_Exec:
			exec(paParams, pvarRetValue);
			return;
		case gl_Index_Method_Get_Result_Initialized_Query:
			getResultsInitializedQueries(paParams, pvarRetValue);
			return;
		case gl_Index_Method_Read_Data_From_Table:
			readDataFromTable(paParams, pvarRetValue);
			return;
		case gl_Index_Method_Read_Data_From_Query:
			readDataFromQuery(paParams, pvarRetValue);
			return;
		default:
			return;
	}
}

#pragma endregion

#pragma region Функции компоненты

void PostgreSQL::connectToDb(const tVariant* paParams) {
	const auto lineConnect = getInputParam<std::string>(paParams);
	if (lineConnect.empty())
		throw LykovException(L"Необходимо указать строку для подключения!", L"Установить соединение с базой");

	closeConnection();

	m_Conn = new pqxx::connection(lineConnect.c_str());
	if (!connectionEstablished())
		throw LykovException(L"Не удалось установить подключение с базой данных!", L"Установить соединение с базой");

	m_NonTransaction = new pqxx::nontransaction(*m_Conn);

	const auto res = m_NonTransaction->exec(
		"SELECT oid,\n"
		"	typcategory = 'B' AS itsBoolean,\n"
		"	typlen AS typLen,\n"
		"	typname AS typname\n"
		"FROM pg_type\n"
		"WHERE typcategory IN('B', 'N')\n");

	for (const auto& row : res) {
		const auto oid = row["oid"].as<pqxx::oid>();
		if (row["itsBoolean"].as<bool>()) {
			m_ColumnsTypes[oid] = gl_Type_Column_PostgreSQL_Boolean;
			continue;
		}

		const auto nameType = pqxx::to_string(row["typname"]);
		if (gl_IEqualsCaseInsensitive(nameType.c_str(), "oid")) {
			m_ColumnsTypes[oid] = gl_Type_Column_PostgreSQL_OID;
			continue;
		}

		const auto typLen = row["typlen"].as<int>();
		if (boost::algorithm::starts_with(nameType, "int") || boost::algorithm::starts_with(nameType, "_int")) {
			switch (typLen) {
				case 2:
					m_ColumnsTypes[oid] = gl_Type_Column_PostgreSQL_Int16;
					break;
				case 4:
					m_ColumnsTypes[oid] = gl_Type_Column_PostgreSQL_Int32;
					break;
				case 8:
					m_ColumnsTypes[oid] = gl_Type_Column_PostgreSQL_Int64;
					break;
				default:
					m_ColumnsTypes[oid] = gl_Type_Column_PostgreSQL_LongDouble;
					break;
			}
		} else {
			switch (typLen) {
				case 2:
				case 4:
					m_ColumnsTypes[oid] = gl_Type_Column_PostgreSQL_Float;
					break;
				case 8:
					m_ColumnsTypes[oid] = gl_Type_Column_PostgreSQL_Double;
					break;
				default:
					m_ColumnsTypes[oid] = gl_Type_Column_PostgreSQL_LongDouble;
					break;
			}
		}
	}

	m_ConnectionLine = gl_CopyStringToChar(lineConnect);
}

void PostgreSQL::exec(const tVariant* paParams, tVariant* pvarRetValue) const {
	if (!connectionEstablished())
		throw LykovException(L"Не установлено соединение с базой PostgreSQL", L"Выполнить запрос");

	const auto request     = getInputParam<std::string>(paParams, 0l);
	const auto countOfRows = getInputParam<int>(paParams, 1l);
	const auto fileResult  = getInputParam<std::wstring>(paParams, 2l);
	const auto toFile      = !fileResult.empty();

	switch (countOfRows) {
		case 0: {
			// Без возвращаемого значения
			m_NonTransaction->exec0(request);
			setReturnedParam(true, pvarRetValue);

			if (toFile)
				gl_WriteStringToFile(fileResult, L"true");

			return;
		}
		case 1: {
			// 1 строка в результате
			const auto res  = m_NonTransaction->exec1(request);
			const auto json = rowResultToJson(res, m_ColumnsTypes, fileResult, toFile);

			if (!toFile)
				setReturnedParam<const std::wstring&>(json, pvarRetValue);
			else
				setReturnedParam(true, pvarRetValue);

			return;
		}
		case -1: {
			// Неизвестное количество строк
			const auto res  = m_NonTransaction->exec(request);
			const auto json = arrayResultToJson(res, m_ColumnsTypes, fileResult, toFile);

			if (!toFile)
				setReturnedParam<const std::wstring&>(json, pvarRetValue);
			else
				setReturnedParam(true, pvarRetValue);

			return;
		}
		default: {
			// Конкретное количество строк
			const auto res  = m_NonTransaction->exec_n(countOfRows, request);
			const auto json = arrayResultToJson(res, m_ColumnsTypes, fileResult, toFile, countOfRows);
			
			if (!toFile)
				setReturnedParam<const std::wstring&>(json, pvarRetValue);
			else
				setReturnedParam(true, pvarRetValue);
		}
	}
}

void PostgreSQL::initQuery(const tVariant* paParams) {
	if (!connectionEstablished())
		throw LykovException(L"Не установлено соединение с базой PostgreSQL", L"Начать выполнение запроса");

	const auto request       = getInputParam<std::string>(paParams, 0l);
	const auto idRequest     = getInputParam<std::wstring>(paParams, 1l);
	const auto idRequestChar = idRequest.c_str();
	const auto initExternal  = getInputParam<bool>(paParams, 2l);
	const auto countOfRows   = getInputParam<int>(paParams, 3l);
	const auto fileResult    = getInputParam<std::wstring>(paParams, 4l);

	if (const auto result = m_ThreadsMap.find(idRequestChar); result != m_ThreadsMap.end()) {
		const auto description = boost::str(
			boost::wformat(L"Запрос с идентификатором '%s' уже инициализирован!") % idRequestChar);
		throw LykovException(description.c_str(), L"Начать выполнение запроса");
	}

	if (initExternal && m_ItsServer)
		throw LykovException(L"Нельзя инициализировать внешнее событие на сервере!", L"Начать выполнение запроса");

	auto* newThread = new std::thread([this, request, idRequest, initExternal, countOfRows, fileResult] {
		initQuery_Thread(request.c_str(), idRequest.c_str(), initExternal, countOfRows, fileResult.c_str());
	});

	m_Mutex.lock();
	m_ThreadsMap[gl_CopyWStringToWChar(idRequestChar)] = newThread;
	m_Mutex.unlock();
}

void PostgreSQL::getResultsInitializedQueries(const tVariant* paParams, tVariant* pvarRetValue) {
	if (!connectionEstablished())
		throw LykovException(L"Не установлено соединение с базой PostgreSQL", L"Получить результаты запросов");

	const auto nameQuery    = getInputParam<std::wstring>(paParams);
	const auto singleResult = !nameQuery.empty();
	if (singleResult) {
		if (const auto res = m_ThreadsMap.find(nameQuery.c_str()); res != m_ThreadsMap.end() && res->second->joinable())
			res->second->join();
	}
	else
		clearAllThreads();

	rapidjson::StringBuffer json;
	rapidjson::Writer writer(json);

	if (singleResult) {
		writer.StartObject();

		if (const auto res = m_ThreadResults.find(nameQuery.c_str()); res != m_ThreadResults.end()) {
			writer.Key(gl_WStringToString(res->first).c_str());
			writer.String(res->second.c_str());

			m_ThreadResults.erase(res);
		}

		writer.EndObject();
	}
	else {
		writer.StartArray();
		for (const auto& it : m_ThreadResults) {
			writer.StartObject();
			writer.Key(gl_WStringToString(it.first).c_str());
			writer.String(it.second.c_str());
			writer.EndObject();
		}
		writer.EndArray();

		m_ThreadResults.clear();
	}

	setReturnedParam<const std::wstring&>(gl_StringToWString(json.GetString()), pvarRetValue);
}

void PostgreSQL::putCopyIn(const tVariant* paParams) const {
	if (!connectionEstablished())
		throw LykovException(L"Не установлено соединение с базой PostgreSQL", L"Put data");

	const auto tableName = getInputParam<std::string>(paParams, 0l);
	if (tableName.empty())
		throw LykovException(L"Пустое наименование таблицы", L"Put data");
	
	const auto columnsNames = getInputParam<std::string>(paParams, 2l);
	const auto fromFile = getInputParam<bool>(paParams, 3l);

	rapidjson::Document data;
	if (fromFile) {
		data = gl_GetInputJsonFromFile(getInputParam<std::wstring>(paParams, 1l));
	} else {
		data = getInputParam<rapidjson::Document>(paParams, 1l);
	}

	if (!data.IsArray())
		throw LykovException(L"Пришёл некорректный JSON для передачи в PostgreSQL (должен быть массив массивов)", L"Put data");

	pqxx::stream_to stream = pqxx::stream_to::raw_table(*m_NonTransaction, tableName, columnsNames);

	unsigned countOfElements = 1;
	auto copyColumnsName = std::string(columnsNames);
	size_t pos = 0;
	while ((pos = copyColumnsName.find(',')) != std::string::npos) {
		copyColumnsName.erase(0, pos + 1);
		countOfElements++;
	}

	std::vector<std::string> rowForStream;
	rowForStream.reserve(countOfElements);

	for (const auto& arrayJson = data.GetArray(); const auto& row : arrayJson) {
		if (!row.IsArray())
			throw LykovException(L"Пришёл некорректный JSON для передачи в PostgreSQL (строка массива не массив)", L"Put data");

		for (const auto& field : row.GetArray()) {
			if (!field.IsString())
				throw LykovException(L"Пришёл некорректный JSON для передачи в PostgreSQL (колонка строки не массив)! Должны быть только строковые значения.");

			rowForStream.emplace_back(field.GetString());
		}

		stream.write_row(rowForStream);
		rowForStream.clear();
	}

	stream.complete();
}

void PostgreSQL::readDataFromTable(const tVariant* paParams, tVariant* pvarRetValue) const {
	if (!connectionEstablished())
		throw LykovException(L"Не установлено соединение с базой PostgreSQL", L"Получить данные из таблицы");

	const auto tableName = getInputParam<std::string>(paParams, 0l);
	if (tableName.empty())
		throw LykovException(L"Пустое наименование таблицы", L"Получить данные из таблицы");

	const auto columnsNames = getInputParam<std::string>(paParams, 1l);
	const auto fileNameResult = getInputParam<std::wstring>(paParams, 2l);
	const auto toFile = !fileNameResult.empty();

	pqxx::stream_from stream = pqxx::stream_from::raw_table(*m_NonTransaction, tableName, columnsNames);
	if (!toFile) {
		rapidjson::StringBuffer json;
		rapidjson::Writer writer(json);

		streamResultToJson(stream, writer);
		setReturnedParam<const std::wstring&>(gl_StringToWString(json.GetString()), pvarRetValue);
	} else {
#ifdef __linux__
		const auto fp = gl_OpenFile(fileNameResult, L"w");
#else
		const auto fp = gl_OpenFile(fileNameResult, L"wb");
#endif

		char writeBuffer[gl_Chunk_Size];
		rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		rapidjson::Writer writer(os);

		streamResultToJson(stream, writer);
		fclose(fp);

		setReturnedParam(true, pvarRetValue);
	}

	stream.complete();
}

void PostgreSQL::readDataFromQuery(const tVariant* paParams, tVariant* pvarRetValue) const {
	if (!connectionEstablished())
		throw LykovException(L"Не установлено соединение с базой PostgreSQL", L"Получить данные из запроса");

	const auto request = getInputParam<std::string>(paParams, 0l);
	const auto fileNameResult = getInputParam<std::wstring>(paParams, 1l);
	const auto toFile = !fileNameResult.empty();

	pqxx::stream_from stream = pqxx::stream_from::query(*m_NonTransaction, request);
	if (!toFile) {
		rapidjson::StringBuffer json;
		rapidjson::Writer writer(json);

		streamResultToJson(stream, writer);
		setReturnedParam<const std::wstring&>(gl_StringToWString(json.GetString()), pvarRetValue);
	}
	else {
#ifdef __linux__
		const auto fp = gl_OpenFile(fileNameResult, L"w");
#else
		const auto fp = gl_OpenFile(fileNameResult, L"wb");
#endif

		char writeBuffer[gl_Chunk_Size];
		rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		rapidjson::Writer writer(os);

		streamResultToJson(stream, writer);
		fclose(fp);

		setReturnedParam(true, pvarRetValue);
	}

	stream.complete();
}

void PostgreSQL::initialize1C() const {
	if (!connectionEstablished())
		throw LykovException(L"Не установлено соединение с базой PostgreSQL", L"Инициализировать функции для 1С");

	const auto request = "\
		CREATE OR REPLACE FUNCTION lykov_HexToInt(_textHex MVARCHAR) RETURNS integer LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
					DECLARE\
						result integer;\
						i integer;\
						len integer;\
						hexchar varchar(1);\
					BEGIN\
						result := 0;\
						len := length(_textHex);\
						\
						for i in 1..len loop\
							hexchar := substr(_textHex, len - i + 1, 1);\
							result := result + round(16 ^ (i - 1)::dec * case\
								when hexchar between '0' and '9' then cast(hexchar as int)\
								when upper(hexchar) between 'A' and 'F' then ascii(upper(hexchar)) - 55\
								end);\
						end loop;\
						\
						RETURN result;\
					END;\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_HexToInt(_textHex text) RETURNS integer LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
					DECLARE\
						result integer;\
						i integer;\
						len integer;\
						hexchar varchar;\
					BEGIN\
						result := 0;\
						len := length(_textHex);\
						\
						for i in 1..len loop\
							hexchar := substr(_textHex, len - i + 1, 1);\
							result := result + round(16 ^ (i - 1)::dec * case\
								when hexchar between '0' and '9' then cast(hexchar as int)\
								when upper(hexchar) between 'A' and 'F' then ascii(upper(hexchar)) - 55\
								end);\
						end loop;\
						\
						RETURN result;\
					END;\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_RRefToUID(_rref BYTEA) RETURNS MVARCHAR(36) LEAKPROOF IMMUTABLE PARALLEL SAFE AS $$\
					DECLARE\
						IDrrefText mvarchar;\
					BEGIN\
						IF (_rref IS NULL) THEN\
							RETURN '00000000-0000-0000-0000-000000000000';\
						END IF;\
						\
						IDrrefText := lpad(encode(_rref, 'hex'), 32, '0');\
						RETURN substr(IDrrefText, 25, 8) || '-'\
							|| substr(IDrrefText, 21, 4) || '-'\
							|| substr(IDrrefText, 17, 4) || '-'\
							|| substr(IDrrefText, 1, 4) || '-'\
							|| substr(IDrrefText, 5, 12);\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_RRefToUID(_rref text) RETURNS MVARCHAR(36) LEAKPROOF IMMUTABLE PARALLEL SAFE AS $$\
					DECLARE\
						IDrrefText mvarchar;\
					BEGIN\
						IF (_rref IS NULL) THEN\
							RETURN '00000000-0000-0000-0000-000000000000';\
						END IF;\
						\
						IDrrefText := lpad(_rref, 32, '0');\
						RETURN substr(IDrrefText, 25, 8) || '-'\
							|| substr(IDrrefText, 21, 4) || '-'\
							|| substr(IDrrefText, 17, 4) || '-'\
							|| substr(IDrrefText, 1, 4) || '-'\
							|| substr(IDrrefText, 5, 12);\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_RRefToUID(_rref MVARCHAR(32)) RETURNS MVARCHAR(36) LEAKPROOF IMMUTABLE PARALLEL SAFE AS $$\
					DECLARE\
						IDrrefText mvarchar;\
					BEGIN\
						IF (_rref IS NULL) THEN\
							RETURN '00000000-0000-0000-0000-000000000000';\
						END IF;\
						\
						IDrrefText := lpad(_rref::text, 32, '0');\
						RETURN substr(IDrrefText, 25, 8) || '-'\
							|| substr(IDrrefText, 21, 4) || '-'\
							|| substr(IDrrefText, 17, 4) || '-'\
							|| substr(IDrrefText, 1, 4) || '-'\
							|| substr(IDrrefText, 5, 12);\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_UIDToRRef(_UID MVARCHAR(36)) RETURNS BYTEA LEAKPROOF IMMUTABLE PARALLEL SAFE AS $$\
					DECLARE\
						UIDText text;\
					BEGIN\
						IF (_UID IS NULL) THEN\
							RETURN decode('00000000000000000000000000000000', 'hex');\
						END IF;\
						\
						UIDText := lpad(replace(_UID::text, '-', ''), 32, '0');\
						RETURN decode(substring(UIDText, 17, 16) || substring(UIDText, 13, 4) || substring(UIDText, 9, 4) || substring(UIDText, 1, 8), 'hex');\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_UIDToRRef(_UID text) RETURNS BYTEA LEAKPROOF IMMUTABLE PARALLEL SAFE AS $$\
					DECLARE\
						UIDText text;\
					BEGIN\
						IF (_UID IS NULL) THEN\
							RETURN decode('00000000000000000000000000000000', 'hex');\
						END IF;\
						\
						UIDText := lpad(replace(_UID, '-', ''), 32, '0');\
						RETURN decode(substring(UIDText, 17, 16) || substring(UIDText, 13, 4) || substring(UIDText, 9, 4) || substring(UIDText, 1, 8), 'hex');\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_TypeToBytea(_type1C integer) RETURNS BYTEA LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
					BEGIN\
						RETURN decode(lpad(to_hex(_type1C), 2, '0'), 'hex');\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_ByteaToType(_typePG BYTEA) RETURNS integer LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
					BEGIN\
						RETURN lykov_HexToInt(encode(_typePG, 'hex'));\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_DDToBytea(_bytea1C text) RETURNS BYTEA LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
					BEGIN\
						RETURN decode(_bytea1C, 'base64');\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_ByteaToDD(_byteaPG BYTEA) RETURNS text LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
					BEGIN\
						RETURN encode(_byteaPG, 'base64');\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_DDToText(_byteaPG BYTEA) RETURNS text LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
				DECLARE\
					str text;\
				BEGIN\
					str := encode(_byteaPG, 'base64');\
	  				IF (substr(str, 15, 4) == '+7v3') THEN\
 						RETURN '{\"' || convert_from(decode(substr(str, 21), 'base64'), 'UTF8');\
 					ELSE\
 						RETURN convert_from(_byteaPG, 'UTF8');\
					END IF;\
				END\
			$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_RTRefToBytea(_rtref1C integer) RETURNS BYTEA LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
					BEGIN\
						RETURN decode(lpad(to_hex(_rtref1C), 8, '0'), 'hex');\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_RTRefToInteger(_rtrefPG BYTEA) RETURNS integer LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
					BEGIN\
						RETURN lykov_HexToInt(encode(_rtrefPG, 'hex'));\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_VersionToInteger(_version1C MVARCHAR(12)) RETURNS integer LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
					BEGIN\
						RETURN lykov_HexToInt(encode(decode(_version1C::text, 'base64'), 'hex'));\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_VersionToInteger(_version1C text) RETURNS integer LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
					BEGIN\
						RETURN lykov_HexToInt(encode(decode(_version1C, 'base64'), 'hex'));\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_VersionTo1C(_versionPG integer) RETURNS text LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
					BEGIN\
						RETURN encode(decode(lpad(to_hex(_versionPG), 16, '0'), 'hex'), 'base64');\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_KeyFieldToBytea(_keyField1C integer) RETURNS BYTEA LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
					BEGIN\
						RETURN decode(lpad(to_hex(_keyField1C), 8, '0'), 'hex');\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE OR REPLACE FUNCTION lykov_KeyFieldToInteger(_keyFieldPG BYTEA) RETURNS integer LEAKPROOF IMMUTABLE STRICT PARALLEL SAFE AS $$\
					BEGIN\
						RETURN lykov_HexToInt(encode(_keyFieldPG, 'hex'));\
					END\
				$$ LANGUAGE PLPGSQL;\
		\
		\
		CREATE EXTENSION IF NOT EXISTS \"uuid-ossp\";";

	m_NonTransaction->exec0(request);
}

void PostgreSQL::reloadConnection() {
	if (!m_Conn)
		return;

	clearAllThreads();
	m_ThreadsMap.clear();
	m_ThreadResults.clear();

	m_NonTransaction = nullptr;
	m_Conn->close();
	m_Conn = nullptr;

	m_Conn = new pqxx::connection(m_ConnectionLine);
	if (!connectionEstablished())
		throw LykovException(L"Не удалось установить подключение с базой данных!", L"Установить соединение с базой");

	m_NonTransaction = new pqxx::nontransaction(*m_Conn);
}

#pragma endregion

#pragma region Вспомогательные функции

PostgreSQL::PostgreSQL() {
	curl_global_sslset(CURLSSLBACKEND_OPENSSL, nullptr, nullptr); // Иначе данная библиотека не присоединяется к OpenSSL (при подключении пишет ошибки) 

	m_Conn = nullptr;
	m_NonTransaction = nullptr;
#ifdef __linux__
	signal(SIGPIPE, SIG_IGN);
#endif
}

PostgreSQL::~PostgreSQL() {
	closeConnection();
}

bool PostgreSQL::connectionEstablished() const {
	return m_Conn && m_Conn->is_open();
}

void PostgreSQL::closeConnection() {
	if (!m_Conn)
		return;

	clearAllThreads();
	m_ThreadResults.clear();

	m_NonTransaction = nullptr;
	m_Conn->close();
	m_Conn = nullptr;
	m_ColumnsTypes.clear();
	m_ConnectionLine = "";
}

void PostgreSQL::initQuery_Thread(const char* request, const wchar_t* idRequest, bool initExternal, int countOfRows, const wchar_t* fileResult) {
	try {
		std::wstring result;
		const auto toFile = wcslen(fileResult) > 0;

		pqxx::connection localConn(m_ConnectionLine);
		pqxx::nontransaction localWork(localConn);

		switch (countOfRows) {
			case 0: {
				// Без возвращаемого значения
				localWork.exec0(request);
				result = L"true";

				if (toFile)
					gl_WriteStringToFile(fileResult, L"true");

				break;
			}
			case 1: {
				// 1 строка в результате
				const auto res  = localWork.exec1(request);
				const auto json = rowResultToJson(res, m_ColumnsTypes, fileResult, toFile);

				result = (toFile ? L"true" : json);
				break;
			}
			case -1: {
				// Неизвестное количество строк
				const auto res  = localWork.exec(request);
				const auto json = arrayResultToJson(res, m_ColumnsTypes, fileResult, toFile);
				result          = (toFile ? L"true" : json);

				break;
			}
			default: {
				// Конкретное количество строк
				const auto res  = localWork.exec_n(countOfRows, request);
				const auto json = arrayResultToJson(res, m_ColumnsTypes, fileResult, toFile, countOfRows);
				result          = (toFile ? L"true" : json);

				break;
			}
		}

		localConn.close();

		if (initExternal)
			externalEvent(L"PostgreSQLRequest", idRequest, result.c_str());
		else {
			m_Mutex.lock();
			m_ThreadResults.insert(std::make_pair(gl_CopyWStringToWChar(idRequest), gl_WStringToString(result)));
			m_Mutex.unlock();
		}
	} catch (const LykovException& message) {
		if (initExternal)
			externalEvent(L"PostgreSQLRequest", message.getSource(), message.whatW());
		else
			m_ThreadResults.insert(std::make_pair(gl_CopyWStringToWChar(idRequest), message.what()));
	}
	catch (const std::runtime_error& message) {
		const auto source = boost::str(boost::wformat(L"PostgreSQL запрос (runtime_error) - Ошибка в потоке %d") % std::this_thread::get_id());
		const auto myExp = LykovException(message.what(), source.c_str());
		if (initExternal)
			externalEvent(L"PostgreSQLRequest", myExp.getSource(), myExp.whatW());
		else
			m_ThreadResults.insert(std::make_pair(gl_CopyWStringToWChar(idRequest), message.what()));
	}
	catch (const std::exception& message) {
		const auto source = boost::str(boost::wformat(L"PostgreSQL запрос (exception) - Ошибка в потоке %d") % std::this_thread::get_id());
		const auto myExp = LykovException(message.what(), source.c_str());
		if (initExternal)
			externalEvent(L"PostgreSQLRequest", myExp.getSource(), myExp.whatW());
		else
			m_ThreadResults.insert(std::make_pair(gl_CopyWStringToWChar(idRequest), message.what()));
	}
	catch (...) {
		const auto source = boost::str(boost::wformat(L"PostgreSQL запрос (...) - Ошибка в потоке %d") % std::this_thread::get_id());
		const auto myExp = LykovException(L"Неизвестная ошибка", source.c_str());
		if (initExternal)
			externalEvent(L"PostgreSQLRequest", myExp.getSource(), myExp.whatW());
		else
			m_ThreadResults.insert(std::make_pair(gl_CopyWStringToWChar(idRequest), myExp.what()));
	}

	m_Mutex.lock();
	m_ThreadsMap.erase(idRequest);
	m_Mutex.unlock();
}

std::unordered_map<const char*, short> PostgreSQL::getTypesOfColumns(const pqxx::row& rowResult, const ColumnTypesType& columnsTypes) {
	std::unordered_map<const char*, short> tmp;
	for (const pqxx::field& column : rowResult) {
		if (const auto founded = columnsTypes.find(column.type()); founded == columnsTypes.end())
			tmp[column.name()] = -1;
		else
			tmp[column.name()] = founded->second;
	}

	return tmp;
}

std::wstring PostgreSQL::arrayResultToJson(const pqxx::result& result, const ColumnTypesType& columnsTypes, const std::wstring& fileResult, const bool toFile, size_t countOfRows) {
	if (countOfRows == -1)
		countOfRows = result.size();

	if (countOfRows == 0) {
		if (toFile)
			gl_WriteStringToFile(fileResult, L"{\"Успешно\":true,\"ТекстОшибки\":\"\",\"РезультатЗапроса\":[]}");

		return L"{\"Успешно\":true,\"ТекстОшибки\":\"\",\"РезультатЗапроса\":[]}";
	}

	const auto tmp = getTypesOfColumns(result[0], columnsTypes);
	if (!toFile) {
		rapidjson::StringBuffer json;
		rapidjson::Writer writer(json);

		writer.StartObject();

		writer.Key("Успешно");
		writer.Bool(true);
		
		writer.Key("ТекстОшибки");
		writer.String("");

		writer.Key("РезультатЗапроса");

		writer.StartArray();
		std::for_each(result.begin(), result.end(), [&tmp, &writer](const auto& row) {
			PostgreSQL::	arrayResultToJson_WriteRow<rapidjson::StringBuffer>(row, tmp, writer);
		});
		writer.EndArray();
		writer.EndObject();

		return gl_StringToWString(json.GetString());
	}
	else {
#ifdef __linux__
		const auto fp = gl_OpenFile(fileResult, L"w");
#else
		const auto fp = gl_OpenFile(fileResult, L"wb");
#endif

		char writeBuffer[gl_Chunk_Size];
		rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		rapidjson::Writer writer(os);

		writer.StartObject();

		writer.Key("Успешно");
		writer.Bool(true);
		
		writer.Key("ТекстОшибки");
		writer.String("");

		writer.Key("РезультатЗапроса");

		writer.StartArray();
		std::for_each(result.begin(), result.end(), [&tmp, &writer](const auto& row) {
			PostgreSQL::arrayResultToJson_WriteRow<rapidjson::FileWriteStream>(row, tmp, writer);
		});
		writer.EndArray();
		writer.EndObject();

		fclose(fp);
		return L"{\"Успешно\":true,\"ТекстОшибки\":\"\",\"РезультатЗапроса\":[]}";
	}
}

std::wstring PostgreSQL::rowResultToJson(const pqxx::row& result, const ColumnTypesType& columnsTypes, const std::wstring& fileResult, const bool toFile) {
	const auto tmp = getTypesOfColumns(result, columnsTypes);
	if (!toFile) {
		rapidjson::StringBuffer json;
		rapidjson::Writer writer(json);

		arrayResultToJson_WriteRow<rapidjson::StringBuffer>(result, tmp, writer);

		return gl_StringToWString(json.GetString());
	}
	else {
#ifdef __linux__
		const auto fp = gl_OpenFile(fileResult, L"w");
#else
		const auto fp = gl_OpenFile(fileResult, L"wb");
#endif

		char writeBuffer[gl_Chunk_Size];
		rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		rapidjson::Writer writer(os);

		arrayResultToJson_WriteRow<rapidjson::FileWriteStream>(result, tmp, writer);
		fclose(fp);

		return L"";
	}
}

template <typename Type>
void PostgreSQL::arrayResultToJson_WriteRow(const pqxx::row& row, const std::unordered_map<const char*, short>& tmp, rapidjson::Writer<Type>& writer) {
	writer.StartObject();
	for (const auto field : row) {
		const auto name = field.name();
		writer.Key(name, static_cast<rapidjson::SizeType>(std::strlen(name)));

		if (row[name].is_null()) {
			writer.Null();
			continue;
		}

		switch (tmp.find(name)->second) {
			case gl_Type_Column_PostgreSQL_OID:
				writer.Uint(field.as<pqxx::oid>());
				continue;
			case gl_Type_Column_PostgreSQL_Int16:
				writer.Int(field.as<int16_t>());
				continue;
			case gl_Type_Column_PostgreSQL_Int32:
				writer.Int(field.as<int32_t>());
				continue;
			case gl_Type_Column_PostgreSQL_Int64:
				writer.Int64(field.as<int64_t>());
				continue;
			case gl_Type_Column_PostgreSQL_Float:
				writer.Double(field.as<float>());
				continue;
			case gl_Type_Column_PostgreSQL_Double:
				writer.Double(field.as<double>());
				continue;
			case gl_Type_Column_PostgreSQL_LongDouble:
				writer.Double(field.as<long double>());
				continue;
			case gl_Type_Column_PostgreSQL_Boolean:
				writer.Bool(field.as<bool>());
				continue;
			default: {
				if (!field.is_null()) {
					const auto str = pqxx::to_string(field);
					writer.String(str.c_str(), static_cast<rapidjson::SizeType>(str.length()));
				} else
					writer.Null();

				continue;
			}
		}
	}
	writer.EndObject();
}

template <typename type>
void PostgreSQL::streamResultToJson(pqxx::stream_from& stream, rapidjson::Writer<type>& writer) {
	writer.StartArray();

	auto currRow = stream.read_row();
	while (currRow) {
		writer.StartArray();

		for (const auto& field : *currRow)
			writer.String(field.c_str(), static_cast<rapidjson::SizeType>(field.size()), true);

		writer.EndArray();
		currRow = stream.read_row();
	}

	writer.EndArray();
}

void PostgreSQL::clearAllThreads() {
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

#pragma endregion
