#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "battery.h"
//==================================================================================

static void acpi_init_colors(Acpi *ai)
{
	GdkColor color;
	gdk_color_parse ("#808080", &color);  ai->rgb_non = CbColor2Rgba (&color);
	gdk_color_parse ("#40e030", &color);  ai->rgb_bat = CbColor2Rgba (&color);
	gdk_color_parse ("#db0000", &color);  ai->rgb_ac  = CbColor2Rgba (&color);
	gdk_color_parse ("#e0e000", &color);  ai->rgb_fun = CbColor2Rgba (&color);
}

static void acpi_free_colors(Acpi *ai)
{
	g_free (ai->rgb_non);
	g_free (ai->rgb_bat);
	g_free (ai->rgb_ac);
	g_free (ai->rgb_fun);
}
// =================================================================================

static void acpi_show_iwin (Acpi *ai)
{
}

static void acpi_create_iwin (Acpi *ai)
{
}
//==================================================================================

static void acpi_paint_battery(cairo_t *cr, Acpi *ai, Battery *bi, int percentage)
{
	double x, y, w, h;
	CbRgba *rgb = (bi->acpi_state == ACPI_STATE_WORKING) ? ai->rgb_bat : ai->rgb_non;

	cairo_set_source_rgb (cr, rgb->red, rgb->green, rgb->blue);
	cairo_set_line_width (cr, 1.0);

	//    cairo_move_to(cr, bi->x + 1 + 0.5, bi->y + 0.5);         // top line
	//    cairo_line_to(cr, bi->x + 3 + 0.5, bi->y + 0.5);
	cairo_rectangle (cr, bi->x + 2.0, bi->y + 0.5, 2.0, 2.0);
	cairo_stroke (cr);
	//    cairo_fill (cr);

	x = bi->x + 0.5;
	y = bi->y + 2.5;
	w = 5;
	h = 12;
	cairo_rectangle (cr, x, y, w, h);
	cairo_stroke (cr);

	if (percentage > 0) {
		int val = 12 * percentage / 100;
		if (percentage < 100 && val > 12) val = 12;
		y = bi->y + 2.5 + (12 - val);
		h = val + 1.0;
		cairo_rectangle (cr, x, y, w, h);
		cairo_fill (cr);
	}
}

static void acpi_paint_ac(cairo_t *cr, Acpi *ai)
{
	double x, y, w, h;
	CbRgba *rgb = //(ai->ac_state == AC_ON_LINE) ? ai->rgb_ac : ai->rgb_non;
				  ai->rgb_ac;
	cairo_set_source_rgb (cr, rgb->red, rgb->green, rgb->blue);
	cairo_set_line_width (cr, 1.5);

	cairo_move_to(cr, ai->ac_x + 0.5, ai->ac_y + 4.5);  // left point
	cairo_line_to(cr, ai->ac_x + 1.5, ai->ac_y + 2.5);
	//    cairo_close_path(cr);
	cairo_stroke (cr);

	cairo_move_to(cr, ai->ac_x + 1.5, ai->ac_y + 2.5);  // left line
	cairo_line_to(cr, ai->ac_x + 2.5, ai->ac_y + 2.5);
	cairo_stroke (cr);

	cairo_move_to(cr, ai->ac_x + 5.5, ai->ac_y + 1.5);  // right line (top)
	cairo_line_to(cr, ai->ac_x + 7.5, ai->ac_y + 1.5);
	cairo_stroke (cr);

	cairo_move_to(cr, ai->ac_x + 5.5, ai->ac_y + 3.5);  // right line (bottom)
	cairo_line_to(cr, ai->ac_x + 7.5, ai->ac_y + 3.5);
	cairo_stroke (cr);

	x = ai->ac_x + 3.5;
	y = ai->ac_y + 0.5;
	w = 3.0;
	h = 5.0;
	cairo_rectangle (cr, x, y, w, h);
	cairo_stroke (cr);
}

