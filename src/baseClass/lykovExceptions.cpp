#include <baseClass/lykovExceptions.hpp>

LykovException::LykovException(const wchar_t* description, const wchar_t* source, const unsigned short wcode, const long codeError) : std::runtime_error("") {
	m_Descriptor = description;
	m_Source     = source;
	m_WCode      = wcode;
	m_CodeError  = codeError;

	m_NeedDeleteDescriptor = false;
	setMessages();
}

LykovException::LykovException(const std::wstring& description, const wchar_t* source, const unsigned short wcode, const long codeError) : std::runtime_error("") {
	m_Descriptor = gl_CopyWStringToWChar(description);
	m_Source = source;
	m_WCode = wcode;
	m_CodeError = codeError;

	setMessages();
	m_NeedDeleteDescriptor = true;
}

LykovException::LykovException(const char* description, const wchar_t* source, const unsigned short wcode, const long codeError) : std::runtime_error(description) {
	m_Descriptor = gl_CharToWChar(description);
	m_Source = source;
	m_WCode = wcode;
	m_CodeError = codeError;

	setMessages();
	m_NeedDeleteDescriptor = true;
}

LykovException::LykovException(const std::string& description, const wchar_t* source, const unsigned short wcode, const long codeError) : std::runtime_error(description) {
	m_Descriptor = gl_CharToWChar(description);
	m_Source = source;
	m_WCode = wcode;
	m_CodeError = codeError;

	setMessages();
	m_NeedDeleteDescriptor = true;
}

LykovException::LykovException(const LykovException& from) : std::runtime_error("") {
	m_Descriptor = from.getDescription();
	m_Source = from.getSource();
	m_WCode = from.getWCode();
	m_CodeError = from.getCodeError();

	setMessages();
	m_NeedDeleteDescriptor = false;
}

LykovException::~LykovException() {
	delete[] m_MessageWhatW;
	delete[] m_MessageWhat;

	if (m_NeedDeleteDescriptor)
		delete[] m_Descriptor;
}

const char* LykovException::what() const noexcept {
	return m_MessageWhat;
}

const wchar_t* LykovException::whatW() const noexcept {
	return m_MessageWhatW;
}

const wchar_t* LykovException::getSource() const {
	return m_Source;
}

const wchar_t* LykovException::getDescription() const {
	return m_Descriptor;
}

unsigned short LykovException::getWCode() const {
	return m_WCode;
}

long LykovException::getCodeError() const {
	return m_CodeError;
}

void LykovException::setMessages() {
	const auto result = boost::str(boost::wformat(L"%s: %s%s")
		% m_Source
		% m_Descriptor
		% (m_CodeError != -1 ? boost::str(boost::wformat(L" (код ошибки %d)") % m_CodeError) : L""));

	const auto size = result.size();

	m_MessageWhatW = gl_CopyWStringToWChar(result, size);
	m_MessageWhat = gl_WCharToChar(result, size);
}
