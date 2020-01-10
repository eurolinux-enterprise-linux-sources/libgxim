/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximtransport.c
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

#include "gximmisc.h"
#include "gximmessages.h"
#include "gximtransport.h"

/*
 * Private functions
 */
static GdkWindow *
g_xim_transport_real_do_create_channel(GXimTransport *trans,
				       GdkWindow     *parent_window)
{
	g_xim_messages_bug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
			   "Interface `%s' for `%s' isn't implemented.",
			   g_type_name(G_TYPE_FROM_INSTANCE (trans)),
			   "do_create_channel");

	return NULL;
}

static GdkNativeWindow
g_xim_transport_real_do_get_native_channel(GXimTransport *trans,
					   gpointer       drawable)
{
	g_xim_messages_bug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
			   "Interface `%s' for `%s' isn't implemented.",
			   g_type_name(G_TYPE_FROM_INSTANCE (trans)),
			   "do_get_native_channel");

	return 0;
}

static void
g_xim_transport_real_do_destroy_channel(GXimTransport *trans)
{
	g_xim_messages_bug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
			   "Interface `%s' for `%s' isn't implemented.",
			   g_type_name(G_TYPE_FROM_INSTANCE (trans)),
			   "do_destroy_channel");
}

static gboolean
g_xim_transport_real_do_send_via_property(GXimTransport *trans,
					  const gchar   *data,
					  gsize          length)
{
	g_xim_messages_bug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
			   "Interface `%s' for `%s' isn't implemented.",
			   g_type_name(G_TYPE_FROM_INSTANCE (trans)),
			   "do_send_via_property");

	return FALSE;
}

static gboolean
g_xim_transport_real_do_send_via_cm(GXimTransport *trans,
				    const gchar   *data,
				    gsize          length,
				    gsize          threshold)
{
	g_xim_messages_bug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
			   "Interface `%s' for `%s' isn't implemented.",
			   g_type_name(G_TYPE_FROM_INSTANCE (trans)),
			   "do_send_via_cm");

	return FALSE;
}

static gboolean
g_xim_transport_real_do_send_via_property_notify(GXimTransport *trans,
						 const gchar   *data,
						 gsize          length)
{
	g_xim_messages_bug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
			   "Interface `%s' for `%s' isn't implemented.",
			   g_type_name(G_TYPE_FROM_INSTANCE (trans)),
			   "do_send_via_property_notify");

	return FALSE;
}

static gboolean
g_xim_transport_real_do_get_property(GXimTransport  *trans,
					    GdkWindow      *window,
					    GdkAtom         property,
					    GdkAtom         type,
					    gulong          length,
					    GdkAtom        *actual_property_type,
					    gint           *actual_format,
					    gint           *actual_length,
					    guchar        **data)
{
	g_xim_messages_bug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
			   "Interface `%s' for `%s' isn't implemented.",
			   g_type_name(G_TYPE_FROM_INSTANCE (trans)),
			   "do_send_via_property_notify");

	return FALSE;
}

static void
g_xim_transport_base_class_init(gpointer g_iface)
{
	GXimTransportIface *iface = g_iface;
	static gboolean initialized = FALSE;

	if (!initialized) {
		initialized = TRUE;
		iface->atom_xim_protocol = gdk_atom_intern_static_string("_XIM_PROTOCOL");
		iface->atom_xim_moredata = gdk_atom_intern_static_string("_XIM_MOREDATA");
		iface->message = g_xim_messages_new();
	} else {
		g_object_ref(iface->message);
	}
	iface->do_create_channel           = g_xim_transport_real_do_create_channel;
	iface->do_get_native_channel       = g_xim_transport_real_do_get_native_channel;
	iface->do_destroy_channel          = g_xim_transport_real_do_destroy_channel;
	iface->do_send_via_property        = g_xim_transport_real_do_send_via_property;
	iface->do_send_via_cm              = g_xim_transport_real_do_send_via_cm;
	iface->do_send_via_property_notify = g_xim_transport_real_do_send_via_property_notify;
	iface->do_get_property             = g_xim_transport_real_do_get_property;
}

