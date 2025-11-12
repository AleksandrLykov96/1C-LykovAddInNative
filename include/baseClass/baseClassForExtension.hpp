
#ifndef __BASE_CLASS_FOR_EXTENSION_HPP__
#define __BASE_CLASS_FOR_EXTENSION_HPP__

#include "baseClass/convertingString.hpp"
#include "baseClass/stdafx.hpp"
#include "baseClass/from1C/AddInDefBase.h"
#include "baseClass/from1C/ComponentBase.h"
#include "baseClass/from1C/IMemoryManager.h"

#include <algorithm>
#include <codecvt>
#include <execution>
#include <filesystem>
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <thread>
#include <type_traits>
#include <variant>
#include <vector>

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/exception/all.hpp>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

// При передаче даты из 1С / в 1С нумерация идёт с 1900 года
#define START_YEAR_1C 1900

namespace native1C {

	// Вспомогательные типы данных
	typedef rapidjson::GenericDocument<rapidjson::UTF16<wchar_t>> json_document; // Тип документа JSON для обмена данными с 1С (с поддержкой русских символов).
	typedef rapidjson::GenericValue<rapidjson::UTF16<wchar_t>> json_value; // Тип значения JSON для обмена данными с 1С (с поддержкой русских символов).
	typedef rapidjson::GenericStringStream<rapidjson::UTF16<wchar_t>> json_string_stream; // Стрим для записи в строку
	typedef rapidjson::GenericStringBuffer<rapidjson::UTF16<wchar_t>> json_string_buffer;
	typedef rapidjson::BasicIStreamWrapper<std::wifstream> json_ifile_stream; // Стрим для чтения из файла
	typedef rapidjson::BasicOStreamWrapper<std::wofstream> json_ofile_stream; // Стрим для чтения в файл
	typedef rapidjson::Writer<json_string_buffer, rapidjson::UTF16<wchar_t>, rapidjson::UTF16<wchar_t>> json_writer;
	typedef rapidjson::Writer<json_ofile_stream, rapidjson::UTF16<wchar_t>, rapidjson::UTF16<wchar_t>> json_writer_file;
	
	// Возможные типы для возвращаемых значений в 1С
	typedef std::variant<
		std::monostate, // NULL
		int8_t,
		int16_t,
		int32_t,
		//int64_t, Не работает, в 1С пишет "Ошибка чтения значения"
		uint8_t,
		uint16_t,
		uint32_t,
		//uint64_t, Не работает, в 1С пишет "Ошибка чтения значения"
		long,
		float,
		double,
		boost::posix_time::ptime, // Для передачи даты с временем.
		bool,
		std::string,
		std::wstring,
		json_document,
		std::vector<char> // Двоичные данные
	> all_returned_types;

	// Возможные типы входных значений из 1С (все остальные косячные, либо превращаются в эти).
	typedef std::variant<
		std::monostate, // NULL / Неопределено
		int32_t,
		double,
		boost::posix_time::ptime, // Для передачи даты с временем.
		bool,
		std::wstring,
		std::vector<char> // Двоичные данные
	> all_input_types;

	// Для возможности использования lambda с выбором типа.
	template <class... Ts>
	struct Overloaded : Ts... {
		using Ts::operator()...;
	};

	template <class... Ts>
	Overloaded(Ts...) -> Overloaded<Ts...>;

	// Основной класс для переопределения
	class IBaseExtensionClass : public IComponentBase {
	public:
		IBaseExtensionClass();
		~IBaseExtensionClass() BOOST_OVERRIDE;
		BOOST_DELETED_FUNCTION(IBaseExtensionClass(const IBaseExtensionClass&))
		BOOST_DELETED_FUNCTION(IBaseExtensionClass(IBaseExtensionClass&&) BOOST_NOEXCEPT)
		BOOST_DELETED_FUNCTION(IBaseExtensionClass& operator=(const IBaseExtensionClass&))
		BOOST_DELETED_FUNCTION(IBaseExtensionClass& operator=(IBaseExtensionClass&&))

