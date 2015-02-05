//#include <libacpi.h>
#include <gtk/gtk.h>

#include "plugin.h"
#include "panel.h"
#include "draw.h"
#include "dbg.h"

#include "acpinfo.h"
//==================================================================================


#define HEIGHT_BATT  16
#define WIDTH_BATT   6
#define NUM_BATT     3
#define TIMER_VAL    1000

typedef struct Acpi_ {
    Plugin   *plug;

    GtkWidget *main;
    GtkWidget *iwin;
    gboolean iwin_showing;
    gboolean scroll_lock;

    char *action;
    int timer;
    int itest;

    CbRgba *rgb_non,  *rgb_bat, *rgb_ac, *rgb_fun;

    int     state_fun;
    Battery batteries[NUM_BATT];
    Adapter ac;

    int pad_x, pad_y;
    int width, height;	// main size
    int ac_x, ac_y;
    int fun_x, fun_y;
    int n_fun_paint;

} Acpi;
