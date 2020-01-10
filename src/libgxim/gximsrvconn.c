/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximsrvconn.c
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

#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <gdk/gdkx.h>
#include "gximacc.h"
#include "gximattr.h"
#include "gximmarshal.h"
#include "gximmessages.h"
#include "gximmisc.h"
#include "gximprotocol.h"
#include "gximtransport.h"
#include "gximtypes.h"
#include "gximsrvconn.h"


enum {
	PROP_0,
	LAST_PROP
};
enum {
	SIGNAL_0,
	IS_AUTH_REQUIRED,
	LAST_SIGNAL
};


static guint signals[LAST_SIGNAL] = { 0 };


G_DEFINE_TYPE (GXimServerConnection, g_xim_server_connection, G_TYPE_XIM_CONNECTION);


/*
 * Private functions
 */
static void
g_xim_server_connection_real_set_property(GObject      *object,
					  guint         prop_id,
					  const GValue *value,
					  GParamSpec   *pspec)
{
	switch (prop_id) {
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_server_connection_real_get_property(GObject    *object,
					  guint       prop_id,
					  GValue     *value,
					  GParamSpec *pspec)
{
	switch (prop_id) {
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_server_connection_real_finalize(GObject *object)
{
	if (G_OBJECT_CLASS (g_xim_server_connection_parent_class)->finalize)
		(* G_OBJECT_CLASS (g_xim_server_connection_parent_class)->finalize) (object);
}

static gboolean
g_xim_server_protocol_real_XIM_CONNECT(GXimProtocol  *proto,
				       guint16        major_version,
				       guint16        minor_version,
				       const GSList  *list)
{
	gboolean ret, retval = FALSE;

	g_signal_emit(proto, signals[IS_AUTH_REQUIRED], 0, &ret);
	if (list) {
		/* client is waiting for XIM_AUTH_REQUIRED */
		if (ret) {
			/* XXX: send XIM_AUTH_REQUIRED */
		} else {
			/* just send out an empty XIM_AUTH_REQUIRED */
			/* XXX */
			retval = g_xim_protocol_send(proto, G_XIM_AUTH_REQUIRED, 0,
						     4,
						     G_XIM_TYPE_BYTE, 0, /* auth-protocol-index */
						     G_XIM_TYPE_PADDING, g_xim_protocol_n_pad4 (1),
						     G_XIM_TYPE_WORD, 0, /* length of authentication data */
						     G_XIM_TYPE_PADDING, g_xim_protocol_n_pad4 (2));
		}
	} else {
		if (ret) {
			/* send out XIM_AUTH_SETUP */
			/* XXX */
		} else {
			retval = g_xim_server_connection_cmd_connect_reply(G_XIM_SERVER_CONNECTION (proto), 1, 0);
		}
	}

	return retval;
}

static gboolean
g_xim_server_protocol_real_XIM_DISCONNECT(GXimProtocol  *proto)
{
	return g_xim_server_connection_cmd_disconnect_reply(G_XIM_SERVER_CONNECTION (proto));
}

static gboolean
g_xim_server_protocol_real_XIM_AUTH_REQUIRED(GXimProtocol  *proto,
					     const gchar   *auth_data,
					     gsize          length)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_AUTH_REPLY(GXimProtocol  *proto,
					  const gchar   *auth_data,
					  gsize          length)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_AUTH_NEXT(GXimProtocol  *proto,
					 const gchar   *auth_data,
					 gsize          length)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_AUTH_NG(GXimProtocol  *proto)
{
	/* XXX: disconnecting */

	return TRUE;
}

static gboolean
g_xim_server_protocol_real_XIM_ERROR(GXimProtocol  *proto,
				     guint16        imid,
				     guint16        icid,
				     guint16        mask,
				     guint16        error_code)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_OPEN(GXimProtocol  *proto,
				    const GXimStr *locale)
{
	static const gchar message[] = "No valid XIM server available.";
	static size_t len = 0;
	const gchar *loc = g_xim_str_get_string(locale);

	g_xim_messages_debug(G_XIM_PROTOCOL_GET_IFACE (proto)->message, "proto/event",
			     "XIM_OPEN[%s]", loc);
	if (len == 0)
		len = strlen(message);

	return g_xim_connection_cmd_error(G_XIM_CONNECTION (proto), 0, 0, G_XIM_EMASK_NO_VALID_ID,
					  G_XIM_ERR_LocaleNotSupported, 0, message);
}

static gboolean
g_xim_server_protocol_real_XIM_CLOSE(GXimProtocol  *proto,
				     guint16        imid)
{
	static const gchar message[] = "No real implementation or any errors occured.";
	static size_t len = 0;

	if (len == 0)
		len = strlen(message);

	return g_xim_connection_cmd_error(G_XIM_CONNECTION (proto), imid, 0, G_XIM_EMASK_VALID_IMID,
					  G_XIM_ERR_BadSomething, 0, message);
}

static gboolean
g_xim_server_protocol_real_XIM_TRIGGER_NOTIFY(GXimProtocol  *proto,
					      guint16        imid,
					      guint16        icid,
					      guint32        flag,
					      guint32        index_,
					      guint32        mask)
{
	static const gchar message[] = "No real implementation or any errors occured.";
	static size_t len = 0;

	if (len == 0)
		len = strlen(message);

	return g_xim_connection_cmd_error(G_XIM_CONNECTION (proto), imid, icid,
					  G_XIM_EMASK_VALID_IMID | G_XIM_EMASK_VALID_ICID,
					  G_XIM_ERR_BadSomething, 0, message);
}

static gboolean
g_xim_server_protocol_real_XIM_ENCODING_NEGOTIATION(GXimProtocol  *proto,
						    guint16        imid,
						    const GSList  *encodings,
						    const GSList  *details)
{
	static const gchar message[] = "No real implementation or any errors occured.";
	static size_t len = 0;

	if (len == 0)
		len = strlen(message);

	return g_xim_connection_cmd_error(G_XIM_CONNECTION (proto), imid, 0,
					  G_XIM_EMASK_VALID_IMID,
					  G_XIM_ERR_BadSomething, 0, message);
}

static gboolean
g_xim_server_protocol_real_XIM_QUERY_EXTENSION(GXimProtocol  *proto,
					       guint16        imid,
					       const GSList  *extensions)
{
	GXimProtocolClosure *c;
	GSList *lc = NULL;

	if (extensions == NULL) {
		/* try to query all extensions */
		lc = g_xim_protocol_get_extensions(proto);
	} else {
		const GSList *l;

		for (l = extensions; l != NULL; l = g_slist_next(l)) {
			GXimStr *s = l->data;

			c = g_xim_protocol_lookup_protocol_by_name(proto,
								   g_xim_str_get_string(s));
			if (c)
				lc = g_slist_append(lc, c);
		}
	}

	g_xim_server_connection_cmd_query_extension_reply(G_XIM_SERVER_CONNECTION (proto), imid, lc);
	g_slist_free(lc);

	return TRUE;
}

static gboolean
g_xim_server_protocol_real_XIM_SET_IM_VALUES(GXimProtocol  *proto,
					     guint16        imid,
					     const GSList  *attributes)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_GET_IM_VALUES(GXimProtocol  *proto,
					     guint16        imid,
					     const GSList  *attr_id)
{
	static const gchar message[] = "No valid Input Method Attribute ID available.";
	static size_t len = 0;

	if (len == 0)
		len = strlen(message);

	g_xim_protocol_send(proto, G_XIM_ERROR, 0,
			    8,
			    G_XIM_TYPE_WORD, imid,
			    G_XIM_TYPE_WORD, 0,
			    G_XIM_TYPE_WORD, 1,
			    G_XIM_TYPE_WORD, G_XIM_ERR_BadSomething,
			    G_XIM_TYPE_WORD, len,
			    G_XIM_TYPE_WORD, 0, /* reserved */
			    G_XIM_TYPE_CHAR, message,
			    G_XIM_TYPE_PADDING, g_xim_protocol_n_pad4 (len));

	return TRUE;
}

static gboolean
g_xim_server_protocol_real_XIM_CREATE_IC(GXimProtocol  *proto,
					 guint16        imid,
					 const GSList  *attributes)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_DESTROY_IC(GXimProtocol  *proto,
					  guint16        imid,
					  guint16        icid)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_SET_IC_VALUES(GXimProtocol  *proto,
					     guint16        imid,
					     guint16        icid,
					     const GSList  *attributes)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_GET_IC_VALUES(GXimProtocol  *proto,
					     guint16        imid,
					     guint16        icid,
					     const GSList  *attr_id)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_SET_IC_FOCUS(GXimProtocol  *proto,
					    guint16        imid,
					    guint16        icid)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_UNSET_IC_FOCUS(GXimProtocol  *proto,
					      guint16        imid,
					      guint16        icid)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_FORWARD_EVENT(GXimProtocol  *proto,
					     guint16        imid,
					     guint16        icid,
					     guint16        flag,
					     GdkEvent      *event)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_SYNC(GXimProtocol  *proto,
				    guint16        imid,
				    guint16        icid)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_SYNC_REPLY(GXimProtocol  *proto,
					  guint16        imid,
					  guint16        icid)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_RESET_IC(GXimProtocol  *proto,
					guint16        imid,
					guint16        icid)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_STR_CONVERSION_REPLY(GXimProtocol    *proto,
						    guint16          imid,
						    guint16          icid,
						    guint32          feedback,
						    GXimStrConvText *text)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_PREEDIT_START_REPLY(GXimProtocol  *proto,
						   guint16        imid,
						   guint16        icid,
						   gint32         return_value)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_protocol_real_XIM_PREEDIT_CARET_REPLY(GXimProtocol  *proto,
						   guint16        imid,
						   guint16        icid,
						   guint32        position)
{
	/* FIXME */

	return FALSE;
}

static GdkWindow *
g_xim_server_transport_real_do_create_channel(GXimTransport *trans,
					      GdkWindow     *parent_window)
{
	GdkWindow *w;
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

	g_return_val_if_fail (parent_window != NULL, NULL);

	w = gdk_window_new(parent_window,
			   &attrs,
			   GDK_WA_X | GDK_WA_Y | GDK_WA_NOREDIR | GDK_WA_TYPE_HINT);

	return w;
}

static GdkNativeWindow
g_xim_server_transport_real_do_get_native_channel(GXimTransport *trans,
						  gpointer       drawable)
{
	if (GDK_IS_WINDOW (drawable))
		return GDK_WINDOW_XID (drawable);
	else if (GDK_IS_PIXMAP (drawable))
		return GDK_PIXMAP_XID (drawable);
	else
		return 0;
}

static void
g_xim_server_transport_real_do_destroy_channel(GXimTransport *trans)
{
	GdkWindow *w = g_xim_transport_get_channel(trans, NULL);

	gdk_window_destroy(w);
}

static gboolean
g_xim_server_transport_real_do_send_via_property(GXimTransport *trans,
						const gchar   *data,
						gsize          length)
{
	GdkEvent *ev;
	GdkNativeWindow client_window = g_xim_transport_get_client_window(trans);
	GdkWindow *w;
	GdkAtom atom;
	GdkDisplay *dpy = g_xim_transport_get_display(trans);
	gboolean retval;

	w = g_xim_get_window(dpy, client_window);
	atom = g_xim_transport_get_atom(trans);

	ev = gdk_event_new(GDK_CLIENT_EVENT);
	G_XIM_CHECK_ALLOC (ev, FALSE);

	ev->client.window = w;
	ev->client.message_type = G_XIM_TRANSPORT_GET_IFACE (trans)->atom_xim_protocol;
	ev->client.data_format = 32;
	ev->client.data.l[0] = length;
	ev->client.data.l[1] = gdk_x11_atom_to_xatom(atom);
	gdk_property_change(w, atom, GDK_SELECTION_TYPE_STRING,
			    8, GDK_PROP_MODE_APPEND,
			    (const guchar *)data, length);

	retval = gdk_event_send_client_message_for_display(dpy, ev, client_window);

	/* gdk_event_free does unref GdkWindow.
	 * So we don't need to invoke gdk_window_destroy explicitly.
	 */
	gdk_event_free(ev);

	return retval;
}

static gboolean
g_xim_server_transport_real_do_send_via_cm(GXimTransport *trans,
					  const gchar   *data,
					  gsize          length,
					  gsize          threshold)
{
	GdkEvent *ev;
	gsize len = length, offset = 0;
	gsize transport_size = g_xim_transport_get_transport_size(trans);
	GdkDisplay *dpy = g_xim_transport_get_display(trans);
	GdkNativeWindow client_window = g_xim_transport_get_client_window(trans);
	GdkWindow *w;
	gboolean retval;

	w = g_xim_get_window(dpy, client_window);

	while (len > threshold) {
		ev = gdk_event_new(GDK_CLIENT_EVENT);
		G_XIM_CHECK_ALLOC (ev, FALSE);

		ev->client.window = g_object_ref(w);
		ev->client.message_type = G_XIM_TRANSPORT_GET_IFACE (trans)->atom_xim_moredata;
		ev->client.data_format = 8;
		memcpy(ev->client.data.b, &data[offset], transport_size);

		gdk_event_send_client_message_for_display(dpy, ev, client_window);

		/* gdk_event_free does unref GdkWindow.
		 * So we don't need to invoke gdk_window_destroy explicitly.
		 */
		gdk_event_free(ev);

		offset += transport_size;
		len -= transport_size;
	}
	ev = gdk_event_new(GDK_CLIENT_EVENT);
	G_XIM_CHECK_ALLOC (ev, FALSE);

	ev->client.window = w;
	ev->client.message_type = G_XIM_TRANSPORT_GET_IFACE (trans)->atom_xim_protocol;
	ev->client.data_format = 8;
	memset(ev->client.data.b, 0, transport_size);
	memcpy(ev->client.data.b, &data[offset], len);

	retval = gdk_event_send_client_message_for_display(dpy, ev, client_window);

	/* gdk_event_free does unref GdkWindow.
	 * So we don't need to invoke gdk_window_destroy explicitly.
	 */
	gdk_event_free(ev);

	return retval;
}

static gboolean
g_xim_server_transport_real_do_send_via_property_notify(GXimTransport *trans,
						       const gchar   *data,
						       gsize          length)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_server_transport_real_do_get_property(GXimTransport  *trans,
					    GdkWindow      *window,
					    GdkAtom         property,
					    GdkAtom         type,
					    gulong          length,
					    GdkAtom        *actual_property_type,
					    gint           *actual_format,
					    gint           *actual_length,
					    guchar        **data)
{
	GXimTransportPrivate *priv = g_xim_transport_get_private(trans);
	gulong offset = (gulong)g_hash_table_lookup(priv->prop_offset, property);
	GdkDisplay *dpy = priv->display;
	GdkNativeWindow nw = g_xim_transport_get_native_channel_from(trans, window);
	Atom ret_type;
	gint ret_format, ret;
	gulong ret_nitems, ret_remain, total_length, remain;
	guchar *ret_prop;
	gboolean retval = TRUE;
	gchar *name;

	/* gdk_property_get doesn't deal with the remaining bytes after reading.
	 * and XGetWindowProperty doesn't delete the property if there are the data
	 * in the property yet. so we need to know how much the length of data is
	 * in the property and set the offset next time properly.
	 */
	ret = XGetWindowProperty(GDK_DISPLAY_XDISPLAY (dpy), nw,
				 gdk_x11_atom_to_xatom_for_display(dpy, property),
				 0, 1, False, AnyPropertyType,
				 &ret_type, &ret_format, &ret_nitems, &ret_remain, &ret_prop);
	if (ret != Success ||
	    (ret_type == None && ret_format == 0)) {
		retval = FALSE;
		offset = 0;
		goto end;
	}
	XFree(ret_prop);

	name = gdk_atom_name(property);
	g_xim_messages_debug(G_XIM_TRANSPORT_GET_IFACE (trans)->message, "transport/property",
			     "Reading from %s with offset %ld, length: %ld",
			     name, offset, length);
	g_free(name);

	if (!gdk_property_get(window, property, type, offset / 4, length, TRUE,
			      actual_property_type, actual_format, actual_length, data)) {
		retval = FALSE;
		offset = 0;
		goto end;
	}

	/* Update the offset to read next time */
	total_length = ret_nitems * *actual_format / 8 + ret_remain;
	remain = total_length - offset - length;

	if (total_length > (offset + length))
		offset += length;
	else
		offset = 0;

	g_xim_messages_debug(G_XIM_TRANSPORT_GET_IFACE (trans)->message, "transport/property",
			     "Read %ld bytes and %ld bytes remaining. next offset will be %ld",
			     length, remain, offset);
  end:
	if (offset == 0) {
		g_hash_table_remove(priv->prop_offset, property);
		/* remove the property to make sure */
		gdk_property_delete(window, property);
	} else {
		g_hash_table_insert(priv->prop_offset, property, (gpointer)offset);
	}

	return retval;
}

static void
g_xim_server_connection_real_protocol_init(GXimProtocol *proto)
{
	GXimProtocolPrivate *priv = g_xim_protocol_get_private(proto);

#define CONNECT(_m_,_n_)						\
	priv->signal_ids[G_ ## _m_] = g_xim_protocol_connect_closure_by_id(proto, \
									   G_ ## _m_, \
									   (_n_), \
									   G_CALLBACK (g_xim_server_protocol_real_ ## _m_), \
									   NULL);
	CONNECT (XIM_CONNECT, 0);
	CONNECT (XIM_DISCONNECT, 0);
	CONNECT (XIM_AUTH_REQUIRED, 0);
	CONNECT (XIM_AUTH_REPLY, 0);
	CONNECT (XIM_AUTH_NEXT, 0);
	CONNECT (XIM_AUTH_NG, 0);
	CONNECT (XIM_ERROR, 0);
	CONNECT (XIM_OPEN, 0);
	CONNECT (XIM_CLOSE, 0);
	CONNECT (XIM_TRIGGER_NOTIFY, 0);
	CONNECT (XIM_ENCODING_NEGOTIATION, 0);
	CONNECT (XIM_QUERY_EXTENSION, 0);
	CONNECT (XIM_SET_IM_VALUES, 0);
	CONNECT (XIM_GET_IM_VALUES, 0);
	CONNECT (XIM_CREATE_IC, 0);
	CONNECT (XIM_DESTROY_IC, 0);
	CONNECT (XIM_SET_IC_VALUES, 0);
	CONNECT (XIM_GET_IC_VALUES, 0);
	CONNECT (XIM_SET_IC_FOCUS, 0);
	CONNECT (XIM_UNSET_IC_FOCUS, 0);
	CONNECT (XIM_FORWARD_EVENT, 0);
	CONNECT (XIM_SYNC, 0);
	CONNECT (XIM_SYNC_REPLY, 0);
	CONNECT (XIM_RESET_IC, 0);
	CONNECT (XIM_STR_CONVERSION_REPLY, 0);
	CONNECT (XIM_PREEDIT_START_REPLY, 0);
	CONNECT (XIM_PREEDIT_CARET_REPLY, 0);

#undef CONNECT
}

static void
g_xim_server_connection_real_transport_init(GXimTransport *trans)
{
	GXimTransportIface *iface = G_XIM_TRANSPORT_GET_IFACE (trans);

	iface->do_create_channel           = g_xim_server_transport_real_do_create_channel;
	iface->do_get_native_channel       = g_xim_server_transport_real_do_get_native_channel;
	iface->do_destroy_channel          = g_xim_server_transport_real_do_destroy_channel;
	iface->do_send_via_property        = g_xim_server_transport_real_do_send_via_property;
	iface->do_send_via_cm              = g_xim_server_transport_real_do_send_via_cm;
	iface->do_send_via_property_notify = g_xim_server_transport_real_do_send_via_property_notify;
	iface->do_get_property             = g_xim_server_transport_real_do_get_property;
}

static gboolean
g_xim_server_connection_real_is_auth_required(GXimServerConnection *conn)
{
	/* no auth required for the default behavior */

	return FALSE;
}

static void
g_xim_server_connection_class_init(GXimServerConnectionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GXimConnectionClass *conn_class = G_XIM_CONNECTION_CLASS (klass);

	object_class->set_property = g_xim_server_connection_real_set_property;
	object_class->get_property = g_xim_server_connection_real_get_property;
	object_class->finalize     = g_xim_server_connection_real_finalize;

	conn_class->protocol_init  = g_xim_server_connection_real_protocol_init;
	conn_class->transport_init = g_xim_server_connection_real_transport_init;

	klass->is_auth_required = g_xim_server_connection_real_is_auth_required;

	/* properties */

	/* signals */
	signals[IS_AUTH_REQUIRED] = g_signal_new("is_auth_required",
						 G_OBJECT_CLASS_TYPE (klass),
						 G_SIGNAL_RUN_LAST,
						 G_STRUCT_OFFSET (GXimServerConnectionClass, is_auth_required),
						 _gxim_acc_signal_accumulator__BOOLEAN,
						 NULL,
						 gxim_marshal_BOOLEAN__VOID,
						 G_TYPE_BOOLEAN, 0);
}

static void
g_xim_server_connection_init(GXimServerConnection *conn)
{
}

/*
 * Public functions
 */
gboolean
g_xim_server_connection_cmd_connect_reply(GXimServerConnection *conn,
					  guint16               major_version,
					  guint16               minor_version)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_CONNECT_REPLY, 0,
				   2,
				   G_XIM_TYPE_WORD, major_version,
				   G_XIM_TYPE_WORD, minor_version);
}

gboolean
g_xim_server_connection_cmd_disconnect_reply(GXimServerConnection *conn)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_DISCONNECT_REPLY, 0, 0);
}

gboolean
g_xim_server_connection_cmd_open_reply(GXimServerConnection *conn,
				       guint16               imid,
				       const GSList         *imattr_list,
				       const GSList         *icattr_list)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_OPEN_REPLY, 0,
				   6,
				   G_XIM_TYPE_WORD, imid, /* imid */
				   G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_IMATTR, /* the number of IM attr_list */
				   G_XIM_TYPE_LIST_OF_IMATTR, imattr_list, /* IM attr list */
				   G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_ICATTR, /* the number of IC attr_list */
				   G_XIM_TYPE_PADDING, 2, /* padding */
				   G_XIM_TYPE_LIST_OF_ICATTR, icattr_list /* IC attr list */);
}