static void acpi_paint_fun(cairo_t *cr, Acpi *ai)
{
	if (ai->n_fun_paint < 0) ai->n_fun_paint = 0;
	ai->n_fun_paint ++;
	if (ai->n_fun_paint > 3) ai->n_fun_paint = 0;

	CbRgba *rgb = ai->rgb_fun;
	cairo_set_source_rgb (cr, rgb->red, rgb->green, rgb->blue);
	cairo_set_line_width (cr, 1.0);

	//    if (ai->n_fun_paint == 0) {
	cairo_move_to(cr, ai->fun_x + 0.5, ai->fun_y + 3.5);
	cairo_line_to(cr, ai->fun_x + 3.5, ai->fun_y + 0.5);
	cairo_stroke (cr);

	cairo_move_to(cr, ai->fun_x + 3.5, ai->fun_y + 0.5);
	cairo_line_to(cr, ai->fun_x + 3.5, ai->fun_y + 2.5);
	cairo_stroke (cr);

	cairo_move_to(cr, ai->fun_x + 0.5, ai->fun_y + 3.5);
	cairo_line_to(cr, ai->fun_x + 2.5, ai->fun_y + 3.5);
	cairo_stroke (cr);
	//    }
	//    else if (ai->n_fun_paint == 1) {
	cairo_move_to(cr, ai->fun_x + 6.5, ai->fun_y + 3.5);
	cairo_line_to(cr, ai->fun_x + 3.5, ai->fun_y + 0.5);
	cairo_stroke (cr);

	cairo_move_to(cr, ai->fun_x + 3.5, ai->fun_y + 0.5);
	cairo_line_to(cr, ai->fun_x + 3.5, ai->fun_y + 2.5);
	cairo_stroke (cr);

	cairo_move_to(cr, ai->fun_x + 4.5, ai->fun_y + 3.5);
	cairo_line_to(cr, ai->fun_x + 6.5, ai->fun_y + 3.5);
	cairo_stroke (cr);
	//    }
	//    else if (ai->n_fun_paint == 2) {
	cairo_move_to(cr, ai->fun_x + 6.5, ai->fun_y + 3.5);
	cairo_line_to(cr, ai->fun_x + 3.5, ai->fun_y + 6.5);
	cairo_stroke (cr);

	cairo_move_to(cr, ai->fun_x + 3.5, ai->fun_y + 6.5);
	cairo_line_to(cr, ai->fun_x + 3.5, ai->fun_y + 4.5);
	cairo_stroke (cr);

	cairo_move_to(cr, ai->fun_x + 6.5, ai->fun_y + 3.5);
	cairo_line_to(cr, ai->fun_x + 4.5, ai->fun_y + 3.5);
	cairo_stroke (cr);
	//    }
	//    else {
	cairo_move_to(cr, ai->fun_x + 0.5, ai->fun_y + 3.5);
	cairo_line_to(cr, ai->fun_x + 3.5, ai->fun_y + 6.5);
	cairo_stroke (cr);

	cairo_move_to(cr, ai->fun_x + 3.5, ai->fun_y + 4.5);
	cairo_line_to(cr, ai->fun_x + 3.5, ai->fun_y + 6.5);
	cairo_stroke (cr);

	cairo_move_to(cr, ai->fun_x + 0.5, ai->fun_y + 3.5);
	cairo_line_to(cr, ai->fun_x + 2.5, ai->fun_y + 3.5);
	cairo_stroke (cr);
	//    }
}

static void acpi_paint_ui(Acpi *ai, cairo_t *cr)
{
	int i, percentage;

	for (i = 0; i < NUM_BATT; i++) {
		Battery  *bi = &ai->batteries[i];
		if (bi->acpi_state != ACPI_STATE_WORKING) continue;

		if (bi->charge_state == ACPI_CHARGING) {
			if (bi->n_paint < 0) bi->n_paint = 0;
			bi->n_paint ++;
			if (bi->n_paint > 5) bi->n_paint = 1;

			if      (bi->n_paint == 1) percentage = 20;
			else if (bi->n_paint == 2) percentage = 40;
			else if (bi->n_paint == 3) percentage = 60;
			else if (bi->n_paint == 4) percentage = 80;
			else                       percentage = 100;
		}
		else {/*
			if      (bi->percentage < 1)  percentage = 0;
			else if (bi->percentage < 35) percentage = 20;
			else if (bi->percentage < 55) percentage = 40;
			else if (bi->percentage < 75) percentage = 60;
			else if (bi->percentage < 95) percentage = 80;
			else                          percentage = 100;*/
			percentage = bi->percentage;
		}
		acpi_paint_battery(cr, ai, bi, percentage);
		break;
	}

	acpi_paint_ac(cr, ai);
	acpi_paint_fun(cr, ai);
}
//==================================================================================

