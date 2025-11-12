
#include "baseClass/convertingString.hpp"

using namespace una;

std::basic_string<WCHAR_T> gl_conv_string_to_WCHAR_T(const std::string& src) {
	const auto normalString = norm::to_nfc_utf8(src);
	return utf8to16<char, WCHAR_T>(normalString);
}

std::wstring gl_conv_string_to_wstring(const std::string& src) {
	const auto normalString = norm::to_nfc_utf8(src);
	return utf8to16<char, wchar_t>(normalString);
}

std::string gl_conv_WCHAR_T_to_string(const std::basic_string_view<WCHAR_T>& src) {
	return utf16to8<WCHAR_T, char>(src);
}

std::string gl_conv_wstring_to_string(const std::wstring& src) {
	return utf16to8<wchar_t, char>(src);
}

bool gl_str_iequal(const std::wstring& str1, const std::wstring& str2) {
	#ifdef _WIN32
		return _wcsicmp(str1.c_str(), str2.c_str()) == 0;
	#else
		return wcscasecmp(str1.c_str(), str2.c_str()) == 0;
	#endif
}

bool gl_str_iequal(const std::string& str1, const std::string& str2) {
#ifndef _WIN32
	return strcasecmp(str1.c_str(), str2.c_str()) == 0;
#else
	return _stricmp(str1.c_str(), str2.c_str()) == 0;
#endif
}

#ifndef _WINDOWS

std::basic_string<WCHAR_T> gl_conv_wstring_to_WCHAR_T(const std::wstring& src) {
	return utf32to16<wchar_t, WCHAR_T>(src);
}

std::wstring gl_conv_WCHAR_T_to_wstring(const std::basic_string_view<WCHAR_T>& src) {
	return utf16to32<WCHAR_T, wchar_t>(src);
}

bool gl_str_iequal(const std::basic_string<WCHAR_T>& str1, const std::basic_string<WCHAR_T>& str2) {
	const auto& tmp1 = gl_conv_WCHAR_T_to_wstring(str1);
	const auto& tmp2 = gl_conv_WCHAR_T_to_wstring(str2);

	return gl_str_iequal(tmp1, tmp2);
}

#endif