gboolean
g_xim_server_connection_cmd_close_reply(GXimServerConnection *conn,
					guint16               imid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_CLOSE_REPLY, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_PADDING, 2);
}

gboolean
g_xim_server_connection_cmd_register_triggerkeys(GXimServerConnection *conn,
						 guint16               imid,
						 const GSList         *onkeys,
						 const GSList         *offkeys)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_REGISTER_TRIGGERKEYS, 0,
				   6,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_PADDING, 2,
				   G_XIM_TYPE_MARKER_N_BYTES_4, G_XIM_TYPE_LIST_OF_HOTKEY_TRIGGER,
				   G_XIM_TYPE_LIST_OF_HOTKEY_TRIGGER, onkeys,
				   G_XIM_TYPE_MARKER_N_BYTES_4, G_XIM_TYPE_LIST_OF_HOTKEY_TRIGGER,
				   G_XIM_TYPE_LIST_OF_HOTKEY_TRIGGER, offkeys);
}

gboolean
g_xim_server_connection_cmd_trigger_notify_reply(GXimServerConnection *conn,
						 guint16               imid,
						 guint16               icid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_TRIGGER_NOTIFY_REPLY, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid);
}

gboolean
g_xim_server_connection_cmd_set_event_mask(GXimServerConnection *conn,
					   guint16               imid,
					   guint16               icid,
					   guint32               forward_event,
					   guint32               sync_event)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_SET_EVENT_MASK, 0,
				   4,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid,
				   G_XIM_TYPE_LONG, forward_event,
				   G_XIM_TYPE_LONG, sync_event);
}