static void
g_xim_transport_base_class_finalize(gpointer g_iface)
{
	GXimTransportIface *iface = g_iface;

	g_object_unref(iface->message);
}

/*
 * Public functions
 */
GType
g_xim_transport_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;
		GTypeInfo info = {
			.class_size     = sizeof (GXimTransportIface),
			.base_init      = (GBaseInitFunc)g_xim_transport_base_class_init,
			.base_finalize  = (GBaseFinalizeFunc)g_xim_transport_base_class_finalize,
			.class_init     = NULL,
			.class_finalize = NULL,
			.class_data     = NULL,
			.instance_size  = 0,
			.n_preallocs    = 0,
			.instance_init  = NULL,
			.value_table    = NULL
		};

		type_id = g_type_register_static(G_TYPE_INTERFACE, "GXimTransport",
						 &info, 0);

		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

void
g_xim_transport_init(GXimTransport *trans)
{
	GXimTransportPrivate *priv;

	g_return_if_fail (G_IS_XIM_TRANSPORT (trans));

	priv = g_new0(GXimTransportPrivate, 1);
	G_XIM_CHECK_ALLOC_WITH_NO_RET (priv);

	priv->atom_comm = GDK_NONE;
	priv->transport_size = G_XIM_TRANSPORT_SIZE;
	priv->transport_max = G_XIM_TRANSPORT_MAX;
	priv->direction = G_XIM_DIR_RIGHT;
	priv->prop_offset = g_hash_table_new(g_direct_hash, g_direct_equal);

	g_object_set_data(G_OBJECT (trans), "libgxim-transport-private", priv);
}

void
g_xim_transport_finalize(GXimTransport *trans)
{
	GXimTransportPrivate *priv;

	g_return_if_fail (G_IS_XIM_TRANSPORT (trans));

	priv = g_xim_transport_get_private(trans);
	if (priv) {
		G_XIM_TRANSPORT_GET_IFACE (trans)->do_destroy_channel(trans);

		g_object_unref(priv->display);
		g_hash_table_destroy(priv->prop_offset);

		g_free(priv);
	}
}

GXimTransportPrivate *
g_xim_transport_get_private(GXimTransport *trans)
{
	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), NULL);

	return g_object_get_data(G_OBJECT (trans), "libgxim-transport-private");
}

void
g_xim_transport_set_version(GXimTransport *trans,
			    guint8         major_version,
			    guint8         minor_version)
{
	GXimTransportPrivate *priv;

	g_return_if_fail (G_IS_XIM_TRANSPORT (trans));

	priv = g_xim_transport_get_private(trans);
	g_object_set_data(G_OBJECT (trans),
			  "libgxim-transport-version",
			  GUINT_TO_POINTER (TRUE));
	priv->major_version = major_version;
	priv->minor_version = minor_version;
}

gboolean
g_xim_transport_get_version(GXimTransport *trans,
			    guint8        *major_version,
			    guint8        *minor_version)
{
	gboolean retval;

	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), FALSE);

	retval = GPOINTER_TO_UINT (g_object_get_data(G_OBJECT (trans),
						     "libgxim-transport-version"));

	if (retval) {
		GXimTransportPrivate *priv = g_xim_transport_get_private(trans);

		if (major_version)
			*major_version = priv->major_version;
		if (minor_version)
			*minor_version = priv->minor_version;
	}

	return retval;
}

void
g_xim_transport_set_transport_size(GXimTransport *trans,
				   gsize          size)
{
	GXimTransportPrivate *priv;

	g_return_if_fail (G_IS_XIM_TRANSPORT (trans));

	priv = g_xim_transport_get_private(trans);
	priv->transport_size = size;
}

gsize
g_xim_transport_get_transport_size(GXimTransport *trans)
{
	GXimTransportPrivate *priv;

	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), G_XIM_TRANSPORT_SIZE);

	priv = g_xim_transport_get_private(trans);

	return priv->transport_size;
}

void
g_xim_transport_set_transport_max(GXimTransport *trans,
				  gsize          size)
{
	GXimTransportPrivate *priv;

	g_return_if_fail (G_IS_XIM_TRANSPORT (trans));

	priv = g_xim_transport_get_private(trans);
	priv->transport_max = size;
}

