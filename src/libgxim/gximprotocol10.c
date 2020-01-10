/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximprotocol10.c
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
#include <gdk/gdkkeysyms.h>
#include <gio/gio.h>
#include "gximattr.h"
#include "gximconnection.h"
#include "gximsrvconn.h"
#include "gximmarshal.h"
#include "gximmessages.h"
#include "gximmisc.h"
#include "gximprotocol.h"
#include "gximprotocol10.h"

#define PROTO_ERROR(_m_,_n_)			(g_xim_protocol_raise_parser_error(proto, G_ ## _m_, (_n_), 0, 0))
#define PROTO_ERROR_IM(_m_,_n_,_imid_)		(g_xim_protocol_raise_parser_error(proto, G_ ##_m_, (_n_), (_imid_), 0))
#define PROTO_ERROR_IMIC(_m_,_n_,_imid_,_icid_)	(g_xim_protocol_raise_parser_error(proto, G_ ##_m_, (_n_), (_imid_), (_icid_)))
#define SIG_NOIMPL(_s_)				(g_xim_protocol10_closure_signal_no_impl(proto, #_s_, G_XIM_EMASK_NO_VALID_ID, 0, 0))
#define SIG_NOIMPL_IM(_s_,_imid_)		(g_xim_protocol10_closure_signal_no_impl(proto, #_s_, G_XIM_EMASK_VALID_IMID, (_imid_), 0))
#define SIG_NOIMPL_IMIC(_s_,_imid_,_icid_)	(g_xim_protocol10_closure_signal_no_impl(proto, #_s_, G_XIM_EMASK_VALID_IMID|G_XIM_EMASK_VALID_ICID, (_imid_), (_icid_)))
#define MSG_NOIMPL(_s_)				(g_xim_messages_error(G_XIM_PROTOCOL_GET_IFACE (proto)->message, "No implementation or an error occurred in %s", #_s_))
#define MSG_NOIMPL_IM(_s_,_imid_)		(g_xim_messages_error(G_XIM_PROTOCOL_GET_IFACE (proto)->message, "No implementation or an error occurred in %s [imid: %d]", #_s_, (_imid_)))
#define MSG_NOIMPL_IMIC(_s_,_imid_,_icid_)	(g_xim_messages_error(G_XIM_PROTOCOL_GET_IFACE (proto)->message, "No implementation or an error occurred in %s [imid: %d, icid: %d]", #_s_, (_imid_), (_icid_)))


/*
 * Private functions
 */