gboolean
g_xim_server_connection_cmd_encoding_negotiation_reply(GXimServerConnection *conn,
						       guint16               imid,
						       guint16               category,
						       gint16                index_)
{
	GXimProtocol *proto;
	GXimConnection *c;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	c = G_XIM_CONNECTION (conn);

	c->encoding_category = category;
	c->encoding_index = index_;

	return g_xim_protocol_send(proto, G_XIM_ENCODING_NEGOTIATION_REPLY, 0,
				   4,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, category,
				   G_XIM_TYPE_WORD, index_,
				   G_XIM_TYPE_PADDING, 2);
}

gboolean
g_xim_server_connection_cmd_query_extension_reply(GXimServerConnection *conn,
						  guint16               imid,
						  const GSList         *extensions)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_QUERY_EXTENSION_REPLY, 0,
				   3,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_EXT,
				   G_XIM_TYPE_LIST_OF_EXT, extensions);
}

gboolean
g_xim_server_connection_cmd_set_im_values_reply(GXimServerConnection *conn,
						guint16               imid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_SET_IM_VALUES_REPLY, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_PADDING, 2);
}

gboolean
g_xim_server_connection_cmd_get_im_values_reply(GXimServerConnection *conn,
						guint16               imid,
						const GSList         *attributes)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_GET_IM_VALUES_REPLY, 0,
				   3,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_IMATTRIBUTE,
				   G_XIM_TYPE_LIST_OF_IMATTRIBUTE, attributes);
}

