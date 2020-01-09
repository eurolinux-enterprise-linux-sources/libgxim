/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximcore.c
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

#include <glib/gi18n-lib.h>
#ifdef GNOME_ENABLE_DEBUG
#include <gdk/gdkx.h>
#endif /* GNOME_ENABLE_DEBUG */
#include "gximacc.h"
#include "gximconnection.h"
#include "gximmarshal.h"
#include "gximmessage.h"
#include "gximmisc.h"
#include "gximprotocol.h"
#include "gximtransport.h"
#include "gximcore.h"

#define G_XIM_CORE_GET_PRIVATE(_o_)	(G_TYPE_INSTANCE_GET_PRIVATE ((_o_), G_TYPE_XIM_CORE, GXimCorePrivate))


/**
 * SECTION:gximcore
 * @Short_Description: Base class for XIM protocol event handling
 * @Title: GXimCore
 *
 * GXimCore provides a common functionality to deal with XIM protocol
 * for both server side and client side.  You usually don't need to use
 * most functions described here because any chance to do something with
 * them is well hidden by the higher-level APIs, except if you want to
 * make any classes that inherited from GXimCore, or to do any particular
 * thing with them.
 */

enum {
	PROP_0,
	PROP_DISPLAY,
	PROP_SELECTION_WINDOW,
	PROP_CONN_GTYPE,
	PROP_PROTO_SIGNALS,
	LAST_PROP
};
enum {
	SIGNAL_0,
	SELECTION_REQUEST_EVENT,
	SELECTION_CLEAR_EVENT,
	SELECTION_NOTIFY_EVENT,
	EXPOSE_EVENT,
	DESTROY_EVENT,
	CLIENT_EVENT,
	LAST_SIGNAL
};

typedef struct _GXimCorePrivate {
	GHashTable              *atom_table;
	GSList                  *proto_signals;
	GdkDisplay              *dpy;
	GdkWindow               *selection_window;
	GType                    conn_gtype;
#ifdef GNOME_ENABLE_DEBUG
	/* for tracking the event watcher */
	GHashTable              *watch_table;
#endif
} GXimCorePrivate;


static guint signals[LAST_SIGNAL] = { 0 };


G_DEFINE_ABSTRACT_TYPE (GXimCore, g_xim_core, G_TYPE_OBJECT);

/*
 * Private functions
 */
static void
g_xim_core_atom_init(GXimCore *core)
{
	GXimCorePrivate *priv = G_XIM_CORE_GET_PRIVATE (core);
	static gchar *xim_atoms[] = {
		"_XIM_XCONNECT",
		"_XIM_PROTOCOL",
		"_XIM_MOREDATA",
		"XIM_SERVERS",
		"LOCALES",
		"TRANSPORT",
		NULL
	};
	gint i;

	/* Ensure the above atoms exists in X Atom as well.
	 * otherwise something related to them may fails in clients.
	 */
	for (i = 0; xim_atoms[i] != NULL; i++) {
		GdkAtom atom = gdk_atom_intern_static_string(xim_atoms[i]);

		gdk_x11_atom_to_xatom_for_display(priv->dpy, atom);
	}

	core->atom_xim_xconnect = gdk_atom_intern_static_string("_XIM_XCONNECT");
	core->atom_xim_servers  = gdk_atom_intern_static_string("XIM_SERVERS");
	core->atom_locales      = gdk_atom_intern_static_string("LOCALES");
	core->atom_transport    = gdk_atom_intern_static_string("TRANSPORT");

	/* XXX: is this really needed? */
	gdk_flush();
}

static GdkFilterReturn
g_xim_core_real_translate_events(GXimCore  *core,
				 GdkXEvent *gdk_xevent,
				 GdkEvent  *event)
{
	g_xim_message_bug(core->message,
			  "Unable to translate events: No implementation for %s",
			  g_type_name(G_TYPE_FROM_INSTANCE (core)));

	return GDK_FILTER_CONTINUE;
}

