
#include "extensions/pdfEditor.hpp"

using namespace std;
using namespace native1C;

#pragma region Методы для переопределения

wstring PDFEditor::getNameExtension() {
	return L"PdfEditor";
}

void PDFEditor::initializeComponent() {
	addMethod(L"ПерезаписатьPDF",
		L"rewritePDF",
		this,
		&PDFEditor::rewritePDF);
}

#pragma endregion

#pragma region Методы компоненты

native1C::all_returned_types PDFEditor::rewritePDF(const all_input_types& fileNameInput) {
	if (!holds_alternative<wstring>(fileNameInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка' (имя файла для открытия)"));

	const auto fileName = get<wstring>(fileNameInput);

	const filesystem::path pathInput(fileName);
	const auto fileNameResult =
			pathInput.parent_path().wstring()
			+ std::filesystem::path::preferred_separator
			+ getBaseFilename(pathInput.filename())
			+ L"_upd.pdf";

	const auto fileNameInputString = gl_conv_wstring_to_string(fileName);
	const auto fileNameOutputString = gl_conv_wstring_to_string(fileNameResult);

	PoDoFo::PdfMemDocument doc;
	
	doc.Load(fileNameInputString);
	doc.Save(fileNameOutputString);

	return fileNameResult;
}

#pragma endregion

#pragma region Вспомогательные методы

wstring PDFEditor::getBaseFilename(const wstring& filename) {
	const size_t pos = filename.rfind(L'.');
	if (pos == wstring::npos || pos == 0)
		return filename;

	return filename.substr(0, pos);
}

#pragma endregion
