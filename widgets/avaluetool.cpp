#include "avaluetool.h"

namespace Aeskulap {
	
ValueTool::ValueTool(StudyView& studyview) : FloatWidget(studyview, 100 , 30) {
	m_label = manage(new Gtk::Label);
	add(*m_label);

	m_label->set_text("value");
	m_label->show();
}

void ValueTool::set_value(double value) {
	char buffer[20];
	g_snprintf(buffer, sizeof(buffer), "%.1lf", value);
	m_label->set_text(buffer);
}

} // namespace Aeskulap