static void acpi_init_devices(Acpi *ai)
{
	char curdir[1024];
	if (getcwd(curdir, 1024) == NULL) curdir[0] = '\0';

	acpi_battery_init(ai->batteries, NUM_BATT);
	acpi_ac_adapter_init(&ai->ac);

	if (curdir[0] != '\0') chdir(curdir);
}

static void acpi_free_devices(Acpi *ai)
{
	(void) ai;
}
//==================================================================================

static int acpi_read_status(Acpi *ai)
{
	acpi_ac_adapter_read(&ai->ac);

	static char tips[2048];
	int i, n = 0;

	for (i = 0; i < NUM_BATT; i++) {
		Battery *bi = &ai->batteries[i];
		if (bi->acpi_state != ACPI_STATE_WORKING) continue;

		acpi_battery_read(bi);
		if (ai->main) {
			snprintf (tips, 2048,
					  "%s: %d%% (state: %s)\n"
					  "  %d %s - design_capacity\n"
					  "  %d %s - last_capacity\n"
					  "  %d %s - remaining_capacity\n"
					  "  %d - present_rate (%s)\n"
					  "  %d - hours\n"
					  "  %d - minutes\n"
					  "  %d - seconds\n"
					  "%s:\n"
					  "  online=%d, type_ac=%d"
					  ,
					  bi->acpi_name, bi->percentage, bi->state_text,
					  bi->design_capacity, bi->capacity_unit,
					  bi->last_capacity, bi->capacity_unit,
					  bi->remaining_capacity, bi->capacity_unit,
					  bi->present_rate, bi->poststr,
					  bi->hours, bi->minutes, bi->seconds,
					  ai->ac.acpi_name, ai->ac.online, ai->ac.type_ac);
			gtk_widget_set_tooltip_text(ai->main, tips);
		}
		bi->x = 1 + n * WIDTH_BATT + ai->pad_x;
		bi->y = 1 + (ai->height - HEIGHT_BATT) / 2;
		ai->ac_x = bi->x + 6;
		ai->ac_y = bi->y + 9;
		ai->fun_x = ai->ac_x + 1;
		ai->fun_y = bi->y;
		n++;
	}

	if (n == 0) {
		Battery *bi = &ai->batteries[0];
		bi->x = 1 + n * WIDTH_BATT + ai->pad_x;
		bi->y = 1 + (ai->height - HEIGHT_BATT) / 2;
		ai->ac_x = bi->x + 6;
		ai->ac_y = bi->y + 9;
		ai->fun_x = ai->ac_x + 1;
		ai->fun_y = bi->y;
	}

	return ACPI_SUCCESS;
}
// =================================================================================

static gint acpi_on_timer (Acpi *ai)
{
	ENTER;

	g_assert(ai != NULL);

	if (acpi_read_status(ai) == ACPI_SUCCESS)
		gtk_widget_queue_draw (ai->main);

	RET(TRUE);	// return TRUE: continue timer
}

//==================================================================================
// event handle
//==================================================================================

static gboolean acpi_on_scroll (GtkWidget *widget, GdkEventScroll *event, Acpi *ai)
{
	(void) widget;

	ai->scroll_lock = TRUE;

	switch (event->direction) {
		case GDK_SCROLL_UP:
			break;
		case GDK_SCROLL_DOWN:
			break;
		case GDK_SCROLL_LEFT:
		case GDK_SCROLL_RIGHT:
			break;
		default:
			break;
	}

	ai->scroll_lock = FALSE;
	return TRUE;
}

static gboolean acpi_on_clicked(GtkWidget *widget, GdkEventButton *event, Acpi *ai)
{
	(void) widget;

	//ENTER2;
	//DBG("%s\n", ai->action);
	switch (event->button) {
		case 1:
			if (ai->iwin) {
				gtk_widget_hide(ai->iwin);
				ai->iwin_showing = FALSE;
			}
			if (ai->action) {
				//system (ai->action);
				g_spawn_command_line_async (ai->action, NULL);
			}
			break;

		case 2:
			break;

		case 3: break;
			//case 4: adjVolume (ai, 5);  break;
			//case 5: adjVolume (ai, -5);  break;
	}
	return TRUE;
}

