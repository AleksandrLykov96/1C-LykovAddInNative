#ifndef __EXTENSION_BASE_FUNCTION__
#define __EXTENSION_BASE_FUNCTION__

#include <baseClass/baseExtensionClass.hpp>

#include <zlib-ng.h>
#include <chrono>
#include <cwctype>

#ifndef __linux__
	#include <atlimage.h>
	#include <conio.h>
#endif

constexpr unsigned short gl_Index_Method_Pause                  = gl_Index_Last_Method + 1;
constexpr unsigned short gl_Index_Method_String_To_Number       = gl_Index_Last_Method + 2;
constexpr unsigned short gl_Index_Method_Clear_String           = gl_Index_Last_Method + 3;
constexpr unsigned short gl_Index_Method_Clear_String_Json      = gl_Index_Last_Method + 4;
constexpr unsigned short gl_Index_Method_Compress               = gl_Index_Last_Method + 5;
constexpr unsigned short gl_Index_Method_Decompress             = gl_Index_Last_Method + 6;
constexpr unsigned short gl_Index_Method_Current_Unix_Timestamp = gl_Index_Last_Method + 7;
constexpr unsigned short gl_Index_Method_Get_Screenshot         = gl_Index_Last_Method + 8;
constexpr unsigned short gl_Index_Method_Interrupt_Handler      = gl_Index_Last_Method + 9;
constexpr unsigned short gl_Index_Method_UUID                   = gl_Index_Last_Method + 10;
constexpr unsigned short gl_Index_Method_StartTimer             = gl_Index_Last_Method + 11;
constexpr unsigned short gl_Index_Method_EndTimer               = gl_Index_Last_Method + 12;

class BaseFunction final : public IBaseExtensionClass
{
public:
	BaseFunction();
	~BaseFunction() override = default;

	// Переопределяемые методы
	const wchar_t* getNameExtension() override; // Должен возвращать фактическое наименование расширения
	const wchar_t* getVersion() override; // Должен возвращать номер версии компоненты
	void setMethodPropsExtension() override; // Инициализация компоненты (дополнение методов и свойств компоненты)
	void getPropByIndex(unsigned short, tVariant*) override; // Должен положить значение параметра по индексу в tVariant или вызвать исключение (например, если свойство не найдено)
	void setPropByIndex(unsigned short, tVariant*) override; // Должен установить значение параметра по индексу в tVariant
	bool getIsPropWritable(unsigned short) override; // Должен вернуть доступность записи параметра по индексу
	long getMethodNParams(unsigned short) override; // Должен вернуть количество параметров метода
	void setParamDefValue(unsigned short, long, tVariant*) override; // Должен установить значения параметров по умолчанию
	bool getHasRetVal(unsigned short) override; // Должен вернуть признак наличия возвращаемого значения
	void callMethodAsProc(unsigned short, const tVariant*, long) override; // Вызвать нужную процедуру
	void callMethodAsFunc(unsigned short, tVariant*, const tVariant*, long) override; // Вызвать нужную функцию

private:
	const std::string m_CorrectValueForNumber = "0123456789.-";
	std::map<std::wstring, long long> m_MapTimer;

	// Методы для вызова из 1С
	void pause(const tVariant*) const; // Пауза (без нагрузки на процессор)
	void stringToNumber(const tVariant*, tVariant*) const; // Строка в число
	void correctString(const tVariant*, tVariant*) const; // Подчистить строку
	void correctStringJson(const tVariant*, tVariant*) const; // Подчистить заголовки JSON
	void compress(const tVariant*, tVariant*) const; // Сжать данные (deflate)
	void decompress(const tVariant*, tVariant*) const; // Разжать данные (deflate)
	void currentUnixTimestamp(tVariant*) const; // Текущий unix timestamp
	void getScreenshot(const tVariant*, tVariant*) const; // Скриншот экрана (только на клиенте)
	void interruptHandler(tVariant*) const; // Обработка прерывания
	void getUuid(tVariant*) const; // Уникальный идентификатор
	void startTimer(const tVariant*); // Начать замер
	void endTimer(const tVariant*, tVariant*) const; // Завершить замер

	// Вспомогательные методы
	static void jsonRecursiveCorrectKey(rapidjson::Document&, const std::wstring&, const std::wstring&, const std::wstring&);
};

#endif
