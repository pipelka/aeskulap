#ifndef AESKULAP_VALUETOOL_H
#define AESKULAP_VALUETOOL_H

#include "afloatwidget.h"
#include "studyview.h"

namespace Aeskulap {
	
class ValueTool : public FloatWidget {
public:

	ValueTool(StudyView& studyview);

	void set_value(double value);

protected:

	Gtk::Label* m_label;
};

} // namespace Aeskulap

#endif // AESKULAP_VALUETOOL_H