static gboolean acpi_on_leave_notify(GtkWidget *widget, GdkEventCrossing *event, Acpi *ai)
{
	(void) widget;
	(void) event;

	if (ai->iwin) {
		gtk_widget_hide(ai->iwin);
		ai->iwin_showing = FALSE;
	}
	return TRUE;
}

static gboolean acpi_on_enter_notify(GtkWidget *widget, GdkEventCrossing *event, Acpi *ai)
{
	(void) widget;
	(void) event;

	if (!ai->iwin) acpi_create_iwin (ai);
	acpi_show_iwin(ai);
	//    gtk_widget_show_all(ai->iwin);
	//    ai->iwin_showing = TRUE;
	return TRUE;
}

static gint acpi_on_expose (GtkWidget *widget, GdkEventExpose *event, Acpi *ai)
{
	(void) event;

	cairo_t *cr = gdk_cairo_create(widget->window);
	acpi_paint_ui(ai, cr);
	cairo_destroy(cr);

	return TRUE;
}
// =================================================================================

static void acpi_resize_ui (Acpi *ai)
{
	ai->pad_x = 1;
	ai->pad_y = 1;
	ai->width = 16 + ai->pad_x + ai->pad_x;
	ai->height = ai->plug->panel->ah;
//	g_print("ai: (width=%d, height=%d)\n", ai->width, ai->height);
	if (ai->main) gtk_widget_set_usize (ai->main, ai->width, ai->height);
}

static void acpi_create_ui (Acpi *ai)
{
	ai->main = gtk_event_box_new();
	if (ai->plug->panel->transparent) {
		gtk_event_box_set_visible_window (GTK_EVENT_BOX (ai->main), FALSE);
	}
	acpi_resize_ui (ai);

	gtk_widget_add_events (ai->main, GDK_BUTTON_PRESS_MASK |GDK_BUTTON_RELEASE_MASK |GDK_EXPOSURE_MASK);
	g_signal_connect(G_OBJECT(ai->main), "button_press_event", G_CALLBACK(acpi_on_clicked), (gpointer) ai);
	g_signal_connect(G_OBJECT(ai->main), "scroll-event", G_CALLBACK(acpi_on_scroll), (gpointer) ai);
	g_signal_connect(G_OBJECT(ai->main), "expose_event", G_CALLBACK(acpi_on_expose), (gpointer) ai);

	g_signal_connect(G_OBJECT(ai->main), "enter_notify_event", G_CALLBACK(acpi_on_enter_notify), (gpointer) ai);
	g_signal_connect(G_OBJECT(ai->main), "leave_notify_event", G_CALLBACK(acpi_on_leave_notify), (gpointer) ai);


	gtk_container_add(GTK_CONTAINER(ai->plug->pwid), ai->main);
	gtk_widget_show_all(ai->main);
}

//==================================================================================
// 
//==================================================================================

static int acpi_constructor(Plugin *p)
{
	ENTER;

	Acpi *ai = g_new0(Acpi, 1);
	g_return_val_if_fail(ai != NULL, 0);

	p->priv = ai;
	ai->plug = p;

	acpi_init_devices(ai);
	acpi_resize_ui (ai);
	acpi_read_status(ai);
	acpi_init_colors(ai);
	acpi_create_ui (ai);
	ai->timer = g_timeout_add(TIMER_VAL, (GSourceFunc) acpi_on_timer, (gpointer) ai);

	RET(1);
}

static void acpi_destructor(Plugin *p)
{
	Acpi *ai = (Acpi *) p->priv;
	if (ai->timer) g_source_remove(ai->timer);
	if (ai->iwin) gtk_widget_destroy (ai->iwin);
	if (ai->main) gtk_widget_destroy (ai->main);
	if (ai->action) g_free (ai->action);
	acpi_free_devices(ai);
	acpi_free_colors(ai);
	g_free(ai);
}
//==================================================================================

PluginClass pluginClass_battery = {
	fname: NULL,
	count: 0,
	type : "battery",
	name : "Acpi Status Monitor",
	version: "1.0",
	description : "Acpi status monitor with tooltip",
	constructor : acpi_constructor,
	destructor  : acpi_destructor,
};
