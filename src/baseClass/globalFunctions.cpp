#include <baseClass/globalFunctions.hpp>

#pragma region Конвертации строк с 1С

WCHAR_T* gl_ConvToShortWchar(WCHAR_T** dest, const wchar_t* source, size_t len) {
#ifdef __linux__

	const auto tmp = boost::locale::conv::utf_to_utf<WCHAR_T>(source);
	if (!len)
		len = tmp.length() + 1;

	WCHAR_T* res;
	if (dest == nullptr)
		res = new WCHAR_T[len];
	else {
		res = *dest;
		if (!res)
			res = new WCHAR_T[len];
	}

	memset(res, 0, len * sizeof(WCHAR_T));
	for (size_t i = 0; i < len; i++)
		res[i] = static_cast<wchar_t>(source[i]);

	return res;

#else

	if (!len)
		len = wcslen(source) + 1;

	WCHAR_T* res;
	if (dest == nullptr)
		res = new WCHAR_T[len];
	else {
		res = *dest;
		if (!res)
			res = new WCHAR_T[len];
	}

	memcpy(res, source, len * sizeof(WCHAR_T));
	return res;

#endif
}

wchar_t* gl_ConvFromShortWchar(const WCHAR_T* source, size_t len) {
#ifdef __linux__
	const auto tmp = boost::locale::conv::utf_to_utf<WCHAR_T>(source);
	if (!len)
		len = tmp.length() + 1;

	auto* res = new wchar_t[len];
	memset(res, 0, len * sizeof(wchar_t));

	for (size_t i = 0; i < len; i++)
		res[i] = static_cast<wchar_t>(source[i]);

	return res;
#else
	if (!len)
		len = wcslen(source) + 1;

	auto* res = new wchar_t[len];
	memcpy(res, source, len * sizeof(wchar_t));

	return res;
#endif
}

#pragma endregion

#pragma region Строковые методы

bool gl_IEqualsCaseInsensitive(const wchar_t* first, const wchar_t* second) {
#ifdef __linux__
	return wcscasecmp(first, second) == 0;
#else
	return _wcsicmp(first, second) == 0;
#endif
}

bool gl_IEqualsCaseInsensitive(const char* first, const char* second) {
#ifdef __linux__
	return strcasecmp(first, second) == 0;
#else
	return _stricmp(first, second) == 0;
#endif
}

std::wstring gl_StringToWString(const std::string& str) {
	Converter_Type convert;
	try {
		return convert.from_bytes(str);
	} catch (...) {
		const auto tmp = utf8::replace_invalid(str);
		return convert.from_bytes(tmp);
	}
}

std::wstring gl_StringToWString(const char* str) {
	Converter_Type convert;
	try {
		return convert.from_bytes(str);
	}
	catch (...) {
		const auto tmp = utf8::replace_invalid(str);
		return convert.from_bytes(tmp);
	}
}

std::string gl_WStringToString(const std::wstring& wstr) {
	Converter_Type convert;
	return convert.to_bytes(wstr);
}

std::string gl_WStringToString(const wchar_t* wstr) {
	Converter_Type convert;
	return convert.to_bytes(wstr);
}

char* gl_CopyStringToChar(const std::string& str, size_t len) {
	if (len == 0)
		len = str.size();

	const auto result = new char[len + 1];

	memcpy(result, str.c_str(), (len + 1) * sizeof(char));
	return result;
}

wchar_t* gl_CharToWChar(const char* str, size_t len) {
	if (len == 0)
		len = strlen(str);

	const auto result = new wchar_t[len + 1];

	memcpy(result, gl_StringToWString(str).c_str(), (len + 1) * sizeof(wchar_t));
	return result;
}

wchar_t* gl_CharToWChar(const std::string& str, size_t len) {
	if (len == 0)
		len = str.size();

	const auto result = new wchar_t[len + 1];

	memcpy(result, gl_StringToWString(str).c_str(), (len + 1) * sizeof(wchar_t));
	return result;
}

char* gl_WCharToChar(const wchar_t* str, size_t len) {
	if (len == 0)
		len = wcslen(str);

	const auto result = new char[len + 1];

	memcpy(result, gl_WStringToString(str).c_str(), (len + 1) * sizeof(char));
	return result;
}

char* gl_WCharToChar(const std::wstring& str, size_t len) {
	if (len == 0)
		len = str.size();

	const auto result = new char[len + 1];

	memcpy(result, gl_WStringToString(str).c_str(), (len + 1) * sizeof(char));
	return result;
}

wchar_t* gl_CopyWStringToWChar(const std::wstring& str, size_t len) {
	if (len == 0)
		len = str.size();

	const auto result = new wchar_t[len + 1];

	memcpy(result, str.c_str(), (len + 1) * sizeof(wchar_t));
	return result;
}

#pragma endregion

#pragma region Работа с JSON

rapidjson::Document gl_GetInputJson(const std::string& str) {
	rapidjson::Document document;
	rapidjson::StringStream s(str.c_str());

	document.ParseStream(s);
	if (document.HasParseError())
		throw std::runtime_error(rapidjson::GetParseError_En(document.GetParseError()));
	
	return document;
}

rapidjson::Document gl_GetInputJson(const std::wstring& str) {
	return gl_GetInputJson(gl_WStringToString(str));
}

rapidjson::Document gl_GetInputJsonFromFile(const std::wstring& str) {
#ifdef __linux__
	const auto fp = gl_OpenFile(str, L"r+");
#else
	const auto fp = gl_OpenFile(str, L"rb+");
#endif

	if (!fp)
		throw std::runtime_error("Не удалось открыть JSON");

	char readBuffer[gl_Chunk_Size];
	rapidjson::FileReadStream bis(fp, readBuffer, sizeof(readBuffer));
	rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::FileReadStream> eis(bis);

	rapidjson::Document document;
	document.ParseStream<0, rapidjson::UTF8<>>(eis);
	fclose(fp);

	if (document.HasParseError())
		throw std::runtime_error(rapidjson::GetParseError_En(document.GetParseError()));

	return document;
}