gboolean
g_xim_server_connection_cmd_create_ic_reply(GXimServerConnection *conn,
					    guint16               imid,
					    guint16               icid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_CREATE_IC_REPLY, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid);
}

gboolean
g_xim_server_connection_cmd_destroy_ic_reply(GXimServerConnection *conn,
					     guint16               imid,
					     guint16               icid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_DESTROY_IC_REPLY, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid);
}

gboolean
g_xim_server_connection_cmd_set_ic_values_reply(GXimServerConnection *conn,
						guint16               imid,
						guint16               icid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_SET_IC_VALUES_REPLY, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid);
}

gboolean
g_xim_server_connection_cmd_get_ic_values_reply(GXimServerConnection *conn,
						guint16               imid,
						guint16               icid,
						const GSList         *attributes)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_GET_IC_VALUES_REPLY, 0,
				   5,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid,
				   G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_ICATTRIBUTE,
				   G_XIM_TYPE_PADDING, 2,
				   G_XIM_TYPE_LIST_OF_ICATTRIBUTE, attributes);
}

gboolean
g_xim_server_connection_cmd_sync(GXimServerConnection *conn,
				 guint16               imid,
				 guint16               icid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_SYNC, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid);
}

gboolean
g_xim_server_connection_cmd_commit(GXimServerConnection *conn,
				   guint16               imid,
				   guint16               icid,
				   guint16               flag,
				   guint32               keysym,
				   GString              *string)
{
	GXimProtocol *proto;
	gint padding = 0;
	gboolean retval;
	GSList *lt = NULL, *lv = NULL;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	lt = g_slist_append(lt, GUINT_TO_POINTER (G_XIM_TYPE_WORD));
	lv = g_slist_append(lv, GUINT_TO_POINTER ((guint)imid));
	lt = g_slist_append(lt, GUINT_TO_POINTER (G_XIM_TYPE_WORD));
	lv = g_slist_append(lv, GUINT_TO_POINTER ((guint)icid));
	lt = g_slist_append(lt, GUINT_TO_POINTER (G_XIM_TYPE_WORD));
	lv = g_slist_append(lv, GUINT_TO_POINTER ((guint)flag));

	if (flag & G_XIM_XLookupKeySym) {
		padding += 2;
		lt = g_slist_append(lt, GUINT_TO_POINTER (G_XIM_TYPE_PADDING));
		lv = g_slist_append(lv, GUINT_TO_POINTER (2));
		lt = g_slist_append(lt, GUINT_TO_POINTER (G_XIM_TYPE_LONG));
		lv = g_slist_append(lv, GUINT_TO_POINTER (keysym));
	}
	if (flag & G_XIM_XLookupChars) {
		g_return_val_if_fail ((flag & G_XIM_XLookupChars) && string != NULL, FALSE);

		lt = g_slist_append(lt, GUINT_TO_POINTER (G_XIM_TYPE_GSTRING));
		lv = g_slist_append(lv, string);
		lt = g_slist_append(lt, GUINT_TO_POINTER (G_XIM_TYPE_AUTO_PADDING));
		lv = g_slist_append(lv, GUINT_TO_POINTER (padding));
	}

	retval = g_xim_protocol_send_with_list(proto, G_XIM_COMMIT, 0,
					       lt, lv);
	g_slist_free(lt);
	g_slist_free(lv);

	return retval;
}

