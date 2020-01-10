/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximcltmpl.c
 * Copyright (C) 2008-2011 Akira TAGOH
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA  02110-1301  USA
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif /* HAVE_STDINT_H */
#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */
#include <glib/gi18n-lib.h>
#include <gdk/gdkx.h>
#include "gximacc.h"
#include "gximconnection.h"
#include "gximerror.h"
#include "gximmarshal.h"
#include "gximmessages.h"
#include "gximmisc.h"
#include "gximprotocol.h"
#include "gximtransport.h"
#include "gximcltmpl.h"


/**
 * SECTION:gximcltmpl
 * @Title: GXimClientTemplate
 * @Short_Description: Base class for XIM client
 * @See_Also: #GXimCore
 *
 * GXimClientTemplate provides a common facility to deal with XIM protocol
 * events, particularly to be working on XIM client.
 */

enum {
	PROP_0,
	PROP_ATOM_SERVER,
	LAST_PROP
};
enum {
	SIGNAL_0,
	NOTIFY_LOCALES,
	NOTIFY_TRANSPORT,
	XCONNECT,
	LAST_SIGNAL
};


static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_ABSTRACT_TYPE (GXimClientTemplate, g_xim_cl_tmpl, G_TYPE_XIM_CORE);

/*
 * Private functions
 */
static GObject *
g_xim_cl_tmpl_real_constructor(GType                  type,
			       guint                  n_construct_properties,
			       GObjectConstructParam *construct_properties)
{
	GObject *object, *retval = NULL;
	GXimCore *core;
	GXimClientTemplate *client;

	object = G_OBJECT_CLASS (g_xim_cl_tmpl_parent_class)->constructor(type,
									  n_construct_properties,
									  construct_properties);
	if (object) {
		GdkDisplay *dpy;
		GdkWindow *owner, *w;
		gboolean is_valid;

		core = G_XIM_CORE (object);
		client = G_XIM_CL_TMPL (core);

		if (client->atom_server == GDK_NONE) {
			g_xim_messages_critical(core->message,
						"Unable to create a server atom.");
			goto end;
		}

		dpy = g_xim_core_get_display(core);
		owner = g_xim_lookup_xim_server(dpy, client->atom_server, &is_valid);
		if (owner == NULL) {
			gchar *s = gdk_atom_name(client->atom_server);

			g_xim_messages_critical(core->message,
						"Unable to find out the XIM server: %s",
						s);
			g_free(s);
			goto end;
		}
		client->server_window = g_object_ref(owner);

		retval = object;

		w = g_xim_core_get_selection_window(core);
		g_xim_core_watch_event(core, w);
	  end:
		if (retval == NULL)
			g_object_unref(object);
	}

	return retval;
}