std::string gl_JsonToString(const rapidjson::Document& json) {
	rapidjson::StringBuffer buffer;
	rapidjson::Writer writer(buffer);
	json.Accept(writer);
	return buffer.GetString();
}

std::wstring gl_JsonToWString(const rapidjson::Document& json) {
	rapidjson::StringBuffer buffer;
	rapidjson::Writer writer(buffer);
	json.Accept(writer);
	return gl_StringToWString(buffer.GetString());
}

std::wstring gl_JsonToFile(const rapidjson::Document& json, std::wstring fileName) {
	if (fileName.empty())
		fileName = gl_TmpFileNameW(L"json");

#ifdef __linux__
	const auto fp = gl_OpenFile(fileName.c_str(), L"w, ccs=UTF-8");
#else
	const auto fp = gl_OpenFile(fileName.c_str(), L"wb, ccs=UTF-8");
#endif

	if (!fp)
		throw std::runtime_error("Не удалось открыть файл для записи результата");

	char writeBuffer[gl_Chunk_Size];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

	rapidjson::Writer writer(os);
	json.Accept(writer);

	fclose(fp);
	return fileName;
}

#pragma endregion

#pragma region Работа с файлами

FILE* gl_OpenFile(const wchar_t* fileName, const wchar_t* modeOpen) {
#ifdef __linux__
	const auto fileNameChar = gl_WCharToChar(fileName);
	const auto modeOpenChar = gl_WCharToChar(modeOpen);

	const auto result = fopen(fileNameChar, modeOpenChar);

	delete[] fileNameChar;
	delete[] modeOpenChar;

	return result;
#else
	return _wfopen(fileName, modeOpen);
#endif
}

FILE* gl_OpenFile(const std::wstring& fileName, const wchar_t* modeOpen) {
	return gl_OpenFile(fileName.c_str(), modeOpen);
}

FILE* gl_OpenFile(const char* fileName, const char* modeOpen) {
	return fopen(fileName, modeOpen);
}

FILE* gl_OpenFile(const std::string& fileName, const char* modeOpen) {
	return gl_OpenFile(fileName.c_str(), modeOpen);
}

std::wstring gl_ReadFileToWString(const std::wstring& fileName) {
#ifdef __linux__
	std::wifstream wif(gl_WStringToString(fileName));
#else
	std::wifstream wif(fileName);
#endif

	wif.imbue(gl_Locale_For_Stream);

	std::wstringstream wss;
	wss << wif.rdbuf();
	wif.close();

	return wss.str();
}

std::wstring gl_ReadFileToWString(const std::string& fileName) {
	return gl_ReadFileToWString(gl_StringToWString(fileName));
}

void gl_WriteStringToFile(const std::wstring& fileName, const std::wstring& data) {
#ifdef __linux__
	std::wofstream wof(gl_WStringToString(fileName));
#else
	std::wofstream wof(fileName);
#endif

	wof.imbue(gl_Locale_For_Stream);
	wof << data;
	wof.close();
}

void gl_WriteStringToFile(const std::wstring& fileName, const std::string& data) {
	gl_WriteStringToFile(fileName, gl_StringToWString(data));
}

void gl_WriteStringToFile(const std::string& fileName, const std::wstring& data) {
	gl_WriteStringToFile(gl_StringToWString(fileName), data);
}

void gl_WriteStringToFile(const std::string& fileName, const std::string& data) {
	gl_WriteStringToFile(gl_StringToWString(fileName), gl_StringToWString(data));
}

#pragma endregion

#pragma region Остальные методы

std::string gl_GetNewUuid() {
	return uuids::to_string(uuids::uuid_system_generator{}());
}

std::wstring gl_GetNewUuidW() {
	return gl_StringToWString(gl_GetNewUuid());
}

std::string gl_TmpFileName(const char* expansion) {
#ifndef __linux__
	char chBuffer[MAX_PATH];
	GetTempPathA(MAX_PATH, chBuffer);
	
	const std::string sTempPath = chBuffer;
	GetLongPathNameA(sTempPath.c_str(), chBuffer, MAX_PATH);

	auto fileName = std::string(chBuffer);
#else
	auto fileName = std::filesystem::temp_directory_path();
#endif

	fileName.append(gl_GetNewUuid());

	if (!boost::algorithm::starts_with(expansion, "."))
		fileName.append(".");

	fileName.append(expansion);

	std::ofstream ofs(fileName);
	ofs.close();

	return fileName;
}

std::wstring gl_TmpFileNameW(const wchar_t* expansion) {
#ifndef __linux__
	wchar_t chBuffer[gl_Chunk_Size];
	GetTempPathW(gl_Chunk_Size, chBuffer);

	const std::wstring sTempPath = chBuffer;
	GetLongPathNameW(sTempPath.c_str(), chBuffer, gl_Chunk_Size);

	auto fileName = std::wstring(chBuffer);
	fileName.append(gl_GetNewUuidW());

	if (!boost::algorithm::starts_with(expansion, L"."))
		fileName.append(L".");

	fileName.append(expansion);

	std::wofstream ofs(fileName);
	ofs.close();

	return fileName;
#else

	auto fileName = gl_TmpFileName(gl_WStringToString(expansion).c_str());
	return gl_StringToWString(fileName);

#endif
}

#pragma endregion
