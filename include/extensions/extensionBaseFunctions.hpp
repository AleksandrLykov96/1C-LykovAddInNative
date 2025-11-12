
#ifndef __EXTENSION_BASE_FUNCTIONS_HPP_
#define __EXTENSION_BASE_FUNCTIONS_HPP_

#include "baseClass/baseClassForExtension.hpp"
#include "baseClass/encrypt.hpp"

#include <regex>
#include <zlib.h>

constexpr unsigned int buffer_size = 131072;

class BaseFunctions final : public native1C::IBaseExtensionClass {
public:
	BOOST_DEFAULTED_FUNCTION(BaseFunctions(), { })
	BOOST_DEFAULTED_FUNCTION(~BaseFunctions() BOOST_OVERRIDE, { })
	BOOST_DELETED_FUNCTION(BaseFunctions(BaseFunctions&&) BOOST_NOEXCEPT)
	BOOST_DELETED_FUNCTION(BaseFunctions(const BaseFunctions&))
	BOOST_DELETED_FUNCTION(BaseFunctions& operator=(const BaseFunctions&))
	BOOST_DELETED_FUNCTION(BaseFunctions& operator=(BaseFunctions&&))


protected:
	virtual std::wstring getNameExtension() BOOST_OVERRIDE;
	virtual void initializeComponent() BOOST_OVERRIDE;


private:
	std::map<std::wstring, long long> p_MapTimer;

	// Методы компоненты
	void pause(const native1C::all_input_types&); // Sleep текущего потока
	native1C::all_returned_types stringToNumber(const native1C::all_input_types&); // Преобразует строку в число (удаляя лишние символы).
	native1C::all_returned_types correctString(const native1C::all_input_types&, const native1C::all_input_types&); // Оставляет в строке только символы из 2-го параметра
	native1C::all_returned_types deflate1C(const native1C::all_input_types&, const native1C::all_input_types&, const native1C::all_input_types&,
	                                       const native1C::all_input_types&, const native1C::all_input_types&, const native1C::all_input_types&); // Сжатие Deflate
	native1C::all_returned_types inflate1C(const native1C::all_input_types&, const native1C::all_input_types&, const native1C::all_input_types&); // Разжатие Deflate
	native1C::all_returned_types unixTimestamp(); // Текущий UnixTimestamp
	native1C::all_returned_types interruptHandler(); // Возвращает флаг того, что нажато сочетание клавиш ctrl + Break
	native1C::all_returned_types regexMatch(const native1C::all_input_types&, const native1C::all_input_types&); // Regex
	native1C::all_returned_types encrypt1C(const native1C::all_input_types&, const native1C::all_input_types&); // Шифрование строки (методом Кузнечик)
	native1C::all_returned_types decrypt1C(const native1C::all_input_types&, const native1C::all_input_types&); // Расшифровка строки (см. encrypt1C)
	void startTimer(const native1C::all_input_types&); // Инициализирует начало таймера
	native1C::all_returned_types stopTimer(const native1C::all_input_types&); // Возвращает время в секундах после начало таймера

	// Вспомогательные методы
	// Deflate
	static void deflateFromFileToFile(z_stream&, const std::wstring&, const std::wstring&);
	static std::vector<char> deflateFromFileToVector(z_stream&, const std::wstring&);
	static void deflateFromStringToFile(z_stream&, const std::wstring&, const std::wstring&);
	static std::vector<char> deflateFromStringToVector(z_stream&, const std::wstring&);

	// Inflate
	static void inflateFromFileToFile(z_stream&, const std::wstring&, const std::wstring&);
	static std::vector<char> inflateFromFileToVector(z_stream&, const std::wstring&);
	static void inflateFromVectorToFile(z_stream&, const std::vector<char>&, const std::wstring&);
	static std::vector<char> inflateFromVectorToVector(z_stream& stream, const std::vector<char>&);
};

#endif
