
#ifndef __EXT_PDF_EDITOR_HPP_
#define __EXT_PDF_EDITOR_HPP_

#include "baseClass/baseClassForExtension.hpp"

#include <podofo/main/PdfMemDocument.h>

class PDFEditor final : public native1C::IBaseExtensionClass {
public:
	BOOST_DEFAULTED_FUNCTION(PDFEditor(), { })
	BOOST_DEFAULTED_FUNCTION(~PDFEditor() BOOST_OVERRIDE, { })
	BOOST_DELETED_FUNCTION(PDFEditor(PDFEditor&&) BOOST_NOEXCEPT)
	BOOST_DELETED_FUNCTION(PDFEditor(const PDFEditor&))
	BOOST_DELETED_FUNCTION(PDFEditor& operator=(const PDFEditor&))
	BOOST_DELETED_FUNCTION(PDFEditor& operator=(PDFEditor&&))


protected:
	virtual std::wstring getNameExtension() BOOST_OVERRIDE;
	virtual void initializeComponent() BOOST_OVERRIDE;


private:
	native1C::all_returned_types rewritePDF(const native1C::all_input_types&); // ѕерезапись PDF с новым форматом

	static std::string getBaseFilename(const std::string&);
};

#endif