gsize
g_xim_transport_get_transport_max(GXimTransport *trans)
{
	GXimTransportPrivate *priv;

	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), G_XIM_TRANSPORT_MAX);

	priv = g_xim_transport_get_private(trans);

	return priv->transport_max;
}

void
g_xim_transport_set_display(GXimTransport *trans,
			    GdkDisplay    *dpy)
{
	GXimTransportPrivate *priv;

	g_return_if_fail (G_IS_XIM_TRANSPORT (trans));

	if (g_xim_transport_get_display(trans) != NULL) {
		g_xim_messages_bug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
				   "GdkDisplay is already set to the transport class.");
	} else {
		gchar *i = g_strdup_printf("%p:%p", trans, dpy);
		gchar *s = g_compute_checksum_for_string(G_CHECKSUM_SHA256,
							 i, -1);
		gchar *n = g_strdup_printf("_LIBGXIM_COMM_%s", s);

		priv = g_xim_transport_get_private(trans);
		priv->display = g_object_ref(dpy);
		priv->atom_comm = gdk_atom_intern(n, FALSE);

		g_free(n);
		g_free(s);
		g_free(i);
	}
}

GdkDisplay *
g_xim_transport_get_display(GXimTransport *trans)
{
	GXimTransportPrivate *priv;

	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), NULL);

	priv = g_xim_transport_get_private(trans);

	return priv->display;
}

GdkAtom
g_xim_transport_get_atom(GXimTransport *trans)
{
	GXimTransportPrivate *priv;
	GdkAtom retval;

	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), GDK_NONE);

	priv = g_xim_transport_get_private(trans);
	retval = priv->atom_comm;
	if (retval == GDK_NONE) {
		g_xim_messages_warning(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
				       "Display has to be set first.");
	}

	return retval;
}

void
g_xim_transport_set_client_window(GXimTransport   *trans,
				  GdkNativeWindow  client_window)
{
	GXimTransportPrivate *priv;
	GdkNativeWindow w;

	g_return_if_fail (G_IS_XIM_TRANSPORT (trans));
	g_return_if_fail (client_window != 0);

	if ((w = g_xim_transport_get_client_window(trans))) {
		g_xim_messages_bug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
				   "Client window has already been set:\n  old: %p, new: %p.",
				   G_XIM_NATIVE_WINDOW_TO_POINTER (w),
				   G_XIM_NATIVE_WINDOW_TO_POINTER (client_window));
	} else {
		priv = g_xim_transport_get_private(trans);
		priv->client_window = client_window;
	}
}

GdkNativeWindow
g_xim_transport_get_client_window(GXimTransport *trans)
{
	GXimTransportPrivate *priv;

	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), 0);

	priv = g_xim_transport_get_private(trans);

	return priv->client_window;
}

GdkNativeWindow
g_xim_transport_get_native_channel(GXimTransport *trans)
{
	GdkWindow *w;

	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), 0);

	w = g_xim_transport_get_channel(trans, NULL);
	if (w == NULL) {
		g_xim_messages_warning(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
				       "No valid channel available for %s:%p",
				       g_type_name(G_TYPE_FROM_INSTANCE (trans)),
				       trans);

		return 0;
	}

	return g_xim_transport_get_native_channel_from(trans, w);
}

GdkNativeWindow
g_xim_transport_get_native_channel_from(GXimTransport *trans,
					gpointer       drawable)
{
	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), 0);
	g_return_val_if_fail (GDK_IS_DRAWABLE (drawable), 0);

	return G_XIM_TRANSPORT_GET_IFACE (trans)->do_get_native_channel(trans, drawable);
}

GdkWindow *
g_xim_transport_get_channel(GXimTransport *trans,
			    GdkWindow     *parent_window)
{
	GXimTransportPrivate *priv;

	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), NULL);

	priv = g_xim_transport_get_private(trans);
	if (priv->comm_window == NULL && parent_window != NULL) {
		if (G_XIM_TRANSPORT_GET_IFACE (trans)->do_create_channel) {
			GdkWindow *v = NULL;

			v = G_XIM_TRANSPORT_GET_IFACE (trans)->do_create_channel(trans,
										 parent_window);
			priv->comm_window = v;
		} else {
			g_xim_messages_bug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
					   "No implementation of do_create_channel");
		}
	}

	return priv->comm_window;
}

