#ifndef __BASE_EXTENSION_CLASS_HPP__
#define __BASE_EXTENSION_CLASS_HPP__

#include <baseClass/lykovExceptions.hpp>
#include <from1C/ComponentBase.h>
#include <from1C/IMemoryManager.h>
#include <from1C/AddInDefBase.h>

#include <map>
#include <type_traits>
#include <boost/bimap.hpp>
#include <rapidjson/stringbuffer.h>

constexpr unsigned short gl_Index_Prop_Last_Error      = 0;
constexpr unsigned short gl_Index_Prop_Event_Buffer    = 1;
constexpr unsigned short gl_Index_Prop_Attach_Isolated = 2;
constexpr unsigned short gl_Index_Prop_App_Version     = 3;
constexpr unsigned short gl_Index_Prop_App_Type        = 4;
constexpr unsigned short gl_Index_Prop_Silent_Mode     = 5;
constexpr unsigned short gl_Index_Last_Prop            = 6; // Номер версии

constexpr short gl_Index_Last_Method = -1;

// Основной класс для переопределения
class IBaseExtensionClass : public IComponentBase {
public:
	IBaseExtensionClass();
	~IBaseExtensionClass() override = default;

#pragma region Методы для переопределения в готовых классах

	virtual const wchar_t* getNameExtension() = 0; // Должен возвращать фактическое наименование расширения
	virtual const wchar_t* getVersion() = 0; // Должен возвращать номер версии компоненты
	virtual void setMethodPropsExtension() = 0; // Инициализация компоненты (дополнение методов и свойств компоненты)
	virtual void getPropByIndex(unsigned short, tVariant*) = 0; // Должен положить значение параметра по индексу в tVariant или вызвать исключение (например, если свойство не найдено)
	virtual void setPropByIndex(unsigned short, tVariant*) = 0; // Должен установить значение параметра по индексу в tVariant
	virtual bool getIsPropWritable(unsigned short) = 0; // Должен вернуть доступность записи параметра по индексу
	virtual long getMethodNParams(unsigned short) = 0; // Должен вернуть количество параметров метода
	virtual void setParamDefValue(unsigned short, long, tVariant*) = 0; // Должен установить значения параметров по умолчанию
	virtual bool getHasRetVal(unsigned short) = 0; // Должен вернуть признак наличия возвращаемого значения
	virtual void callMethodAsProc(unsigned short, const tVariant*, long) = 0; // Вызвать нужную процедуру
	virtual void callMethodAsFunc(unsigned short, tVariant*, const tVariant*, long) = 0; // Вызвать нужную функцию

#pragma endregion

#pragma region Интерфейс компоненты

	bool ADDIN_API Init(void*) override; // При загрузке "1С:Предприятие" инициализирует объект компоненты
	bool ADDIN_API setMemManager(void* mem) override; // Установка менеджера памяти для компоненты
	long ADDIN_API GetInfo() override; // "1С:Предприятие" вызывает этот метод для получения информации о компоненте
	void ADDIN_API Done() override; // 1С:Предприятие" вызывает этот метод при завершении работы с объектом компоненты.
	bool ADDIN_API RegisterExtensionAs(WCHAR_T**) override; // В переменную wsExtensionName помещается наименование расширения
	long ADDIN_API GetNProps() override; // Возвращает количество свойств данного расширения
	long ADDIN_API FindProp(const WCHAR_T*) override; // Возвращает порядковый номер свойства с именем wsPropName
	const WCHAR_T* ADDIN_API GetPropName(long, long) override; // В возвращаемое значение помещается имя свойства с порядковым номером lPropNum
	bool ADDIN_API GetPropVal(long, tVariant*) override; // Возвращает значение свойства с указанным порядковым номером
	bool ADDIN_API SetPropVal(long, tVariant*) override; // Устанавливает значение свойства с указанным порядковым номером
	bool ADDIN_API IsPropReadable(long) override; // Возвращает флаг возможности чтения свойства с указанным порядковым номером
	bool ADDIN_API IsPropWritable(long) override; // Возвращает флаг возможности записи свойства с указанным порядковым номером
	long ADDIN_API GetNMethods() override; // Возвращается количество методов данного расширения
	long ADDIN_API FindMethod(const WCHAR_T*) override; // Возвращается порядковый номер метода с именем wsMethodName
	const WCHAR_T* ADDIN_API GetMethodName(long, long) override; // Возвращается имя метода с порядковым номером lMethodNum
	long ADDIN_API GetNParams(long) override; // Возвращает количество свойств данного метода, 0 – при отсутствии свойств
	bool ADDIN_API GetParamDefValue(long, long, tVariant*) override; // Возвращает значение по умолчанию указанного параметра указанного метода
	bool ADDIN_API HasRetVal(long) override; // Возвращает флаг наличия у метода с указанным порядковым номером возвращаемого значения
	bool ADDIN_API CallAsProc(long, tVariant*, long) override; // Выполняет процедуру с указанным порядковым номером
	bool ADDIN_API CallAsFunc(long, tVariant*, tVariant*, long) override; // Выполняет функцию с указанным порядковым номером
	void ADDIN_API SetLocale(const WCHAR_T*) override; // "1С:Предприятие" вызывает этот метод для локализации компоненты в соответствии с используемым кодом локализации
	void ADDIN_API SetUserInterfaceLanguageCode(const WCHAR_T*) override; // "1С:Предприятие" вызывает этот метод для локализации компоненты в соответствии с используемым кодом локализации (новый метод)

#pragma endregion

protected:
	struct NamesFor1C {
		const wchar_t* nameRu;
		const wchar_t* name;

