
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

all_returned_types PDFEditor::rewritePDF(const all_input_types& fileNameInput) {
	if (!holds_alternative<wstring>(fileNameInput))
		BOOST_THROW_EXCEPTION(invalid_argument("Тип параметра №1 должен быть 'Строка' (имя файла для открытия)"));

	const auto fileName = get<wstring>(fileNameInput);
	const auto fileNameString = gl_conv_wstring_to_string(fileName);
	const auto fileNameResult = getBaseFilename(fileNameString) + "_upd.pdf";

	PoDoFo::PdfMemDocument doc;

	doc.Load(fileNameString);
	doc.Save(fileNameResult);

	return fileNameResult;
}

#pragma endregion

#pragma region Вспомогательные методы

string PDFEditor::getBaseFilename(const string& filename) {
	const size_t pos = filename.rfind('.');
	if (pos == string::npos)
		return filename;
	if (pos == 0)
		return filename;

	return filename.substr(0, pos);
}

#pragma endregion