static void
g_xim_cl_tmpl_real_set_property(GObject      *object,
				guint         prop_id,
				const GValue *value,
				GParamSpec   *pspec)
{
	GXimClientTemplate *client = G_XIM_CL_TMPL (object);

	switch (prop_id) {
	    case PROP_ATOM_SERVER:
		    client->atom_server = g_value_get_pointer(value);
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_cl_tmpl_real_get_property(GObject    *object,
				guint       prop_id,
				GValue     *value,
				GParamSpec *pspec)
{
	GXimClientTemplate *client = G_XIM_CL_TMPL (object);

	switch (prop_id) {
	    case PROP_ATOM_SERVER:
		    g_value_set_pointer(value, GDK_ATOM_TO_POINTER (client->atom_server));
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_cl_tmpl_real_dispose(GObject *object)
{
	GXimClientTemplate *client = G_XIM_CL_TMPL (object);
	GXimCore *core = G_XIM_CORE (object);
	GXimTransport *trans;
	GdkDisplay *dpy = g_xim_core_get_display(core);
	GdkWindow *w;
	GdkNativeWindow nw;

	if (client->connection) {
		trans = G_XIM_TRANSPORT (client->connection);
		nw = g_xim_transport_get_client_window(trans);
		w = g_xim_get_window(dpy, nw);
		if (w)
			g_xim_core_unwatch_event(core, w);
		g_object_unref(w);
		w = g_xim_transport_get_channel(G_XIM_TRANSPORT (client->connection), NULL);
		if (w)
			g_xim_core_unwatch_event(core, w);
	}
	w = g_xim_core_get_selection_window(core);
	if (w)
		g_xim_core_unwatch_event(core, w);

	if (G_OBJECT_CLASS (g_xim_cl_tmpl_parent_class)->dispose)
		(* G_OBJECT_CLASS (g_xim_cl_tmpl_parent_class)->dispose) (object);
}

static void
g_xim_cl_tmpl_real_finalize(GObject *object)
{
	GXimClientTemplate *client = G_XIM_CL_TMPL (object);

	if (client->server_window)
		g_object_unref(client->server_window);
	if (client->connection)
		g_object_unref(client->connection);

	if (G_OBJECT_CLASS (g_xim_cl_tmpl_parent_class)->finalize)
		(* G_OBJECT_CLASS (g_xim_cl_tmpl_parent_class)->finalize) (object);
}

static GdkFilterReturn
g_xim_cl_tmpl_real_translate_events(GXimCore  *core,
				    GdkXEvent *gdk_xevent,
				    GdkEvent  *event)
{
	GdkFilterReturn retval = GDK_FILTER_CONTINUE;
	XEvent *xev = (XEvent *)gdk_xevent;
	GdkDisplay *dpy = g_xim_core_get_display(core);
	GdkWindow *w;

	switch (xev->type) {
	    case MotionNotify:
	    case LeaveNotify:
	    case FocusIn:
	    case FocusOut:
	    case CreateNotify:
		    /* we don't deal with it */
		    break;
	    case DestroyNotify:
		    w = g_xim_get_window(dpy, xev->xdestroywindow.window);
		    /* this is a bit tricky. window might be already destroyed.
		     * and in this case, g_xim_get_window() will returns NULL.
		     * which means this event won't be useful, valid event to
		     * deliver.
		     */
		    if (w) {
			    event->any.type = GDK_DESTROY;
			    event->any.window = w;
			    event->any.send_event = xev->xdestroywindow.send_event ? TRUE : FALSE;
			    retval = GDK_FILTER_TRANSLATE;
		    }
		    /* XXX: this should be a common part */
		    g_object_unref(G_XIM_CL_TMPL (core)->connection);
		    G_XIM_CL_TMPL (core)->connection = NULL;
#ifdef GNOME_ENABLE_DEBUG
		    g_print("destroyed a client connection %p\n", (gpointer)(gulong)xev->xdestroywindow.window);
#endif
		    break;
	    case UnmapNotify:
	    case MapNotify:
	    case ReparentNotify:
	    case ConfigureNotify:
	    case PropertyNotify:
		    /* we don't deal with it */
		    break;
	    case SelectionClear:
		    event->selection.type = GDK_SELECTION_CLEAR;
		    event->selection.window = g_xim_get_window(dpy, xev->xselectionclear.window);
		    event->selection.send_event = xev->xselectionclear.send_event ? TRUE : FALSE;
		    event->selection.selection = gdk_x11_xatom_to_atom_for_display(dpy, xev->xselectionclear.selection);
		    event->selection.target = GDK_NONE;
		    event->selection.property = GDK_NONE;
		    event->selection.time = xev->xselectionclear.time;
		    event->selection.requestor = None;
		    retval = GDK_FILTER_TRANSLATE;
		    break;
	    case SelectionRequest:
		    event->selection.type = GDK_SELECTION_REQUEST;
		    event->selection.window = g_xim_get_window(dpy, xev->xselectionrequest.owner);
		    event->selection.send_event = xev->xselectionrequest.send_event ? TRUE : FALSE;
		    event->selection.selection = gdk_x11_xatom_to_atom_for_display(dpy, xev->xselectionrequest.selection);
		    event->selection.target = gdk_x11_xatom_to_atom_for_display(dpy, xev->xselectionrequest.target);
		    event->selection.property = gdk_x11_xatom_to_atom_for_display(dpy, xev->xselectionrequest.property);
		    event->selection.time = xev->xselectionrequest.time;
		    event->selection.requestor = xev->xselectionrequest.requestor;
		    retval = GDK_FILTER_TRANSLATE;
		    break;
	    case SelectionNotify:
		    event->selection.type = GDK_SELECTION_NOTIFY;
		    event->selection.window = g_xim_get_window(dpy, xev->xselection.requestor);
		    event->selection.send_event = xev->xselection.send_event ? TRUE : FALSE;
		    event->selection.selection = gdk_x11_xatom_to_atom_for_display(dpy, xev->xselection.selection);
		    event->selection.target = gdk_x11_xatom_to_atom_for_display(dpy, xev->xselection.target);
		    event->selection.property = gdk_x11_xatom_to_atom_for_display(dpy, xev->xselection.property);
		    event->selection.time = xev->xselection.time;
		    event->selection.requestor = xev->xselection.requestor;
		    retval = GDK_FILTER_TRANSLATE;
		    break;
	    case ClientMessage:
		    event->client.type = GDK_CLIENT_EVENT;
		    event->client.window = g_xim_get_window(dpy, xev->xclient.window);
		    event->client.send_event = xev->xclient.send_event ? TRUE : FALSE;
		    event->client.message_type = gdk_x11_xatom_to_atom_for_display(dpy, xev->xclient.message_type);
		    event->client.data_format = xev->xclient.format;
		    memcpy(&event->client.data, &xev->xclient.data, sizeof (event->client.data));
		    retval = GDK_FILTER_TRANSLATE;
		    break;
	    case Expose:
	    case MappingNotify:
	    default:
		    g_xim_messages_warning(core->message,
					   "Unhandled event received: %d",
					   xev->type);
		    break;
	}

	return retval;
}

static gboolean
g_xim_cl_tmpl_real_selection_request_event(GXimCore          *core,
					   GdkEventSelection *event)
{
	gchar *s = gdk_atom_name(event->property);

	g_xim_messages_bug(core->message, "SelectionRequest is unlikely to appear in the client instance: property: %s",
			   s);

	g_free(s);

	return FALSE;
}

static gboolean
g_xim_cl_tmpl_real_selection_clear_event(GXimCore          *core,
					 GdkEventSelection *event)
{
	g_xim_messages_bug(core->message, "SelectionClear is unlikely to appear in the client instance.");

	return FALSE;
}

static gboolean
g_xim_cl_tmpl_real_selection_notify_event(GXimCore          *core,
					  GdkEventSelection *event)
{
	GXimClientTemplate *client = G_XIM_CL_TMPL (core);
	GdkDisplay *dpy = g_xim_core_get_display(core);
	GdkWindow *w = NULL;
	GdkAtom atom_type;
	gint format, bytes;
	gchar *prop = NULL, *p = NULL;
	guint32 error_code;
	gboolean retval = TRUE;

	g_xim_error_push();
	w = g_xim_get_window(dpy, event->requestor);
	gdk_property_get(w, event->property, event->target,
			 0, 8192, FALSE,
			 &atom_type, &format, &bytes,
			 (guchar **)(uintptr_t)&prop);
	if (prop)
		p = g_strndup(prop, bytes);
	error_code = g_xim_error_pop();
	if (!p || error_code != 0) {
		gchar *s = gdk_atom_name(event->property);

		g_xim_messages_critical(core->message,
					"Unable to get a property %s",
					s);
		g_free(s);
		retval = FALSE;
		goto end;
	}
	if (atom_type != event->target ||
	    format != 8) {
		gchar *s1 = gdk_atom_name(event->property);
		gchar *s2 = gdk_atom_name(event->target);
		gchar *s3 = gdk_atom_name(atom_type);

		g_xim_messages_critical(core->message,
					"Atom type mismatches for %s property: expected %s, actual %s",
					s1, s2, s3);
		g_free(s3);
		g_free(s2);
		g_free(s1);
		retval = FALSE;
		goto end;
	}

	if (event->property == core->atom_locales) {
		gchar **strv = NULL;

		g_xim_messages_debug(core->message, "client/event",
				     "%p <- SelectionNotify[%s]",
				     G_XIM_NATIVE_WINDOW_TO_POINTER (event->requestor),
				     p);
		client->is_locale_initialized = GXC_ESTABLISHED;
		if (strncmp(p, "@locale=", 8) == 0)
			strv = g_strsplit(&p[8], ",", 0);
		g_signal_emit(core, signals[NOTIFY_LOCALES], 0, strv);
		g_strfreev(strv);
	} else if (event->property == core->atom_transport) {
		gchar **strv = NULL;

		g_xim_messages_debug(core->message, "client/event",
				     "%p <- SelectionNotify[%s]",
				     G_XIM_NATIVE_WINDOW_TO_POINTER (event->requestor),
				     p);
		client->is_transport_initialized = GXC_ESTABLISHED;
		if (strncmp(p, "@transport=", 11) == 0)
			strv = g_strsplit(&p[11], ",", 0);
		g_signal_emit(core, signals[NOTIFY_TRANSPORT], 0, strv);
		g_strfreev(strv);
	} else {
		gchar *s = gdk_atom_name(event->property);

		g_xim_messages_warning(core->message,
				       "%p <- Unsupported property `%s' notified.",
				       G_XIM_NATIVE_WINDOW_TO_POINTER (event->requestor),
				       s);
		g_free(s);
		goto end;
	}
  end:
	if (w)
		g_object_unref(w);
	g_free(prop);
	g_free(p);

	return retval;
}

static gboolean
g_xim_cl_tmpl_real_expose_event(GXimCore       *core,
				GdkEventExpose *event)
{
	g_xim_messages_warning(core->message, "FIXME!! received a Expose event.");

	return FALSE;
}

static gboolean
g_xim_cl_tmpl_real_destroy_event(GXimCore    *core,
				 GdkEventAny *event)
{
	g_xim_messages_warning(core->message, "FIXME!! received a DestroyNotify event.");

	return FALSE;
}

static gboolean
g_xim_cl_tmpl_real_client_event(GXimCore       *core,
				GdkEventClient *event)
{
	static gboolean is_atom_registered = FALSE;
	GXimClientTemplate *client = G_XIM_CL_TMPL (core);
	gboolean retval = TRUE;

	if (event->message_type == core->atom_xim_xconnect) {
		GXimProtocolIface *iface;
		GdkWindow *w;
		GdkEventMask mask;
		gboolean ret = FALSE;

		g_signal_emit(core, signals[XCONNECT], 0,
			      event, &ret);
		if (!is_atom_registered) {
			iface = G_XIM_PROTOCOL_GET_IFACE (client->connection);
			g_xim_core_add_client_message_filter(core, iface->atom_xim_protocol);
			g_xim_core_add_client_message_filter(core, iface->atom_xim_moredata);
			is_atom_registered = TRUE;
		}
		/* Add a proc to the hash table here,
		 * because comm_window would be setup in
		 * xim_xconnect signal.
		 */
		w = g_xim_transport_get_channel(G_XIM_TRANSPORT (client->connection), NULL);
		mask = gdk_window_get_events(w);
		gdk_window_set_events(w, mask | GDK_STRUCTURE_MASK);
		g_xim_core_watch_event(core, w);
	} else {
		if (client->connection == NULL) {
			if (g_xim_core_lookup_client_message_filter(core, event->message_type)) {
				gchar *s = gdk_atom_name(event->message_type);

				g_xim_messages_warning(core->message,
						       "Event was sent from the unknown client: message_type: %s",
						       s);
				g_free(s);
			}
		} else {
			GError *error = NULL;

			retval = g_xim_protocol_process_event(G_XIM_PROTOCOL (client->connection),
							      event, &error);
			if (error) {
				g_xim_messages_gerror(core->message, error);
				g_error_free(error);
			}
		}
	}

	return retval;
}

static gboolean
g_xim_cl_tmpl_real_xconnect(GXimClientTemplate *client,
			    GdkEventClient     *event)
{
	GXimCore *core = G_XIM_CORE (client);
	GdkNativeWindow nw = event->data.l[0], comm_window;
	GXimTransport *trans;

	trans = G_XIM_TRANSPORT (client->connection);
	g_xim_transport_set_version(trans, event->data.l[1], event->data.l[2]);
	g_xim_transport_set_transport_max(trans, event->data.l[3]);
	g_xim_transport_set_client_window(trans, nw);
	g_xim_core_add_client_message_filter(core,
					     g_xim_transport_get_atom(trans));
	comm_window = g_xim_transport_get_native_channel(trans);
	g_xim_messages_debug(core->message, "client/event",
			     "%p <- XIM_XCONNECT[%p]",
			     G_XIM_NATIVE_WINDOW_TO_POINTER (comm_window),
			     G_XIM_NATIVE_WINDOW_TO_POINTER (nw));
	client->is_connection_initialized = GXC_ESTABLISHED;

	return TRUE;
}

static void
g_xim_cl_tmpl_class_init(GXimClientTemplateClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GXimCoreClass *core_class = G_XIM_CORE_CLASS (klass);

	object_class->constructor  = g_xim_cl_tmpl_real_constructor;
	object_class->set_property = g_xim_cl_tmpl_real_set_property;
	object_class->get_property = g_xim_cl_tmpl_real_get_property;
	object_class->dispose      = g_xim_cl_tmpl_real_dispose;
	object_class->finalize     = g_xim_cl_tmpl_real_finalize;

	core_class->translate_events        = g_xim_cl_tmpl_real_translate_events;
	core_class->selection_request_event = g_xim_cl_tmpl_real_selection_request_event;
	core_class->selection_clear_event   = g_xim_cl_tmpl_real_selection_clear_event;
	core_class->selection_notify_event  = g_xim_cl_tmpl_real_selection_notify_event;
	core_class->expose_event            = g_xim_cl_tmpl_real_expose_event;
	core_class->destroy_event           = g_xim_cl_tmpl_real_destroy_event;
	core_class->client_event            = g_xim_cl_tmpl_real_client_event;

	klass->xconnect = g_xim_cl_tmpl_real_xconnect;

	/* properties */
	/**
	 * GXimClientTemplate:atom-server:
	 *
	 * the #GdkAtom where the client instance should connects to. the atom
	 * name would be something what you specify with XMODIFIERS=@im=.
	 */
	g_object_class_install_property(object_class, PROP_ATOM_SERVER,
					g_param_spec_pointer("atom_server",
							     _("GDK Atom"),
							     _("GDK Atom for XIM server"),
							     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	/* signals */
	/**
	 * GXimClientTemplate::notify-locales:
	 * @cltmpl: the object which received the signal.
	 * @locales: an array of the string contains the locale names with
	 *  %NULL-terminated.
	 *
	 * The ::notify-locales signal will be emitted when the @cltmpl received
	 * a reply of acquiring the supported locales in XIM server.
	 *
	 * This is a convenience signal to deal with
	 * #GXimCore::selection-notify-event for a reply of %LOCALES request.
	 */
	signals[NOTIFY_LOCALES] = g_signal_new("notify_locales",
					       G_OBJECT_CLASS_TYPE (klass),
					       G_SIGNAL_RUN_FIRST,
					       G_STRUCT_OFFSET (GXimClientTemplateClass, notify_locales),
					       NULL, NULL,
					       gxim_marshal_VOID__BOXED,
					       G_TYPE_NONE, 1,
					       G_TYPE_STRV);
	/**
	 * GXimClientTemplate::notify-transport:
	 * @cltmpl: the object which received the signal.
	 * @transport: an array of the string contains the transport-specific
	 *  names with %NULL-terminated.
	 *
	 * The ::notify-transport signal will be emitted when the @cltmpl
	 * received a reply of acquiring the supported transport-specific names
	 * in XIM server.
	 *
	 * This is a convenience signal to deal with
	 * #GXimCore::selection-notify-event for a reply of %TRANSPORT request.
	 */
	signals[NOTIFY_TRANSPORT] = g_signal_new("notify_transport",
						 G_OBJECT_CLASS_TYPE (klass),
						 G_SIGNAL_RUN_FIRST,
						 G_STRUCT_OFFSET (GXimClientTemplateClass, notify_transport),
						 NULL, NULL,
						 gxim_marshal_VOID__BOXED,
						 G_TYPE_NONE, 1,
						 G_TYPE_STRV);
	/**
	 * GXimClientTemplate::xconnect:
	 * @cltmpl: the object which received the signal.
	 * @event: the #GdkEventClient which triggered this signal.
	 *
	 * The ::xconnect signal will be emitted when %XIM_XCONNECT event is
	 * dispatched from the XIM server in order to respond to %XIM_XCONNECT
	 * event from the client.
	 * See The Input Method Protocol, XIM specification document for more
	 * details about %XIM_XCONNECT.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 *  the event. %FALSE to propagate the event further.
	 */
	signals[XCONNECT] = g_signal_new("xconnect",
					 G_OBJECT_CLASS_TYPE (klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET (GXimClientTemplateClass, xconnect),
					 _gxim_acc_signal_accumulator__BOOLEAN, NULL,
					 gxim_marshal_BOOLEAN__BOXED,
					 G_TYPE_BOOLEAN, 1,
					 GDK_TYPE_EVENT);
}

static void
g_xim_cl_tmpl_init(GXimClientTemplate *client)
{
}

/*
 * Public functions
 */
GQuark
g_xim_cl_tmpl_get_error_quark(void)
{
	static GQuark quark = 0;

	if (!quark)
		quark = g_quark_from_static_string("g-xim-cl-tmpl-error");

	return quark;
}

/**
 * g_xim_cl_tmpl_is_initialized:
 * @cltmpl: a #GXimClientTemplate.
 *
 * Checks if an instance of @cltmpl is ready to process XIM protocols.
 *
 * Returns: %TRUE to be initialized the instance properly.
 */
gboolean
g_xim_cl_tmpl_is_initialized(GXimClientTemplate *cltmpl)
{
	g_return_val_if_fail (G_IS_XIM_CL_TMPL (cltmpl), FALSE);

	return cltmpl->is_connection_initialized >= GXC_ESTABLISHED;
}

/**
 * g_xim_cl_tmpl_is_pending_negotiation:
 * @cltmpl: a #GXimClientTemplate.
 *
 * Checks if an instance of @cltmpl is waiting for finish the negotiation.
 *
 * Returns: %TRUE to be pending in any negotiation state.
 */
gboolean
g_xim_cl_tmpl_is_pending_negotiation(GXimClientTemplate *cltmpl)
{
	return cltmpl->is_locale_initialized == GXC_NEGOTIATING ||
		cltmpl->is_transport_initialized == GXC_NEGOTIATING ||
		cltmpl->is_connection_initialized == GXC_NEGOTIATING;
}

/**
 * g_xim_cl_tmpl_start_negotiation:
 * @cltmpl: a #GXimClientTemplate.
 * @error: a location to store error, or %NULL.
 *
 * Starts the transaction to negotiate a connection between XIM server and
 * the client.
 *
 * This is a convenience function to send a request with
 * g_xim_cl_tmpl_send_selection_request().
 *
 * Returns: %TRUE to be sent a request for first negotiation successfully.
 */
gboolean
g_xim_cl_tmpl_start_negotiation(GXimClientTemplate  *cltmpl,
				GError             **error)
{
	g_return_val_if_fail (G_IS_XIM_CL_TMPL (cltmpl), FALSE);

	return g_xim_cl_tmpl_send_selection_request(cltmpl,
						    G_XIM_CORE (cltmpl)->atom_locales,
						    error);
}

/**
 * g_xim_cl_tmpl_send_selection_request:
 * @cltmpl: a #GXimClientTemplate.
 * @atom: a #GdkAtom to determine which request would be sent.
 * @error: a location to store error, or %NULL.
 *
 * Sends a request of @atom to the XIM server with %SelectionRequest event.
 *
 * Returns: %TRUE to be sent a request successfully.
 */
gboolean
g_xim_cl_tmpl_send_selection_request(GXimClientTemplate  *cltmpl,
				     GdkAtom              atom,
				     GError             **error)
{
	GXimCore *core = G_XIM_CORE (cltmpl);
	GdkDisplay *dpy = g_xim_core_get_display(core);
	GdkNativeWindow nw;
	GdkWindow *selection_window;
	gchar *s;

	g_return_val_if_fail (G_IS_XIM_CL_TMPL (cltmpl), FALSE);
	g_return_val_if_fail (error != NULL, FALSE);

	selection_window = g_xim_core_get_selection_window(core);
	nw = GDK_WINDOW_XID (selection_window);
	s = gdk_atom_name(atom);
	g_xim_messages_debug(core->message, "client/event",
			     "%p -> SelectionRequest[%s]",
			     G_XIM_NATIVE_WINDOW_TO_POINTER (nw),
			     s);
	XConvertSelection(GDK_DISPLAY_XDISPLAY (dpy),
			  gdk_x11_atom_to_xatom_for_display(dpy, cltmpl->atom_server),
			  gdk_x11_atom_to_xatom_for_display(dpy, atom),
			  gdk_x11_atom_to_xatom_for_display(dpy, atom),
			  nw,
			  CurrentTime);

	g_free(s);
	if (atom == core->atom_locales) {
		cltmpl->is_locale_initialized = GXC_NEGOTIATING;
	} else if (atom == core->atom_transport) {
		cltmpl->is_transport_initialized = GXC_NEGOTIATING;
	}

	return TRUE;
}

/**
 * g_xim_cl_tmpl_connect_to_server:
 * @cltmpl: a #GXimClientTemplate.
 * @error: a location to store error, or %NULL.
 *
 * Connects to XIM server. you have to call this function before doing something
 * with XIM protocols.
 *
 * Returns: %TRUE to be starting the negotiation to be connected successfully.
 */
gboolean
g_xim_cl_tmpl_connect_to_server(GXimClientTemplate  *cltmpl,
				GError             **error)
{
	GXimCore *core;
	GXimConnection *conn;
	GXimTransport *trans;
	GType gtype;
	GdkEvent *ev;
	GdkDisplay *dpy;
	GdkWindow *w, *cw;
	GdkNativeWindow comm_window, server_window;
	glong transport_major, transport_minor;
	gchar *s;

	g_return_val_if_fail (G_IS_XIM_CL_TMPL (cltmpl), FALSE);
	g_return_val_if_fail (cltmpl->is_connection_initialized < GXC_ESTABLISHED, FALSE);
	g_return_val_if_fail (cltmpl->connection == NULL, FALSE);
	g_return_val_if_fail (error != NULL, FALSE);

	core = G_XIM_CORE (cltmpl);
	gtype = g_xim_core_get_connection_gtype(core);
	if (!g_type_is_a(gtype, G_TYPE_XIM_CONNECTION)) {
		g_set_error(error, G_XIM_CL_TMPL_ERROR,
			    G_XIM_CL_TMPL_ERROR_INVALID_CONNECTION_TYPE | G_XIM_NOTICE_BUG,
			    "connection type isn't inherited from GXimConnection. This is an application bug.");
		return FALSE;
	}
	cltmpl->connection = conn = g_object_new(gtype,
						 "proto_signals", g_xim_core_get_protocol_signal_connector(core),
						 NULL);
	G_XIM_GERROR_CHECK_ALLOC (conn, error,
				  G_XIM_CL_TMPL_ERROR, FALSE);

	g_xim_core_setup_connection(core, conn);
	dpy = g_xim_core_get_display(core);
	w = gdk_screen_get_root_window(gdk_display_get_default_screen(dpy));
	trans = G_XIM_TRANSPORT (conn);
	g_xim_transport_set_display(trans, dpy);
	cw = g_xim_transport_get_channel(trans, w);
	comm_window = g_xim_transport_get_native_channel(trans);
	g_xim_core_watch_event(core, cw);
	transport_major = G_XIM_DEFAULT_TRANSPORT_MAJOR;
	transport_minor = G_XIM_DEFAULT_TRANSPORT_MINOR;
	s = gdk_atom_name(cltmpl->atom_server);
	server_window = g_xim_transport_get_native_channel_from(trans, cltmpl->server_window);
	g_xim_messages_debug(core->message, "client/event",
			     "%p -> XIM_XCONNECT[%s/%p,m:%ld,n:%ld]",
			     G_XIM_NATIVE_WINDOW_TO_POINTER (comm_window),
			     s, G_XIM_NATIVE_WINDOW_TO_POINTER (server_window),
			     transport_major,
			     transport_minor);
	g_free(s);

	ev = gdk_event_new(GDK_CLIENT_EVENT);
	G_XIM_GERROR_CHECK_ALLOC (ev, error,
				  G_XIM_CL_TMPL_ERROR, FALSE);

	ev->client.window = g_object_ref(cltmpl->server_window);
	ev->client.message_type = core->atom_xim_xconnect;
	ev->client.data_format = 32;
	ev->client.data.l[0] = comm_window;
	ev->client.data.l[1] = transport_major;
	ev->client.data.l[2] = transport_minor;

	gdk_event_send_client_message_for_display(dpy, ev, server_window);

	gdk_event_free(ev);
	cltmpl->is_connection_initialized = GXC_NEGOTIATING;

	return TRUE;
}
