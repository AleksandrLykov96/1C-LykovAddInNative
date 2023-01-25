#include <extensions/mySQLConn.hpp>

#pragma region Переопределение методов

MySqlConn::MySqlConn() {
	conn = mysql_init(nullptr);
	mysql_options(conn, MYSQL_READ_DEFAULT_GROUP, "LYKOV_ADD_IN_NATIVE");
}

MySqlConn::~MySqlConn() {
	mysql_close(conn);
}

const wchar_t* MySqlConn::getNameExtension() {
	return L"MySqlConn";
}

void MySqlConn::setMethodPropsExtension() {
	// Свойства
	m_PropNames[gl_Index_Prop_MySQL_Connection_Info] = NamesFor1C(L"ИнформацияОСоединении", L"ConnectionInfo");

	// Методы
	m_MethodNames[gl_Index_Method_MySQL_Connect_Db] = NamesFor1C(L"УстановитьСоединение", L"ConnectDB");
	m_MethodNames[gl_Index_Method_MySQL_Exec]       = NamesFor1C(L"ВыполнитьЗапрос", L"Exec");
}

void MySqlConn::getPropByIndex(const unsigned short indexProp, tVariant* pvarPropVal) {
	switch (indexProp) {
		case gl_Index_Prop_MySQL_Connection_Info: {
			if (!conn->host_info)
				setReturnedParam("", pvarPropVal);
			else
				setReturnedParam<const std::string&>(std::string(conn->host_info), pvarPropVal);

			return;
		}
		default:
			return;
	}
}

void MySqlConn::setPropByIndex(const unsigned short indexProp, tVariant* varPropVal) {
	
}

bool MySqlConn::getIsPropWritable(const unsigned short indexProp) {
	return false;
}

long MySqlConn::getMethodNParams(const unsigned short indexMethod) {
	switch (indexMethod) {
		case gl_Index_Method_MySQL_Connect_Db:
			return 5l;
		case gl_Index_Method_MySQL_Exec:
			return 2l;
		default:
			return 0l;
	}
}

void MySqlConn::setParamDefValue(const unsigned short indexMethod, const long indexParam, tVariant* pvarParamDefValue) {
	switch (indexMethod) {
		case gl_Index_Method_MySQL_Connect_Db: {
			if (indexParam == 4l) {
				setReturnedParam(0, pvarParamDefValue);
				return;
			}

			break;
		}
		case gl_Index_Method_MySQL_Exec: {
			if (indexParam == 1l) {
				setReturnedParam("", pvarParamDefValue);
				return;
			}

			break;
		}
		default:
			break;
	}

	TV_VT(pvarParamDefValue) = VTYPE_EMPTY;
}

bool MySqlConn::getHasRetVal(const unsigned short indexMethod) {
	switch (indexMethod) {
		case gl_Index_Method_MySQL_Exec:
			return true;
		default:
			return false;
	}
}

void MySqlConn::callMethodAsProc(const unsigned short indexMethod, const tVariant* paParams, const long paParamsSize) {
	switch (indexMethod) {
		case gl_Index_Method_MySQL_Connect_Db:
			connectToDb(paParams);
			return;
		default:
			break;
	}
}

void MySqlConn::callMethodAsFunc(const unsigned short indexMethod, tVariant* pvarRetValue, const tVariant* paParams, const long paParamsSize) {
	switch (indexMethod) {
		case gl_Index_Method_MySQL_Exec:
			exec(paParams, pvarRetValue);
			return;
		default:
			return;
	}
}

#pragma endregion

#pragma region Функции компоненты

void MySqlConn::connectToDb(const tVariant* paParams) const {
	const auto host = getInputParam<std::string>(paParams, 0l);
	const auto user = getInputParam<std::string>(paParams, 1l);
	const auto passwd = getInputParam<std::string>(paParams, 2l);
	const auto nameDB = getInputParam<std::string>(paParams, 3l);
	const auto port = getInputParam<unsigned int>(paParams, 4l);

	if (!mysql_real_connect(conn, host.c_str(), user.c_str(), passwd.c_str(), nameDB.c_str(), port, nullptr, 0))
		throw LykovException(mysql_error(conn), L"Ошибка при подключении к MySQL");
}

