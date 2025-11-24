
#include "baseClass/baseClassForExtension.hpp"

using namespace std;
using namespace native1C;

#pragma region Основные методы для вызова из подкласса (аля ПрограммныйИнтерфейс)

// Метод "addMethod" в hpp файле, т.к. ошибка error LNK2019 - "Определения шаблонов должны полностью находиться в заголовочном файле".

void IBaseExtensionClass::addProperty(const wstring& nameRu, const wstring& name,
                                      const function<all_returned_types()>& getter, const function<void(all_input_types&&)>& setter) {
	log1C(plog::debug, format(L"Добавление свойства компоненты {} ({})", nameRu, name));

	const P_ExtProperty newProperty{nameRu, name, getter, setter};
	p_ExtProperties.push_back(newProperty);
}

void IBaseExtensionClass::addErrorToLastError(const wstring& message) {
	if (lastErrors.empty())
		lastErrors = message;
	else
		lastErrors = lastErrors + L"\n\n" + message;
}

#pragma region Работа с JSON

json_document IBaseExtensionClass::getInputJSON(const wstring& source) {
	const auto basicStringError = L"Не удалось преобразовать строку в JSON";
	json_document documentResult;

	try {
		json_string_stream s(source.c_str());

		documentResult.ParseStream(s);
		if (documentResult.HasParseError())
			BOOST_THROW_EXCEPTION(invalid_argument(GetParseError_En(documentResult.GetParseError())));
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return documentResult;
}

wstring IBaseExtensionClass::getStringFromJSON(const json_document& json) {
	using namespace rapidjson;
	const auto basicStringError = L"Не удалось преобразовать JSON в строку";

	try {
		json_string_buffer buffer;
		json_writer writer(buffer);

		json.Accept(writer);
		return buffer.GetString();
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return L"";
}

json_document IBaseExtensionClass::readJSONFromFile(const wstring& fileName) {
	using namespace rapidjson;
	json_document result;

	const auto basicStringError = L"Не удалось прочитать JSON из файла";

	try {
		if (fileName.empty())
			BOOST_THROW_EXCEPTION(invalid_argument("Имя файла должно быть указано!"));

		wifstream ifs;

		const filesystem::path inputPath(fileName);
		ifs.open(inputPath);

		json_ifile_stream isw(ifs);
		result.ParseStream(isw);
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return result;
}

#pragma endregion

#pragma region Интерфейс 1С предприятия

bool IBaseExtensionClass::registerProfileAs(const wstring& profileName) {
	bool result = false;

	// Только на клиенте
	if (itsServer || !p_Connector1C)
		return result;

	const auto basicStringError = L"Не удалось зарегистрировать список параметров компоненты";
	log1C(plog::debug, format(L"Регистрация списков параметров компоненты для '{}'", profileName));

	try {
		WCHAR_T* wProfileName = nullptr;
		p_WriteValueTo1C(profileName, &wProfileName);

		result = p_Connector1C->RegisterProfileAs(wProfileName);
		log1C(plog::info, format(L"Регистрация списков параметров компоненты для '{}', результат '{}'", profileName, result));

		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&wProfileName));
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return result;
}

bool IBaseExtensionClass::readFromCache(const wstring& propName, tVariant* pVal) {
	bool result = false;

	// Только на клиенте
	if (itsServer || !p_Connector1C)
		return result;

	const auto basicStringError = L"Не удалось прочитать значение параметра из кэша";
	log1C(plog::debug, format(L"Чтение сохраненного значения '{}'", propName));

	try {
		long errorCode = 0;
		WCHAR_T* wErrorDescription = nullptr;
		WCHAR_T* wPropName = nullptr;

		p_WriteValueTo1C(propName, &wPropName);

		result = p_Connector1C->Read(wPropName, pVal, &errorCode, &wErrorDescription);
		log1C(plog::info, format(L"Регистрация списков параметров компоненты для '{}', код ошибки '{}', текст ошибки '{}'", propName, errorCode,
				gl_conv_WCHAR_T_to_wstring(wErrorDescription)));

		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&wPropName));
		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&wErrorDescription));
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return result;
}

bool IBaseExtensionClass::writeToCache(const wstring& propName, tVariant* pVal) {
	bool result = false;

	// Только на клиенте
	if (itsServer || !p_Connector1C)
		return result;

	const auto basicStringError = L"Не удалось записать значение параметра в кэш";
	log1C(plog::debug, format(L"Запись сохраняемого значения '{}'", propName));

	try {
		WCHAR_T* wPropName = nullptr;
		p_WriteValueTo1C(propName, &wPropName);

		result = p_Connector1C->Write(wPropName, pVal);
		log1C(plog::info, format(L"Запись сохраняемого значения '{}', результат '{}'", propName, result));

		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&wPropName));
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return result;
}