static gboolean
g_xim_protocol10_closure_XIM_CONNECT(GXimProtocolClosure  *closure,
				     GXimProtocol         *proto,
				     GDataInputStream     *stream,
				     GError              **error,
				     gpointer              user_data)
{
	guint8 b;
	guint16 major = 0, minor = 0;
	GSList *list = NULL;
	gboolean retval = FALSE;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 6,
				       G_XIM_TYPE_BYTE, &b,
				       G_XIM_TYPE_PADDING, 1,
				       G_XIM_TYPE_WORD, &major,
				       G_XIM_TYPE_WORD, &minor,
				       G_XIM_TYPE_MARKER_N_ITEMS_2, G_XIM_TYPE_LIST_OF_STRING,
				       G_XIM_TYPE_LIST_OF_STRING, &list)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    major, minor, list);
	} else {
		/* better handling error */
		PROTO_ERROR (XIM_CONNECT, 0);
	}

	g_slist_foreach(list, (GFunc)g_xim_string_free, NULL);
	g_slist_free(list);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_CONNECT_REPLY(GXimProtocolClosure  *closure,
					   GXimProtocol         *proto,
					   GDataInputStream     *stream,
					   GError              **error,
					   gpointer              user_data)
{
	guint16 major, minor;
	gboolean retval = FALSE;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &major,
				       G_XIM_TYPE_WORD, &minor)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    major, minor);
	} else {
		/* better handling error */
		PROTO_ERROR (XIM_CONNECT_REPLY, 0);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_DISCONNECT(GXimProtocolClosure  *closure,
					GXimProtocol         *proto,
					GDataInputStream     *stream,
					GError              **error,
					gpointer              user_data)
{
	gboolean retval = FALSE;

	retval = g_xim_protocol_closure_emit_signal(closure, proto);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_DISCONNECT_REPLY(GXimProtocolClosure  *closure,
					      GXimProtocol         *proto,
					      GDataInputStream     *stream,
					      GError              **error,
					      gpointer              user_data)
{
	GXimProtocolPrivate *priv;
	gboolean retval = FALSE;

	priv = g_xim_protocol_get_private(proto);

	retval = g_xim_protocol_closure_emit_signal(closure, proto);

	priv->is_disconnected = TRUE;

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_AUTH_REQUIRED(GXimProtocolClosure  *closure,
					   GXimProtocol         *proto,
					   GDataInputStream     *stream,
					   GError              **error,
					   gpointer              user_data)
{
	gboolean retval = FALSE;

	/* XXX */
	PROTO_ERROR (XIM_AUTH_REQUIRED, 0);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_AUTH_REPLY(GXimProtocolClosure  *closure,
					GXimProtocol         *proto,
					GDataInputStream     *stream,
					GError              **error,
					gpointer              user_data)
{
	gboolean retval = FALSE;

	/* XXX */
	PROTO_ERROR (XIM_AUTH_REPLY, 0);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_AUTH_NEXT(GXimProtocolClosure  *closure,
				       GXimProtocol         *proto,
				       GDataInputStream     *stream,
				       GError              **error,
				       gpointer              user_data)
{
	gboolean retval = FALSE;

	/* XXX */
	PROTO_ERROR (XIM_AUTH_NEXT, 0);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_AUTH_SETUP(GXimProtocolClosure  *closure,
					GXimProtocol         *proto,
					GDataInputStream     *stream,
					GError              **error,
					gpointer              user_data)
{
	gboolean retval = FALSE;

	/* XXX */
	PROTO_ERROR (XIM_AUTH_SETUP, 0);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_AUTH_NG(GXimProtocolClosure  *closure,
				     GXimProtocol         *proto,
				     GDataInputStream     *stream,
				     GError              **error,
				     gpointer              user_data)
{
	gboolean retval = FALSE;

	retval = g_xim_protocol_closure_emit_signal(closure, proto);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_ERROR(GXimProtocolClosure  *closure,
				   GXimProtocol         *proto,
				   GDataInputStream     *stream,
				   GError              **error,
				   gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid, icid, flag, detail;
	GXimErrorCode error_code = 0;
	gchar *error_message = NULL;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 8,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid,
				       G_XIM_TYPE_WORD, &flag,
				       G_XIM_TYPE_WORD, &error_code,
				       G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_CHAR,
				       G_XIM_TYPE_WORD, &detail,
				       G_XIM_TYPE_CHAR, &error_message,
				       G_XIM_TYPE_AUTO_PADDING, 0)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid, flag, error_code, detail, error_message);
	}
	g_free(error_message);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_OPEN(GXimProtocolClosure  *closure,
				  GXimProtocol         *proto,
				  GDataInputStream     *stream,
				  GError              **error,
				  gpointer              user_data)
{
	GXimStr *locale = NULL;
	gboolean retval = FALSE;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_STR, &locale,
				       G_XIM_TYPE_AUTO_PADDING, 0)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    locale);
	} else {
		/* better handling error */
		PROTO_ERROR (XIM_OPEN, 0);
	}

	g_xim_str_free(locale);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_OPEN_REPLY(GXimProtocolClosure  *closure,
					GXimProtocol         *proto,
					GDataInputStream     *stream,
					GError              **error,
					gpointer              user_data)
{
	GSList *imlist = NULL, *iclist = NULL, *l;
	GXimAttr *imattr = NULL, *icattr = NULL;
	guint16 imid = 0;
	gboolean retval = FALSE;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 6,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_IMATTR,
				       G_XIM_TYPE_LIST_OF_IMATTR, &imlist,
				       G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_ICATTR,
				       G_XIM_TYPE_PADDING, 2,
				       G_XIM_TYPE_LIST_OF_ICATTR, &iclist)) {
		imattr = g_object_new(G_TYPE_XIM_IM_ATTR, NULL);
		icattr = g_object_new(G_TYPE_XIM_IC_ATTR, NULL);
		for (l = imlist; l != NULL; l = g_slist_next(l)) {
			g_xim_attr_set_raw_attr(imattr, l->data);
			g_xim_raw_attr_free(l->data);
		}
		for (l = iclist; l != NULL; l = g_slist_next(l)) {
			g_xim_attr_set_raw_attr(icattr, l->data);
			g_xim_raw_attr_free(l->data);
		}
		g_slist_free(imlist);
		g_slist_free(iclist);

		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, imattr, icattr);
	} else {
		/* better handling error */
		PROTO_ERROR_IM (XIM_OPEN_REPLY, 0, imid);
	}

	if (imattr)
		g_object_unref(imattr);
	if (icattr)
		g_object_unref(icattr);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_CLOSE(GXimProtocolClosure  *closure,
				   GXimProtocol         *proto,
				   GDataInputStream     *stream,
				   GError              **error,
				   gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_PADDING, 2)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid);
	} else {
		/* better handling error */
		PROTO_ERROR_IM (XIM_CLOSE, 0, imid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_CLOSE_REPLY(GXimProtocolClosure  *closure,
					 GXimProtocol         *proto,
					 GDataInputStream     *stream,
					 GError              **error,
					 gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_PADDING, 2)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid);
	} else {
		/* better handling error */
		PROTO_ERROR_IM (XIM_CLOSE_REPLY, 0, imid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_REGISTER_TRIGGERKEYS(GXimProtocolClosure  *closure,
						  GXimProtocol         *proto,
						  GDataInputStream     *stream,
						  GError              **error,
						  gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid;
	GSList *onkeys = NULL, *offkeys = NULL;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 4,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_PADDING, 2,
				       G_XIM_TYPE_MARKER_N_BYTES_4, G_XIM_TYPE_LIST_OF_HOTKEY_TRIGGER,
				       G_XIM_TYPE_LIST_OF_HOTKEY_TRIGGER, &onkeys)) {
		/* check the off-keys list separately.
		 * some IMs sends XIM_REGISTER_TRIGGERKEYS without
		 * the off-keys list.
		 */
		g_xim_protocol_read_format(proto, stream, NULL, error, 2,
					   G_XIM_TYPE_MARKER_N_BYTES_4, G_XIM_TYPE_LIST_OF_HOTKEY_TRIGGER,
					   G_XIM_TYPE_LIST_OF_HOTKEY_TRIGGER, &offkeys);
		if (*error) {
			g_xim_messages_warning(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					       "No off-keys received in XIM_REGISTER_TRIGGERKEYS:\n  %s",
					       (*error)->message);
			g_clear_error(error);
		}
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, onkeys, offkeys);
	}
	g_slist_foreach(onkeys, (GFunc)g_xim_hotkey_trigger_free, NULL);
	g_slist_foreach(offkeys, (GFunc)g_xim_hotkey_trigger_free, NULL);
	g_slist_free(onkeys);
	g_slist_free(offkeys);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_TRIGGER_NOTIFY(GXimProtocolClosure  *closure,
					    GXimProtocol         *proto,
					    GDataInputStream     *stream,
					    GError              **error,
					    gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;
	guint32 flag, index_, mask;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 5,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid,
				       G_XIM_TYPE_LONG, &flag,
				       G_XIM_TYPE_LONG, &index_,
				       G_XIM_TYPE_LONG, &mask)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid, flag, index_, mask);
	} else {
		/* better handling error */
		PROTO_ERROR_IMIC (XIM_TRIGGER_NOTIFY, 0, imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_TRIGGER_NOTIFY_REPLY(GXimProtocolClosure  *closure,
						  GXimProtocol         *proto,
						  GDataInputStream     *stream,
						  GError              **error,
						  gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	} else {
		/* better handling error */
		PROTO_ERROR_IMIC (XIM_TRIGGER_NOTIFY_REPLY, 0, imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_SET_EVENT_MASK(GXimProtocolClosure  *closure,
					    GXimProtocol         *proto,
					    GDataInputStream     *stream,
					    GError              **error,
					    gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid, icid;
	guint32 forward_mask, sync_mask;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 4,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid,
				       G_XIM_TYPE_LONG, &forward_mask,
				       G_XIM_TYPE_LONG, &sync_mask)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid, forward_mask, sync_mask);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_ENCODING_NEGOTIATION(GXimProtocolClosure  *closure,
						  GXimProtocol         *proto,
						  GDataInputStream     *stream,
						  GError              **error,
						  gpointer              user_data)
{
	gboolean retval = FALSE;
	GSList *list = NULL, *enclist = NULL;
	guint16 imid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 7,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_STR,
				       G_XIM_TYPE_LIST_OF_STR, &list,
				       G_XIM_TYPE_AUTO_PADDING, 0,
				       G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_ENCODINGINFO,
				       G_XIM_TYPE_PADDING, 2,
				       G_XIM_TYPE_LIST_OF_ENCODINGINFO, &enclist)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, list, enclist);
	} else {
		/* better handling error */
		PROTO_ERROR_IM (XIM_ENCODING_NEGOTIATION, 0, imid);
	}

	g_slist_foreach(list,
			(GFunc)g_xim_str_free,
			NULL);
	g_slist_foreach(enclist,
			(GFunc)g_xim_encodinginfo_free,
			NULL);
	g_slist_free(list);
	g_slist_free(enclist);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_ENCODING_NEGOTIATION_REPLY(GXimProtocolClosure  *closure,
							GXimProtocol         *proto,
							GDataInputStream     *stream,
							GError              **error,
							gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, category;
	gint16 index_;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 4,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &category,
				       G_XIM_TYPE_WORD, &index_,
				       G_XIM_TYPE_PADDING, 2)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, category, index_);
	} else {
		/* better handling error */
		PROTO_ERROR_IM (XIM_ENCODING_NEGOTIATION_REPLY, 0, imid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_QUERY_EXTENSION(GXimProtocolClosure  *closure,
					     GXimProtocol         *proto,
					     GDataInputStream     *stream,
					     GError              **error,
					     gpointer              user_data)
{
	gboolean retval = FALSE;
	GSList *list = NULL;
	guint16 imid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 4,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_STR,
				       G_XIM_TYPE_LIST_OF_STR, &list,
				       G_XIM_TYPE_AUTO_PADDING, 0)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, list);
	} else {
		/* better handling error */
		PROTO_ERROR_IM (XIM_QUERY_EXTENSION, 0, imid);
	}

	g_slist_foreach(list,
			(GFunc)g_xim_str_free,
			NULL);
	g_slist_free(list);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_QUERY_EXTENSION_REPLY(GXimProtocolClosure  *closure,
						   GXimProtocol         *proto,
						   GDataInputStream     *stream,
						   GError              **error,
						   gpointer              user_data)
{
	gboolean retval = FALSE;
	GSList *list = NULL;
	guint16 imid;

	if (!g_xim_protocol_read_format(proto, stream, NULL, error, 1,
					G_XIM_TYPE_WORD, &imid)) {
		/* XXX: There may be no way of recoverying from here */
		return FALSE;
	}
	g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				   G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_EXT,
				   G_XIM_TYPE_LIST_OF_EXT, &list);
	if (*error) {
		/* deal with it as warning really, because we can
		 * recover it.
		 */
		g_xim_messages_warning(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
				       "%s", (*error)->message);
		g_clear_error(error);
	}
	/* ignore an error. we could just send the null list */
	retval = g_xim_protocol_closure_emit_signal(closure, proto,
						    imid, list);

	g_slist_foreach(list,
			(GFunc)g_xim_ext_free,
			NULL);
	g_slist_free(list);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_SET_IM_VALUES(GXimProtocolClosure  *closure,
					   GXimProtocol         *proto,
					   GDataInputStream     *stream,
					   GError              **error,
					   gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0;
	GSList *list = NULL;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 3,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_IMATTRIBUTE,
				       G_XIM_TYPE_LIST_OF_IMATTRIBUTE, &list)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, list);
	} else {
		/* better handling error */
		PROTO_ERROR_IM (XIM_SET_IM_VALUES, 0, imid);
	}

	g_slist_foreach(list,
			(GFunc)g_xim_attribute_free,
			NULL);
	g_slist_free(list);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_SET_IM_VALUES_REPLY(GXimProtocolClosure  *closure,
						 GXimProtocol         *proto,
						 GDataInputStream     *stream,
						 GError              **error,
						 gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_PADDING, 2)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid);
	} else {
		/* better handling error */
		PROTO_ERROR_IM (XIM_SET_IM_VALUES_REPLY, 0, imid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_GET_IM_VALUES(GXimProtocolClosure  *closure,
					   GXimProtocol         *proto,
					   GDataInputStream     *stream,
					   GError              **error,
					   gpointer              user_data)
{
	gboolean retval = FALSE;
	GSList *list = NULL;
	guint16 imid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 4,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_CARD16,
				       G_XIM_TYPE_LIST_OF_CARD16, &list,
				       G_XIM_TYPE_AUTO_PADDING, 0)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, list);
	} else {
		/* better handling error */
		PROTO_ERROR_IM (XIM_GET_IM_VALUES, 0, imid);
	}

	g_slist_free(list);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_GET_IM_VALUES_REPLY(GXimProtocolClosure  *closure,
						 GXimProtocol         *proto,
						 GDataInputStream     *stream,
						 GError              **error,
						 gpointer              user_data)
{
	gboolean retval = FALSE;
	GSList *list = NULL;
	guint16 imid;
	GError *err = NULL;

	if (!g_xim_protocol_read_format(proto, stream, NULL, error, 1,
					G_XIM_TYPE_WORD, &imid)) {
		/* XXX: There may be no way of recovering from here. */
		return FALSE;
	}
	g_xim_protocol_read_format(proto, stream, NULL, &err, 2,
				   G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_IMATTRIBUTE,
				   G_XIM_TYPE_LIST_OF_IMATTRIBUTE, &list);
	if (err) {
		/* deal with it as warning really, because we can
		 * recover it.
		 */
		g_xim_messages_warning(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
				       "%s", err->message);
		g_error_free(err);
	}
	/* ignore an error. we could just send the null list */
	retval = g_xim_protocol_closure_emit_signal(closure, proto,
						    imid, list);

	g_slist_foreach(list, (GFunc)g_xim_attribute_free, NULL);
	g_slist_free(list);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_CREATE_IC(GXimProtocolClosure  *closure,
				       GXimProtocol         *proto,
				       GDataInputStream     *stream,
				       GError              **error,
				       gpointer              user_data)
{
	gboolean retval = FALSE;
	GSList *list = NULL;
	guint16 imid;

	if (!g_xim_protocol_read_format(proto, stream, NULL, error, 1,
				       G_XIM_TYPE_WORD, &imid))
		return FALSE;

	g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				   G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_ICATTRIBUTE,
				   G_XIM_TYPE_LIST_OF_ICATTRIBUTE, &list);
	/* deal with all of errors as warnings to give aid to
	 * the attributes retrieved successfully.
	 */
	/* XXX: or should we just send back the error? */
	if (*error) {
		g_xim_messages_warning(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
				       "%s", (*error)->message);
		g_clear_error(error);
	}
	retval = g_xim_protocol_closure_emit_signal(closure, proto,
						    imid, list);

	g_slist_foreach(list, (GFunc)g_xim_attribute_free, NULL);
	g_slist_free(list);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_CREATE_IC_REPLY(GXimProtocolClosure  *closure,
					     GXimProtocol         *proto,
					     GDataInputStream     *stream,
					     GError              **error,
					     gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	} else {
		/* better handling error */
		PROTO_ERROR_IMIC (XIM_CREATE_IC_REPLY, 0, imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_DESTROY_IC(GXimProtocolClosure  *closure,
					GXimProtocol         *proto,
					GDataInputStream     *stream,
					GError              **error,
					gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	} else {
		/* better handling error */
		PROTO_ERROR_IMIC (XIM_DESTROY_IC, 0, imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_DESTROY_IC_REPLY(GXimProtocolClosure  *closure,
					      GXimProtocol         *proto,
					      GDataInputStream     *stream,
					      GError              **error,
					      gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	} else {
		/* better handling error */
		PROTO_ERROR_IMIC (XIM_DESTROY_IC_REPLY, 0, imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_SET_IC_VALUES(GXimProtocolClosure  *closure,
					   GXimProtocol         *proto,
					   GDataInputStream     *stream,
					   GError              **error,
					   gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;
	GSList *list = NULL;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 5,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid,
				       G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_ICATTRIBUTE,
				       G_XIM_TYPE_PADDING, 2,
				       G_XIM_TYPE_LIST_OF_ICATTRIBUTE, &list)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid, list);
	} else {
		/* better handling error */
		PROTO_ERROR_IMIC (XIM_SET_IC_VALUES, 0, imid, icid);
	}

	g_slist_foreach(list,
			(GFunc)g_xim_attribute_free,
			NULL);
	g_slist_free(list);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_SET_IC_VALUES_REPLY(GXimProtocolClosure  *closure,
						 GXimProtocol         *proto,
						 GDataInputStream     *stream,
						 GError              **error,
						 gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	} else {
		/* better handling error */
		PROTO_ERROR_IMIC (XIM_SET_IC_VALUES_REPLY, 0, imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_GET_IC_VALUES(GXimProtocolClosure  *closure,
					   GXimProtocol         *proto,
					   GDataInputStream     *stream,
					   GError              **error,
					   gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;
	GSList *list = NULL;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 5,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid,
				       G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_CARD16,
				       G_XIM_TYPE_LIST_OF_CARD16, &list,
				       G_XIM_TYPE_AUTO_PADDING, 2)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid, list);
	} else {
		/* better handling error */
		PROTO_ERROR_IMIC (XIM_GET_IC_VALUES, 0, imid, icid);
	}

	g_slist_free(list);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_GET_IC_VALUES_REPLY(GXimProtocolClosure  *closure,
						 GXimProtocol         *proto,
						 GDataInputStream     *stream,
						 GError              **error,
						 gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;
	GSList *list = NULL;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 5,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid,
				       G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_ICATTRIBUTE,
				       G_XIM_TYPE_PADDING, 2,
				       G_XIM_TYPE_LIST_OF_ICATTRIBUTE, &list)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid, list);
	} else {
		/* better handling error */
		PROTO_ERROR_IMIC (XIM_GET_IC_VALUES_REPLY, 0, imid, icid);
	}

	g_slist_foreach(list, (GFunc)g_xim_attribute_free, NULL);
	g_slist_free(list);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_SET_IC_FOCUS(GXimProtocolClosure  *closure,
					  GXimProtocol         *proto,
					  GDataInputStream     *stream,
					  GError              **error,
					  gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid, icid;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_UNSET_IC_FOCUS(GXimProtocolClosure  *closure,
					    GXimProtocol         *proto,
					    GDataInputStream     *stream,
					    GError              **error,
					    gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid, icid;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_FORWARD_EVENT(GXimProtocolClosure  *closure,
					   GXimProtocol         *proto,
					   GDataInputStream     *stream,
					   GError              **error,
					   gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0, flag = 0;
	GdkEvent *event = NULL;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 4,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid,
				       G_XIM_TYPE_WORD, &flag,
				       G_XIM_TYPE_GDKEVENT, &event)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid, flag, event);
	} else {
		/* better handling error */
		if (flag & G_XIM_Event_Synchronous)
			g_xim_connection_cmd_sync_reply(G_XIM_CONNECTION (proto), imid, icid);

		retval = g_xim_connection_cmd_forward_event(G_XIM_CONNECTION (proto), imid, icid, flag & ~G_XIM_Event_Synchronous, event);
	}
	if (event)
		gdk_event_free(event);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_SYNC(GXimProtocolClosure  *closure,
				  GXimProtocol         *proto,
				  GDataInputStream     *stream,
				  GError              **error,
				  gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	} else {
		/* better handling error */
		PROTO_ERROR_IMIC (XIM_SYNC, 0, imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_SYNC_REPLY(GXimProtocolClosure  *closure,
					GXimProtocol         *proto,
					GDataInputStream     *stream,
					GError              **error,
					gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid, icid;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_COMMIT(GXimProtocolClosure  *closure,
				    GXimProtocol         *proto,
				    GDataInputStream     *stream,
				    GError              **error,
				    gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0, flag = 0;
	gint padding = 0;
	guint32 keysym = GDK_KEY_VoidSymbol;
	GString *string = NULL;

	if (!g_xim_protocol_read_format(proto, stream, NULL, error, 3,
					G_XIM_TYPE_WORD, &imid,
					G_XIM_TYPE_WORD, &icid,
					G_XIM_TYPE_WORD, &flag))
		goto fail;

	if (flag & G_XIM_XLookupKeySym) {
		if (!g_xim_protocol_read_format(proto, stream, NULL, error, 2,
						G_XIM_TYPE_PADDING, 2,
						G_XIM_TYPE_LONG, &keysym))
			goto fail;
		padding += 2;
	}
	if (flag & G_XIM_XLookupChars) {
		if (!g_xim_protocol_read_format(proto, stream, NULL, error, 2,
						G_XIM_TYPE_GSTRING, &string,
						G_XIM_TYPE_AUTO_PADDING, padding))
			goto fail;
	}
	retval = g_xim_protocol_closure_emit_signal(closure, proto,
						    imid, icid, flag, keysym, string);
	if (string)
		g_string_free(string, TRUE);

	return retval;
  fail:
	if (string)
		g_string_free(string, TRUE);

	if (flag & G_XIM_XLookupSynchronous)
		return g_xim_connection_cmd_sync_reply(G_XIM_CONNECTION (proto), imid, icid);

	return FALSE;
}

static gboolean
g_xim_protocol10_closure_XIM_RESET_IC(GXimProtocolClosure  *closure,
				      GXimProtocol         *proto,
				      GDataInputStream     *stream,
				      GError              **error,
				      gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	} else {
		/* better handling error */
		PROTO_ERROR_IMIC (XIM_RESET_IC, 0, imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_RESET_IC_REPLY(GXimProtocolClosure  *closure,
					    GXimProtocol         *proto,
					    GDataInputStream     *stream,
					    GError              **error,
					    gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;
	GString *string = NULL;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 5,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid,
				       G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_LIST_OF_BYTE,
				       G_XIM_TYPE_LIST_OF_BYTE, &string,
				       G_XIM_TYPE_AUTO_PADDING, 2)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid, string);
	} else {
		/* better handling error */
		PROTO_ERROR_IMIC (XIM_RESET_IC_REPLY, 0, imid, icid);
	}
	g_string_free(string, TRUE);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_GEOMETRY(GXimProtocolClosure  *closure,
				      GXimProtocol         *proto,
				      GDataInputStream     *stream,
				      GError              **error,
				      gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid, icid;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_STR_CONVERSION(GXimProtocolClosure  *closure,
					    GXimProtocol         *proto,
					    GDataInputStream     *stream,
					    GError              **error,
					    gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;

	/* XXX */
	g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				   G_XIM_TYPE_WORD, &imid,
				   G_XIM_TYPE_WORD, &icid);
	PROTO_ERROR_IMIC (XIM_STR_CONVERSION, 0, imid, icid);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_STR_CONVERSION_REPLY(GXimProtocolClosure  *closure,
						  GXimProtocol         *proto,
						  GDataInputStream     *stream,
						  GError              **error,
						  gpointer              user_data)
{
	gboolean retval = FALSE;

	/* XXX */
	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_PREEDIT_START(GXimProtocolClosure  *closure,
					   GXimProtocol         *proto,
					   GDataInputStream     *stream,
					   GError              **error,
					   gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	} else {
		/* better handling error */
		/* XXX: should we send back PREEDIT_START_REPLY with an error in the return value? */
		PROTO_ERROR_IMIC (XIM_PREEDIT_START, 0, imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_PREEDIT_START_REPLY(GXimProtocolClosure  *closure,
						 GXimProtocol         *proto,
						 GDataInputStream     *stream,
						 GError              **error,
						 gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid, icid;
	gint32 ret;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 3,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid,
				       G_XIM_TYPE_LONG, &ret)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid, ret);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_PREEDIT_DRAW(GXimProtocolClosure  *closure,
					  GXimProtocol         *proto,
					  GDataInputStream     *stream,
					  GError              **error,
					  gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid, icid;
	GXimPreeditDraw *draw;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 3,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid,
				       G_XIM_TYPE_PREEDIT_DRAW, &draw)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid, draw);
	}
	g_xim_preedit_draw_free(draw);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_PREEDIT_CARET(GXimProtocolClosure  *closure,
					   GXimProtocol         *proto,
					   GDataInputStream     *stream,
					   GError              **error,
					   gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid = 0, icid = 0;
	GXimPreeditCaret *caret;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 3,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid,
				       G_XIM_TYPE_PREEDIT_CARET, &caret)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid, caret);
	} else {
		/* better handling error */
		/* XXX: should we send back PREEDIT_CARET_REPLY with an error in the return value? */
		PROTO_ERROR_IMIC (XIM_PREEDIT_CARET, 0, imid, icid);
	}
	g_xim_preedit_caret_free(caret);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_PREEDIT_CARET_REPLY(GXimProtocolClosure  *closure,
						 GXimProtocol         *proto,
						 GDataInputStream     *stream,
						 GError              **error,
						 gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid, icid;
	guint32 position;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 3,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid,
				       G_XIM_TYPE_LONG, &position)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid, position);
	}

	/* XXX */
	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_PREEDIT_DONE(GXimProtocolClosure  *closure,
					  GXimProtocol         *proto,
					  GDataInputStream     *stream,
					  GError              **error,
					  gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid, icid;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_STATUS_START(GXimProtocolClosure  *closure,
					  GXimProtocol         *proto,
					  GDataInputStream     *stream,
					  GError              **error,
					  gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid, icid;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_STATUS_DRAW(GXimProtocolClosure  *closure,
					 GXimProtocol         *proto,
					 GDataInputStream     *stream,
					 GError              **error,
					 gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid, icid;
	GXimStatusDraw *draw;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 3,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid,
				       G_XIM_TYPE_STATUS_DRAW, &draw)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid, draw);
	}
	g_xim_status_draw_free(draw);

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_STATUS_DONE(GXimProtocolClosure  *closure,
					 GXimProtocol         *proto,
					 GDataInputStream     *stream,
					 GError              **error,
					 gpointer              user_data)
{
	gboolean retval = FALSE;
	guint16 imid, icid;

	if (g_xim_protocol_read_format(proto, stream, NULL, error, 2,
				       G_XIM_TYPE_WORD, &imid,
				       G_XIM_TYPE_WORD, &icid)) {
		retval = g_xim_protocol_closure_emit_signal(closure, proto,
							    imid, icid);
	}

	return retval;
}

static gboolean
g_xim_protocol10_closure_XIM_PREEDITSTATE(GXimProtocolClosure  *closure,
					  GXimProtocol         *proto,
					  GDataInputStream     *stream,
					  GError              **error,
					  gpointer              user_data)
{
	gboolean retval = FALSE;

	/* XXX */
	return retval;
}

static void
gxim_marshal_FIXME(GClosure     *closure,
		   GValue       *return_value,
		   guint         n_param_values,
		   const GValue *param_values,
		   gpointer      invocation_hint,
		   gpointer      marshal_data)
{
	g_warning("FIXME");
}

static void
g_xim_protocol10_closure_signal_no_impl(GXimProtocol  *proto,
					const gchar   *func,
					GXimErrorMask  flag,
					guint16        imid,
					guint16        icid)
{
	GXimProtocolIface *iface;
	static const gchar message[] = "Not yet implemented or any errors occurred";
	static size_t len = 0;

	iface = G_XIM_PROTOCOL_GET_IFACE (proto);
	if (len == 0)
		len = strlen(message);

	g_xim_messages_bug(iface->message, "No real implementation of `%s' or any errors occurred.", func);
	g_xim_connection_cmd_error(G_XIM_CONNECTION (proto), imid, icid, flag,
				   G_XIM_ERR_BadSomething, 0, message);
}

static gboolean
g_xim_protocol10_closure_signal_XIM_CONNECT(GXimProtocol *proto,
					    guint16       major_version,
					    guint16       minor_version,
					    const GSList *list)
{
	return g_xim_connection_cmd_auth_ng(G_XIM_CONNECTION (proto));
}

static gboolean
g_xim_protocol10_closure_signal_XIM_CONNECT_REPLY(GXimProtocol *proto,
						  guint16       major_version,
						  guint16       minor_version)
{
	MSG_NOIMPL(XIM_CONNECT_REPLY);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_DISCONNECT(GXimProtocol *proto)
{
	SIG_NOIMPL(XIM_DISCONNECT);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_DISCONNECT_REPLY(GXimProtocol *proto)
{
	MSG_NOIMPL(XIM_DISCONNECT_REPLY);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_AUTH_REQUIRED(GXimProtocol *proto,
						  const gchar  *auth_data,
						  gsize         length)
{
	return g_xim_connection_cmd_auth_ng(G_XIM_CONNECTION (proto));
}

static gboolean
g_xim_protocol10_closure_signal_XIM_AUTH_REPLY(GXimProtocol *proto,
					       const gchar  *auth_data,
					       gsize         length)
{
	return g_xim_connection_cmd_auth_ng(G_XIM_CONNECTION (proto));
}

static gboolean
g_xim_protocol10_closure_signal_XIM_AUTH_NEXT(GXimProtocol *proto,
					      const gchar  *auth_data,
					      gsize         length)
{
	return g_xim_connection_cmd_auth_ng(G_XIM_CONNECTION (proto));
}

static gboolean
g_xim_protocol10_closure_signal_XIM_AUTH_SETUP(GXimProtocol *proto,
					       const GSList *list)
{
	return g_xim_connection_cmd_auth_ng(G_XIM_CONNECTION (proto));
}

static gboolean
g_xim_protocol10_closure_signal_XIM_AUTH_NG(GXimProtocol *proto)
{
	MSG_NOIMPL(XIM_AUTH_NG);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_ERROR(GXimProtocol  *proto,
					  guint16        imid,
					  guint16        icid,
					  GXimErrorMask  flag,
					  GXimErrorCode  error_code,
					  guint16        detail,
					  const gchar   *error_message)
{
	gchar *simid, *sicid;

	if (flag & G_XIM_EMASK_VALID_IMID)
		simid = g_strdup_printf("imid: %d, ", imid);
	else
		simid = g_strdup("");
	if (flag & G_XIM_EMASK_VALID_ICID)
		sicid = g_strdup_printf("icid: %d, ", icid);
	else
		sicid = g_strdup("");
	g_xim_messages_error(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
			     "Received an error: %s%s error_code: %d, detail: %d, message: %s",
			     simid, sicid, error_code, detail, error_message);
	g_free(simid);
	g_free(sicid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_OPEN(GXimProtocol  *proto,
					 const GXimStr *locale)
{
	SIG_NOIMPL(XIM_OPEN);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_OPEN_REPLY(GXimProtocol *proto,
					       guint16       imid,
					       GXimIMAttr   *imattr,
					       GXimICAttr   *icattr)
{
	MSG_NOIMPL_IM(XIM_OPEN_REPLY, imid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_CLOSE(GXimProtocol *proto,
					  guint16       imid)
{
	SIG_NOIMPL_IM(XIM_CLOSE, imid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_CLOSE_REPLY(GXimProtocol *proto,
						guint16       imid)
{
	MSG_NOIMPL_IM(XIM_CLOSE_REPLY, imid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_REGISTER_TRIGGERKEYS(GXimProtocol *proto,
							 guint16       imid,
							 const GSList *onkeys,
							 const GSList *offkeys)
{
	MSG_NOIMPL_IM(XIM_REGISTER_TRIGGERKEYS, imid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_TRIGGER_NOTIFY(GXimProtocol *proto,
						   guint16       imid,
						   guint16       icid,
						   guint32       flag,
						   guint32       index_,
						   guint32       mask)
{
	SIG_NOIMPL_IMIC(XIM_TRIGGER_NOTIFY, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_TRIGGER_NOTIFY_REPLY(GXimProtocol *proto,
							 guint16       imid,
							 guint16       icid)
{
	MSG_NOIMPL_IMIC(XIM_TRIGGER_NOTIFY_REPLY, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_SET_EVENT_MASK(GXimProtocol *proto,
						   guint16       imid,
						   guint16       icid,
						   guint32       forward_event_mask,
						   guint32       synchronous_event_mask)
{
	MSG_NOIMPL_IMIC(XIM_SET_EVENT_MASK, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_ENCODING_NEGOTIATION(GXimProtocol *proto,
							 guint16       imid,
							 const GSList *encodings,
							 const GSList *details)
{
	SIG_NOIMPL_IM(XIM_ENCODING_NEGOTIATION, imid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_ENCODING_NEGOTIATION_REPLY(GXimProtocol *proto,
							       guint16       imid,
							       guint16       category,
							       gint16        index_)
{
	MSG_NOIMPL_IM(XIM_ENCODING_NEGOTIATION_REPLY, imid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_QUERY_EXTENSION(GXimProtocol *proto,
						    guint16       imid,
						    const GSList *extensions)
{
	SIG_NOIMPL_IM(XIM_QUERY_EXTENSION, imid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_QUERY_EXTENSION_REPLY(GXimProtocol *proto,
							  guint16       imid,
							  const GSList *extensions)
{
	MSG_NOIMPL_IM(XIM_QUERY_EXTENSION_REPLY, imid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_SET_IM_VALUES(GXimProtocol *proto,
						  guint16       imid,
						  const GSList *attributes)
{
	SIG_NOIMPL_IM(XIM_SET_IM_VALUES, imid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_SET_IM_VALUES_REPLY(GXimProtocol *proto,
							guint16       imid)
{
	MSG_NOIMPL_IM(XIM_SET_IM_VALUES_REPLY, imid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_GET_IM_VALUES(GXimProtocol *proto,
						  guint16       imid,
						  const GSList *attr_id)
{
	SIG_NOIMPL_IM(XIM_GET_IM_VALUES, imid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_GET_IM_VALUES_REPLY(GXimProtocol *proto,
							guint16       imid,
							const GSList *attributes)
{
	MSG_NOIMPL_IM(XIM_GET_IM_VALUES_REPLY, imid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_CREATE_IC(GXimProtocol *proto,
					      guint16       imid,
					      const GSList *attributes)
{
	SIG_NOIMPL_IM(XIM_CREATE_IC, imid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_CREATE_IC_REPLY(GXimProtocol *proto,
						    guint16       imid,
						    guint16       icid)
{
	MSG_NOIMPL_IMIC(XIM_CREATE_IC_REPLY, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_DESTROY_IC(GXimProtocol *proto,
					       guint16       imid,
					       guint16       icid)
{
	MSG_NOIMPL_IMIC(XIM_DESTROY_IC, imid, icid);

	if (!G_IS_XIM_SERVER_CONNECTION (proto)) {
		g_xim_messages_error(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
				     "Non-server connection received XIM_DESTROY_IC [imid: %d, icid: %d]",
				     imid, icid);
		return FALSE;
	}

	return g_xim_server_connection_cmd_destroy_ic_reply(G_XIM_SERVER_CONNECTION (proto), imid, icid);
}

static gboolean
g_xim_protocol10_closure_signal_XIM_DESTROY_IC_REPLY(GXimProtocol *proto,
						     guint16       imid,
						     guint16       icid)
{
	MSG_NOIMPL_IMIC(XIM_DESTROY_IC_REPLY, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_SET_IC_VALUES(GXimProtocol *proto,
						  guint16       imid,
						  guint16       icid,
						  const GSList *attributes)
{
	SIG_NOIMPL_IMIC(XIM_SET_IC_VALUES, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_SET_IC_VALUES_REPLY(GXimProtocol *proto,
							guint16       imid,
							guint16       icid)
{
	MSG_NOIMPL_IMIC(XIM_SET_IC_VALUES_REPLY, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_GET_IC_VALUES(GXimProtocol *proto,
						  guint16       imid,
						  guint16       icid,
						  const GSList *attr_id)
{
	SIG_NOIMPL_IMIC(XIM_GET_IC_VALUES, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_GET_IC_VALUES_REPLY(GXimProtocol *proto,
							guint16       imid,
							guint16       icid,
							const GSList *attributes)
{
	MSG_NOIMPL_IMIC(XIM_GET_IC_VALUES_REPLY, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_SET_IC_FOCUS(GXimProtocol *proto,
						 guint16       imid,
						 guint16       icid)
{
	MSG_NOIMPL_IMIC(XIM_SET_IC_FOCUS, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_UNSET_IC_FOCUS(GXimProtocol *proto,
						   guint16       imid,
						   guint16       icid)
{
	MSG_NOIMPL_IMIC(XIM_UNSET_IC_FOCUS, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_FORWARD_EVENT(GXimProtocol *proto,
						  guint16       imid,
						  guint16       icid,
						  guint16       flag,
						  GdkEvent     *event)
{
	MSG_NOIMPL_IMIC(XIM_FORWARD_EVENT, imid, icid);

	if (flag & G_XIM_Event_Synchronous)
		g_xim_connection_cmd_sync_reply(G_XIM_CONNECTION (proto), imid, icid);

	g_xim_connection_cmd_forward_event(G_XIM_CONNECTION (proto), imid, icid, flag & ~G_XIM_Event_Synchronous, event);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_SYNC(GXimProtocol *proto,
					 guint16       imid,
					 guint16       icid)
{
	MSG_NOIMPL_IMIC(XIM_SYNC, imid, icid);

	return g_xim_connection_cmd_sync_reply(G_XIM_CONNECTION (proto), imid, icid);
}

static gboolean
g_xim_protocol10_closure_signal_XIM_SYNC_REPLY(GXimProtocol *proto,
					       guint16       imid,
					       guint16       icid)
{
	MSG_NOIMPL_IMIC(XIM_SYNC_REPLY, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_COMMIT(GXimProtocol *proto,
					   guint16       imid,
					   guint16       icid,
					   guint16       flag,
					   guint32       keysym,
					   GString      *string)
{
	MSG_NOIMPL_IMIC(XIM_COMMIT, imid, icid);

	if (flag & G_XIM_XLookupSynchronous)
		return g_xim_connection_cmd_sync_reply(G_XIM_CONNECTION (proto), imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_RESET_IC(GXimProtocol *proto,
					     guint16       imid,
					     guint16       icid)
{
	SIG_NOIMPL_IMIC(XIM_RESET_IC, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_RESET_IC_REPLY(GXimProtocol  *proto,
						   guint16        imid,
						   guint16        icid,
						   const GString *string)
{
	MSG_NOIMPL_IMIC(XIM_RESET_IC_REPLY, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_GEOMETRY(GXimProtocol *proto,
					     guint16       imid,
					     guint16       icid)
{
	SIG_NOIMPL_IMIC(XIM_GEOMETRY, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_STR_CONVERSION(GXimProtocol *proto,
						   guint16       imid,
						   guint16       icid,
						   guint16       position,
						   guint32       caret_direction,
						   guint16       factor,
						   guint16       operation,
						   gint16        type_length)
{
	/* XXX */
	SIG_NOIMPL_IMIC(XIM_STR_CONVERSION, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_STR_CONVERSION_REPLY(GXimProtocol    *proto,
							 guint16          imid,
							 guint16          icid,
							 guint32          feedback,
							 GXimStrConvText *text)
{
	MSG_NOIMPL_IMIC(XIM_STR_CONVERSION_REPLY, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_PREEDIT_START(GXimProtocol *proto,
						  guint16       imid,
						  guint16       icid)
{
	/* XXX */
	SIG_NOIMPL_IMIC(XIM_PREEDIT_START, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_PREEDIT_START_REPLY(GXimProtocol *proto,
							guint16       imid,
							guint16       icid,
							gint32        return_value)
{
	SIG_NOIMPL_IMIC(XIM_PREEDIT_START_REPLY, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_PREEDIT_DRAW(GXimProtocol    *proto,
						 guint16          imid,
						 guint16          icid,
						 GXimPreeditDraw *draw)
{
	MSG_NOIMPL_IMIC(XIM_PREEDIT_DRAW, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_PREEDIT_CARET(GXimProtocol     *proto,
						  guint16           imid,
						  guint16           icid,
						  GXimPreeditCaret *caret)
{
	/* XXX */
	SIG_NOIMPL_IMIC(XIM_PREEDIT_CARET, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_PREEDIT_CARET_REPLY(GXimProtocol *proto,
							guint16       imid,
							guint16       icid,
							guint32       position)
{
	MSG_NOIMPL_IMIC(XIM_PREEDIT_CARET_REPLY, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_PREEDIT_DONE(GXimProtocol *proto,
						 guint16       imid,
						 guint16       icid)
{
	MSG_NOIMPL_IMIC(XIM_PREEDIT_DONE, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_STATUS_START(GXimProtocol *proto,
						 guint16       imid,
						 guint16       icid)
{
	MSG_NOIMPL_IMIC(XIM_STATUS_START, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_STATUS_DRAW(GXimProtocol   *proto,
						guint16         imid,
						guint16         icid,
						GXimStatusDraw *draw)
{
	MSG_NOIMPL_IMIC(XIM_STATUS_DRAW, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_STATUS_DONE(GXimProtocol *proto,
						guint16       imid,
						guint16       icid)
{
	MSG_NOIMPL_IMIC(XIM_STATUS_DONE, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_signal_XIM_PREEDITSTATE(GXimProtocol *proto,
						 guint16       imid,
						 guint16       icid,
						 guint32       mask)
{
	MSG_NOIMPL_IMIC(XIM_PREEDITSTATE, imid, icid);

	return TRUE;
}

static gboolean
g_xim_protocol10_closure_real_parser_error(GXimProtocol *proto,
					   guint         major_opcode,
					   guint         minor_opcode,
					   guint         imid,
					   guint         icid,
					   gpointer      data)
{
	gboolean retval = TRUE;
	gchar *msg;
	guint flag = G_XIM_EMASK_NO_VALID_ID;

	if (minor_opcode != 0)
		return FALSE;

	msg = g_strdup_printf("Unable to parse the protocol %s properly",
			      g_xim_protocol_name(major_opcode));

	g_xim_messages_error(G_XIM_PROTOCOL_GET_IFACE (proto)->message, msg);
	switch (major_opcode) {
	    case G_XIM_CONNECT:
	    case G_XIM_AUTH_REQUIRED:
	    case G_XIM_AUTH_REPLY:
	    case G_XIM_AUTH_NEXT:
	    case G_XIM_AUTH_SETUP:
		    retval = g_xim_connection_cmd_auth_ng(G_XIM_CONNECTION (proto));
		    break;
	    case G_XIM_SYNC:
		    retval = g_xim_connection_cmd_sync_reply(G_XIM_CONNECTION (proto), imid, icid);
		    break;
	    default:
		    if (imid > 0)
			    flag |= G_XIM_EMASK_VALID_IMID;
		    if (icid > 0)
			    flag |= G_XIM_EMASK_VALID_ICID;
		    retval = g_xim_connection_cmd_error(G_XIM_CONNECTION (proto),
							imid, icid, flag,
							G_XIM_ERR_BadProtocol, 0, msg);
		    break;
	}
	g_free(msg);

	return retval;
}

/*
 * Public functions
 */
gboolean
g_xim_protocol10_closure_init(GXimProtocol *proto,
			      gpointer      data)
{
	GXimProtocolPrivate *priv = g_xim_protocol_get_private(proto);

#define SET(_m_,_n_,_f_,...)						\
	G_STMT_START {							\
		GXimProtocolClosure *_c_;				\
									\
		_c_ = g_xim_protocol_closure_new(G_ ## _m_, (_n_), #_m_, FALSE); \
		G_XIM_CHECK_ALLOC (_c_, FALSE);				\
		g_xim_protocol_closure_set_marshal(_c_, g_xim_protocol10_closure_ ## _m_, data); \
		g_xim_protocol_closure_add_signal(_c_, (_f_), __VA_ARGS__); \
		g_xim_protocol_add_protocol(proto, _c_);		\
		priv->base_signal_ids[G_ ## _m_] = g_xim_protocol_connect_closure_by_id(proto, \
											G_ ## _m_, \
											(_n_), \
											G_CALLBACK (g_xim_protocol10_closure_signal_ ## _m_), \
											NULL); \
	} G_STMT_END

	SET (XIM_CONNECT, 0, gxim_marshal_BOOLEAN__UINT_UINT_POINTER,
	     3, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_POINTER);
	SET (XIM_CONNECT_REPLY, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_DISCONNECT, 0, gxim_marshal_BOOLEAN__VOID,
	     0);
	SET (XIM_DISCONNECT_REPLY, 0, gxim_marshal_BOOLEAN__VOID,
	     0);
	SET (XIM_AUTH_REQUIRED, 0, gxim_marshal_FIXME, 0);
	SET (XIM_AUTH_REPLY, 0, gxim_marshal_FIXME, 0);
	SET (XIM_AUTH_NEXT, 0, gxim_marshal_FIXME, 0);
	SET (XIM_AUTH_SETUP, 0, gxim_marshal_FIXME, 0);
	SET (XIM_AUTH_NG, 0, gxim_marshal_BOOLEAN__VOID,
	     0);
	SET (XIM_ERROR, 0, gxim_marshal_BOOLEAN__UINT_UINT_UINT_UINT_UINT_STRING,
	     6, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_STRING);
	SET (XIM_OPEN, 0, gxim_marshal_BOOLEAN__BOXED,
	     1, G_TYPE_XIM_STR);
	SET (XIM_OPEN_REPLY, 0, gxim_marshal_BOOLEAN__UINT_OBJECT_OBJECT,
	     3, G_TYPE_UINT, G_TYPE_XIM_IM_ATTR, G_TYPE_XIM_IC_ATTR);
	SET (XIM_CLOSE, 0, gxim_marshal_BOOLEAN__UINT,
	     1, G_TYPE_UINT);
	SET (XIM_CLOSE_REPLY, 0, gxim_marshal_BOOLEAN__UINT,
	     1, G_TYPE_UINT);
	SET (XIM_REGISTER_TRIGGERKEYS, 0, gxim_marshal_BOOLEAN__UINT_POINTER_POINTER,
	     3, G_TYPE_UINT, G_TYPE_POINTER, G_TYPE_POINTER);
	SET (XIM_TRIGGER_NOTIFY, 0, gxim_marshal_BOOLEAN__UINT_UINT_ULONG_ULONG_ULONG,
	     5, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_ULONG, G_TYPE_ULONG, G_TYPE_ULONG);
	SET (XIM_TRIGGER_NOTIFY_REPLY, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_SET_EVENT_MASK, 0, gxim_marshal_BOOLEAN__UINT_UINT_ULONG_ULONG,
	     4, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_ULONG, G_TYPE_ULONG);
	SET (XIM_ENCODING_NEGOTIATION, 0, gxim_marshal_BOOLEAN__UINT_POINTER_POINTER,
	     3, G_TYPE_UINT, G_TYPE_POINTER, G_TYPE_POINTER);
	SET (XIM_ENCODING_NEGOTIATION_REPLY, 0, gxim_marshal_BOOLEAN__UINT_UINT_INT,
	     3, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_INT);
	SET (XIM_QUERY_EXTENSION, 0, gxim_marshal_BOOLEAN__UINT_POINTER,
	     2, G_TYPE_UINT, G_TYPE_POINTER);
	SET (XIM_QUERY_EXTENSION_REPLY, 0, gxim_marshal_BOOLEAN__UINT_POINTER,
	     2, G_TYPE_UINT, G_TYPE_POINTER);
	SET (XIM_SET_IM_VALUES, 0, gxim_marshal_BOOLEAN__UINT_POINTER,
	     2, G_TYPE_UINT, G_TYPE_POINTER);
	SET (XIM_SET_IM_VALUES_REPLY, 0, gxim_marshal_BOOLEAN__UINT,
	     1, G_TYPE_UINT);
	SET (XIM_GET_IM_VALUES, 0, gxim_marshal_BOOLEAN__UINT_POINTER,
	     2, G_TYPE_UINT, G_TYPE_POINTER);
	SET (XIM_GET_IM_VALUES_REPLY, 0, gxim_marshal_BOOLEAN__UINT_POINTER,
	     2, G_TYPE_UINT, G_TYPE_POINTER);
	SET (XIM_CREATE_IC, 0, gxim_marshal_BOOLEAN__UINT_POINTER,
	     2, G_TYPE_UINT, G_TYPE_POINTER);
	SET (XIM_CREATE_IC_REPLY, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_DESTROY_IC, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_DESTROY_IC_REPLY, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_SET_IC_VALUES, 0, gxim_marshal_BOOLEAN__UINT_UINT_POINTER,
	     3, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_POINTER);
	SET (XIM_SET_IC_VALUES_REPLY, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_GET_IC_VALUES, 0, gxim_marshal_BOOLEAN__UINT_UINT_POINTER,
	     3, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_POINTER);
	SET (XIM_GET_IC_VALUES_REPLY, 0, gxim_marshal_BOOLEAN__UINT_UINT_POINTER,
	     3, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_POINTER);
	SET (XIM_SET_IC_FOCUS, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_UNSET_IC_FOCUS, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_FORWARD_EVENT, 0, gxim_marshal_BOOLEAN__UINT_UINT_UINT_BOXED,
	     4, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, GDK_TYPE_EVENT);
	SET (XIM_SYNC, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_SYNC_REPLY, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_COMMIT, 0, gxim_marshal_BOOLEAN__UINT_UINT_UINT_ULONG_BOXED,
	     5, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_ULONG, G_TYPE_GSTRING);
	SET (XIM_RESET_IC, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_RESET_IC_REPLY, 0, gxim_marshal_BOOLEAN__UINT_UINT_BOXED,
	     3, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_GSTRING);
	SET (XIM_GEOMETRY, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_STR_CONVERSION, 0, gxim_marshal_FIXME, 0);
	SET (XIM_STR_CONVERSION_REPLY, 0, gxim_marshal_FIXME, 0);
	SET (XIM_PREEDIT_START, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_PREEDIT_START_REPLY, 0, gxim_marshal_BOOLEAN__UINT_UINT_LONG,
	     3, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_LONG);
	SET (XIM_PREEDIT_DRAW, 0, gxim_marshal_BOOLEAN__UINT_UINT_BOXED,
	     3, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_XIM_PREEDIT_DRAW);
	SET (XIM_PREEDIT_CARET, 0, gxim_marshal_BOOLEAN__UINT_UINT_BOXED,
	     3, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_XIM_PREEDIT_CARET);
	SET (XIM_PREEDIT_CARET_REPLY, 0, gxim_marshal_BOOLEAN__UINT_UINT_ULONG,
	     3, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_ULONG);
	SET (XIM_PREEDIT_DONE, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_STATUS_START, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_STATUS_DRAW, 0, gxim_marshal_BOOLEAN__UINT_UINT_BOXED,
	     3, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_XIM_STATUS_DRAW);
	SET (XIM_STATUS_DONE, 0, gxim_marshal_BOOLEAN__UINT_UINT,
	     2, G_TYPE_UINT, G_TYPE_UINT);
	SET (XIM_PREEDITSTATE, 0, gxim_marshal_FIXME, 0);

#undef SET

	g_signal_connect_after(proto, "parser_error",
			       G_CALLBACK (g_xim_protocol10_closure_real_parser_error),
			       NULL);

	return TRUE;
}

void
g_xim_protocol10_closure_finalize(GXimProtocol *proto)
{
#define UNSET(_m_,_n_)							\
	g_xim_protocol_remove_protocol_by_id(proto, G_ ## _m_, (_n_))

	UNSET (XIM_CONNECT, 0);
	UNSET (XIM_CONNECT_REPLY, 0);
	UNSET (XIM_DISCONNECT, 0);
	UNSET (XIM_DISCONNECT_REPLY, 0);
	UNSET (XIM_AUTH_REQUIRED, 0);
	UNSET (XIM_AUTH_REPLY, 0);
	UNSET (XIM_AUTH_NEXT, 0);
	UNSET (XIM_AUTH_SETUP, 0);
	UNSET (XIM_AUTH_NG, 0);
	UNSET (XIM_ERROR, 0);
	UNSET (XIM_OPEN, 0);
	UNSET (XIM_OPEN_REPLY, 0);
	UNSET (XIM_CLOSE, 0);
	UNSET (XIM_CLOSE_REPLY, 0);
	UNSET (XIM_REGISTER_TRIGGERKEYS, 0);
	UNSET (XIM_TRIGGER_NOTIFY, 0);
	UNSET (XIM_TRIGGER_NOTIFY_REPLY, 0);
	UNSET (XIM_SET_EVENT_MASK, 0);
	UNSET (XIM_ENCODING_NEGOTIATION, 0);
	UNSET (XIM_ENCODING_NEGOTIATION_REPLY, 0);
	UNSET (XIM_QUERY_EXTENSION, 0);
	UNSET (XIM_QUERY_EXTENSION_REPLY, 0);
	UNSET (XIM_SET_IM_VALUES, 0);
	UNSET (XIM_SET_IM_VALUES_REPLY, 0);
	UNSET (XIM_GET_IM_VALUES, 0);
	UNSET (XIM_GET_IM_VALUES_REPLY, 0);
	UNSET (XIM_CREATE_IC, 0);
	UNSET (XIM_CREATE_IC_REPLY, 0);
	UNSET (XIM_DESTROY_IC, 0);
	UNSET (XIM_DESTROY_IC_REPLY, 0);
	UNSET (XIM_SET_IC_VALUES, 0);
	UNSET (XIM_SET_IC_VALUES_REPLY, 0);
	UNSET (XIM_GET_IC_VALUES, 0);
	UNSET (XIM_GET_IC_VALUES_REPLY, 0);
	UNSET (XIM_SET_IC_FOCUS, 0);
	UNSET (XIM_UNSET_IC_FOCUS, 0);
	UNSET (XIM_FORWARD_EVENT, 0);
	UNSET (XIM_SYNC, 0);
	UNSET (XIM_SYNC_REPLY, 0);
	UNSET (XIM_COMMIT, 0);
	UNSET (XIM_RESET_IC, 0);
	UNSET (XIM_RESET_IC_REPLY, 0);
	UNSET (XIM_GEOMETRY, 0);
	UNSET (XIM_STR_CONVERSION, 0);
	UNSET (XIM_STR_CONVERSION_REPLY, 0);
	UNSET (XIM_PREEDIT_START, 0);
	UNSET (XIM_PREEDIT_START_REPLY, 0);
	UNSET (XIM_PREEDIT_DRAW, 0);
	UNSET (XIM_PREEDIT_CARET, 0);
	UNSET (XIM_PREEDIT_CARET_REPLY, 0);
	UNSET (XIM_PREEDIT_DONE, 0);
	UNSET (XIM_STATUS_START, 0);
	UNSET (XIM_STATUS_DRAW, 0);
	UNSET (XIM_STATUS_DONE, 0);
	UNSET (XIM_PREEDITSTATE, 0);

#undef UNSET
}
