/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximerror.c
 * Copyright (C) 2008 Akira TAGOH
 * 
 * Authors:
 *   Akira TAGOH  <akira@tagoh.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <X11/Xlib.h>
#include "gximtypes.h"
#include "gximerror.h"
#include "gximmisc.h"


typedef struct _GXimErrorHandler {
	int (*old_handler) (Display *dpy, XErrorEvent *ev);
	guint32 error_code;
} GXimErrorHandler;

static GQueue *__g_xim_error_queue = NULL;
G_LOCK_DEFINE_STATIC (g_xim_error);

/*
 * Private functions
 */
static int
_g_xim_error_handler(Display     *dpy,
		     XErrorEvent *ev)
{
	gchar buf[64];

	G_LOCK (g_xim_error);

	XGetErrorText(dpy, ev->error_code, buf, 63);
	if (g_queue_is_empty(__g_xim_error_queue)) {
		g_printerr("Unexpected X error received: %s\n"
			   "  Details:\n"
			   "    serial %ld\n"
			   "    error_code %d\n"
			   "    request_code %d\n"
			   "    minor_code %d\n",
			   buf,
			   ev->serial,
			   ev->error_code,
			   ev->request_code,
			   ev->minor_code);
	} else {
		/* We are in the error trap */
		GXimErrorHandler *h = g_queue_peek_tail(__g_xim_error_queue);

		d(g_print("X error received: %s"
			  " (serial: %ld"
			  " error_code: %d"
			  " request_code: %d"
			  " minor_code: %d)\n",
			  buf,
			  ev->serial,
			  ev->error_code,
			  ev->request_code,
			  ev->minor_code));
		h->error_code = G_XIM_ERROR_ERROR_CODE (ev->error_code,
							ev->request_code,
							ev->minor_code);
	}

	G_UNLOCK (g_xim_error);

	return 0;
}

static void
_g_xim_error_init(void)
{
	if (__g_xim_error_queue == NULL)
		__g_xim_error_queue = g_queue_new();
}

/*
 * Public functions
 */
void
g_xim_error_push(void)
{
	GXimErrorHandler *handler;

	G_LOCK (g_xim_error);

	_g_xim_error_init();

	handler = g_new0(GXimErrorHandler, 1);
	G_XIM_CHECK_ALLOC_WITH_NO_RET (handler);

	handler->old_handler = XSetErrorHandler(_g_xim_error_handler);
	g_queue_push_tail(__g_xim_error_queue, handler);

	G_UNLOCK (g_xim_error);
}

guint32
g_xim_error_pop(void)
{
	GXimErrorHandler *handler;
	guint32 retval;

	G_LOCK (g_xim_error);

	if (__g_xim_error_queue == NULL ||
	    g_queue_is_empty(__g_xim_error_queue)) {
		g_warning("No trap handler was pushed.");
		return G_XIM_ERROR_ERROR_CODE (G_XIM_ERR_BadSomething, 0, 0);
	}

	handler = g_queue_pop_tail(__g_xim_error_queue);

	G_UNLOCK (g_xim_error);

	retval = handler->error_code;

	XSetErrorHandler(handler->old_handler);
	g_free(handler);

	return retval;
}