bool IBaseExtensionClass::externalEvent(const wstring& source, const wstring& message, const wstring& data) {
	bool result = false;

	// Только на клиенте
	if (itsServer || !p_Connector1C)
		return result;

	const auto basicStringError = L"Не удалось инициализировать внешнее событие";
	log1C(plog::debug, format(L"Инициализация внешнего события, источник '{}', наименование события '{}'", source, message));

	try {
		WCHAR_T* wSource = nullptr;
		WCHAR_T* wMessage = nullptr;
		WCHAR_T* wData = nullptr;

		p_WriteValueTo1C(source, &wSource);
		p_WriteValueTo1C(message, &wMessage);
		p_WriteValueTo1C(data, &wData);

		result = p_Connector1C->ExternalEvent(wSource, wMessage, wData);
		log1C(plog::info, format(L"Инициализация внешнего события, источник '{}', наименование события '{}', результат инициализации '{}'", source, message, result));

		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&wSource));
		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&wMessage));
		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&wData));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return result;
}

void IBaseExtensionClass::cleanEventBuffer() {
	// Только на клиенте
	if (itsServer || !p_Connector1C)
		return;

	const auto basicStringError = L"Не удалось очистить очередь внешних событий";
	log1C(plog::debug, L"Очистка очереди событий");

	try {
		p_Connector1C->CleanEventBuffer();
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}
}

bool IBaseExtensionClass::setStatusLine(const wstring& message) {
	bool result = false;

	// Только на клиенте
	if (itsServer || !p_Connector1C)
		return result;

	const auto basicStringError = L"Не удалось установить сообщение строки состояния";
	log1C(plog::debug, format(L"Установка текста строки состояния, текст '{}'", message));

	try {
		WCHAR_T* wMessage = nullptr;
		p_WriteValueTo1C(message, &wMessage);

		result = p_Connector1C->SetStatusLine(wMessage);
		log1C(plog::info, format(L"Установка текста строки состояния, текст '{}', результат '{}'", message, result));

		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&wMessage));
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return result;
}

void IBaseExtensionClass::resetStatusLine() {
	// Только на клиенте
	if (itsServer || !p_Connector1C)
		return;

	const auto basicStringError = L"Не удалось установить очистить сообщения строки состояния";
	log1C(plog::debug, L"Очистка строки состояния");

	try {
		p_Connector1C->ResetStatusLine();
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}
}

IInterface* IBaseExtensionClass::getInterface(const Interfaces interface1C) {
	if (!p_Connector1C)
		return nullptr;

	const auto basicStringError = L"Не удалось получить интерфейс 1С";
	log1C(plog::debug, L"Получение интерфейса 1С (GetInterface)");

	try {
		const auto cnn = static_cast<IAddInDefBaseEx*>(p_Connector1C); // Оставить static_cast!!!
		if (!cnn)
			BOOST_THROW_EXCEPTION(runtime_error("1С ничего не вернул"));

		return cnn->GetInterface(interface1C);
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return nullptr;
}

bool IBaseExtensionClass::showQuestions(const wstring& message, bool& answer) {
	bool result = false;

	// Только на клиенте
	if (silentMode || itsServer || !p_Connector1C)
		return result;

	const auto messageBox = static_cast<IMsgBox*>(getInterface(eIMsgBox)); // Оставить static_cast!!!
	if (!messageBox)
		return result;

	const auto basicStringError = L"Не удалось вывести диалог вопроса в 1С";
	log1C(plog::debug, format(L"Вывод диалога вопроса, текст '{}'", message));

	try {
		WCHAR_T* wMessage = nullptr;

		const auto pVal = new tVariant;
		DATA_SET_BEGIN(pVal)

		p_WriteValueTo1C(message, &wMessage);

		result = messageBox->Confirm(wMessage, pVal);
		log1C(plog::info, format(L"Вывод диалога вопроса, текст '{}', результат '{}'", message, result));

		if (result)
			answer = TV_BOOL(pVal);

		delete pVal;
		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&wMessage));
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return result;
}

bool IBaseExtensionClass::showMessage(const wstring& message) {
	bool result = false;

	// Только на клиенте
	if (silentMode || itsServer || !p_Connector1C)
		return result;

	const auto messageBox = static_cast<IMsgBox*>(getInterface(eIMsgBox)); // Оставить static_cast!!!
	if (!messageBox)
		return result;

	const auto basicStringError = L"Не удалось вывести окно сообщения в 1С";
	log1C(plog::debug, format(L"Вывод диалогового окна, текст '{}'", message));

	try {
		WCHAR_T* wMessage = nullptr;
		p_WriteValueTo1C(message, &wMessage);

		result = messageBox->Alert(wMessage);
		log1C(plog::info, format(L"Вывод диалогового окна, текст '{}', результат '{}'", message, result));

		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&wMessage));
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return result;
}

