#include <sys/wait.h>
#include <stddef.h>
#include <QDebug>

#include "panelsignal.h"
#include "panelapplication.h"
//==================================================================================

void setXErrorHandler(bool isUnset);

// generic signal handler - this sets a bit in m_signals, which
// will be handled later by the event loop (ie. if signal 2 is caught,
// bit 2 is set)
void signalHandler(int sig)
{
//	sig_atomic_t &ss = sigs();
//	ss |= (1 << sig);

	switch (sig) {
		case SIGINT:  // qDebug() << ":SIGINT";				// ctrl-c
		case SIGQUIT: // qDebug() << ":SIGQUIT"; break;
		case SIGTERM: // qDebug() << ":SIGTERM"; break;		// kill
			qDebug() << "\n";
			QApplication::instance()->quit();
			break;

		case SIGHUP:  // qDebug() << ":SIGHUP";  break;
		case SIGPIPE: // qDebug() << ":SIGPIPE"; break;
		case SIGUSR1: // qDebug() << ":SIGUSR1"; break;
		case SIGUSR2: // qDebug() << ":SIGUSR2";
			break;

		case SIGCHLD: // qDebug() << ":SIGCHLD"; break;
		//	int unused;
		//	while (waitpid(-1, &unused, WNOHANG | WUNTRACED) > 0) {}
			break;

		default:      // qDebug() << ":SIG ???"; break;
			// generate a core dump for unknown signals
			break;
	}
}

void updateSignalHandler(bool isUnset)
{
	struct sigaction action;
	action.sa_handler = isUnset ? SIG_DFL : signalHandler;
	action.sa_mask = sigset_t();
	action.sa_flags = SA_NOCLDSTOP;

	// non-fatal signals
	sigaction(SIGHUP,  &action, NULL);
	sigaction(SIGINT,  &action, NULL);
	sigaction(SIGQUIT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGPIPE, &action, NULL);
	sigaction(SIGCHLD, &action, NULL);
	sigaction(SIGUSR1, &action, NULL);
	sigaction(SIGUSR2, &action, NULL);

	setXErrorHandler(isUnset);
}

void PanelSignal::initSignalHandler()
{
	updateSignalHandler(false);
}

void PanelSignal::clearSignalHandler()
{
	updateSignalHandler(true);
}
//==================================================================================

#include <X11/Xlib.h>

int xerrorHandler(Display *d, XErrorEvent *e)
{
#if 0  // DEBUG
	char errtxt[128];

	XGetErrorText(d, e->error_code, errtxt, 128);
	fprintf(stderr, "%s:  X error: %s(%d) opcodes %d/%d\n  resource 0x%lx\n",
		base_app->applicationName().c_str(), errtxt, e->error_code,
		e->request_code, e->minor_code, e->resourceid);
#else
	// shutup gcc
	(void) d;
	(void) e;
#endif // DEBUG

	return 0;
}

static XErrorHandler g_prevErrorHandler = NULL;

void setXErrorHandler(bool isUnset)
{
	if (isUnset) {
		XSetErrorHandler(g_prevErrorHandler);
	} else {
		g_prevErrorHandler = XSetErrorHandler(xerrorHandler);
	}
}
//==================================================================================