gboolean
g_xim_transport_send_via_property(GXimTransport *trans,
				  const gchar   *data,
				  gsize          length)
{
	guint8 major, minor;
	gboolean retval;

	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), FALSE);
	g_return_val_if_fail (data != NULL, FALSE);
	g_return_val_if_fail (g_xim_transport_get_version(trans, &major, &minor), FALSE);

	if (major == 0 && minor != 1) {
		if (G_XIM_TRANSPORT_GET_IFACE (trans)->do_send_via_property) {
			GdkNativeWindow client_window = g_xim_transport_get_client_window(trans);
			GdkNativeWindow comm_window = g_xim_transport_get_native_channel(trans);

			retval = G_XIM_TRANSPORT_GET_IFACE (trans)->do_send_via_property(trans,
											 data,
											 length);
			g_xim_messages_debug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
					     "transport",
					     "              Sending through Property[%p->%p]\n  result:%s",
					     G_XIM_NATIVE_WINDOW_TO_POINTER (comm_window),
					     G_XIM_NATIVE_WINDOW_TO_POINTER (client_window),
					     retval ? "success" : "failed");
			g_xim_transport_dump(trans, data, length, TRUE);

			return retval;
		}
	}

	g_xim_messages_bug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
			   "No implementation or wrongly bringing up to send packets through Property:\n"
			   "  transport version: %d.%d", major, minor);

	return FALSE;
}

gboolean
g_xim_transport_send_via_cm(GXimTransport *trans,
			    const gchar   *data,
			    gsize          length,
			    gsize          threshold)
{
	guint8 major, minor;
	gboolean retval;

	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), FALSE);
	g_return_val_if_fail (data != NULL, FALSE);
	g_return_val_if_fail (g_xim_transport_get_version(trans, &major, &minor), FALSE);

	if (major == 0|| major == 2) {
		if (G_XIM_TRANSPORT_GET_IFACE (trans)->do_send_via_cm) {
			GdkNativeWindow client_window = g_xim_transport_get_client_window(trans);
			GdkNativeWindow comm_window = g_xim_transport_get_native_channel(trans);

			retval = G_XIM_TRANSPORT_GET_IFACE (trans)->do_send_via_cm(trans,
										   data,
										   length,
										   threshold);
			g_xim_messages_debug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
					     "transport",
					     "              Sending through ClientMessage[%p->%p]\n  result:%s",
					     G_XIM_NATIVE_WINDOW_TO_POINTER (comm_window),
					     G_XIM_NATIVE_WINDOW_TO_POINTER (client_window),
					     retval ? "success" : "failed");
			g_xim_transport_dump(trans, data, length, TRUE);

			return retval;
		}
	}

	g_xim_messages_bug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
			   "No implementation or wrongly bringing up to send packets through ClientMessage:\n"
			   "  transport version: %d.%d", major, minor);

	return FALSE;
}

gboolean
g_xim_transport_send_via_property_notify(GXimTransport *trans,
					 const gchar   *data,
					 gsize          length)
{
	guint8 major, minor;
	gboolean retval;

	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), FALSE);
	g_return_val_if_fail (data != NULL, FALSE);
	g_return_val_if_fail (g_xim_transport_get_version(trans, &major, &minor), FALSE);

	if (major == 1 || major == 2) {
		if (G_XIM_TRANSPORT_GET_IFACE (trans)->do_send_via_property_notify) {
			GdkNativeWindow client_window = g_xim_transport_get_client_window(trans);
			GdkNativeWindow comm_window = g_xim_transport_get_native_channel(trans);

			retval = G_XIM_TRANSPORT_GET_IFACE (trans)->do_send_via_property_notify(trans,
												data,
												length);
			g_xim_messages_debug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
					     "transport",
					     "              Sending through PropertyNotify[%p->%p]\n  result:%s",
					     G_XIM_NATIVE_WINDOW_TO_POINTER (comm_window),
					     G_XIM_NATIVE_WINDOW_TO_POINTER (client_window),
					     retval ? "success" : "failed");
			g_xim_transport_dump(trans, data, length, TRUE);

			return retval;
		}
	}

	g_xim_messages_bug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
			   "No implementation or wrongly bringing up to send packets through PropertyNotify:\n"
			   "  transport version: %d.%d", major, minor);

	return FALSE;
}

