/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximconnection.c
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
#include "gximprotocol.h"
#include "gximtransport.h"
#include "gximmisc.h"
#include "gximconnection.h"


/**
 * SECTION:gximconnection
 * @Title: GXimConnection
 * @Short_Description: Base class for XIM connection
 * @See_Also: #GXimProtocol, #GXimTransport
 *
 * GXimConnection provides a common facility to deal with XIM protocol events
 * after the connection established.
 */

enum {
	PROP_0,
	PROP_PROTO_SIGNALS,
	LAST_PROP
};
enum {
	SIGNAL_0,
	LAST_SIGNAL
};


static void g_xim_connection_protocol_init (gpointer g_iface,
					    gpointer data);
static void g_xim_connection_transport_init(gpointer g_iface,
					    gpointer data);


//static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (GXimConnection, g_xim_connection, G_TYPE_OBJECT,
				  G_IMPLEMENT_INTERFACE (G_TYPE_XIM_TRANSPORT,
							 g_xim_connection_transport_init);
				  G_IMPLEMENT_INTERFACE (G_TYPE_XIM_PROTOCOL,
							 g_xim_connection_protocol_init));

/*
 * Private functions
 */
static void
g_xim_connection_real_set_property(GObject      *object,
				   guint         prop_id,
				   const GValue *value,
				   GParamSpec   *pspec)
{
	GXimConnection *conn = G_XIM_CONNECTION (object);

	switch (prop_id) {
	    case PROP_PROTO_SIGNALS:
		    if (!conn->proto_signals) {
			    conn->proto_signals = g_value_get_pointer(value);
		    }
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_connection_real_get_property(GObject    *object,
				   guint       prop_id,
				   GValue     *value,
				   GParamSpec *pspec)
{
	GXimConnection *conn = G_XIM_CONNECTION (object);

	switch (prop_id) {
	    case PROP_PROTO_SIGNALS:
		    g_value_set_pointer(value, conn->proto_signals);
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_connection_real_dispose(GObject *object)
{
	g_xim_protocol_dispose(object);

	if (G_OBJECT_CLASS (g_xim_connection_parent_class)->dispose)
		(* G_OBJECT_CLASS (g_xim_connection_parent_class)->dispose) (object);
}

static void
g_xim_connection_real_finalize(GObject *object)
{
	GXimConnection *conn = G_XIM_CONNECTION (object);

	if (conn->imattr)
		g_object_unref(conn->imattr);
	if (conn->default_icattr)
		g_object_unref(conn->default_icattr);
	if (conn->encodings)
		g_value_array_free(conn->encodings);
	if (conn->encoding_details)
		g_value_array_free(conn->encoding_details);

	g_xim_protocol_finalize(object);
	g_xim_transport_finalize(G_XIM_TRANSPORT (object));

	if (G_OBJECT_CLASS (g_xim_connection_parent_class)->finalize)
		(* G_OBJECT_CLASS (g_xim_connection_parent_class)->finalize) (object);
}

static void
g_xim_connection_class_init(GXimConnectionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = g_xim_connection_real_set_property;
	object_class->get_property = g_xim_connection_real_get_property;
	object_class->dispose      = g_xim_connection_real_dispose;
	object_class->finalize     = g_xim_connection_real_finalize;

	/* properties */
	/**
	 * GXimConnection:proto-signals:
	 *
	 * a #GSList contains #GXimLazySignalConnector to connected the signals
	 * when the connection is configured.
	 */
	g_object_class_install_property(object_class, PROP_PROTO_SIGNALS,
					g_param_spec_pointer("proto_signals",
							     _("Signals for Protocol class"),
							     _("A list of signal connector for Protocol class"),
							     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	/* signals */
}

static void
g_xim_connection_init(GXimConnection *conn)
{
}

static void
g_xim_connection_protocol_init(gpointer g_iface,
			       gpointer data)
{
	/* do not initialize the interface here.
	 * we would allow overriding the interface's behavior
	 * for inherited classes.
	 */
}

static void
g_xim_connection_transport_init(gpointer g_iface,
				gpointer data)
{
	/* do not initialize the interface here.
	 * we would allow overriding the interface's behavior
	 * for inherited classes.
	 */
}

/*
 * Public functions
 */
/**
 * g_xim_connection_setup:
 * @conn: a #GXimConnection.
 *
 * Configures @conn to get it working. this is a helper function since
 * #GXimConnection is an abstract class. if you have any connection class being
 * inherited from #GXimConnection, you will have to invoke this function in
 * appropriate timing.
 */
void
g_xim_connection_setup(GXimConnection *conn)
{
	g_return_if_fail (G_IS_XIM_CONNECTION (conn));

	g_xim_transport_init(G_XIM_TRANSPORT (conn));
	g_xim_protocol_init(G_XIM_PROTOCOL (conn));
	if (G_XIM_CONNECTION_GET_CLASS (conn)->protocol_init)
		G_XIM_CONNECTION_GET_CLASS (conn)->protocol_init(G_XIM_PROTOCOL (conn));

	if (G_XIM_CONNECTION_GET_CLASS (conn)->transport_init)
		G_XIM_CONNECTION_GET_CLASS (conn)->transport_init(G_XIM_TRANSPORT (conn));

	if (conn->proto_signals) {
		GXimProtocol *proto = G_XIM_PROTOCOL (conn);
		GSList *l;

		for (l = conn->proto_signals; l != NULL; l = g_slist_next(l)) {
			GXimLazySignalConnector *s = l->data;

			s->id = g_xim_protocol_connect_closure_by_name(proto, s->signal_name,
								       s->function, s->data);
		}
	}
}

/**
 * g_xim_connection_cmd_error:
 * @conn: a #GXimConnection.
 * @imid: input-method ID
 * @icid: input-context ID
 * @flag: a flag to validate the parameters, which should be #GXimErrorMask.
 * @error_code: error code returned by XIM protocol events.
 * @detail: unused in XIM protocol but just reserved.
 * @error_message: error message which should be more human readable.
 *
 * Sends %XIM_ERROR to the opposite direction on the connection. which would
 * respond a request as error.
 *
 * This is asynchronous event so it doesn't ensure if the client really receives
 * this event.
 *
 * Returns: %TRUE to be sent the event successfully.
 */
gboolean
g_xim_connection_cmd_error(GXimConnection *conn,
			   guint16         imid,
			   guint16         icid,
			   GXimErrorMask   flag,
			   GXimErrorCode   error_code,
			   guint16         detail,
			   const gchar    *error_message)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	return g_xim_protocol_send(proto, G_XIM_ERROR, 0,
				   8,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid,
				   G_XIM_TYPE_WORD, flag,
				   G_XIM_TYPE_WORD, error_code,
				   G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_CHAR,
				   G_XIM_TYPE_WORD, detail,
				   G_XIM_TYPE_CHAR, error_message,
				   G_XIM_TYPE_AUTO_PADDING, 0);
}

/**
 * g_xim_connection_cmd_auth_ng:
 * @conn: a #GXimConnection.
 *
 * Sends %XIM_AUTH_NG to the opposite direction on the connection. which would
 * respond the authentication request as error.
 *
 * This is asynchronous event so it doesn't ensure if the client really receives
 * this event.
 *
 * Returns: %TRUE to be sent the event successfully.
 */
gboolean
g_xim_connection_cmd_auth_ng(GXimConnection *conn)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	return g_xim_protocol_send(proto, G_XIM_AUTH_NG, 0, 0);
}

/**
 * g_xim_connection_cmd_forward_event:
 * @conn: a #GXimConnection.
 * @imid: input-method ID
 * @icid: input-context ID
 * @flag: a flag to give additional requests, which is #GXimEventFlags.
 * @event: the #GdkEvent which send to the opposite direction on the connection.
 *
 * Sends %XIM_FORWARD_EVENT to the opposite direction on the connection. which would
 * notify the key event or respond %XIM_FORWARD_EVENT.
 *
 * Returns: %TRUE to be sent the event successfully.
 */
gboolean
g_xim_connection_cmd_forward_event(GXimConnection *conn,
				   guint16         imid,
				   guint16         icid,
				   guint16         flag,
				   GdkEvent       *event)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_CONNECTION (conn), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_FORWARD_EVENT, 0,
				   4,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid,
				   G_XIM_TYPE_WORD, flag,
				   G_XIM_TYPE_GDKEVENT, event);
}

/**
 * g_xim_connection_cmd_sync_reply:
 * @conn: a #GXimConnection.
 * @imid: input-method ID
 * @icid: input-context ID
 *
 * Sends %XIM_SYNC_REPLY to the opposite direction on the connection. which would
 * respond %XIM_SYNC or any synchronous requests.
 *
 * Returns: %TRUE to be sent the event successfully.
 */
gboolean
g_xim_connection_cmd_sync_reply(GXimConnection *conn,
				guint16         imid,
				guint16         icid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_SYNC_REPLY, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid);
}