void MySqlConn::exec(const tVariant* paParams, tVariant* pvarRetValue) const {
	const auto request     = getInputParam<std::string>(paParams, 0l);
	const auto fileResult  = getInputParam<std::wstring>(paParams, 1l);
	const auto toFile      = !fileResult.empty();

	if(mysql_query(conn, request.c_str()) != 0)
		throw LykovException(mysql_error(conn), L"Ошибка при выполнении запроса");

	const auto result = mysql_use_result(conn);
	const auto numFields = mysql_num_fields(result);
	const auto fields = mysql_fetch_fields(result);

	std::map<unsigned, ColumnInfo> columns;
	for (unsigned i = 0; i < numFields; i++) {
		switch (fields[i].type) {
			case MYSQL_TYPE_DECIMAL:
			case MYSQL_TYPE_TINY:
			case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_INT24:
			case MYSQL_TYPE_YEAR:
			case MYSQL_TYPE_NEWDECIMAL:
				columns[i] = ColumnInfo(fields[i].name, gl_Type_Column_MySQL_Int, fields[i].name_length);
				break;
			case MYSQL_TYPE_LONGLONG:
			case MYSQL_TYPE_LONG:
				columns[i] = ColumnInfo(fields[i].name, gl_Type_Column_MySQL_Long, fields[i].name_length);
				break;
			case MYSQL_TYPE_FLOAT:
			case MYSQL_TYPE_DOUBLE:
				columns[i] = ColumnInfo(fields[i].name, gl_Type_Column_MySQL_Double, fields[i].name_length);
				break;
			default:
				columns[i] = ColumnInfo(fields[i].name, gl_Type_Column_MySQL_String, fields[i].name_length);
				break;
		}
	}

	MYSQL_ROW row;

	if (!toFile) {
		rapidjson::StringBuffer json;
		rapidjson::Writer writer(json);

		writer.StartArray();
		while ((row = mysql_fetch_row(result))) {
			writer.StartObject();
			for (const auto &it : columns) {
				writer.Key(it.second.name, it.second.nameLength);
				if (row[it.first] == nullptr) {
					writer.Null();
					continue;
				}

				switch (it.second.type) {
					case gl_Type_Column_MySQL_Int:
						writer.Int(std::atoi(row[it.first]));
						break;
					case gl_Type_Column_MySQL_Long:
						writer.Int64(std::atol(row[it.first]));
						break;
					case gl_Type_Column_MySQL_Double:
						writer.Double(std::atof(row[it.first]));
						break;
					default:
						writer.String(row[it.first]);
				}
			}
			writer.EndObject();
		}
		writer.EndArray();

		setReturnedParam<const std::wstring&>(gl_StringToWString(json.GetString()), pvarRetValue);
	} else {
#ifdef __linux__
		const auto fp = gl_OpenFile(fileResult, L"w");
#else
		const auto fp = gl_OpenFile(fileResult, L"wb");
#endif

		char writeBuffer[gl_Chunk_Size];
		rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		rapidjson::Writer writer(os);

		writer.StartArray();
		while ((row = mysql_fetch_row(result))) {
			writer.StartObject();
			for (const auto& it : columns) {
				writer.Key(it.second.name, it.second.nameLength);
				if (row[it.first] == nullptr) {
					writer.Null();
					continue;
				}

				switch (it.second.type) {
				case gl_Type_Column_MySQL_Int:
					writer.Int(std::atoi(row[it.first]));
					break;
				case gl_Type_Column_MySQL_Long:
					writer.Int64(std::atol(row[it.first]));
					break;
				case gl_Type_Column_MySQL_Double:
					writer.Double(std::atof(row[it.first]));
					break;
				default:
					writer.String(row[it.first]);
				}
			}
			writer.EndObject();
		}
		writer.EndArray();

		fclose(fp);
		setReturnedParam(L"", pvarRetValue);
	}

	mysql_free_result(result);
}

#pragma endregion