		NamesFor1C() = default;
		NamesFor1C(const wchar_t* nameRu, const wchar_t* name);
		~NamesFor1C() = default;
	};

	using NamesType = std::map<unsigned short, NamesFor1C>;
	
	IMemoryManager* m_IMemory;
	bool m_ItsServer = false; // Флаг того, что компоненты подключена на сервере
	const wchar_t* m_LastError = L""; // Текст последней ошибки
	bool itsIsolate = false; // Флаг того, что подключение изолировано
	bool silentMode = false; // Флаг того, что не нужно выводить модальные окна сообщений при ошибках

	NamesType m_PropNames; // Имена свойств для 1С
	NamesType m_MethodNames; // Имена методов для 1С 

#pragma region Интерфейс 1С предприятия

	void addError(const LykovException&); // Выводит ошибку в 1С. Если сервер, то просто записывает текст ошибки в lastError.
	bool registerProfileAs(const wchar_t*) const; // Регистрирует компоненту в реестр (для кэша)
	bool readFromCache(const wchar_t*, tVariant*); // Читает сохраненное значение параметра компоненты.
	bool writeToCache(const wchar_t*, tVariant*) const; // Сохраняет значение параметра компоненты
	bool externalEvent(const wchar_t*, const wchar_t*, const wchar_t*) const; // Помещает событие в очередь (ВнешнееСобытие)
	void cleanEventBuffer() const; // Очищает события в очереди.
	bool setStatusLine(const wchar_t*) const; // Устанавливает текст в строку состояния
	void resetStatusLine() const; // Очищает текст из строки состояния
	[[nodiscard]] IInterface* getInterface(Interfaces) const; // Запрашивает интерфейс платформы. Значение перечисления Interfaces: eIMsgBox, eIPlatformInfo
	bool showQuestions(const wchar_t*, tVariant*) const; // Показать вопрос. В tVariant кладется результат (Значение true соответствует кнопке ОК, false – Отмена).
	bool showMessage(const wchar_t*) const; // Показать сообщение.
	[[nodiscard]] const IPlatformInfo::AppInfo* getPlatformInfo() const; // AppInfo* - указатель на структуру с полями: AppVersion — Тип: WCHAR_T*, версия приложения, Application — Тип: перечисление, тип подключившего компоненту приложения, UserAgentInformation — Тип: WCHAR_T*, информация о браузере (только для веб-клиента)
	[[nodiscard]] IAttachedInfo::AttachedType getAttachedInfo() const; // eAttachedIsolated = 0 - подключена изолированно, eAttachedNotIsolated = 1 - подключена не изолированно.

#pragma endregion

	// Получить входящий параметр
	template <typename Type>
	Type getInputParam(const tVariant*, long = 0l) const;
	template <> bool getInputParam(const tVariant*, long) const;
	template <> std::wstring getInputParam(const tVariant*, long) const;
	template <> const wchar_t* getInputParam(const tVariant*, long) const;
	template <> std::string getInputParam(const tVariant*, long) const;
	template <> int getInputParam(const tVariant*, long) const;
	template <> unsigned int getInputParam(const tVariant*, long) const;
	template <> double getInputParam(const tVariant*, long) const;
	template <> rapidjson::Document getInputParam(const tVariant*, long) const;

	// Установить возвращаемый параметр
	template <typename Type>
	void setReturnedParam(Type, tVariant*) const;
	template <> void setReturnedParam(bool, tVariant*) const;
	template <> void setReturnedParam(const std::wstring&, tVariant*) const;
	template <> void setReturnedParam(const wchar_t*, tVariant*) const;
	template <> void setReturnedParam(const std::string&, tVariant*) const;
	template <> void setReturnedParam(const char*, tVariant*) const;
	template <> void setReturnedParam(long, tVariant*) const;
	template <> void setReturnedParam(int, tVariant*) const;
	template <> void setReturnedParam(double, tVariant*) const;
	template <> void setReturnedParam(const rapidjson::Document&, tVariant*) const;

private:
	IAddInDefBase* m_IConnect;

	// Вспомогательные функции
	static long findNameFromList(const NamesType&, const WCHAR_T*);
	static const wchar_t* findIndexFromList(const NamesType&, unsigned short);
	static void getInputParam_Exception(long, const wchar_t*, TYPEVAR);
};

#endif