static void
g_xim_core_real_setup_connection(GXimCore       *core,
				 GXimConnection *conn)
{
	g_xim_message_bug(core->message,
			  "Unable to setup connection: No implementation for %s",
			  g_type_name(G_TYPE_FROM_INSTANCE (core)));
}

static GdkFilterReturn
g_xim_core_dispatch_events(GdkXEvent *gdk_xevent,
			   GdkEvent  *event,
			   gpointer   data)
{
	GXimCore *core = G_XIM_CORE (data);
	GdkFilterReturn retval;

	retval = (* G_XIM_CORE_GET_CLASS (core)->translate_events) (core,
								    gdk_xevent,
								    event);

	if (retval == GDK_FILTER_TRANSLATE) {
		gboolean ret = FALSE;

		switch (event->type) {
		    case GDK_SELECTION_REQUEST:
			    g_signal_emit(core, signals[SELECTION_REQUEST_EVENT], 0,
					  &event->selection, &ret);
			    break;
		    case GDK_SELECTION_CLEAR:
			    g_signal_emit(core, signals[SELECTION_CLEAR_EVENT], 0,
					  &event->selection, &ret);
			    break;
		    case GDK_SELECTION_NOTIFY:
			    g_signal_emit(core, signals[SELECTION_NOTIFY_EVENT], 0,
					  &event->selection, &ret);
			    break;
		    case GDK_EXPOSE:
			    g_signal_emit(core, signals[EXPOSE_EVENT], 0,
					  &event->expose, &ret);
			    break;
		    case GDK_DESTROY:
			    g_signal_emit(core, signals[DESTROY_EVENT], 0,
					  &event->any, &ret);
			    break;
		    case GDK_CLIENT_EVENT:
			    g_signal_emit(core, signals[CLIENT_EVENT], 0,
					  &event->client, &ret);
			    break;
		    default:
			    g_xim_message_warning(core->message,
						  "Unhandled translated event received: %d", event->type);
			    break;
		}
		if (ret)
			retval = GDK_FILTER_REMOVE;
		else
			retval = GDK_FILTER_CONTINUE;
	}

	return retval;
}

static GObject *
g_xim_core_real_constructor(GType                  type,
			    guint                  n_construct_properties,
			    GObjectConstructParam *construct_properties)
{
	GObject *object, *retval = NULL;
	GXimCorePrivate *priv;
	GXimCore *core;

	object = (* G_OBJECT_CLASS (g_xim_core_parent_class)->constructor) (type,
									    n_construct_properties,
									    construct_properties);

	if (object) {
		GdkWindowAttr attrs = {
			.title = NULL,
			.event_mask = GDK_STRUCTURE_MASK | GDK_PROPERTY_CHANGE_MASK,
			.x = 0,
			.y = 0,
			.width = 1,
			.height = 1,
			.wclass = GDK_INPUT_ONLY,
			.visual = NULL,
			.colormap = NULL,
			.window_type = GDK_WINDOW_TOPLEVEL,
			.cursor = NULL,
			.wmclass_name = NULL,
			.wmclass_class = NULL,
			.override_redirect = TRUE,
			.type_hint = GDK_WINDOW_TYPE_HINT_NORMAL,
		};
		GdkWindow *root_window;

		core = G_XIM_CORE (object);
		priv = G_XIM_CORE_GET_PRIVATE (core);
		if (priv->dpy == NULL) {
			g_xim_message_critical(core->message,
					       "Property \"display\" has to be given");
			goto end;
		}
		root_window = gdk_screen_get_root_window(gdk_display_get_default_screen(priv->dpy));
		priv->selection_window = gdk_window_new(root_window,
							&attrs,
							GDK_WA_X | GDK_WA_Y | GDK_WA_NOREDIR | GDK_WA_TYPE_HINT);
		if (priv->selection_window == NULL) {
			g_xim_message_critical(core->message,
					       "Unable to create a selection window");
			goto end;
		}
		retval = object;
	  end:
		if (retval == NULL)
			g_object_unref(object);
	}

	return retval;
}

