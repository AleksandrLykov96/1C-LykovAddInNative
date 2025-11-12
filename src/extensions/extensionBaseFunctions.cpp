
#include "extensions/extensionBaseFunctions.hpp"

using namespace std;
using namespace native1C;

#pragma region Методы для переопределения

wstring BaseFunctions::getNameExtension() {
	return L"BaseFunctions";
}

void BaseFunctions::initializeComponent() {
	addMethod(L"Пауза",
	          L"pause",
	          this,
	          &BaseFunctions::pause,
	          {{0, 500}});

	addMethod(L"СтрокаВЧисло",
	          L"stringToNumber",
	          this,
	          &BaseFunctions::stringToNumber);

	addMethod(L"ТолькоБуквыВСтроку",
	          L"correctString",
	          this,
	          &BaseFunctions::correctString,
	          {{1, L"_ "}});

	addMethod(L"СжатьДанныеDeflate",
	          L"deflate",
	          this,
	          &BaseFunctions::deflate1C,
	          {
		          {1, Z_DEFAULT_COMPRESSION},
		          {2, false},
		          {3, L""},
		          {4, 15},
		          {5, Z_DEFAULT_STRATEGY},
	          });

	addMethod(L"РазжатьДанныеInflate",
	          L"inflate",
	          this,
	          &BaseFunctions::inflate1C,
	          {{1, L""}, {2, 15}});

	addMethod(L"ТекущийUnixTimestamp",
	          L"unixTimestamp",
	          this,
	          &BaseFunctions::unixTimestamp);

	addMethod(L"СтрокаСоответствуетРегулярномуВыражению",
	          L"regexMatch",
	          this,
	          &BaseFunctions::regexMatch);

	addMethod(L"ЗашифроватьСтроку",
	          L"encrypt",
	          this,
	          &BaseFunctions::encrypt1C,
	          {{1, L"Ключ по-умолчанию"}});

	addMethod(L"РасшифроватьСтроку",
	          L"decrypt",
	          this,
	          &BaseFunctions::decrypt1C,
	          {{1, L"Ключ по-умолчанию"}});

	addMethod(L"НачатьЗамер",
	          L"startTimer",
	          this,
	          &BaseFunctions::startTimer,
	          {{0, L"default"}});

	addMethod(L"ЗавершитьЗамер",
	          L"stopTimer",
	          this,
	          &BaseFunctions::stopTimer,
	          {{0, L"default"}});

	if (!itsServer) {
		addMethod(L"ОбработкаПрерывания",
		          L"interruptHandler",
		          this,
		          &BaseFunctions::interruptHandler);
	}
}

#pragma endregion

#pragma region Методы компоненты

void BaseFunctions::pause(const all_input_types& millisecondsInput) {
	visit(Overloaded{
		      [](const int32_t& milliseconds) {
			      if (milliseconds < 0)
				      BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра должен быть 'Неотрицательное число' (количество миллисекунд для паузы)"));

			      this_thread::sleep_for(chrono::milliseconds(milliseconds));
		      },

		      [](const double& milliseconds) {
			      const auto msLong = llround(milliseconds);
			      if (msLong < 0)
				      BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра должен быть 'Неотрицательное число' (количество миллисекунд для паузы)"));

			      this_thread::sleep_for(chrono::milliseconds(msLong));
		      },

		      [](auto) {
			      BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра должен быть 'Неотрицательное число' (количество миллисекунд для паузы)"));
		      }
	      }, millisecondsInput);
}