gboolean
g_xim_server_connection_cmd_reset_ic_reply(GXimServerConnection *conn,
					   guint16               imid,
					   guint16               icid,
					   const GString        *preedit_string)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_RESET_IC_REPLY, 0,
				   5,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid,
				   G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_BYTE,
				   G_XIM_TYPE_LIST_OF_BYTE, preedit_string,
				   G_XIM_TYPE_AUTO_PADDING, 2);
}

gboolean
g_xim_server_connection_cmd_preedit_start(GXimServerConnection *conn,
					  guint16               imid,
					  guint16               icid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_PREEDIT_START, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid);
}

gboolean
g_xim_server_connection_cmd_preedit_draw(GXimServerConnection *conn,
					 guint16               imid,
					 guint16               icid,
					 GXimPreeditDraw      *draw)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);
	g_return_val_if_fail (draw != NULL, FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_PREEDIT_DRAW, 0,
				   3,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid,
				   G_XIM_TYPE_PREEDIT_DRAW, draw);
}

gboolean
g_xim_server_connection_cmd_preedit_caret(GXimServerConnection *conn,
					  guint16               imid,
					  guint16               icid,
					  GXimPreeditCaret     *caret)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);
	g_return_val_if_fail (caret != NULL, FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_PREEDIT_CARET, 0,
				   3,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid,
				   G_XIM_TYPE_PREEDIT_CARET, caret);
}

gboolean
g_xim_server_connection_cmd_preedit_done(GXimServerConnection *conn,
					 guint16               imid,
					 guint16               icid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_PREEDIT_DONE, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid);
}

gboolean
g_xim_server_connection_cmd_status_start(GXimServerConnection *conn,
					 guint16               imid,
					 guint16               icid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_STATUS_START, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid);
}

gboolean
g_xim_server_connection_cmd_status_draw(GXimServerConnection *conn,
					guint16               imid,
					guint16               icid,
					GXimStatusDraw       *draw)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);
	g_return_val_if_fail (draw != NULL, FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_STATUS_DRAW, 0,
				   3,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid,
				   G_XIM_TYPE_STATUS_DRAW, draw);
}

gboolean
g_xim_server_connection_cmd_status_done(GXimServerConnection *conn,
					guint16               imid,
					guint16               icid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_SERVER_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);

	return g_xim_protocol_send(proto, G_XIM_STATUS_DONE, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid);
}