		// Методы вызова из 1С. Не трогать.
		// https://its.1c.ru/db/metod8dev/content/3221/hdoc
		virtual bool ADDIN_API Init(void*) BOOST_FINAL; // При загрузке "1С:Предприятие" инициализирует объект компоненты
		virtual bool ADDIN_API setMemManager(void*) BOOST_FINAL; // Установка менеджера памяти для компоненты
		virtual long ADDIN_API GetInfo() BOOST_FINAL; // "1С:Предприятие" вызывает этот метод для получения информации о компоненте
		virtual void ADDIN_API Done() BOOST_FINAL; // "1С:Предприятие" вызывает этот метод при завершении работы с объектом компоненты.
		virtual bool ADDIN_API RegisterExtensionAs(WCHAR_T**) BOOST_FINAL; // В переменную wsExtensionName помещается наименование расширения
		virtual long ADDIN_API GetNProps() BOOST_FINAL; // Возвращает количество свойств данного расширения
		virtual long ADDIN_API FindProp(const WCHAR_T*) BOOST_FINAL; // Возвращает порядковый номер свойства с именем wsPropName
		virtual const WCHAR_T* ADDIN_API GetPropName(long, long) BOOST_FINAL; // В возвращаемое значение помещается имя свойства с порядковым номером lPropNum
		virtual bool ADDIN_API GetPropVal(long, tVariant*) BOOST_FINAL; // Возвращает значение свойства с указанным порядковым номером
		virtual bool ADDIN_API SetPropVal(long, tVariant*) BOOST_FINAL; // Устанавливает значение свойства с указанным порядковым номером
		virtual bool ADDIN_API IsPropReadable(long) BOOST_FINAL; // Возвращает флаг возможности чтения свойства с указанным порядковым номером
		virtual bool ADDIN_API IsPropWritable(long) BOOST_FINAL; // Возвращает флаг возможности записи свойства с указанным порядковым номером
		virtual long ADDIN_API GetNMethods() BOOST_FINAL; // Возвращается количество методов данного расширения
		virtual long ADDIN_API FindMethod(const WCHAR_T*) BOOST_FINAL; // Возвращается порядковый номер метода с именем wsMethodName
		virtual const WCHAR_T* ADDIN_API GetMethodName(long, long) BOOST_FINAL; // Возвращается имя метода с порядковым номером lMethodNum
		virtual long ADDIN_API GetNParams(long) BOOST_FINAL; // Возвращает количество свойств данного метода, 0 – при отсутствии свойств
		virtual bool ADDIN_API GetParamDefValue(long, long, tVariant*) BOOST_FINAL; // Возвращает значение по умолчанию указанного параметра указанного метода
		virtual bool ADDIN_API HasRetVal(long) BOOST_FINAL; // Возвращает флаг наличия у метода с указанным порядковым номером возвращаемого значения
		virtual bool ADDIN_API CallAsProc(long, tVariant*, long) BOOST_FINAL; // Выполняет процедуру с указанным порядковым номером
		virtual bool ADDIN_API CallAsFunc(long, tVariant*, tVariant*, long) BOOST_FINAL; // Выполняет функцию с указанным порядковым номером

		// "1С:Предприятие" вызывает этот метод для локализации компоненты в соответствии с используемым кодом локализации
		virtual void ADDIN_API SetLocale(const WCHAR_T*) BOOST_FINAL;

		// "1С:Предприятие" вызывает этот метод для локализации компоненты в соответствии с используемым кодом локализации (новый метод, с 8.3.21)
		virtual void ADDIN_API SetUserInterfaceLanguageCode(const WCHAR_T*) BOOST_FINAL;


	protected:
		// Методы для переопределения в подклассе
		virtual std::wstring getNameExtension() = 0; // Должен возвращать фактическое наименование расширения.
		virtual void initializeComponent() = 0; // Вызывается при инициализации компоненты.

	#ifdef NDEBUG
		bool debuggingMode = false; // Флаг того, что компонента подключена в режиме отладки.
	#else
		bool debuggingMode = true; // Флаг того, что компонента подключена в режиме отладки.
	#endif

		bool itsServer = true; // Флаг того, что компонента подключена на сервере.
		bool itsIsolate = false; // Флаг того, что компонента подключена в отдельном потоке.
		bool silentMode = true; // Флаг того, что не нужно выводить модальные окна сообщений / исключения при ошибках.
		std::wstring lastErrors; // Текст последней ошибки (можно получить из свойств компоненты).

