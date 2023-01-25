#ifndef __LYKOV_EXCEPTION_HPP__
#define __LYKOV_EXCEPTION_HPP__

#include <baseClass/globalFunctions.hpp>

class LykovException final : public std::runtime_error
{
public:
	explicit LykovException(const wchar_t*, const wchar_t* = L"Lykov_AddInNative", unsigned short = ADDIN_E_MSGBOX_FAIL, long = -1);
	explicit LykovException(const std::wstring&, const wchar_t* = L"Lykov_AddInNative", unsigned short = ADDIN_E_MSGBOX_FAIL, long = -1);
	explicit LykovException(const char*, const wchar_t* = L"Lykov_AddInNative", unsigned short = ADDIN_E_MSGBOX_FAIL, long = -1);
	explicit LykovException(const std::string&, const wchar_t* = L"Lykov_AddInNative", unsigned short = ADDIN_E_MSGBOX_FAIL, long = -1);
	LykovException(const LykovException&);
	~LykovException() override;
	[[nodiscard]] const char* what() const noexcept override;
	[[nodiscard]] const wchar_t* whatW() const noexcept;

	[[nodiscard]] const wchar_t* getSource() const;
	[[nodiscard]] const wchar_t* getDescription() const;
	[[nodiscard]] unsigned short getWCode() const;
	[[nodiscard]] long getCodeError() const;
private:
	bool m_NeedDeleteDescriptor;

	const wchar_t* m_Source; // Источник ошибки
	const wchar_t* m_Descriptor; // Описание ошибки
	unsigned short m_WCode; // Тип ошибки ошибки ADDIN_E_INFO
	long m_CodeError; // Код ошибки

	const wchar_t* m_MessageWhatW;
	const char* m_MessageWhat;

	void setMessages();
};

#endif