bool IBaseExtensionClass::addError(const unsigned short wCode, const wstring& source, const wstring& description, const long code) {
	bool result = false;

	// Только на клиенте
	if (silentMode || itsServer || !p_Connector1C)
		return result;

	try {
		WCHAR_T* wSource = nullptr;
		WCHAR_T* wDescription = nullptr;

		p_WriteValueTo1C(source, &wSource);
		p_WriteValueTo1C(description, &wDescription);

		result = p_Connector1C->AddError(wCode, wSource, wDescription, code);

		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&wSource));
		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&wDescription));
	}
	catch (...) {
		addErrorToLastError(L"Не удалось вывести окно ошибки 1С");
	}

	return result;
}

bool IBaseExtensionClass::addError(const unsigned short wCode, const string& source, const wstring& description, const long code) {
	return addError(wCode, gl_conv_string_to_wstring(source), description, code);
}

const IPlatformInfo::AppInfo* IBaseExtensionClass::getPlatformInfo() {
	if (!p_Connector1C)
		return nullptr;

	const auto info = static_cast<IPlatformInfo*>(getInterface(eIPlatformInfo));
	return info->GetPlatformInfo();
}

IAttachedInfo::AttachedType IBaseExtensionClass::getAttachedInfo() const {
	if (!p_Connector1C)
		return IAttachedInfo::AttachedType::eAttachedNotIsolated;

	const auto cnn = static_cast<IAddInDefBaseEx*>(p_Connector1C);
	if (!cnn)
		return IAttachedInfo::AttachedType::eAttachedNotIsolated;

	const auto con_Info = static_cast<IAttachedInfo*>(cnn->GetInterface(eIAttachedInfo));
	return !con_Info ? IAttachedInfo::AttachedType::eAttachedNotIsolated : con_Info->GetAttachedInfo();
}

#pragma endregion

#pragma region Логирование

void IBaseExtensionClass::log1C(const plog::Severity level, const wstring& msg, const source_location& location) {
	if (debuggingMode) {
		IF_PLOG_(PLOG_DEFAULT_INSTANCE_ID, level)
			*plog::get<PLOG_DEFAULT_INSTANCE_ID>() += plog::Record(level, location.function_name(), location.line(), location.file_name(), PLOG_GET_THIS(),
			                                                       PLOG_DEFAULT_INSTANCE_ID).ref() << msg;
	}

	switch (level) {
		case plog::warning: {
			addErrorToLastError(msg);
			break;
		}
		case plog::error: {
			addErrorToLastError(msg);
			addError(ADDIN_E_FAIL, getNameExtension(), msg, 1);

			break;
		}
		case plog::fatal: {
			addErrorToLastError(msg);
			addError(ADDIN_E_MSGBOX_FAIL, getNameExtension(), msg, 1);

			break;
		}
		default:
			break;
	}
}

void IBaseExtensionClass::log1C(const plog::Severity level, const string& msg, const source_location& location) {
	const auto tmp = gl_conv_string_to_wstring(msg);
	log1C(level, tmp, location);
}

// По-умолчанию diagnostic_information всегда возвращает с номером строки функции.
// Если передавать verbose = false, то вообще ничего не возвращает.
// Как хочется: с режимом отладки информация с номером строки.
// Без режима отладки - только сообщение ошибки.

wstring IBaseExtensionClass::getInfoFromException(const boost::exception& ex) const {
	string messageError;

	if (debuggingMode)
		messageError = boost::diagnostic_information(ex, true);
	else
		messageError = dynamic_cast<exception const*>(&ex)->what();

	return gl_conv_string_to_wstring(messageError);
}

wstring IBaseExtensionClass::getInfoFromException(const exception& ex) const {
	string messageError;

	if (debuggingMode)
		messageError = boost::diagnostic_information(ex, true);
	else
		messageError = ex.what();

	return gl_conv_string_to_wstring(messageError);
}

#pragma endregion

#pragma endregion

#pragma region Методы вызова из 1С