all_returned_types BaseFunctions::stringToNumber(const all_input_types& forConvertInput) {
	if (!holds_alternative<wstring>(forConvertInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра должен быть 'Строка' (строка для преобразования)"));

	auto forConvert = get<wstring>(forConvertInput);
	const wstring correct = L"0123456789-.";

	ranges::replace(forConvert, L',', L'.');

	erase_if(forConvert, [correct](const wchar_t& c) {
		return correct.find(c) == wstring::npos;
	});

	const auto convertString = gl_conv_wstring_to_string(forConvert);
	return stod(convertString);
}

all_returned_types BaseFunctions::correctString(const all_input_types& forConvertInput, const all_input_types& addSymbolsInput) {
	if (!holds_alternative<wstring>(forConvertInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка' (строка для конвертации)"));

	if (!holds_alternative<wstring>(addSymbolsInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №2 должен быть 'Строка' (символы, которые оставляем)"));

	auto forConvert = get<wstring>(forConvertInput);
	const auto addSymbols = get<wstring>(addSymbolsInput);

	erase_if(forConvert, [addSymbols](const wchar_t& c) {
		return iswalnum(c) == 0 && addSymbols.find(c) == wstring::npos;
	});

	return forConvert;
}

all_returned_types BaseFunctions::deflate1C(const all_input_types& forCompressInput, const all_input_types& levelCompressionInput,
                                            const all_input_types& fromFileInput, const all_input_types& fileNameResultInput,
                                            const all_input_types& winBitsInput, const all_input_types& strategyInput) {
	if (!holds_alternative<int32_t>(levelCompressionInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №2 должен быть 'Неотрицательное число' (уровень сжатия)"));
	if (!holds_alternative<bool>(fromFileInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №3 должен быть 'Булево' (читать исходные данные из файла)"));
	if (!holds_alternative<wstring>(fileNameResultInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №4 должен быть 'Строка' (имя файла для результата)"));
	if (!holds_alternative<int32_t>(winBitsInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №5 должен быть 'Число' (winbits)"));
	if (!holds_alternative<int32_t>(strategyInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №6 должен быть 'Число' (strategy)"));

	const auto levelCompression = get<int32_t>(levelCompressionInput);
	switch (levelCompression) {
		case Z_NO_COMPRESSION:
		case Z_BEST_SPEED:
		case Z_BEST_COMPRESSION:
		case Z_DEFAULT_COMPRESSION:
			break;
		default: {
			const auto errorMessage = std::format("Параметр №2 содержит некорректное значение (уровень сжатия). Возможные значения:\n"
	              "\t {} - Z_NO_COMPRESSION\n"
	              "\t {} - Z_BEST_SPEED\n"
	              "\t {} - Z_BEST_COMPRESSION\n"
	              "\t {} - Z_DEFAULT_COMPRESSION", Z_NO_COMPRESSION, Z_BEST_SPEED, Z_BEST_COMPRESSION, Z_DEFAULT_COMPRESSION);

			BOOST_THROW_EXCEPTION(invalid_argument(errorMessage));
		}
	}

	const auto winBits = get<int32_t>(winBitsInput);
	const auto strategy = get<int32_t>(strategyInput);

	switch (strategy) {
		case Z_FILTERED:
		case Z_HUFFMAN_ONLY:
		case Z_RLE:
		case Z_FIXED:
		case Z_DEFAULT_STRATEGY:
			break;
		default: {
			const auto errorMessage = std::format("Параметр №6 содержит некорректное значение (strategy). Возможные значения:\n"
                  "\t {} - Z_FILTERED\n"
                  "\t {} - Z_HUFFMAN_ONLY\n"
                  "\t {} - Z_RLE\n"
                  "\t {} - Z_FIXED\n"
                  "\t {} - Z_DEFAULT_STRATEGY", Z_FILTERED, Z_HUFFMAN_ONLY, Z_RLE, Z_FIXED, Z_DEFAULT_STRATEGY);

			BOOST_THROW_EXCEPTION(invalid_argument(errorMessage));
		}
	}

	log1C(plog::debug, L"Инициализация deflateInit");

	z_stream stream = {};

	stream.zalloc = nullptr;
	stream.zfree = nullptr;
	stream.opaque = nullptr;
	stream.avail_in = 0;
	stream.next_in = nullptr;

	if (const auto success = deflateInit2(&stream, levelCompression, Z_DEFLATED, winBits, 8, strategy); success != Z_OK) {
		const auto errorMessage = std::format("Не удалось инициализировать zlib\n\t %s)", stream.msg);
		BOOST_THROW_EXCEPTION(runtime_error(errorMessage.c_str()));
	}

	const auto fileNameResult = get<wstring>(fileNameResultInput);
	const auto toVector = fileNameResult.empty();

	vector<char> result;

	visit(Overloaded{
		      [&](const wstring& src) {
			      if (get<bool>(fromFileInput)) {
				      log1C(plog::info, std::format(L"Сжатие данных из файла '{}'", src));
				      if (toVector)
					      result = deflateFromFileToVector(stream, src);
				      else
					      deflateFromFileToFile(stream, src, fileNameResult);
			      }
			      else {
				      log1C(plog::info, L"Сжатие данных строки");
				      if (toVector)
					      result = deflateFromStringToVector(stream, src);
				      else
					      deflateFromStringToFile(stream, src, fileNameResult);
			      }
		      },
		      [&](const vector<char>& src) {
			      log1C(plog::info, L"Сжатие двоичных данных");
			      const string str(src.begin(), src.end());
			      if (toVector)
				      result = deflateFromStringToVector(stream, gl_conv_string_to_wstring(str));
			      else
				      deflateFromStringToFile(stream, gl_conv_string_to_wstring(str), fileNameResult);
		      },

		      [](auto) {
			      BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть либо 'Строка', либо 'Двоичные данные'"));
		      }
	      }, forCompressInput);

	log1C(plog::debug, L"Деструктор deflateEnd");
	deflateEnd(&stream);

	return result;
}

all_returned_types BaseFunctions::inflate1C(const all_input_types& forCompressInput, const all_input_types& fileNameResultInput, const all_input_types& winBitsInput) {
	if (!holds_alternative<wstring>(fileNameResultInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №2 должен быть 'Строка' (имя файла для результата)"));
	if (!holds_alternative<int32_t>(winBitsInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №3 должен быть 'Число' (winbits)"));

	const auto winBits = get<int32_t>(winBitsInput);

	log1C(plog::debug, L"Инициализация inflateInit");

	z_stream stream = {};

	stream.zalloc = nullptr;
	stream.zfree = nullptr;
	stream.opaque = nullptr;
	stream.avail_in = 0;
	stream.next_in = nullptr;

	if (const auto success = inflateInit2(&stream, winBits); success != Z_OK) {
		const auto errorMessage = std::format("Не удалось инициализировать zlib\n\t {})", stream.msg);
		BOOST_THROW_EXCEPTION(runtime_error(errorMessage));
	}

	const auto fileNameResult = get<wstring>(fileNameResultInput);
	const auto toVector = fileNameResult.empty();

	vector<char> result;

	visit(Overloaded{
		      [&](const wstring& src) {
			      log1C(plog::info, std::format(L"Разжатие данных из файла '{}'", src));
			      if (toVector)
				      result = inflateFromFileToVector(stream, src);
			      else
				      inflateFromFileToFile(stream, src, fileNameResult);
		      },
		      [&](const vector<char>& src) {
			      log1C(plog::info, L"Разжатие данных из двоичных данных");
			      if (toVector)
				      result = inflateFromVectorToVector(stream, src);
			      else
				      inflateFromVectorToFile(stream, src, fileNameResult);
		      },

		      [](auto) {
			      BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть либо 'Строка', либо 'Двоичные данные'"));
		      }
	      }, forCompressInput);

	log1C(plog::debug, L"Деструктор inflateEnd");
	inflateEnd(&stream);

	return result;
}

all_returned_types BaseFunctions::unixTimestamp() {
	const auto result = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch());
	return static_cast<double>(result.count()) / 1000000.;
}

all_returned_types BaseFunctions::interruptHandler() {
#ifdef _WINDOWS
	return (GetAsyncKeyState(VK_CANCEL) & 0x8000) != 0;
#else
	return false;
#endif
}

all_returned_types BaseFunctions::regexMatch(const all_input_types& stringInput, const all_input_types& regexInput) {
	if (!holds_alternative<wstring>(stringInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка' (строка для проверки)"));
	if (!holds_alternative<wstring>(regexInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №2 должен быть 'Строка' (regex шаблон)"));

	const auto str = get<wstring>(stringInput);
	const auto regex = get<wstring>(regexInput);

	const wregex pattern(regex);
	return regex_match(str, pattern);
}

all_returned_types BaseFunctions::encrypt1C(const all_input_types& strInput, const all_input_types& keyInput) {
	if (!holds_alternative<wstring>(strInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка' (строка для шифрования)"));
	if (!holds_alternative<wstring>(keyInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №2 должен быть 'Строка' (ключ для шифрования)"));

	const auto str = get<wstring>(strInput);
	const auto key = get<wstring>(keyInput);

	return kuz::Kuznechik(key).encrypt(str);
}

all_returned_types BaseFunctions::decrypt1C(const all_input_types& strInput, const all_input_types& keyInput) {
	if (!holds_alternative<wstring>(strInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка' (зашифрованные данные)"));
	if (!holds_alternative<wstring>(keyInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №2 должен быть 'Строка' (ключ для шифрования)"));

	const auto str = get<wstring>(strInput);
	const auto key = get<wstring>(keyInput);

	return kuz::Kuznechik(key).decrypt(str);
}

void BaseFunctions::startTimer(const all_input_types& keyInput) {
	if (!holds_alternative<wstring>(keyInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка' (ключ для таймера)"));

	const auto key = get<wstring>(keyInput);
	p_MapTimer[key] = chrono::duration_cast<chrono::nanoseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

all_returned_types BaseFunctions::stopTimer(const all_input_types& keyInput) {
	if (!holds_alternative<wstring>(keyInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка' (ключ для таймера)"));

	const auto key = get<wstring>(keyInput);
	if (const auto startTimer = p_MapTimer.find(key); startTimer != p_MapTimer.end()) {
		const auto result = static_cast<double>((chrono::duration_cast<chrono::nanoseconds>(chrono::system_clock::now().time_since_epoch()).count() - startTimer->second)) / 1000000000.;

		p_MapTimer.erase(startTimer);
		return result;
	}

	return 0;
}

#pragma endregion

#pragma region Вспомогательные методы

void BaseFunctions::deflateFromFileToFile(z_stream& stream, const wstring& source, const wstring& destination) {
	const filesystem::path inputPath(source);
	ifstream input(inputPath, ios_base::binary);

	if (!input || !input.is_open())
		BOOST_THROW_EXCEPTION(runtime_error("Не удалось открыть поток для чтения файла"));

	const filesystem::path outputPath(destination);
	ofstream output(outputPath, ios_base::binary);

	if (!output || !output.is_open())
		BOOST_THROW_EXCEPTION(runtime_error("Не удалось открыть поток для записи файла"));

	int flush;
	do {
		char buffer[buffer_size];

		input.read(buffer, buffer_size);
		flush = input.eof() ? Z_FINISH : Z_NO_FLUSH;

		stream.avail_in = static_cast<uint32_t>(input.gcount());
		stream.next_in = reinterpret_cast<Bytef*>(buffer);

		do {
			char outBuffer[buffer_size];

			stream.avail_out = buffer_size;
			stream.next_out = reinterpret_cast<Bytef*>(outBuffer);
			deflate(&stream, flush);

			output.write(outBuffer, buffer_size - stream.avail_out);
		}
		while (stream.avail_out == 0);
	}
	while (flush != Z_FINISH);

	input.close();
	output.close();
}

vector<char> BaseFunctions::deflateFromFileToVector(z_stream& stream, const wstring& source) {
	const filesystem::path inputPath(source);

	ifstream input(inputPath, ios_base::binary);
	if (!input || !!input.is_open())
		BOOST_THROW_EXCEPTION(runtime_error("Не удалось открыть поток для чтения файла"));

	string output;

	int flush;
	do {
		char buffer[buffer_size];

		input.read(buffer, buffer_size);
		flush = input.eof() ? Z_FINISH : Z_NO_FLUSH;

		stream.avail_in = static_cast<uint32_t>(input.gcount());
		stream.next_in = reinterpret_cast<Bytef*>(buffer);

		do {
			char outBuffer[buffer_size];

			stream.avail_out = buffer_size;
			stream.next_out = reinterpret_cast<Bytef*>(outBuffer);
			deflate(&stream, flush);

			if (output.size() < stream.total_out)
				output.append(buffer, stream.total_out - output.size());
		}
		while (stream.avail_out == 0);
	}
	while (flush != Z_FINISH);

	input.close();

	return {output.begin(), output.end()};
}

void BaseFunctions::deflateFromStringToFile(z_stream& stream, const wstring& source, const wstring& destination) {
	const filesystem::path outputPath(destination);

	ofstream output(outputPath, ios_base::binary);
	if (!output || !output.is_open())
		BOOST_THROW_EXCEPTION(runtime_error("Не удалось открыть поток для записи файла"));

	auto input = gl_conv_wstring_to_string(source);

	stream.next_in = reinterpret_cast<Bytef*>(input.data());
	stream.avail_in = static_cast<uint32_t>(input.size());

	do {
		char outBuffer[buffer_size];

		stream.avail_out = buffer_size;
		stream.next_out = reinterpret_cast<Bytef*>(outBuffer);
		deflate(&stream, Z_FINISH);

		output.write(outBuffer, buffer_size - stream.avail_out);
	}
	while (stream.avail_out == 0);

	output.close();
}

vector<char> BaseFunctions::deflateFromStringToVector(z_stream& stream, const wstring& source) {
	auto input = gl_conv_wstring_to_string(source);
	string output;

	stream.next_in = reinterpret_cast<Bytef*>(input.data());
	stream.avail_in = static_cast<uint32_t>(input.size());

	do {
		char buffer[buffer_size];

		stream.avail_out = buffer_size;
		stream.next_out = reinterpret_cast<Bytef*>(buffer);
		deflate(&stream, Z_FINISH);

		if (output.size() < stream.total_out)
			output.append(buffer, stream.total_out - output.size());
	}
	while (stream.avail_out == 0);

	return {output.begin(), output.end()};
}

void BaseFunctions::inflateFromFileToFile(z_stream& stream, const wstring& source, const wstring& destination) {
	const filesystem::path inputPath(source);
	ifstream input(inputPath, ios_base::binary);

	if (!input || !input.is_open())
		BOOST_THROW_EXCEPTION(runtime_error("Не удалось открыть поток для чтения файла"));

	const filesystem::path outputPath(destination);
	ofstream output(outputPath, ios_base::binary);

	if (!output || !output.is_open())
		BOOST_THROW_EXCEPTION(runtime_error("Не удалось открыть поток для записи файла"));

	int ret;
	int flush;
	do {
		char buffer[buffer_size];

		input.read(buffer, buffer_size);
		flush = input.eof() ? Z_FINISH : Z_NO_FLUSH;

		stream.avail_in = static_cast<uint32_t>(input.gcount());
		stream.next_in = reinterpret_cast<Bytef*>(buffer);

		do {
			char outBuffer[buffer_size];

			stream.avail_out = buffer_size;
			stream.next_out = reinterpret_cast<Bytef*>(outBuffer);

			ret = inflate(&stream, flush);
			if (ret <= Z_ERRNO)
				BOOST_THROW_EXCEPTION(runtime_error(stream.msg));

			output.write(outBuffer, buffer_size - stream.avail_out);
		}
		while (stream.avail_out == 0);
	}
	while (flush != Z_FINISH && ret != Z_STREAM_END);

	input.close();
	output.close();
}

vector<char> BaseFunctions::inflateFromFileToVector(z_stream& stream, const wstring& source) {
	const filesystem::path inputPath(source);
	ifstream input(inputPath, ios_base::binary);

	if (!input || !input.is_open())
		BOOST_THROW_EXCEPTION(runtime_error("Не удалось открыть поток для чтения файла"));

	string output;

	int ret;
	int flush;
	do {
		char buffer[buffer_size];

		input.read(buffer, buffer_size);
		flush = input.eof() ? Z_FINISH : Z_NO_FLUSH;

		stream.avail_in = static_cast<uint32_t>(input.gcount());
		stream.next_in = reinterpret_cast<Bytef*>(buffer);

		do {
			char outBuffer[buffer_size];

			stream.avail_out = buffer_size;
			stream.next_out = reinterpret_cast<Bytef*>(outBuffer);

			ret = inflate(&stream, flush);
			if (ret <= Z_ERRNO)
				BOOST_THROW_EXCEPTION(runtime_error(stream.msg));

			if (output.size() < stream.total_out)
				output.append(buffer, stream.total_out - output.size());
		}
		while (flush != Z_FINISH && ret != Z_STREAM_END);
	}
	while (flush != Z_FINISH && ret != Z_STREAM_END);

	input.close();

	return {output.begin(), output.end()};
}

void BaseFunctions::inflateFromVectorToFile(z_stream& stream, const vector<char>& source, const wstring& destination) {
	const filesystem::path outputPath(destination);
	ofstream output(outputPath, ios_base::binary);

	if (!output || !output.is_open())
		BOOST_THROW_EXCEPTION(runtime_error("Не удалось открыть поток для записи файла"));

	string input(source.begin(), source.end());

	stream.next_in = reinterpret_cast<Bytef*>(input.data());
	stream.avail_in = static_cast<uint32_t>(input.size());

	int ret;
	do {
		char outBuffer[buffer_size];

		stream.avail_out = buffer_size;
		stream.next_out = reinterpret_cast<Bytef*>(outBuffer);

		ret = inflate(&stream, Z_NO_FLUSH);
		if (ret <= Z_ERRNO)
			BOOST_THROW_EXCEPTION(runtime_error(stream.msg));

		output.write(outBuffer, buffer_size - stream.avail_out);
	}
	while (stream.avail_out == 0 && ret != Z_STREAM_END);

	output.close();
}

vector<char> BaseFunctions::inflateFromVectorToVector(z_stream& stream, const vector<char>& source) {
	string input(source.begin(), source.end());
	string output;

	stream.next_in = reinterpret_cast<Bytef*>(input.data());
	stream.avail_in = static_cast<uint32_t>(input.size());

	int ret;
	do {
		char buffer[buffer_size];

		stream.avail_out = buffer_size;
		stream.next_out = reinterpret_cast<Bytef*>(buffer);

		ret = inflate(&stream, Z_NO_FLUSH);
		if (ret <= Z_ERRNO)
			BOOST_THROW_EXCEPTION(runtime_error(stream.msg));

		if (output.size() < stream.total_out)
			output.append(buffer, stream.total_out - output.size());
	}
	while (stream.avail_out == 0 && ret != Z_STREAM_END);

	return {output.begin(), output.end()};
}

#pragma endregion