static void
g_xim_core_real_set_property(GObject      *object,
			     guint         prop_id,
			     const GValue *value,
			     GParamSpec   *pspec)
{
	GXimCore *core = G_XIM_CORE (object);
	GXimCorePrivate *priv = G_XIM_CORE_GET_PRIVATE (core);
	GXimLazySignalConnector *sigs, *s;
	gsize i;

	switch (prop_id) {
	    case PROP_DISPLAY:
		    priv->dpy = GDK_DISPLAY_OBJECT (g_value_get_object(value));
		    g_xim_core_atom_init(core);
		    break;
	    case PROP_CONN_GTYPE:
		    priv->conn_gtype = g_value_get_gtype(value);
		    break;
	    case PROP_PROTO_SIGNALS:
		    if (priv->proto_signals != NULL) {
			    g_xim_message_warning(G_XIM_CORE (object)->message,
						  "Unable to update the signal list.");
			    break;
		    }
		    sigs = g_value_get_pointer(value);
		    if (sigs) {
			    for (i = 0; sigs[i].signal_name != NULL; i++) {
				    s = g_new0(GXimLazySignalConnector, 1);
				    G_XIM_CHECK_ALLOC_WITH_NO_RET (s);

				    s->signal_name = g_strdup(sigs[i].signal_name);
				    s->function = sigs[i].function;
				    s->data = sigs[i].data;
				    priv->proto_signals = g_slist_append(priv->proto_signals, s);
			    }
		    }
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_core_real_get_property(GObject    *object,
			     guint       prop_id,
			     GValue     *value,
			     GParamSpec *pspec)
{
	GXimCorePrivate *priv = G_XIM_CORE_GET_PRIVATE (object);

	switch (prop_id) {
	    case PROP_DISPLAY:
		    g_value_set_object(value, priv->dpy);
		    break;
	    case PROP_SELECTION_WINDOW:
		    g_value_set_object(value, priv->selection_window);
		    break;
	    case PROP_CONN_GTYPE:
		    g_value_set_gtype(value, priv->conn_gtype);
		    break;
	    case PROP_PROTO_SIGNALS:
		    g_value_set_pointer(value, priv->proto_signals);
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_core_real_finalize(GObject *object)
{
	GXimCore *core = G_XIM_CORE (object);
	GXimCorePrivate *priv = G_XIM_CORE_GET_PRIVATE (object);
	GSList *l;

	for (l = priv->proto_signals; l != NULL; l = g_slist_next(l)) {
		GXimLazySignalConnector *s = l->data;

		g_free(s->signal_name);
		g_free(s);
	}
	g_slist_free(priv->proto_signals);
	gdk_window_destroy(priv->selection_window);
	g_hash_table_destroy(priv->atom_table);
#ifdef GNOME_ENABLE_DEBUG
	if (g_hash_table_size(priv->watch_table) > 0) {
		GHashTableIter iter;
		gpointer key, val;

		g_print("\n*** %s: %d watchers are still reachable for the event.\n",
			g_type_name(G_TYPE_FROM_INSTANCE (object)),
			g_hash_table_size(priv->watch_table));
		g_hash_table_iter_init(&iter, priv->watch_table);
		while (g_hash_table_iter_next(&iter, &key, &val)) {
			g_print("  window: %p\n", key);
		}
	} else {
		g_print("\n=== %s: No watchers are reachable.\n",
			g_type_name(G_TYPE_FROM_INSTANCE (object)));
	}
	g_hash_table_destroy(priv->watch_table);
#endif
	g_object_unref(core->message);

	if (G_OBJECT_CLASS (g_xim_core_parent_class)->finalize)
		(* G_OBJECT_CLASS (g_xim_core_parent_class)->finalize) (object);
}

static void
g_xim_core_class_init(GXimCoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private(klass, sizeof (GXimCorePrivate));

	object_class->constructor  = g_xim_core_real_constructor;
	object_class->set_property = g_xim_core_real_set_property;
	object_class->get_property = g_xim_core_real_get_property;
	object_class->finalize     = g_xim_core_real_finalize;

	klass->translate_events    = g_xim_core_real_translate_events;
	klass->setup_connection    = g_xim_core_real_setup_connection;

	/* properties */
	g_object_class_install_property(object_class, PROP_DISPLAY,
					g_param_spec_object("display",
							    _("GdkDisplay"),
							    _("Gdk display to use"),
							    GDK_TYPE_DISPLAY,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * GXimCore:selection-window:
	 *
	 * a #GdkWindow to communicate with the XIM server or the client
	 * application.
	 */
	g_object_class_install_property(object_class, PROP_SELECTION_WINDOW,
					g_param_spec_object("selection_window",
							    _("Selection Window"),
							    _("A GdkWindow for selection"),
							    GDK_TYPE_WINDOW,
							    G_PARAM_READABLE));
	/**
	 * GXimCore:connection-gtype:
	 *
	 * a #GType inherited from #GXimConnection. this will be a instance and
	 * determines the connection type when the connection is requested.
	 */
	g_object_class_install_property(object_class, PROP_CONN_GTYPE,
					g_param_spec_gtype("connection_gtype",
							   _("GType of Connection class"),
							   _("A GType that deal with XIM connection"),
							   G_TYPE_XIM_CONNECTION,
							   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * GXimCore:proto-signals:
	 *
	 * a set of #GXimLazySignalConnector to connect signals to a instance
	 * of #GXimCore:connection-gtype when the connection is negothating.
	 */
	g_object_class_install_property(object_class, PROP_PROTO_SIGNALS,
					g_param_spec_pointer("proto_signals",
							     _("Signals for Protocol class"),
							     _("A structure of signals for Protocol class"),
							     G_PARAM_READWRITE));

	/* signals */
	/**
	 * GXimCore::selection-request-event:
	 * @core: the object which received the signal.
	 * @event: the #GdkEventSelection which triggered this signal.
	 *
	 * The ::selection-request-event signal will be emitted when the client
	 * applications requests a set of names of the supported locales and
	 * the preregistered formats for transport-specific names. this signal
	 * won't appears in the client instance, such as inherited from
	 * #GXimClientTemplate.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 * the event. %FALSE to propagate the event further.
	 */
	signals[SELECTION_REQUEST_EVENT] = g_signal_new("selection_request_event",
							G_OBJECT_CLASS_TYPE (klass),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET (GXimCoreClass, selection_request_event),
							_gxim_acc_signal_accumulator__BOOLEAN,
							NULL,
							gxim_marshal_BOOLEAN__BOXED,
							G_TYPE_BOOLEAN, 1,
							GDK_TYPE_EVENT);
	/**
	 * GXimCore::selection-clear-event:
	 * @core: the object which received the signal.
	 * @event: the #GdkEventSelection which triggered this signal.
	 *
	 * The ::selection-clear-event signal will be emitted when the
	 * @core's window has lost ownership of a selection. this signal
	 * won't appears in the client instance, such as inherited from
	 * #GXimClientTemplate.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 * the event. %FALSE to propagate the event further.
	 */
	signals[SELECTION_CLEAR_EVENT] = g_signal_new("selection_clear_event",
						      G_OBJECT_CLASS_TYPE (klass),
						      G_SIGNAL_RUN_LAST,
						      G_STRUCT_OFFSET (GXimCoreClass, selection_clear_event),
						      _gxim_acc_signal_accumulator__BOOLEAN,
						      NULL,
						      gxim_marshal_BOOLEAN__BOXED,
						      G_TYPE_BOOLEAN, 1,
						      GDK_TYPE_EVENT);
	/**
	 * GXimCore::selection-notify-event:
	 * @core: the object which received the signal.
	 * @event: the #GdkEventSelection which triggered this signal.
	 *
	 * The ::selection-notify-event signal will be emitted when the
	 * the XIM server responded requests to get the supported locales
	 * and the transport-specific names. this signal won't appears in
	 * the server instance, such as inherited from #GXimServerTemplate.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 * the event. %FALSE to propagate the event further.
	 */
	signals[SELECTION_NOTIFY_EVENT] = g_signal_new("selection_notify_event",
						       G_OBJECT_CLASS_TYPE (klass),
						       G_SIGNAL_RUN_LAST,
						       G_STRUCT_OFFSET (GXimCoreClass, selection_notify_event),
						       _gxim_acc_signal_accumulator__BOOLEAN,
						       NULL,
						       gxim_marshal_BOOLEAN__BOXED,
						       G_TYPE_BOOLEAN, 1,
						       GDK_TYPE_EVENT);
	signals[EXPOSE_EVENT] = g_signal_new("expose_event",
					     G_OBJECT_CLASS_TYPE (klass),
					     G_SIGNAL_RUN_LAST,
					     G_STRUCT_OFFSET (GXimCoreClass, expose_event),
					     _gxim_acc_signal_accumulator__BOOLEAN,
					     NULL,
					     gxim_marshal_BOOLEAN__BOXED,
					     G_TYPE_BOOLEAN, 1,
					     GDK_TYPE_EVENT);
	/**
	 * GXimCore::destroy-event:
	 * @core: the object which received the signal.
	 * @event: the #GdkEvent which triggered this signal.
	 *
	 * The ::destroy-event signal will be emitted when a #GdkWindow
	 * is destroyed. typically you will see that when the client connection
	 * is disconnected from the client side.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 * the event. %FALSE to propagate the event further.
	 */
	signals[DESTROY_EVENT] = g_signal_new("destroy_event",
					      G_OBJECT_CLASS_TYPE (klass),
					      G_SIGNAL_RUN_LAST,
					      G_STRUCT_OFFSET (GXimCoreClass, destroy_event),
					      _gxim_acc_signal_accumulator__BOOLEAN,
					      NULL,
					      gxim_marshal_BOOLEAN__BOXED,
					      G_TYPE_BOOLEAN, 1,
					      GDK_TYPE_EVENT);
	/**
	 * GXimCore::client-event:
	 * @core: the object which received the signal.
	 * @event: the #GdkEventClient which triggered this signal.
	 *
	 * The ::client-event signal will be emitted when the @core's window
	 * receives a message (via ClientMessage event) from another application.
	 * all of XIM protocol is delivered with this signal.
	 *
	 * Returns: %TRUE to stop other handlers from being invoked for
	 * the event. %FALSE to propagate the event further.
	 */
	signals[CLIENT_EVENT] = g_signal_new("client_event",
					     G_OBJECT_CLASS_TYPE (klass),
					     G_SIGNAL_RUN_LAST,
					     G_STRUCT_OFFSET (GXimCoreClass, client_event),
					     _gxim_acc_signal_accumulator__BOOLEAN,
					     NULL,
					     gxim_marshal_BOOLEAN__BOXED,
					     G_TYPE_BOOLEAN, 1,
					     GDK_TYPE_EVENT);
}

static void
g_xim_core_init(GXimCore *core)
{
	GXimCorePrivate *priv = G_XIM_CORE_GET_PRIVATE (core);

	priv->atom_table = g_hash_table_new(g_direct_hash, g_direct_equal);
#ifdef GNOME_ENABLE_DEBUG
	priv->watch_table = g_hash_table_new(g_direct_hash, g_direct_equal);
#endif
	core->message = g_xim_message_new();
}

/*
 * Public functions
 */
GQuark
g_xim_core_get_error_quark(void)
{
	static GQuark quark = 0;

	if (!quark)
		quark = g_quark_from_static_string("g-xim-core-error");

	return quark;
}

/**
 * g_xim_core_watch_event:
 * @core: a #GXimCore
 * @window: a #GdkWindow that watches the event.
 *
 * Adds the event filter to @window and catches up events with @core's event
 * handler.  This has to be done before starting the protocol negotiation.
 * otherwise it will be lost and another application may be frozen in worst case.
 */
void
g_xim_core_watch_event(GXimCore  *core,
		       GdkWindow *window)
{
	g_return_if_fail (G_IS_XIM_CORE (core));
	g_return_if_fail (GDK_IS_WINDOW (window));

#ifdef GNOME_ENABLE_DEBUG
	G_STMT_START {
		GXimCorePrivate *priv = G_XIM_CORE_GET_PRIVATE (core);
		GdkNativeWindow nw = GDK_WINDOW_XID (window);

		g_xim_message_debug(core->message, "core/event",
				    "%s: Watch the event on %p",
				    g_type_name(G_TYPE_FROM_INSTANCE (core)),
				    G_XIM_NATIVE_WINDOW_TO_POINTER (nw));
		g_hash_table_insert(priv->watch_table,
				    G_XIM_NATIVE_WINDOW_TO_POINTER (nw),
				    GUINT_TO_POINTER (1));
	} G_STMT_END;
#endif
	gdk_window_add_filter(window,
			      g_xim_core_dispatch_events,
			      core);
}

/**
 * g_xim_core_unwatch_event:
 * @core: a #GXimCore
 * @window: a #GdkWindow that wants to remove from the watch list.
 *
 * Removes the event filter from @window and finishes watching events at @core.
 * This has to be done after finishing the protocol connection.
 * otherwise some events will be lost and another application may be frozen
 * in worst case.
 */
void
g_xim_core_unwatch_event(GXimCore  *core,
			 GdkWindow *window)
{
	g_return_if_fail (G_IS_XIM_CORE (core));
	g_return_if_fail (GDK_IS_WINDOW (window));

#ifdef GNOME_ENABLE_DEBUG
	G_STMT_START {
		GXimCorePrivate *priv = G_XIM_CORE_GET_PRIVATE (core);
		GdkNativeWindow nw = GDK_WINDOW_XID (window);

		g_xim_message_debug(core->message, "core/event",
				    "%s: Unwatch the event on %p",
				    g_type_name(G_TYPE_FROM_INSTANCE (core)),
				    G_XIM_NATIVE_WINDOW_TO_POINTER (nw));
		g_hash_table_remove(priv->watch_table,
				    G_XIM_NATIVE_WINDOW_TO_POINTER (nw));
		g_print("remaining watcher(s): %d\n", g_hash_table_size(priv->watch_table));
	} G_STMT_END;
#endif
	gdk_window_remove_filter(window,
				 g_xim_core_dispatch_events,
				 core);
}

/**
 * g_xim_core_get_display:
 * @core: a #GXimCore
 *
 * Obtains the #GdkDisplay that currently use.
 *
 * Return value: a display to use.
 */
GdkDisplay *
g_xim_core_get_display(GXimCore *core)
{
	gpointer retval;

	g_return_val_if_fail (G_IS_XIM_CORE (core), NULL);

	g_object_get(G_OBJECT (core), "display", &retval, NULL);

	return GDK_DISPLAY_OBJECT (retval);
}

/**
 * g_xim_core_get_selection_window:
 * @core: a #GXimCore
 *
 * Obtains a selection window. this function may be necessary when you want to
 * communicate the XIM server or clients through XIM protocol.
 *
 * Return value: #GXimCore:selection-window in @core.
 */
GdkWindow *
g_xim_core_get_selection_window(GXimCore *core)
{
	GdkWindow *w;

	g_return_val_if_fail (G_IS_XIM_CORE (core), NULL);

	g_object_get(G_OBJECT (core), "selection_window", &w, NULL);

	return w;
}

/**
 * g_xim_core_get_connection_gtype:
 * @core: a #GXimCore
 *
 * Obtains a #GType of the class inherited from #GXimConnection.
 * This function is typically used to create an instance of the connection.
 *
 * Return value: #GXimCore:connection-gtype in @core.
 */
GType
g_xim_core_get_connection_gtype(GXimCore *core)
{
	GType gtype;

	g_return_val_if_fail (G_IS_XIM_CORE (core), G_TYPE_INVALID);

	g_object_get(G_OBJECT (core), "connection_gtype", &gtype, NULL);

	return gtype;
}

/**
 * g_xim_core_get_protocol_signal_connector:
 * @core: a #GXimCore
 *
 * Obtains a list of the signal callbacks.
 * This function is typically used to initialize the signal connectioon after
 * creating an instance of the connection.
 *
 * Return value: a list of #GXimLazySignalConnector.
 */
GSList *
g_xim_core_get_protocol_signal_connector(GXimCore *core)
{
	GSList *retval;

	g_return_val_if_fail (G_IS_XIM_CORE (core), NULL);

	g_object_get(G_OBJECT (core), "proto_signals", &retval, NULL);

	return retval;
}

/**
 * g_xim_core_setup_connection:
 * @core: a #GXimCore
 * @conn: a #GXimConnection to be initialized
 *
 * Initialize a @conn's connection to get it working.
 */
void
g_xim_core_setup_connection(GXimCore       *core,
			    GXimConnection *conn)
{
	g_return_if_fail (G_IS_XIM_CORE (core));
	g_return_if_fail (G_IS_XIM_CONNECTION (conn));

	g_xim_connection_setup(conn);
	G_XIM_CORE_GET_CLASS (core)->setup_connection(core, conn);
}

/**
 * g_xim_core_add_client_message_filter:
 * @core: a #GXimCore
 * @atom: a #GdkAtom to deal with in the event filter.
 *
 * Deals with the event when @atom is specified as the message_type in
 * #GXimCore::client-event.
 */
void
g_xim_core_add_client_message_filter(GXimCore *core,
				     GdkAtom   atom)
{
	GXimCorePrivate *priv;

	g_return_if_fail (G_IS_XIM_CORE (core));

	priv = G_XIM_CORE_GET_PRIVATE (core);
	g_hash_table_replace(priv->atom_table, atom, GUINT_TO_POINTER (1));
}

/**
 * g_xim_core_remove_client_message_filter:
 * @core: a #GXimCore
 * @atom: a #GdkAtom to not deal with in the event filter any more.
 *
 * Removes @atom from the table and the event filter won't deals with it
 * even when it's specified to the message_type in #GXimCore::client-event.
 */
void
g_xim_core_remove_client_message_filter(GXimCore *core,
					GdkAtom   atom)
{
	GXimCorePrivate *priv;

	g_return_if_fail (G_IS_XIM_CORE (core));

	priv = G_XIM_CORE_GET_PRIVATE (core);
	g_hash_table_remove(priv->atom_table, atom);
}

/**
 * g_xim_core_lookup_client_message_filter:
 * @core: a #GXimCore
 * @atom: a #GdkAtom to look up
 *
 * Looks up @atom if it's registered to deal with it in #GXimCore::client-event.
 *
 * Return value: %TRUE if @atom has already been added to the table.
 */
gboolean
g_xim_core_lookup_client_message_filter(GXimCore *core,
					GdkAtom   atom)
{
	GXimCorePrivate *priv;

	g_return_val_if_fail (G_IS_XIM_CORE (core), FALSE);

	priv = G_XIM_CORE_GET_PRIVATE (core);
	return g_hash_table_lookup(priv->atom_table, atom) != NULL;
}
