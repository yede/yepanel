#ifndef PANEL_SIGNAL_H_INC
#define PANEL_SIGNAL_H_INC

#include <signal.h>
//==================================================================================

class PanelSignal
{
public:
	static void initSignalHandler();
	static void clearSignalHandler();
};

//==================================================================================
#endif
