#include <extensions/extensionBaseFunction.hpp>

#pragma region Переопределение методов

const wchar_t* BaseFunction::getNameExtension() {
	return L"BaseFunction";
}

const wchar_t* BaseFunction::getVersion() {
	return L"1.0.0.18";
}

void BaseFunction::setMethodPropsExtension() {
	m_MethodNames[gl_Index_Method_Pause]                  = NamesFor1C(L"Пауза", L"Pause");
	m_MethodNames[gl_Index_Method_String_To_Number]       = NamesFor1C(L"СтрокаВЧисло", L"StringToNumber");
	m_MethodNames[gl_Index_Method_Clear_String]           = NamesFor1C(L"ТолькоБуквыВСтроку", L"CorrectString");
	m_MethodNames[gl_Index_Method_Clear_String_Json]      = NamesFor1C(L"ТолькоБуквыВСтрокуJSON", L"CorrectStringJSON");
	m_MethodNames[gl_Index_Method_Compress]               = NamesFor1C(L"СжатьДанные", L"Compress");
	m_MethodNames[gl_Index_Method_Decompress]             = NamesFor1C(L"РазжатьДанные", L"Decompress");
	m_MethodNames[gl_Index_Method_Current_Unix_Timestamp] = NamesFor1C(L"ТекущийUnixTimestamp", L"CurrentUnixTimestamp");
	m_MethodNames[gl_Index_Method_UUID]                   = NamesFor1C(L"УникальныйИдентификатор", L"UUID");
	m_MethodNames[gl_Index_Method_StartTimer]             = NamesFor1C(L"НачатьЗамер", L"StartTimer");
	m_MethodNames[gl_Index_Method_EndTimer]               = NamesFor1C(L"ЗавершитьЗамер", L"EndTimer");
	m_MethodNames[gl_Index_Method_Regex_Match]            = NamesFor1C(L"СтрокаСоответствуетРегулярномуВыражению", L"RegexMatch");

#ifndef __linux__
	if (!m_ItsServer) {
		m_MethodNames[gl_Index_Method_Get_Screenshot]    = NamesFor1C(L"Скриншот", L"Screenshot");
		m_MethodNames[gl_Index_Method_Interrupt_Handler] = NamesFor1C(L"ОбработкаПрерывания", L"InterruptHandler");
	}
#endif
}

void BaseFunction::getPropByIndex(const unsigned short indexProp, tVariant* pvarPropVal) {

}

void BaseFunction::setPropByIndex(const unsigned short indexProp, tVariant* varPropVal) {

}

bool BaseFunction::getIsPropWritable(const unsigned short indexProp) {
	return false;
}

long BaseFunction::getMethodNParams(const unsigned short indexMethod) {
	switch (indexMethod) {
		case gl_Index_Method_Pause:
		case gl_Index_Method_String_To_Number:
		case gl_Index_Method_Get_Screenshot:
		case gl_Index_Method_StartTimer:
		case gl_Index_Method_EndTimer:
			return 1l;
		case gl_Index_Method_Clear_String:
		case gl_Index_Method_Decompress:
		case gl_Index_Method_Regex_Match:
			return 2l;
		case gl_Index_Method_Compress:
			return 4l;
		case gl_Index_Method_Clear_String_Json:
			return 5l;
		default:
			return 0l;
	}
}