void
g_xim_transport_set_direction(GXimTransport *trans,
			      GXimDirection  direction)
{
	GXimTransportPrivate *priv;

	g_return_if_fail (G_IS_XIM_TRANSPORT (trans));

	priv = g_xim_transport_get_private(trans);
	priv->direction = direction;
}

GXimDirection
g_xim_transport_get_direction(GXimTransport *trans)
{
	GXimTransportPrivate *priv;

	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), G_XIM_DIR_RIGHT);

	priv = g_xim_transport_get_private(trans);

	return priv->direction;
}

gboolean
g_xim_transport_get_property(GXimTransport  *trans,
			     GdkWindow      *window,
			     GdkAtom         property,
			     GdkAtom         type,
			     gulong          length,
			     GdkAtom        *actual_property_type,
			     gint           *actual_format,
			     gint           *actual_length,
			     guchar        **data)
{
	g_return_val_if_fail (G_IS_XIM_TRANSPORT (trans), FALSE);
	g_return_val_if_fail (GDK_IS_WINDOW (window), FALSE);
	g_return_val_if_fail (property != GDK_NONE, FALSE);
	g_return_val_if_fail (data != NULL, FALSE);

	return G_XIM_TRANSPORT_GET_IFACE (trans)->do_get_property(trans,
								  window,
								  property,
								  type,
								  length,
								  actual_property_type,
								  actual_format,
								  actual_length,
								  data);
}

void
g_xim_transport_dump(GXimTransport *trans,
		     const gchar   *data,
		     gsize          length,
		     gboolean       is_sent)
{
	gsize i, j;
	GdkNativeWindow client_window;
	static const gchar *arrow_right[] = { "->", "<-" };
	static const gchar *arrow_left[] = { "<-", "->" };
	const gchar **arrow;
	GXimDirection direction = g_xim_transport_get_direction(trans);
	GString *msg, *visible;

	if (direction == G_XIM_DIR_RIGHT)
		arrow = arrow_right;
	else
		arrow = arrow_left;
	client_window = g_xim_transport_get_client_window(trans);
	g_xim_messages_debug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
			     "transport/summary",
			     "%p %s opcode: %s(major: %d, minor %d) length: %" G_GSIZE_FORMAT,
			     G_XIM_NATIVE_WINDOW_TO_POINTER (client_window),
			     is_sent ? arrow[1] : arrow[0],
			     g_xim_protocol_name((guint16)data[0]),
			     data[0], data[1], length);
	g_xim_messages_debug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
			     "transport/dump",
			     " 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 0123456789ABCDEF");
	g_xim_messages_debug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
			     "transport/dump",
			     "--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--=----------------");

	msg = g_string_new(NULL);
	visible = g_string_new(NULL);
	G_XIM_CHECK_ALLOC_WITH_NO_RET (msg);
	G_XIM_CHECK_ALLOC_WITH_NO_RET (visible);

	for (i = 0; i < length; i += 16) {
		for (j = 0; (j + i) < length && j < 16; j++) {
			if (msg->len > 0)
				g_string_append_printf(msg, ":");
			g_string_append_printf(msg, "%02x", data[j + i] & 0xff);
			if (g_ascii_isprint(data[j + i] & 0xff))
				g_string_append_c(visible, data[j + i] & 0xff);
			else
				g_string_append_c(visible, '.');
		}
		for (; j < 16; j++) {
			g_string_append(msg, "   ");
		}
		g_xim_messages_debug(G_XIM_TRANSPORT_GET_IFACE (trans)->message,
				     "transport/dump",
				     "%s %s",
				     msg->str, visible->str);
		g_string_truncate(msg, 0);
		g_string_truncate(visible, 0);
	}
	g_string_free(msg, TRUE);
	g_string_free(visible, TRUE);
}