bool IBaseExtensionClass::Init(void* connection1C) {
	// Инициализация логов
	if (debuggingMode)
		p_SetupLoggingToFile();

	p_Connector1C = static_cast<IAddInDefBase*>(connection1C);
	const auto success = p_Connector1C != nullptr;

	if (success) {
		// Базовые свойства компоненты
		addProperty(L"ТекстПоследнейОшибки",
		            L"lastError",
		            [this] { return lastErrors; } );

		addProperty(L"ВерсияКомпоненты",
		            L"version",
		            [] { return L"1.0.2.0"; } );

		addProperty(L"РежимОтладки",
		            L"debuggingMode",
		            [this] { return debuggingMode; },
		            [&](const all_input_types& res) {
			            if (!holds_alternative<bool>(res))
				            BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра должен быть 'Булево'"));

			            debuggingMode = get<bool>(res);
			            p_SetupLoggingToFile();
		            } );

		addProperty(L"ПодключениеИзолированно",
		            L"attachIsolated",
		            [this] { return itsIsolate; } );

		addProperty(L"ТихийРежим",
		            L"silentMode",
		            [this] { return silentMode; },
		            [&](const all_input_types& res) {
			            if (!holds_alternative<bool>(res))
				            BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра должен быть 'Булево'"));

			            silentMode = get<bool>(res);
		            } );

		if (const auto iPlatformInfo = static_cast<IPlatformInfo*>(getInterface(eIPlatformInfo))) {
			const auto platformInfo = iPlatformInfo->GetPlatformInfo();
			itsServer = platformInfo->Application == IPlatformInfo::eAppServer;
		}

		// Дополнительные свойства компоненты
		if (!itsServer) {
			p_Connector1C->SetEventBufferDepth(1024);

			addProperty(L"МаксимальныйРазмерОчередиВнешнихСобытий",
			            L"maxExternalEventBufferDepth",
			            [this] {
				            return p_Connector1C->GetEventBufferDepth();
			            },
			            [&](const all_input_types& res) {
				            visit(Overloaded{
					                  [&](const int32_t& buffer) {
						                  if (buffer < 0)
							                  BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра должен быть 'Неотрицательное число'"));

						                  p_Connector1C->SetEventBufferDepth(static_cast<long>(buffer));
					                  },

					                  [&](const double& buffer) {
						                  const auto eventBuffer = lround(buffer);
						                  if (eventBuffer < 0)
							                  BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра должен быть 'Неотрицательное число'"));
						                  if (buffer > 0 && eventBuffer == 0)
							                  BOOST_THROW_EXCEPTION(invalid_argument("Число превышает допустимое значение (long)"));

						                  p_Connector1C->SetEventBufferDepth(eventBuffer);
					                  },

					                  [](auto) {
						                  BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра должен быть 'Неотрицательное число'"));
					                  }
				                  }, res);
			            });
		}

		itsIsolate = getAttachedInfo() == IAttachedInfo::AttachedType::eAttachedIsolated;
		initializeComponent();

		log1C(plog::warning, format(L"Инициализация компоненты '{}' завершена", getNameExtension()));
	}
	else
		log1C(plog::fatal, format(L"Не удалось инициализировать компоненту '{}'!", getNameExtension()));

	return success;
}

bool IBaseExtensionClass::setMemManager(void* memoryManager) {
	log1C(plog::debug, L"Идёт установка менеджера памяти");

	p_MemoryManager = static_cast<IMemoryManager*>(memoryManager);
	if (!p_MemoryManager) {
		log1C(plog::fatal, L"Не удалось установить менеджер памяти!");
		BOOST_THROW_EXCEPTION(runtime_error("Не удалось установить менеджер памяти!"));
	}

	return true;
}

long IBaseExtensionClass::GetInfo() {
	log1C(plog::debug, L"Получена версия компоненты");
	return 2100;
}

void IBaseExtensionClass::Done() {
	log1C(plog::info, L"Завершена работа с компонентой");
	lastErrors.clear();
}

bool IBaseExtensionClass::RegisterExtensionAs(WCHAR_T** extensionName) {
	log1C(plog::debug, L"Установка текущего имени компоненты");

	const auto realExtensionName = getNameExtension();
	p_WriteValueTo1C(realExtensionName, extensionName);

	return true;
}

long IBaseExtensionClass::GetNProps() {
	log1C(plog::debug, L"Получено количество свойств компоненты");
	return static_cast<long>(p_ExtProperties.size());
}

long IBaseExtensionClass::FindProp(const WCHAR_T* propertyName) {
	using namespace boost::algorithm;

	const wstring name = gl_conv_WCHAR_T_to_wstring(propertyName);

	log1C(plog::debug, format(L"Поиск номера свойства '{}' компоненты", name));
	for (long i = 0; i < static_cast<long>(p_ExtProperties.size()); i++) {
		if (gl_str_iequal(name, p_ExtProperties[i].nameRu) || gl_str_iequal(name, p_ExtProperties[i].name))
			return i;
	}

	return -1;
}

const WCHAR_T* IBaseExtensionClass::GetPropName(const long propertyNumber, const long propertyAlias) {
	WCHAR_T* result = nullptr;

	if (propertyAlias == 0) {
		log1C(plog::debug, format(L"Получение имени свойства №{} компоненты на английском = {}", propertyNumber, p_ExtProperties[propertyNumber].name));
		p_WriteValueTo1C(p_ExtProperties[propertyNumber].name, &result);
	}
	else {
		log1C(plog::debug, format(L"Получение имени свойства №{} компоненты на русском = {}", propertyNumber, p_ExtProperties[propertyNumber].nameRu));
		p_WriteValueTo1C(p_ExtProperties[propertyNumber].nameRu, &result);
	}

	return result;
}