void BaseFunction::setParamDefValue(const unsigned short indexMethod, const long indexParam, tVariant* pvarParamDefValue) {
	switch (indexMethod) {
		case gl_Index_Method_Pause: {
			if (indexParam == 0l) {
				setReturnedParam(500, pvarParamDefValue);
				return;
			}
			break;
		}
		case gl_Index_Method_Clear_String: {
			if (indexParam == 1l) {
				setReturnedParam(L"_ ", pvarParamDefValue);
				return;
			}

			break;
		}
		case gl_Index_Method_Clear_String_Json: {
			switch (indexParam) {
				case 1l:
					setReturnedParam(L"_ ", pvarParamDefValue);
					return;
				case 2l:
					setReturnedParam(L"a_", pvarParamDefValue);
					return;
				case 3l:
					setReturnedParam(L"_default", pvarParamDefValue);
					return;
				case 4l:
					setReturnedParam(false, pvarParamDefValue);
					return;
				default:
					break;
			}

			break;
		}
		case gl_Index_Method_Compress: {
			switch (indexParam) {
				case 2l:
					setReturnedParam(4, pvarParamDefValue);
					return;
				case 3l:
					setReturnedParam(0, pvarParamDefValue);
					return;
				default:
					break;
			}

			break;
		}
		case gl_Index_Method_Get_Screenshot:
		case gl_Index_Method_StartTimer:
		case gl_Index_Method_EndTimer: {
			if (indexParam == 0l) {
				setReturnedParam("", pvarParamDefValue);
				return;
			}

			break;
		}
		default:
			break;
	}

	TV_VT(pvarParamDefValue) = VTYPE_EMPTY;
}

bool BaseFunction::getHasRetVal(const unsigned short indexMethod) {
	switch (indexMethod) {
		case gl_Index_Method_String_To_Number:
		case gl_Index_Method_Clear_String:
		case gl_Index_Method_Clear_String_Json:
		case gl_Index_Method_Compress:
		case gl_Index_Method_Decompress:
		case gl_Index_Method_Current_Unix_Timestamp:
		case gl_Index_Method_Get_Screenshot:
		case gl_Index_Method_Interrupt_Handler:
		case gl_Index_Method_UUID:
		case gl_Index_Method_EndTimer:
		case gl_Index_Method_Regex_Match:
			return true;
		default:
			return false;
	}
}

void BaseFunction::callMethodAsProc(const unsigned short indexMethod, const tVariant* paParams, const long paParamsSize) {
	switch (indexMethod) {
		case gl_Index_Method_Pause:
			pause(paParams);
			return;
		case gl_Index_Method_StartTimer:
			startTimer(paParams);
			return;
		default:
			return;
	}
}

void BaseFunction::callMethodAsFunc(const unsigned short indexMethod, tVariant* pvarRetValue, const tVariant* paParams, const long paParamsSize) {
	switch (indexMethod) {
		case gl_Index_Method_String_To_Number:
			stringToNumber(paParams, pvarRetValue);
			return;
		case gl_Index_Method_Clear_String:
			correctString(paParams, pvarRetValue);
			return;
		case gl_Index_Method_Clear_String_Json:
			correctStringJson(paParams, pvarRetValue);
			return;
		case gl_Index_Method_Compress:
			compress(paParams, pvarRetValue);
			return;
		case gl_Index_Method_Decompress:
			decompress(paParams, pvarRetValue);
			return;
		case gl_Index_Method_Current_Unix_Timestamp:
			currentUnixTimestamp(pvarRetValue);
			return;
		case gl_Index_Method_Get_Screenshot:
			getScreenshot(paParams, pvarRetValue);
			return;
		case gl_Index_Method_Interrupt_Handler:
			interruptHandler(pvarRetValue);
			return;
		case gl_Index_Method_UUID:
			getUuid(pvarRetValue);
			return;
		case gl_Index_Method_EndTimer:
			endTimer(paParams, pvarRetValue);
			return;
		case gl_Index_Method_Regex_Match:
			localRegexMatch(paParams, pvarRetValue);
			return;
		default:
			break;
	}
}

#pragma endregion

#pragma region Функции текущей компоненты

