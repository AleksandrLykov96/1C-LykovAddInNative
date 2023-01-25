
#ifndef __GLOBAL_FUNCTION_HPP__
#define __GLOBAL_FUNCTION_HPP__

#define RAPIDJSON_ASSERT(x) ((void)(x))

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_BEAST_USE_STD_STRING_VIEW

#define RAPIDJSON_HAS_STDSTRING 1
#define RAPIDJSON_VALUE_DEFAULT_ARRAY_CAPACITY 32
#define RAPIDJSON_VALUE_DEFAULT_OBJECT_CAPACITY 32
#define RAPIDJSON_USE_MEMBERSMAP 1

#define UUID_SYSTEM_GENERATOR

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#ifndef __linux__
	#define RAPIDJSON_SIMD
	#define RAPIDJSON_SSE42
	#define BOOST_UUID_USE_SSE41
#endif

#include <from1C/types.h>

#include <execution>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <codecvt>
#include <execution>
#include <thread>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/encodedstream.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/error.h>
#include <rapidjson/error/en.h>
#include <uuid.h>
#include <utf8/cpp17.h>

// Глобальные переменные
using Converter_Type = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>;
constexpr unsigned int gl_Chunk_Size = 65536;
const std::locale gl_Locale_For_Stream = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());

// Конвертации строк с 1С
WCHAR_T* gl_ConvToShortWchar(WCHAR_T**, const wchar_t*, size_t = 0); // Конвертировать wchar_t в 1С-кий строковый тип
wchar_t* gl_ConvFromShortWchar(const WCHAR_T*, size_t = 0); // Конвертировать входной из 1С строковый тип в wchar_t

// Строковые методы
bool gl_IEqualsCaseInsensitive(const wchar_t*, const wchar_t*); // Сравнение строк без учёта регистра
bool gl_IEqualsCaseInsensitive(const char*, const char*); // Сравнение строк без учёта регистра
std::wstring gl_StringToWString(const std::string&); // Конвертировать String to WString
std::wstring gl_StringToWString(const char*); // Конвертировать char* to WString
std::string gl_WStringToString(const std::wstring&); // Конвертировать WString to String
std::string gl_WStringToString(const wchar_t*); // Конвертировать WString to String
char* gl_CopyStringToChar(const std::string&, size_t = 0); // Скопировать строку string с выделением памяти
wchar_t* gl_CharToWChar(const char*, size_t = 0); // Конвертировать char в wchar_t. Чистить за собой память.
wchar_t* gl_CharToWChar(const std::string&, size_t = 0); // Конвертировать string в wchar_t. Чистить за собой память.
char* gl_WCharToChar(const wchar_t*, size_t = 0); // Конвертировать wchar_t в wchar_t. Чистить за собой память.
char* gl_WCharToChar(const std::wstring&, size_t = 0); // Конвертировать wstring в wchar_t. Чистить за собой память.
wchar_t* gl_CopyWStringToWChar(const std::wstring&, size_t = 0); // Скопировать строку string с выделением памяти. Чистить за собой память.

// Работа с JSON
rapidjson::Document gl_GetInputJson(const std::string&); // Конвертировать входную строку в JSON
rapidjson::Document gl_GetInputJson(const std::wstring&); // Конвертировать входную строку в JSON
rapidjson::Document gl_GetInputJsonFromFile(const std::wstring&); // Конвертировать файл в JSON
std::string gl_JsonToString(const rapidjson::Document&);
std::wstring gl_JsonToWString(const rapidjson::Document&);
std::wstring gl_JsonToFile(const rapidjson::Document&, std::wstring = L""); // Конвертировать JSON в файл

// Работа с файлами
FILE* gl_OpenFile(const wchar_t*, const wchar_t*);
FILE* gl_OpenFile(const std::wstring&, const wchar_t*);
FILE* gl_OpenFile(const char*, const char*);
FILE* gl_OpenFile(const std::string*, const char*);

std::wstring gl_ReadFileToWString(const std::string&); // Прочитать файл в строку
std::wstring gl_ReadFileToWString(const std::wstring&); // Прочитать файл в строку
void gl_WriteStringToFile(const std::wstring&, const std::string&); // Записать строку в файл
void gl_WriteStringToFile(const std::string&, const std::string&); // Записать строку в файл
void gl_WriteStringToFile(const std::string&, const std::wstring&); // Записать строку в файл
void gl_WriteStringToFile(const std::wstring&, const std::wstring&); // Записать строку в файл

// Остальные методы
std::string gl_GetNewUuid(); // Уникальный идентификатор
std::wstring gl_GetNewUuidW(); // Уникальный идентификатор (wstring)
std::string gl_TmpFileName(const char* = "tmp"); // Подготовить временный файл
std::wstring gl_TmpFileNameW(const wchar_t* = L"tmp"); // Подготовить временный файл

#endif