bool IBaseExtensionClass::GetPropVal(const long propertyNumber, tVariant* valueResult1C) {
	bool result = false;
	const auto basicStringError = format(L"Не удалось получение значение свойства №{} компоненты", propertyNumber);

	try {
		log1C(plog::debug, format(L"Получение значения свойства №{} компоненты ({})", propertyNumber, p_ExtProperties[propertyNumber].nameRu));

		const auto tmp = p_ExtProperties[propertyNumber].getter();
		p_WriteReturnValue(tmp, valueResult1C);

		result = true;
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const out_of_range& e) {
		log1C(plog::error, format(L"{} <out_of_range>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const string& e) {
		log1C(plog::error, format(L"{} <string>!\n{}", basicStringError, gl_conv_string_to_wstring(e)));
	}
	catch (const char*& e) {
		log1C(plog::error, format(L"{} <string>!\n{}", basicStringError, gl_conv_string_to_wstring(e)));
	}
	catch (const wstring& e) {
		log1C(plog::error, format(L"{} <wstring>!\n{}", basicStringError, e));
	}
	catch (const wchar_t*& e) {
		log1C(plog::error, format(L"{} <wstring>!\n{}", basicStringError, e));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return silentMode ? true : result;
}

bool IBaseExtensionClass::SetPropVal(const long propertyNumber, tVariant* value1C) {
	bool result = false;
	const auto basicStringError = format(L"Не удалось установить значение свойства №{} компоненты", propertyNumber);

	try {
		log1C(plog::debug, format(L"Установка значения свойства №{} компоненты ({})", propertyNumber, p_ExtProperties[propertyNumber].nameRu));

		auto tmp = p_GetParametersValue(value1C);
		p_ExtProperties[propertyNumber].setter(move(tmp));

		result = true;
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const out_of_range& e) {
		log1C(plog::error, format(L"{} <out_of_range>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const string& e) {
		log1C(plog::error, format(L"{} <string>!\n{}", basicStringError, gl_conv_string_to_wstring(e)));
	}
	catch (const char*& e) {
		log1C(plog::error, format(L"{} <string>!\n{}", basicStringError, gl_conv_string_to_wstring(e)));
	}
	catch (const wstring& e) {
		log1C(plog::error, format(L"{} <wstring>!\n{}", basicStringError, e));
	}
	catch (const wchar_t*& e) {
		log1C(plog::error, format(L"{} <wstring>!\n{}", basicStringError, e));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return silentMode ? true : result;
}

bool IBaseExtensionClass::IsPropReadable(const long propertyNumber) {
	log1C(plog::debug, format(L"Получение признака возможности чтения свойства №{} компоненты ({})", propertyNumber, p_ExtProperties[propertyNumber].nameRu));
	return static_cast<bool>(p_ExtProperties[propertyNumber].getter);
}

bool IBaseExtensionClass::IsPropWritable(const long propertyNumber) {
	log1C(plog::debug, format(L"Получение признака возможности изменения свойства №{} компоненты ({})", propertyNumber, p_ExtProperties[propertyNumber].nameRu));
	return static_cast<bool>(p_ExtProperties[propertyNumber].setter);
}

long IBaseExtensionClass::GetNMethods() {
	log1C(plog::debug, L"Получено количество методов компоненты");
	return static_cast<long>(p_ExtMethods.size());
}

long IBaseExtensionClass::FindMethod(const WCHAR_T* propertyName) {
	using namespace boost::algorithm;

	const wstring name = gl_conv_WCHAR_T_to_wstring(propertyName);
	log1C(plog::debug, format(L"Поиск номера метода '{}' компоненты", name));

	for (long i = 0L; i < static_cast<long>(p_ExtMethods.size()); i++) {
		if (gl_str_iequal(name, p_ExtMethods[i].nameRu) || gl_str_iequal(name, p_ExtMethods[i].name))
			return i;
	}

	return -1;
}

const WCHAR_T* IBaseExtensionClass::GetMethodName(const long propertyNumber, const long propertyAlias) {
	WCHAR_T* result = nullptr;

	if (propertyAlias == 0) {
		log1C(plog::debug, format(L"Получение наименования метода №{} компоненты на английском = {}", propertyNumber, p_ExtMethods[propertyNumber].name));
		p_WriteValueTo1C(p_ExtMethods[propertyNumber].name, &result);
	}
	else {
		log1C(plog::debug, format(L"Получение наименования метода №{} компоненты на русском = {}", propertyNumber, p_ExtMethods[propertyNumber].nameRu));
		p_WriteValueTo1C(p_ExtMethods[propertyNumber].nameRu, &result);
	}

	return result;
}

long IBaseExtensionClass::GetNParams(const long methodNumber) {
	log1C(plog::debug, format(L"Получение количества параметров метода №{} компоненты", methodNumber));
	return p_ExtMethods[methodNumber].paramsCount;
}

bool IBaseExtensionClass::GetParamDefValue(const long methodNumber, const long parameterNumber, tVariant* defaultValue) {
	log1C(plog::debug, format(L"Получение параметра №{} по-умолчанию метода №{} компоненты ({})", parameterNumber, methodNumber, p_ExtMethods[methodNumber].nameRu));

	auto& defaultArgs = p_ExtMethods[methodNumber].defaultArgs;

	const auto mapKeyValue = defaultArgs.find(parameterNumber);
	if (mapKeyValue == defaultArgs.end()) {
		return false;
	}

	p_WriteReturnValue(mapKeyValue->second, defaultValue);
	return true;
}

bool IBaseExtensionClass::HasRetVal(const long methodNumber) {
	log1C(plog::debug, format(L"Получение признака возвращаемого значения метода №{} компоненты ({})", methodNumber, p_ExtMethods[methodNumber].nameRu));
	return p_ExtMethods[methodNumber].returnsValue;
}

bool IBaseExtensionClass::CallAsProc(const long methodNumber, tVariant* parameters, const long parametersSize) {
	bool result = false;
	const auto basicStringError = format(L"Не удалось выполнить метод '{}' (без возвращаемого значения)", p_ExtMethods[methodNumber].nameRu);

	lastErrors.clear();

	try {
		log1C(plog::info, format(L"Выполнение метода '{}' (без возвращаемого значения)", p_ExtMethods[methodNumber].nameRu));

		auto args = p_GetInputParameters(parameters, parametersSize);
		p_ExtMethods[methodNumber].call(args);

		log1C(plog::debug, format(L"Выполнение метода '{}' (без возвращаемого значения) завершено", p_ExtMethods[methodNumber].nameRu));

		if (p_ExtMethods[methodNumber].canChangeInputParam) {
			log1C(plog::debug, format(L"Выполнение метода '{}' (без возвращаемого значения) - замена входных параметров", p_ExtMethods[methodNumber].nameRu));
			for (long i = 0L; i < parametersSize; i++) {
				p_WriteReturnValue(args[i], &parameters[i]);
			}
		}

		result = true;
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const out_of_range& e) {
		log1C(plog::error, format(L"{} <out_of_range>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const string& e) {
		log1C(plog::error, format(L"{} <string>!\n{}", basicStringError, gl_conv_string_to_wstring(e)));
	}
	catch (const char*& e) {
		log1C(plog::error, format(L"{} <string>!\n{}", basicStringError, gl_conv_string_to_wstring(e)));
	}
	catch (const wstring& e) {
		log1C(plog::error, format(L"{} <wstring>!\n{}", basicStringError, e));
	}
	catch (const wchar_t*& e) {
		log1C(plog::error, format(L"{} <wstring>!\n{}", basicStringError, e));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return silentMode ? true : result;
}

bool IBaseExtensionClass::CallAsFunc(const long methodNumber, tVariant* resultValue, tVariant* parameters, const long parametersSize) {
	bool result = false;
	const auto basicStringError = format(L"Не удалось выполнить метод '{}' (c возвращаемым значением)", p_ExtMethods[methodNumber].nameRu);

	lastErrors.clear();

	try {
		log1C(plog::info, format(L"Выполнение метода '{}' (с возвращаемым значением)", p_ExtMethods[methodNumber].nameRu));

		auto args = p_GetInputParameters(parameters, parametersSize);
		const all_returned_types resultMethod = p_ExtMethods[methodNumber].call(args);

		log1C(plog::debug, format(L"Выполнение метода '{}' (с возвращаемым значением) завершено", p_ExtMethods[methodNumber].nameRu));

		p_WriteReturnValue(resultMethod, resultValue);

		if (p_ExtMethods[methodNumber].canChangeInputParam) {
			for (long i = 0L; i < parametersSize; i++)
				p_WriteReturnValue(args[i], &parameters[i]);
		}

		result = true;
	}
	catch (const boost::exception& e) {
		log1C(plog::error, format(L"{} <boost::exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const runtime_error& e) {
		log1C(plog::error, format(L"{} <runtime_error>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const bad_alloc& e) {
		log1C(plog::error, format(L"{} <bad_alloc>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const out_of_range& e) {
		log1C(plog::error, format(L"{} <out_of_range>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const invalid_argument& e) {
		log1C(plog::error, format(L"{} <invalid_argument>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const exception& e) {
		log1C(plog::error, format(L"{} <exception>!\n{}", basicStringError, getInfoFromException(e)));
	}
	catch (const string& e) {
		log1C(plog::error, format(L"{} <string>!\n{}", basicStringError, gl_conv_string_to_wstring(e)));
	}
	catch (const char*& e) {
		log1C(plog::error, format(L"{} <string>!\n{}", basicStringError, gl_conv_string_to_wstring(e)));
	}
	catch (const wstring& e) {
		log1C(plog::error, format(L"{} <wstring>!\n{}", basicStringError, e));
	}
	catch (const wchar_t*& e) {
		log1C(plog::error, format(L"{} <wstring>!\n{}", basicStringError, e));
	}
	catch (...) {
		log1C(plog::error, format(L"{}! Неизвестная ошибка.", basicStringError));
	}

	return silentMode ? true : result;
}

void IBaseExtensionClass::SetLocale(const WCHAR_T* loc) {
	// Все локали инициализированы в dllExport.cpp
}

void IBaseExtensionClass::SetUserInterfaceLanguageCode(const WCHAR_T* loc) {
	// Все локали инициализированы в dllExport.cpp
}

#pragma endregion

#pragma region Вспомогательные функции

IBaseExtensionClass::IBaseExtensionClass() {
	p_MemoryManager = nullptr;
	p_Connector1C = nullptr;
}

IBaseExtensionClass::~IBaseExtensionClass() {
	if (debuggingMode && logsInitialize)
		PLOG_WARNING << L"Вызван деструктор";
}

void IBaseExtensionClass::p_SetupLoggingToFile() {
	if (!debuggingMode || logsInitialize)
		return;

	const auto fileName = filesystem::temp_directory_path().string() + FILE_SEPARATOR + gl_conv_wstring_to_string(getNameExtension()) + ".log";

	init(plog::debug, fileName.c_str());
	log1C(plog::info, L"Начало логирования");

	if (!itsServer) {
		const auto messageFor1C = format(L"Инициализирован вывод логов в файл '{}'", gl_conv_string_to_wstring(fileName));
		showMessage(messageFor1C);
	}

	logsInitialize = true;
}

void IBaseExtensionClass::p_WriteReturnValue(const all_returned_types& result, tVariant* for1C) {
	if (TV_VT(for1C) == VTYPE_PWSTR && TV_WSTR(for1C) != nullptr)
		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&TV_WSTR(for1C)));
	if ((TV_VT(for1C) == VTYPE_PSTR || TV_VT(for1C) == VTYPE_BLOB) && TV_STR(for1C) != nullptr)
		p_MemoryManager->FreeMemory(reinterpret_cast<void**>(&TV_STR(for1C)));

	visit(Overloaded{
		      [&for1C](monostate) { TV_VT(for1C) = VTYPE_NULL; },

		      [&for1C](nullptr_t) { TV_VT(for1C) = VTYPE_NULL; },

		      [&for1C](const int8_t& res) {
			      TV_VT(for1C) = VTYPE_I1;
			      TV_I1(for1C) = res;
		      },

		      [&for1C](const int16_t& res) {
			      TV_VT(for1C) = VTYPE_I2;
			      TV_I2(for1C) = res;
		      },

		      [&for1C](const int32_t& res) {
			      TV_VT(for1C) = VTYPE_I4;
			      TV_I4(for1C) = res;
		      },

		      [&for1C](const uint8_t& res) {
			      TV_VT(for1C) = VTYPE_UI1;
			      TV_UI1(for1C) = res;
		      },

		      [&for1C](const uint16_t& res) {
			      TV_VT(for1C) = VTYPE_UI2;
			      TV_UI2(for1C) = res;
		      },

		      [&for1C](const uint32_t& res) {
			      TV_VT(for1C) = VTYPE_UI4;
			      TV_UI4(for1C) = res;
		      },

		      [&for1C](const long& res) {
			      TV_VT(for1C) = VTYPE_I4;
			      TV_I4(for1C) = res;
		      },

		      [&for1C](const float& res) {
			      // Если передавать VTYPE_R4, то почему-то в 1С всё равно читает VTYPE_R8.
			      TV_VT(for1C) = VTYPE_R8;
			      TV_R8(for1C) = res;
		      },

		      [&for1C](const double& res) {
			      TV_VT(for1C) = VTYPE_R8;
			      TV_R8(for1C) = res;
		      },

		      [&for1C](const boost::posix_time::ptime& res) {
			      TV_VT(for1C) = VTYPE_TM;

			      const auto date = res.date();
			      const auto time = res.time_of_day();

			      for1C->tmVal.tm_sec = static_cast<int>(time.seconds());
			      for1C->tmVal.tm_min = static_cast<int>(time.minutes());
			      for1C->tmVal.tm_hour = static_cast<int>(time.hours());
			      for1C->tmVal.tm_mday = static_cast<int>(date.day().as_number());
			      for1C->tmVal.tm_mon = date.month().as_number() - 1;
			      for1C->tmVal.tm_year = static_cast<int>(date.year()) - START_YEAR_1C;
		      },

		      [&for1C](const bool& res) {
			      TV_VT(for1C) = VTYPE_BOOL;
			      TV_BOOL(for1C) = res;
		      },

		      [&](const string& res) {
			      TV_VT(for1C) = VTYPE_PSTR;
			      for1C->strLen = static_cast<uint32_t>(res.length());

			      const unsigned long size = static_cast<unsigned long>((for1C->strLen + 1) * sizeof(char));
			      if (!p_MemoryManager || !p_MemoryManager->AllocMemory(reinterpret_cast<void**>(&for1C->pstrVal), size))
				      BOOST_THROW_EXCEPTION(bad_alloc());

			      memcpy(for1C->pstrVal, res.data(), size);
		      },

		      [&](const wstring& res) {
			      TV_VT(for1C) = VTYPE_PWSTR;

			      const auto& forResult = gl_conv_wstring_to_WCHAR_T(res);
			      for1C->wstrLen = static_cast<uint32_t>(forResult.length());

			      const unsigned long size = static_cast<unsigned long>((for1C->wstrLen + 1) * sizeof(WCHAR_T));
			      if (!p_MemoryManager || !p_MemoryManager->AllocMemory(reinterpret_cast<void**>(&for1C->pwstrVal), size))
				      BOOST_THROW_EXCEPTION(bad_alloc());

			      memcpy(for1C->pwstrVal, forResult.data(), size);
		      },

		      [&](const json_document& res) {
			      wstring json = getStringFromJSON(res);
			      const auto tmp = static_cast<all_returned_types>(json);

			      p_WriteReturnValue(tmp, for1C);
		      },

		      [&](const vector<char>& res) {
			      TV_VT(for1C) = VTYPE_BLOB;
			      for1C->strLen = static_cast<uint32_t>(res.size());

			      if (!p_MemoryManager || !p_MemoryManager->AllocMemory(reinterpret_cast<void**>(&TV_STR(for1C)), for1C->strLen))
				      BOOST_THROW_EXCEPTION(bad_alloc());

			      memcpy(TV_STR(for1C), res.data(), res.size());
		      },

	      }, result);
}

void IBaseExtensionClass::p_WriteReturnValue(const all_input_types& result, tVariant* for1C) {
	visit(Overloaded{
		      [&for1C](monostate) { TV_VT(for1C) = VTYPE_NULL; },

		      [&for1C](nullptr_t) { TV_VT(for1C) = VTYPE_NULL; },

		      [&](const int32_t& res) {
			      const auto tmp = static_cast<all_returned_types>(res);
			      p_WriteReturnValue(tmp, for1C);
		      },
		      [&](const double& res) {
			      const auto tmp = static_cast<all_returned_types>(res);
			      p_WriteReturnValue(tmp, for1C);
		      },

		      [&](const boost::posix_time::ptime& res) {
			      const auto tmp = static_cast<all_returned_types>(res);
			      p_WriteReturnValue(tmp, for1C);
		      },

		      [&](const bool& res) {
			      const auto tmp = static_cast<all_returned_types>(res);
			      p_WriteReturnValue(tmp, for1C);
		      },

		      [&](const wstring& res) {
			      const auto tmp = static_cast<all_returned_types>(res);
			      p_WriteReturnValue(tmp, for1C);
		      },

		      [&](const vector<char>& res) {
			      const auto tmp = static_cast<all_returned_types>(res);
			      p_WriteReturnValue(tmp, for1C);
		      },

	      }, result);
}

all_input_types IBaseExtensionClass::p_GetParametersValue(tVariant* from1C) {
	switch (TV_VT(from1C)) {
		case VTYPE_EMPTY:
		case VTYPE_NULL:
			return monostate();
		case VTYPE_I1:
		case VTYPE_I2:
		case VTYPE_I4:
		case VTYPE_INT:
		case VTYPE_UI1:
		case VTYPE_UI2:
			return TV_I4(from1C);
		case VTYPE_HRESULT:
		case VTYPE_R4:
		case VTYPE_R8:
		case VTYPE_UI4:
		case VTYPE_UINT:
			return TV_R8(from1C);
		case VTYPE_TM: {
			const boost::gregorian::date date(START_YEAR_1C + from1C->tmVal.tm_year, from1C->tmVal.tm_mon + 1, from1C->tmVal.tm_mday);
			const boost::posix_time::time_duration time(from1C->tmVal.tm_hour, from1C->tmVal.tm_min, from1C->tmVal.tm_sec);

			return boost::posix_time::ptime(date, time);
		}
		case VTYPE_BOOL:
			return TV_BOOL(from1C);
		case VTYPE_PSTR:
			return gl_conv_string_to_wstring(TV_STR(from1C));
		case VTYPE_PWSTR:
			return gl_conv_WCHAR_T_to_wstring(TV_WSTR(from1C));
		case VTYPE_BLOB:
			return vector<char>(from1C->pstrVal, from1C->pstrVal + from1C->strLen);
		default:
			BOOST_THROW_EXCEPTION(invalid_argument("Пришёл неизвестный тип переменной из 1С"));
	}
}

vector<all_input_types> IBaseExtensionClass::p_GetInputParameters(tVariant* from1C, const long countParameters) {
	vector<all_input_types> result;

	const auto size = static_cast<const unsigned long>(countParameters);
	result.reserve(size);

	for (unsigned long i = 0; i < size; i++)
		result.emplace_back(p_GetParametersValue(&from1C[i]));

	return result;
}

void IBaseExtensionClass::p_WriteValueTo1C(const wstring& src, WCHAR_T** dst) const {
	const auto& tmp = gl_conv_wstring_to_WCHAR_T(src);
	const unsigned long size = static_cast<unsigned long>((tmp.length() + 1) * sizeof(WCHAR_T));

	if (!p_MemoryManager || !p_MemoryManager->AllocMemory(reinterpret_cast<void**>(dst), size))
		BOOST_THROW_EXCEPTION(bad_alloc());

	memcpy(*dst, tmp.data(), size);
}

#pragma endregion