void BaseFunction::pause(const tVariant* paParams) const {
	if (const auto milliseconds = getInputParam<unsigned int>(paParams); milliseconds > 0)
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void BaseFunction::stringToNumber(const tVariant* paParams, tVariant* pvarRetValue) const {
	auto input = getInputParam<std::string>(paParams);
	std::replace(input.begin(), input.end(), ',', '.');

	input.erase(std::remove_if(input.begin(), input.end(), [&correct = this->m_CorrectValueForNumber](const char& c) {
	            return correct.find(c) == std::string::npos;
            }),
		input.end());

	setReturnedParam(std::stod(input.c_str()), pvarRetValue);
}

void BaseFunction::correctString(const tVariant* paParams, tVariant* pvarRetValue) const {
	auto input                 = getInputParam<std::wstring>(paParams, 0l);
	const auto wcorrectSymbols = getInputParam<std::wstring>(paParams, 1l);

	input.erase(std::remove_if(input.begin(), input.end(), [&wcorrectSymbols](const wchar_t& c)
	            {
		            return std::iswalnum(c) == 0 && wcorrectSymbols.find(c) == std::wstring::npos;
	            }),
	            input.end());

	setReturnedParam<const std::wstring&>(input, pvarRetValue);
}

void BaseFunction::correctStringJson(const tVariant* paParams, tVariant* pvarRetValue) const {
	const auto wcorrectSymbols = getInputParam<std::wstring>(paParams, 1l);
	const auto wprefix         = getInputParam<std::wstring>(paParams, 2l);
	const auto wdefaultName    = getInputParam<std::wstring>(paParams, 3l);
	
	rapidjson::Document js = getInputParam<bool>(paParams, 4l)
		? gl_GetInputJsonFromFile(getInputParam<std::wstring>(paParams))
		: getInputParam<rapidjson::Document>(paParams);

	jsonRecursiveCorrectKey(js, wcorrectSymbols, wdefaultName, wprefix);
	setReturnedParam<const rapidjson::Document&>(js, pvarRetValue);
}

void BaseFunction::compress(const tVariant* paParams, tVariant* pvarRetValue) const {
	const auto fileNameSrc = getInputParam<std::wstring>(paParams, 0l);
	const auto fileNameRes = getInputParam<std::wstring>(paParams, 1l);

	const auto src = gl_OpenFile(fileNameSrc, L"rb+");
	const auto dst = gl_OpenFile(fileNameRes, L"wb");

	if (!src)
		throw LykovException(L"Не удалось открыть исходный файл", L"Compress");
	if (!dst)
		throw LykovException(L"Не удалось открыть файл результат", L"Compress");

	const auto level    = getInputParam<unsigned int>(paParams, 2l);
	const auto strategy = getInputParam<unsigned int>(paParams, 3l);

	if (level < 1 || level > 9)
		throw LykovException(L"Некорректный уровень сжатия", L"Compress");

	zng_stream stream = {nullptr};

	if (const auto init = zng_deflateInit2(&stream, level, Z_DEFLATED, MAX_WBITS, MAX_MEM_LEVEL, strategy); init != Z_OK)
		throw LykovException(L"Не удалось инициализировать deflate! Проверьте входные данные", L"Compress");

	int flush;
	do {
		uint8_t inbuff[gl_Chunk_Size];
		stream.avail_in = static_cast<uint32_t>(fread(inbuff, 1, gl_Chunk_Size, src));
		if (ferror(src))
			throw LykovException(L"Не удалось прочитать данные из исходного файла!", L"Compress");

		flush          = feof(src) ? Z_FINISH : Z_NO_FLUSH;
		stream.next_in = inbuff;

		do {
			uint8_t outbuff[gl_Chunk_Size];
			stream.avail_out = gl_Chunk_Size;
			stream.next_out  = outbuff;
			zng_deflate(&stream, flush);

			if (const auto nbytes = gl_Chunk_Size - stream.avail_out; fwrite(outbuff, 1, nbytes, dst) != nbytes ||
				ferror(dst))
				throw LykovException(L"Не удалось записать данные в файл результат!", L"Compress");
		}
		while (stream.avail_out == 0);

	}
	while (flush != Z_FINISH);

	zng_deflateEnd(&stream);
	fclose(src);
	fclose(dst);

	setReturnedParam(true, pvarRetValue);
}

void BaseFunction::decompress(const tVariant* paParams, tVariant* pvarRetValue) const {
	const auto fileNameSrc = getInputParam<std::wstring>(paParams, 0l);
	const auto fileNameRes = getInputParam<std::wstring>(paParams, 1l);

	const auto src = gl_OpenFile(fileNameSrc, L"rb+");
	const auto dst = gl_OpenFile(fileNameRes, L"wb");
	
	if (!src)
		throw LykovException(L"Не удалось открыть исходный файл", L"Decompress");
	if (!dst)
		throw LykovException(L"Не удалось открыть файл результат", L"Decompress");

	zng_stream stream = {nullptr};

	auto result = zng_inflateInit(&stream);
	if (result != Z_OK) {
		if (stream.msg != nullptr)
			throw LykovException(stream.msg, L"Decompress");

		throw LykovException(L"Не удалось инициализировать inflate!", L"Decompress");
	}

	do {
		uint8_t inbuff[gl_Chunk_Size];
		stream.avail_in = static_cast<uint32_t>(fread(inbuff, 1, gl_Chunk_Size, src));
		if (ferror(src))
			throw LykovException(L"Не удалось прочитать данные из файла!", L"Decompress");

		if (stream.avail_in == 0)
			break;

		stream.next_in = inbuff;

		do {
			uint8_t outbuff[gl_Chunk_Size];
			stream.avail_out = gl_Chunk_Size;
			stream.next_out  = outbuff;

			result = zng_inflate(&stream, Z_NO_FLUSH);
			if (result == Z_NEED_DICT || result == Z_DATA_ERROR || result == Z_MEM_ERROR) {
				if (stream.msg != nullptr)
					throw LykovException(stream.msg, L"Decompress");

				throw LykovException(L"Не удалось разжать данные! Неизвестная ошибка.", L"Decompress");
			}

			if (const uint32_t nbytes = gl_Chunk_Size - stream.avail_out; fwrite(outbuff, 1, nbytes, dst) != nbytes ||
				ferror(dst))
				throw LykovException(L"Не удалось записать данные в файл результат!", L"Decompress");
		}
		while (stream.avail_out == 0);
	}
	while (result != Z_STREAM_END);

	zng_inflateEnd(&stream);
	fclose(src);
	fclose(dst);

	setReturnedParam(true, pvarRetValue);
}

void BaseFunction::currentUnixTimestamp(tVariant* pvarRetValue) const {
	setReturnedParam(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000000., pvarRetValue);
}

void BaseFunction::getScreenshot(const tVariant* paParams, tVariant* pvarRetValue) const {
#ifndef __linux__
	auto fileName = getInputParam<std::wstring>(paParams);
	bool toFile   = true;

	if (fileName.empty()) {
		toFile   = false;
		fileName = gl_TmpFileNameW(L"jpg");
	}

	const HDC hScreenDc = GetDC(nullptr);
	const HDC hMemoryDc = CreateCompatibleDC(hScreenDc);

	const int width       = GetDeviceCaps(hScreenDc, HORZRES);
	const int height      = GetDeviceCaps(hScreenDc, VERTRES);
	HBITMAP hBitmap       = CreateCompatibleBitmap(hScreenDc, width, height);
	const auto hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDc, hBitmap));
	BitBlt(hMemoryDc, 0, 0, width, height, hScreenDc, 0, 0, SRCCOPY);
	hBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDc, hOldBitmap));

	CImage image;
	image.Attach(hBitmap);
	if (const auto resultSave = image.Save(fileName.c_str()); resultSave != S_OK)
		throw LykovException(L"Не удалось записать картинку в файл " + fileName, L"Скриншот (WIN)");

	DeleteDC(hMemoryDc);
	DeleteDC(hScreenDc);

	if (toFile) {
		setReturnedParam(true, pvarRetValue);
	}
	else {
		const auto file = gl_OpenFile(fileName, L"rb+");
		if (!file)
			throw LykovException(L"Не удалось открыть файл для чтения/сохранения скриншота: " + fileName, L"Скриншот (WIN)");

		fseek(file, 0, SEEK_END);
		auto size = static_cast<size_t>(ftell(file));

		if (!m_IMemory->AllocMemory(reinterpret_cast<void**>(&pvarRetValue->pstrVal), static_cast<unsigned long>(size)))
			throw LykovException(L"Не удалось выделить память для binary data", L"getScreenshot");

		fseek(file, 0, SEEK_SET);
		size                 = fread(pvarRetValue->pstrVal, 1, size, file);
		pvarRetValue->strLen = static_cast<uint32_t>(size);
		TV_VT(pvarRetValue)  = VTYPE_BLOB;
		fclose(file);

		_wremove(fileName.c_str());
	}