		// Для работы с 1С (типовые платформенные возможности).
		bool registerProfileAs(const std::wstring& profileName); // Регистрирует компоненту в реестр (для кэша).
		bool readFromCache(const std::wstring& propName, tVariant* pVal); // Читает сохраненное значение параметра компоненты.
		bool writeToCache(const std::wstring& propName, tVariant* pVal); // Сохраняет значение параметра компоненты.
		bool externalEvent(const std::wstring& source, const std::wstring& message, const std::wstring& data); // Помещает событие в очередь (ВнешнееСобытие).
		void cleanEventBuffer(); // Очищает события в очереди.
		bool setStatusLine(const std::wstring& message); // Устанавливает текст в строку состояния.
		void resetStatusLine(); // Очищает текст из строки состояния.
		IInterface* getInterface(Interfaces interface1C); // Запрашивает интерфейс платформы. Значение перечисления Interfaces: eIMsgBox, eIPlatformInfo.
		bool showQuestions(const std::wstring& message, bool& answer);
		// Показать вопрос. В answer кладется результат (Значение true соответствует кнопке ОК, false – Отмена).
		bool showMessage(const std::wstring& message); // Показать сообщение.
		bool addError(unsigned short wCode, const std::wstring& source, const std::wstring& description, long code); // Добавить ошибку в 1С.
		bool addError(unsigned short wCode, const std::string& source, const std::wstring& description, long code); // Добавить ошибку в 1С.

		// AppInfo* - указатель на структуру с полями:
		//		AppVersion — Тип: WCHAR_T*, версия приложения
		//		Application — Тип: перечисление, тип подключившего компоненту приложения,
		//		UserAgentInformation — Тип: WCHAR_T*, информация о браузере (только для веб-клиента).
		const IPlatformInfo::AppInfo* getPlatformInfo();

		// eAttachedIsolated = 0 - подключена изолированно, eAttachedNotIsolated = 1 - подключена не изолированно.
		BOOST_ATTRIBUTE_NODISCARD IAttachedInfo::AttachedType getAttachedInfo() const;

		// Добавляет в текст последней ошибки.
		void addErrorToLastError(const std::wstring& message);

		// Метод для добавления нового свойства компоненты.
		void addProperty(const std::wstring& nameRu, const std::wstring& name, const std::function<all_returned_types()>& getter = nullptr,
		                 const std::function<void(all_input_types&&)>& setter = nullptr);

		// Метод для добавления нового метода компоненты.
		template <typename T, typename C, typename... Ts>
		void addMethod(const std::wstring& nameRu, const std::wstring& name, C* c, T (C::*f)(Ts...), std::map<long, all_input_types>&& defaultValues = {},
		               bool canChangeInputParameters = false);

		// Конвертация данных с JSON
		json_document getInputJSON(const std::wstring& source); // Получить JSON по строке.
		std::wstring getStringFromJSON(const json_document& json); // Преобразовать JSON в строку.
		json_document readJSONFromFile(const std::wstring& fileName); // Читает JSON из файла

		// Методы для логирования
		void log1C(plog::Severity level, const std::wstring& msg, const std::source_location& = std::source_location::current());
		void log1C(plog::Severity level, const std::string& msg, const std::source_location& = std::source_location::current());
		BOOST_ATTRIBUTE_NODISCARD std::wstring getInfoFromException(const boost::exception&) const; // Получает текст ошибки из исключения
		BOOST_ATTRIBUTE_NODISCARD std::wstring getInfoFromException(const std::exception&) const; // Получает текст ошибки из исключения

	private:
		IMemoryManager* p_MemoryManager; // Менеджер памяти от 1С.
		IAddInDefBase* p_Connector1C; // Класс для работы с 1С.
		bool logsInitialize = false; // Логи инициализированы (чтобы лишний раз не зачищать).

		void p_SetupLoggingToFile(); // Настройка логов
		void p_WriteReturnValue(const all_returned_types&, tVariant*); // Записывает возвращаемое значение для 1С.
		void p_WriteReturnValue(const all_input_types&, tVariant*); // Записывает возвращаемое значение для 1С.
		static all_input_types p_GetParametersValue(tVariant*); // Возвращает значение параметра 1С.
		static std::vector<all_input_types> p_GetInputParameters(tVariant*, long); // Возвращает массив параметров 1С.

		template <size_t... Indices>
		static auto p_RefTupleGen(std::vector<all_input_types>&, std::index_sequence<Indices...>);

		void p_WriteValueTo1C(const std::wstring&, WCHAR_T**) const;

		// Структура для хранения данных свойства компоненты.
		struct P_ExtProperty {
			std::wstring nameRu; // Наименование на русском.
			std::wstring name; // Наименование на английском.
			std::function<all_returned_types()> getter; // Функция для получения значения свойства.
			std::function<void(all_input_types&&)> setter; // Функция для установки значения свойства.

