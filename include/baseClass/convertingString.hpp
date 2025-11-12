
#ifndef __CONVERTING_STRING_H__
#define __CONVERTING_STRING_H__

#include "baseClass/from1C/types.h"

#ifdef _WINDOWS
	#include <xstring>
#else
	#include <string>
	#include <wchar.h>
#endif

#include <uni_algo/all.h>

// Вся работа ведётся в wchar_t. Но чтобы в 1С всё красиво шло, необходимы некоторые преобразования.
std::basic_string<WCHAR_T> gl_conv_string_to_WCHAR_T(const std::string& src); // Преобразует string для возврата в 1С
std::wstring gl_conv_string_to_wstring(const std::string& src); // Преобразует string в wstring

std::string gl_conv_WCHAR_T_to_string(const std::basic_string_view<WCHAR_T>& src); // Преобразует строку из 1С в string
std::string gl_conv_wstring_to_string(const std::wstring& src); // Преобразует wstring в string

#ifdef _WINDOWS

	#define gl_conv_wstring_to_WCHAR_T(src) (src) // Преобразования не требуются, в 1С 2-х байтовая строка для WIN
	#define gl_conv_WCHAR_T_to_wstring(src) (src) // Преобразования не требуются, в 1С 2-х байтовая строка для WIN

#else
	
	std::basic_string<WCHAR_T> gl_conv_wstring_to_WCHAR_T(const std::wstring& src); // Преобразует wstring для возврата в 1С
	std::wstring gl_conv_WCHAR_T_to_wstring(const std::basic_string_view<WCHAR_T>& src); // Преобразует строку из 1С в wstring

#endif

// Сравнивает 2 любые строки без учёта регистра
bool gl_str_iequal(const std::wstring& str1, const std::wstring& str2);
bool gl_str_iequal(const std::string& str1, const std::string& str2);

#ifndef _WINDOWS
	bool gl_str_iequal(const std::basic_string<WCHAR_T>& str1, const std::basic_string<WCHAR_T>& str2);
#endif

#endif