#endif
}

void BaseFunction::interruptHandler(tVariant* pvarRetValue) const {
#ifndef __linux__
	setReturnedParam((GetAsyncKeyState(VK_CANCEL) & 0x8000) != 0, pvarRetValue);
#endif
}

void BaseFunction::getUuid(tVariant* pvarRetValue) const {
	setReturnedParam<const std::string&>(gl_GetNewUuid(), pvarRetValue);
}

void BaseFunction::startTimer(const tVariant* paParams) {
	auto id = getInputParam<std::wstring>(paParams);
	if (id.empty())
		id = L"default";

	m_MapTimer[id] = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void BaseFunction::endTimer(const tVariant* paParams, tVariant* pvarRetValue) const {
	auto id = getInputParam<std::wstring>(paParams);
	if (id.empty())
		id = L"default";

	if (const auto startTimer = m_MapTimer.find(id); startTimer != m_MapTimer.end())
		setReturnedParam(
			static_cast<double>((std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - startTimer->second)) / 1000000000.,
			pvarRetValue);
	else
		setReturnedParam(0, pvarRetValue);
}

void BaseFunction::localRegexMatch(const tVariant* paParams, tVariant* pvarRetValue) const {
	const auto strForMatch = getInputParam<std::wstring>(paParams, 0);
	const auto strRegex = getInputParam<std::wstring>(paParams, 1);

	static const std::wregex regex(strRegex);
	
	const auto res = std::regex_match(strForMatch, regex);
	setReturnedParam<bool>(res, pvarRetValue);
}

#pragma endregion

#pragma region Вспомогательные методы

BaseFunction::BaseFunction() {
	m_MapTimer[L"default"] = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void BaseFunction::jsonRecursiveCorrectKey(rapidjson::Document& object, const std::wstring& wcorrectSymbols, const std::wstring& wdefaultName, const std::wstring& wprefix) {
	if (object.IsArray()) {
		const auto tmp = object.GetArray();
		std::for_each(tmp.Begin(), tmp.End(), [&wcorrectSymbols, &wdefaultName, &wprefix](const auto& it) {
			jsonRecursiveCorrectKey((rapidjson::Document&)(it), wcorrectSymbols, wdefaultName, wprefix);
		});
	}
	else if (object.IsObject()) {
		for (auto& it : object.GetObj()) {
			const auto oldKey = gl_StringToWString(it.name.GetString());
			auto newKey = std::wstring(oldKey);

			newKey.erase(std::remove_if(newKey.begin(), newKey.end(), [&wcorrectSymbols](const wchar_t& c){
					return std::iswalnum(c) == 0 && wcorrectSymbols.find(c) == std::wstring::npos;
				}),
				newKey.end());

			if (newKey.empty())
				newKey = wdefaultName;
			else if (iswalpha(newKey[0]) == 0)
				newKey = wprefix + newKey;

			if (!gl_IEqualsCaseInsensitive(oldKey.c_str(), newKey.c_str()))
				it.name.SetString(gl_WStringToString(newKey), object.GetAllocator());

			jsonRecursiveCorrectKey(static_cast<rapidjson::Document&>(it.value), wcorrectSymbols, wdefaultName, wprefix);
		}
	}
}

#pragma endregion