			BOOST_DELETED_FUNCTION(P_ExtProperty())
			BOOST_DEFAULTED_FUNCTION(~P_ExtProperty(), {})
			BOOST_DEFAULTED_FUNCTION(P_ExtProperty(const P_ExtProperty&), {})
			BOOST_DEFAULTED_FUNCTION(P_ExtProperty(P_ExtProperty&&) BOOST_NOEXCEPT, {})
			BOOST_DELETED_FUNCTION(P_ExtProperty& operator=(const P_ExtProperty&))
			BOOST_DELETED_FUNCTION(P_ExtProperty& operator=(P_ExtProperty&&))

			P_ExtProperty(std::wstring m_NameRu,
			              std::wstring m_Name,
			              std::function<all_returned_types()> m_Getter,
			              std::function<void(all_input_types&&)> m_Setter)
				: nameRu(std::move(m_NameRu)),
				  name(std::move(m_Name)),
				  getter(std::move(m_Getter)),
				  setter(std::move(m_Setter)) {
			}
		};

		// Структура для хранения данных метода компоненты.
		struct P_ExtMethod {
			std::wstring nameRu; // Наименование на русском.
			std::wstring name; // Наименование на английском.
			long paramsCount; // Количество входных параметров.
			bool returnsValue; // Флаг того, что есть возвращаемое значение.
			std::map<long, all_input_types> defaultArgs; // Для хранения значений по-умолчанию.
			std::function<all_returned_types(std::vector<all_input_types>&)> call; // Функция вызова метода.
			bool canChangeInputParam; // Вызов метода меняет переданные значения параметров.

			BOOST_DELETED_FUNCTION(P_ExtMethod())
			BOOST_DEFAULTED_FUNCTION(~P_ExtMethod(), {})
			BOOST_DEFAULTED_FUNCTION(P_ExtMethod(const P_ExtMethod&), {})
			BOOST_DEFAULTED_FUNCTION(P_ExtMethod(P_ExtMethod&&) BOOST_NOEXCEPT, {})
			BOOST_DELETED_FUNCTION(P_ExtMethod& operator=(const P_ExtMethod&))
			BOOST_DELETED_FUNCTION(P_ExtMethod& operator=(P_ExtMethod&&))

			P_ExtMethod(std::wstring m_NameRu,
			            std::wstring m_Name,
			            const long m_ParamsCount,
			            const bool m_ReturnsValue,
			            std::map<long, all_input_types> m_DefaultArgs,
			            std::function<all_returned_types(std::vector<all_input_types>&)> m_Call,
			            const bool m_CanChangeInputParam = false)
				: nameRu(std::move(m_NameRu)),
				  name(std::move(m_Name)),
				  paramsCount(m_ParamsCount),
				  returnsValue(m_ReturnsValue),
				  defaultArgs(std::move(m_DefaultArgs)),
				  call(std::move(m_Call)),
				  canChangeInputParam(m_CanChangeInputParam) {
			}
		};

		std::vector<P_ExtProperty> p_ExtProperties; // Хранит все свойства компоненты.
		std::vector<P_ExtMethod> p_ExtMethods; // Хранит все методы компоненты.
	};

	template <size_t... Indices>
	auto IBaseExtensionClass::p_RefTupleGen(std::vector<all_input_types>& v, std::index_sequence<Indices...>) {
		return std::forward_as_tuple(v[Indices]...);
	}

	template <typename T, typename C, typename... Ts>
	void IBaseExtensionClass::addMethod(const std::wstring& nameRu, const std::wstring& name, C* c, T (C::*f)(Ts...),
	                                    std::map<long, all_input_types>&& defaultValues, bool canChangeInputParameters) {
		log1C(plog::debug, std::format(L"Добавление метода компоненты '{}' ({})", nameRu, name));

		P_ExtMethod meta{
				nameRu,
				name,
				sizeof...(Ts),
				!std::is_same_v<T, void>,
				defaultValues,
				[f, c](std::vector<all_input_types>& params) -> all_returned_types {
					auto args = p_RefTupleGen(params, std::make_index_sequence<sizeof...(Ts)>());
					if constexpr (std::is_same_v<T, void>) {
						std::apply(f, std::tuple_cat(std::make_tuple(c), args));
						return std::monostate();
					}
					else {
						return std::apply(f, std::tuple_cat(std::make_tuple(c), args));
					}
				},
				canChangeInputParameters
			};

		p_ExtMethods.push_back(std::move(meta));
	}

}

#endif
