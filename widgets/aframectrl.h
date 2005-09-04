#ifndef AESKULAP_FRAMECTRL_H
#define AESKULAP_FRAMECTRL_H

#include <gtkmm.h>

namespace Aeskulap {

class Display;

class FrameCtrl {
public:

	virtual void connect(Display* display) = 0;
	
	virtual void disconnect() = 0;
};

} // namespace Aeskulap

#endif // AESKULAP_FRAMECTRL_H
