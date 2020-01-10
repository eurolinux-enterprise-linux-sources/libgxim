/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximclconn.c
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
#include "gximmessages.h"
#include "gximmisc.h"
#include "gximprotocol.h"
#include "gximtransport.h"
#include "gximclconn.h"


enum {
	PROP_0,
	LAST_PROP
};
enum {
	SIGNAL_0,
	LAST_SIGNAL
};


//static guint signals[LAST_SIGNAL] = { 0 };


G_DEFINE_TYPE (GXimClientConnection, g_xim_client_connection, G_TYPE_XIM_CONNECTION);


/*
 * Private functions
 */
static void
g_xim_client_connection_real_set_property(GObject      *object,
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
g_xim_client_connection_real_get_property(GObject    *object,
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
g_xim_client_connection_real_finalize(GObject *object)
{
	if (G_OBJECT_CLASS (g_xim_client_connection_parent_class)->finalize)
		(* G_OBJECT_CLASS (g_xim_client_connection_parent_class)->finalize) (object);
}

static gboolean
g_xim_client_protocol_real_XIM_CONNECT_REPLY(GXimProtocol *proto,
					     guint16       major_version,
					     guint16       minor_version)
{
	/* XXX: wnat should we do? */

	return TRUE;
}

static gboolean
g_xim_client_protocol_real_XIM_DISCONNECT_REPLY(GXimProtocol *proto)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_AUTH_REQUIRED(GXimProtocol *proto,
					     const gchar  *auth_data,
					     gsize         length)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_AUTH_NEXT(GXimProtocol *proto,
					 const gchar  *auth_data,
					 gsize         length)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_AUTH_SETUP(GXimProtocol *proto,
					  const GSList *list)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_AUTH_NG(GXimProtocol *proto)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_ERROR(GXimProtocol *proto,
				     guint16       imid,
				     guint16       icid,
				     guint16       mask,
				     guint16       error_code)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_OPEN_REPLY(GXimProtocol *proto,
					  guint16       imid,
					  GXimIMAttr   *imattr,
					  GXimICAttr   *icattr)
{
	GXimConnection *conn = G_XIM_CONNECTION (proto);

	conn->imid = imid;
	conn->imattr = g_object_ref(imattr);
	conn->default_icattr = g_object_ref(icattr);

	return TRUE;
}

static gboolean
g_xim_client_protocol_real_XIM_CLOSE_REPLY(GXimProtocol *proto,
					   guint16       imid)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_REGISTER_TRIGGERKEYS(GXimProtocol *proto,
						    guint16       imid,
						    const GSList *onkeys,
						    const GSList *offkeys)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_TRIGGER_NOTIFY_REPLY(GXimProtocol *proto,
						    guint16       imid,
						    guint16       icid)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_SET_EVENT_MASK(GXimProtocol *proto,
					      guint16       imid,
					      guint16       icid,
					      guint32       forward_event_mask,
					  guint32       synchronous_event_mask)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_ENCODING_NEGOTIATION_REPLY(GXimProtocol *proto,
							  guint16       imid,
							  guint16       category,
							  gint16        index_)
{
	GXimConnection *conn = G_XIM_CONNECTION (proto);

	conn->encoding_category = category;
	conn->encoding_index = index_;

	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_QUERY_EXTENSION_REPLY(GXimProtocol *proto,
						     guint16       imid,
						     const GSList *extensions)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_SET_IM_VALUES_REPLY(GXimProtocol *proto,
						   guint16       imid)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_GET_IM_VALUES_REPLY(GXimProtocol *proto,
						   guint16       imid,
						   const GSList *attributes)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_CREATE_IC_REPLY(GXimProtocol *proto,
					       guint16       imid,
					       guint16       icid)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_DESTROY_IC_REPLY(GXimProtocol *proto,
						guint16       imid,
						guint16       icid)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_SET_IC_VALUES_REPLY(GXimProtocol *proto,
						   guint16       imid,
						   guint16       icid)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_GET_IC_VALUES_REPLY(GXimProtocol *proto,
						   guint16       imid,
						   guint16       icid,
						   const GSList *attributes)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_FORWARD_EVENT(GXimProtocol *proto,
					     guint16       imid,
					     guint16       icid,
					     guint16       flag,
					     GdkEvent     *event)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_SYNC(GXimProtocol *proto,
				    guint16       imid,
				    guint16       icid)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_SYNC_REPLY(GXimProtocol *proto,
					  guint16       imid,
					  guint16       icid)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_COMMIT(GXimProtocol *proto,
				      guint16       imid,
				      guint16       icid,
				      guint16       flag,
				      const gchar  *packets,
				      gsize        *length)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_RESET_IC_REPLY(GXimProtocol *proto,
					      guint16       imid,
					      guint16       icid,
					      GString      *string)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_GEOMETRY(GXimProtocol *proto,
					guint16       imid,
					guint16       icid)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_STR_CONVERSION(GXimProtocol *proto,
					      guint16       imid,
					      guint16       icid,
					      guint16       position,
					      guint32       caret_direction,
					      guint16       factor,
					      guint16       operation,
					      gint16        type_length)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_PREEDIT_START(GXimProtocol *proto,
					     guint16       imid,
					     guint16       icid)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_PREEDIT_DRAW(GXimProtocol  *proto,
					    guint16        imid,
					    guint16        icid,
					    gint32         caret,
					    gint32         chg_first,
					    gint32         chg_length,
					    guint32        length,
					    const GString *string,
					    const GSList  *feedbacks)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_PREEDIT_CARET(GXimProtocol *proto,
					     guint16       imid,
					     guint16       icid,
					     gint32        position,
					     guint32       direction,
					     guint32       style)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_PREEDIT_DONE(GXimProtocol *proto,
					    guint16       imid,
					    guint16       icid)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_STATUS_START(GXimProtocol *proto,
					    guint16       imid,
					    guint16       icid)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_STATUS_DRAW(GXimProtocol *proto,
					   guint16       imid,
					   guint16       icid,
					   guint32       type,
					   const gchar  *packets,
					   gsize        *length)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_STATUS_DONE(GXimProtocol *proto,
					   guint16       imid,
					   guint16       icid)
{
	/* FIXME */
	return FALSE;
}

static gboolean
g_xim_client_protocol_real_XIM_PREEDITSTATE(GXimProtocol *proto,
					    guint16       imid,
					    guint16       icid,
					    guint32       mask)
{
	/* FIXME */
	return FALSE;
}

static GdkWindow *
g_xim_client_transport_real_do_create_channel(GXimTransport *trans,
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
g_xim_client_transport_real_do_get_native_channel(GXimTransport *trans,
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
g_xim_client_transport_real_do_destroy_channel(GXimTransport *trans)
{
	GdkWindow *w = g_xim_transport_get_channel(trans, NULL);

	gdk_window_destroy(w);
}

static gboolean
g_xim_client_transport_real_do_send_via_property(GXimTransport *trans,
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
g_xim_client_transport_real_do_send_via_cm(GXimTransport *trans,
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
g_xim_client_transport_real_do_send_via_property_notify(GXimTransport *trans,
						       const gchar   *data,
						       gsize          length)
{
	/* FIXME */

	return FALSE;
}

static gboolean
g_xim_client_transport_real_do_get_property(GXimTransport  *trans,
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
g_xim_client_connection_real_protocol_init(GXimProtocol *proto)
{
	GXimProtocolPrivate *priv = g_xim_protocol_get_private(proto);

	priv->byte_order = g_xim_get_byte_order();

#define CONNECT(_m_,_n_)						\
	priv->signal_ids[G_ ## _m_] = g_xim_protocol_connect_closure_by_id(proto, \
									   G_ ## _m_, \
									   (_n_), \
									   G_CALLBACK (g_xim_client_protocol_real_ ## _m_), \
									   NULL);

	CONNECT (XIM_CONNECT_REPLY, 0);
	CONNECT (XIM_DISCONNECT_REPLY, 0);
	CONNECT (XIM_AUTH_REQUIRED, 0);
	CONNECT (XIM_AUTH_NEXT, 0);
	CONNECT (XIM_AUTH_SETUP, 0);
	CONNECT (XIM_AUTH_NG, 0);
	CONNECT (XIM_ERROR, 0);
	CONNECT (XIM_OPEN_REPLY, 0);
	CONNECT (XIM_CLOSE_REPLY, 0);
	CONNECT (XIM_REGISTER_TRIGGERKEYS, 0);
	CONNECT (XIM_TRIGGER_NOTIFY_REPLY, 0);
	CONNECT (XIM_SET_EVENT_MASK, 0);
	CONNECT (XIM_ENCODING_NEGOTIATION_REPLY, 0);
	CONNECT (XIM_QUERY_EXTENSION_REPLY, 0);
	CONNECT (XIM_SET_IM_VALUES_REPLY, 0);
	CONNECT (XIM_GET_IM_VALUES_REPLY, 0);
	CONNECT (XIM_CREATE_IC_REPLY, 0);
	CONNECT (XIM_DESTROY_IC_REPLY, 0);
	CONNECT (XIM_SET_IC_VALUES_REPLY, 0);
	CONNECT (XIM_GET_IC_VALUES_REPLY, 0);
	CONNECT (XIM_FORWARD_EVENT, 0);
	CONNECT (XIM_SYNC, 0);
	CONNECT (XIM_SYNC_REPLY, 0);
	CONNECT (XIM_COMMIT, 0);
	CONNECT (XIM_RESET_IC_REPLY, 0);
	CONNECT (XIM_GEOMETRY, 0);
	CONNECT (XIM_STR_CONVERSION, 0);
	CONNECT (XIM_PREEDIT_START, 0);
	CONNECT (XIM_PREEDIT_DRAW, 0);
	CONNECT (XIM_PREEDIT_CARET, 0);
	CONNECT (XIM_PREEDIT_DONE, 0);
	CONNECT (XIM_STATUS_START, 0);
	CONNECT (XIM_STATUS_DRAW, 0);
	CONNECT (XIM_STATUS_DONE, 0);
	CONNECT (XIM_PREEDITSTATE, 0);

#undef CONNECT
}

static void
g_xim_client_connection_real_transport_init(GXimTransport *trans)
{
	GXimTransportIface *iface = G_XIM_TRANSPORT_GET_IFACE (trans);

	iface->do_create_channel           = g_xim_client_transport_real_do_create_channel;
	iface->do_get_native_channel       = g_xim_client_transport_real_do_get_native_channel;
	iface->do_destroy_channel          = g_xim_client_transport_real_do_destroy_channel;
	iface->do_send_via_property        = g_xim_client_transport_real_do_send_via_property;
	iface->do_send_via_cm              = g_xim_client_transport_real_do_send_via_cm;
	iface->do_send_via_property_notify = g_xim_client_transport_real_do_send_via_property_notify;
	iface->do_get_property             = g_xim_client_transport_real_do_get_property;
	g_xim_transport_set_direction(trans, G_XIM_DIR_LEFT);
}

static void
g_xim_client_connection_class_init(GXimClientConnectionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GXimConnectionClass *conn_class = G_XIM_CONNECTION_CLASS (klass);

	object_class->set_property = g_xim_client_connection_real_set_property;
	object_class->get_property = g_xim_client_connection_real_get_property;
	object_class->finalize     = g_xim_client_connection_real_finalize;

	conn_class->protocol_init  = g_xim_client_connection_real_protocol_init;
	conn_class->transport_init = g_xim_client_connection_real_transport_init;

	/* properties */

	/* signals */
}

static void
g_xim_client_connection_init(GXimClientConnection *conn)
{
}

/*
 * Public functions
 */
gboolean
g_xim_client_connection_cmd_connect(GXimClientConnection *conn,
				    guint16               protocol_major_version,
				    guint16               protocol_minor_version,
				    GSList               *auth_list,
				    gboolean              is_async)
{
	GXimProtocol *proto;
	GXimProtocolPrivate *priv;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	priv = g_xim_protocol_get_private(proto);
	retval = g_xim_protocol_send(proto, G_XIM_CONNECT, 0,
				     6,
				     G_XIM_TYPE_BYTE, priv->byte_order == G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN ? 0x42 : 0x6c,
				     G_XIM_TYPE_BYTE, 0,
				     G_XIM_TYPE_WORD, protocol_major_version,
				     G_XIM_TYPE_WORD, protocol_minor_version,
				     G_XIM_TYPE_WORD, g_slist_length(auth_list),
				     G_XIM_TYPE_LIST_OF_STRING, auth_list);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_CONNECT_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_disconnect(GXimClientConnection *conn,
				       gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_DISCONNECT, 0, 0);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_DISCONNECT_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_open_im(GXimClientConnection *conn,
				    const GXimStr        *locale,
				    gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);
	g_return_val_if_fail (locale != NULL, FALSE);

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_OPEN, 0,
				     2,
				     G_XIM_TYPE_STR, locale,
				     G_XIM_TYPE_AUTO_PADDING, 0);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_OPEN_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_close_im(GXimClientConnection *conn,
				     guint16               imid,
				     gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_CLOSE, 0,
				     2,
				     G_XIM_TYPE_WORD, imid,
				     G_XIM_TYPE_PADDING, 2);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_CLOSE_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_trigger_notify(GXimClientConnection *conn,
					   guint16               imid,
					   guint16               icid,
					   guint32               flag,
					   guint32               index_,
					   guint32               event_mask,
					   gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_TRIGGER_NOTIFY, 0,
				     5,
				     G_XIM_TYPE_WORD, imid,
				     G_XIM_TYPE_WORD, icid,
				     G_XIM_TYPE_LONG, flag,
				     G_XIM_TYPE_LONG, index_,
				     G_XIM_TYPE_LONG, event_mask);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_TRIGGER_NOTIFY_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_encoding_negotiation(GXimClientConnection *conn,
						 guint16               imid,
						 GSList               *encodings,
						 GSList               *details,
						 gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;
	GArray *e;
	GSList *l;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	if ((e = G_XIM_CONNECTION (conn)->encodings))
		g_array_unref(e);
	if ((e = G_XIM_CONNECTION (conn)->encoding_details))
		g_array_unref(e);
	e = G_XIM_CONNECTION (conn)->encodings = g_array_new(TRUE, TRUE, g_slist_length(encodings));
	for (l = encodings; l != NULL; l = g_slist_next(l)) {
		GValue v = { 0, };

		g_value_init(&v, G_TYPE_XIM_STR);
		g_value_set_boxed(&v, l->data);
		g_array_append_vals(e, &v, 1);
	}
	e = G_XIM_CONNECTION (conn)->encoding_details = g_array_new(TRUE, TRUE, g_slist_length(details));
	for (l = details; l != NULL; l = g_slist_next(l)) {
		GValue v = { 0, };

		g_value_init(&v, G_TYPE_XIM_ENCODINGINFO);
		g_value_set_boxed(&v, l->data);
		g_array_append_vals(e, &v, 1);
	}

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_ENCODING_NEGOTIATION, 0,
				     7,
				     G_XIM_TYPE_WORD, imid,
				     G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_STR,
				     G_XIM_TYPE_LIST_OF_STR, encodings,
				     G_XIM_TYPE_AUTO_PADDING, 0,
				     G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_ENCODINGINFO,
				     G_XIM_TYPE_PADDING, 2,
				     G_XIM_TYPE_LIST_OF_ENCODINGINFO, details);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_ENCODING_NEGOTIATION_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_query_extension(GXimClientConnection *conn,
					    guint16               imid,
					    const GSList         *extensions,
					    gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_QUERY_EXTENSION, 0,
				     4,
				     G_XIM_TYPE_WORD, imid,
				     G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_STR,
				     G_XIM_TYPE_LIST_OF_STR, extensions,
				     G_XIM_TYPE_AUTO_PADDING, 0);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_QUERY_EXTENSION_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_set_im_values(GXimClientConnection *conn,
					  guint16               imid,
					  const GSList         *attributes,
					  gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_SET_IM_VALUES, 0,
				     3,
				     G_XIM_TYPE_WORD, imid,
				     G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_IMATTRIBUTE,
				     G_XIM_TYPE_LIST_OF_IMATTRIBUTE, attributes);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_SET_IM_VALUES_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_get_im_values(GXimClientConnection *conn,
					  guint16               imid,
					  const GSList         *attr_id,
					  gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_GET_IM_VALUES, 0,
				     4,
				     G_XIM_TYPE_WORD, imid,
				     G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_CARD16,
				     G_XIM_TYPE_LIST_OF_CARD16, attr_id,
				     G_XIM_TYPE_AUTO_PADDING, 0);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_GET_IM_VALUES_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_create_ic(GXimClientConnection *conn,
				      guint16               imid,
				      const GSList         *attributes,
				      gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_CREATE_IC, 0,
				     3,
				     G_XIM_TYPE_WORD, imid,
				     G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_ICATTRIBUTE,
				     G_XIM_TYPE_LIST_OF_ICATTRIBUTE, attributes);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_CREATE_IC_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_destroy_ic(GXimClientConnection *conn,
				       guint16               imid,
				       guint16               icid,
				       gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_DESTROY_IC, 0,
				     2,
				     G_XIM_TYPE_WORD, imid,
				     G_XIM_TYPE_WORD, icid);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_DESTROY_IC_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_set_ic_values(GXimClientConnection *conn,
					  guint16               imid,
					  guint16               icid,
					  const GSList         *attributes,
					  gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_SET_IC_VALUES, 0,
				     5,
				     G_XIM_TYPE_WORD, imid,
				     G_XIM_TYPE_WORD, icid,
				     G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_ICATTRIBUTE,
				     G_XIM_TYPE_PADDING, 2,
				     G_XIM_TYPE_LIST_OF_ICATTRIBUTE, attributes);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_SET_IC_VALUES_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_get_ic_values(GXimClientConnection *conn,
					  guint16               imid,
					  guint16               icid,
					  const GSList         *attr_id,
					  gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_GET_IC_VALUES, 0,
				     5,
				     G_XIM_TYPE_WORD, imid,
				     G_XIM_TYPE_WORD, icid,
				     G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_CARD16,
				     G_XIM_TYPE_LIST_OF_CARD16, attr_id,
				     G_XIM_TYPE_AUTO_PADDING, 2);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_GET_IC_VALUES_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_set_ic_focus(GXimClientConnection *conn,
					 guint16               imid,
					 guint16               icid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	return g_xim_protocol_send(proto, G_XIM_SET_IC_FOCUS, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid);
}

gboolean
g_xim_client_connection_cmd_unset_ic_focus(GXimClientConnection *conn,
					   guint16               imid,
					   guint16               icid)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	return g_xim_protocol_send(proto, G_XIM_UNSET_IC_FOCUS, 0,
				   2,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid);
}

gboolean
g_xim_client_connection_cmd_sync(GXimClientConnection *conn,
				 guint16               imid,
				 guint16               icid,
				 gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_SYNC, 0,
				     2,
				     G_XIM_TYPE_WORD, imid,
				     G_XIM_TYPE_WORD, icid);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_SYNC_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_reset_ic(GXimClientConnection *conn,
				     guint16               imid,
				     guint16               icid,
				     gboolean              is_async)
{
	GXimProtocol *proto;
	gboolean retval;
	GError *error = NULL;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	retval = g_xim_protocol_send(proto, G_XIM_RESET_IC, 0,
				     2,
				     G_XIM_TYPE_WORD, imid,
				     G_XIM_TYPE_WORD, icid);
	if (retval && !is_async) {
		retval = g_xim_protocol_wait_for_reply(proto, G_XIM_RESET_IC_REPLY, 0, &error);
		if (!retval && error) {
			g_xim_messages_gerror(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      error);
			g_error_free(error);
		}
	}

	return retval;
}

gboolean
g_xim_client_connection_cmd_preedit_start_reply(GXimClientConnection *conn,
						guint16               imid,
						guint16               icid,
						gint32                return_value)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	return g_xim_protocol_send(proto, G_XIM_PREEDIT_START_REPLY, 0,
				   3,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid,
				   G_XIM_TYPE_LONG, return_value);
}

gboolean
g_xim_client_connection_cmd_preedit_caret_reply(GXimClientConnection *conn,
						guint16               imid,
						guint16               icid,
						guint32               position)
{
	GXimProtocol *proto;

	g_return_val_if_fail (G_IS_XIM_CLIENT_CONNECTION (conn), FALSE);

	proto = G_XIM_PROTOCOL (conn);
	return g_xim_protocol_send(proto, G_XIM_PREEDIT_CARET_REPLY, 0,
				   3,
				   G_XIM_TYPE_WORD, imid,
				   G_XIM_TYPE_WORD, icid,
				   G_XIM_TYPE_LONG, position);
}
