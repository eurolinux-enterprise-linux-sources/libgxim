/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximsrvtmpl.c
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
#include "gximsrvtmpl.h"


/**
 * SECTION:gximsrvtmpl
 * @Title: GXimServerTemplate
 * @Short_Description: Base class for XIM server
 * @See_Also: #GXimCore
 *
 * GXimServerTemplate provides a common facility to deal with XIM protocol
 * events, particularly to be working on XIM server.
 */

enum {
	PROP_0,
	PROP_SERVER_NAME,
	LAST_PROP
};
enum {
	SIGNAL_0,
	DESTROY,
	GET_SUPPORTED_LOCALES,
	GET_SUPPORTED_TRANSPORT,
	XCONNECT,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };
static const gchar *locales[] = {
	"a3_AZ",
	"aa_DJ",
	"aa_ER",
	"aa_ET",
	"af_ZA",
	"am_ET",
	"an_ES",
	"ar_AA",
	"ar_AE",
	"ar_BH",
	"ar_DZ",
	"ar_EG",
	"ar_IN",
	"ar_IQ",
	"ar_JO",
	"ar_KW",
	"ar_LB",
	"ar_LY",
	"ar_MA",
	"ar_OM",
	"ar_QA",
	"ar_SA",
	"ar_SD",
	"ar_SY",
	"ar_TN",
	"ar_YE",
	"as_IN",
	"ast_ES",
	"az_AZ",
	"be_BY",
	"ber_DZ",
	"ber_MA",
	"bg_BG",
	"bn_BD",
	"bn_IN",
	"br_FR",
	"bs_BA",
	"byn_ER",
	"ca_AD",
	"ca_ES",
	"ca_FR",
	"ca_IT",
	"crh_UA",
	"cs_CZ",
	"csb_PL",
	"cy_GB",
	"cz_CZ",
	"da_DK",
	"de_AT",
	"de_BE",
	"de_CH",
	"de_DE",
	"de_LI",
	"de_LU",
	"dz_BT",
	"el_CY",
	"el_GR",
	"en_AU",
	"en_BW",
	"en_BZ",
	"en_CA",
	"en_DK",
	"en_GB",
	"en_HK",
	"en_IE",
	"en_IN",
	"en_NG",
	"en_NZ",
	"en_PH",
	"en_SG",
	"en_TT",
	"en_UK",
	"en_US",
	"en_ZA",
	"en_ZW",
	"eo",
	"es_AR",
	"es_BO",
	"es_CL",
	"es_CO",
	"es_CR",
	"es_DO",
	"es_EC",
	"es_ES",
	"es_GT",
	"es_HN",
	"es_MX",
	"es_NI",
	"es_PA",
	"es_PE",
	"es_PR",
	"es_PY",
	"es_SV",
	"es_US",
	"es_UY",
	"es_VE",
	"et_EE",
	"eu_ES",
	"eu_FR",
	"fa_IR",
	"fi_FI",
	"fil_PH",
	"fo_FO",
	"fr_BE",
	"fr_CA",
	"fr_CH",
	"fr_FR",
	"fr_LU",
	"fur_IT",
	"fy_DE",
	"fy_NL",
	"ga_IE",
	"gd_GB",
	"gez_ER",
	"gez_ET",
	"gl_ES",
	"gu_IN",
	"gv_GB",
	"ha_NG",
	"he_IL",
	"hi_IN",
	"hr_HR",
	"hsb_DE",
	"hu_HU",
	"hy_AM",
	"ia",
	"id_ID",
	"ig_NG",
	"ik_CA",
	"is_IS",
	"it_CH",
	"it_IT",
	"iu_CA",
	"iw_IL",
	"ja_JP",
	"ka_GE",
	"kk_KZ",
	"kl_GL",
	"km_KH",
	"kn_IN",
	"ko_KR",
	"ks_IN",
	"ku_TR",
	"kw_GB",
	"ky_KG",
	"lg_UG",
	"li_BE",
	"li_NL",
	"lo_LA",
	"lt_LT",
	"lv_LV",
	"mai_IN",
	"mg_MG",
	"mi_NZ",
	"mk_MK",
	"ml_IN",
	"mn_MN",
	"mr_IN",
	"ms_MY",
	"mt_MT",
	"nb_NO",
	"nds_DE",
	"nds_NL",
	"ne_NP",
	"nl_BE",
	"nl_NL",
	"no_NO",
	"nn_NO",
	"nr_ZA",
	"nso_ZA",
	"ny_NO",
	"oc_FR",
	"om_ET",
	"om_KE",
	"or_IN",
	"pa_IN",
	"pa_PK",
	"pap_AN",
	"pd_DE",
	"pd_US",
	"ph_PH",
	"pl_PL",
	"pp_AN",
	"pt_BR",
	"pt_PT",
	"ro_RO",
	"ru_RU",
	"ru_UA",
	"rw_RW",
	"sa_IN",
	"sc_IT",
	"se_NO",
	"si_LK",
	"sid_ET",
	"sk_SK",
	"sl_SI",
	"so_DJ",
	"so_ET",
	"so_KE",
	"so_SO",
	"sq_AL",
	"sr_CS",
	"sr_ME",
	"sr_RS",
	"sr_YU",
	"ss_ZA",
	"st_ZA",
	"sv_FI",
	"sv_SE",
	"ta_IN",
	"te_IN",
	"tg_TJ",
	"th_TH",
	"ti_ER",
	"ti_ET",
	"tig_ER",
	"tk_TM",
	"tl_PH",
	"tn_ZA",
	"tr_CY",
	"tr_TR",
	"ts_ZA",
	"tt_RU",
	"ug_CN",
	"uk_UA",
	"ur_PK",
	"uz_UZ",
	"ve_ZA",
	"vi_VN",
	"wa_BE",
	"wo_SN",
	"xh_ZA",
	"yi_US",
	"yo_NG",
	"zh_CN",
	"zh_HK",
	"zh_SG",
	"zh_TW",
	"zu_ZA",
	NULL
};

G_DEFINE_ABSTRACT_TYPE (GXimServerTemplate, g_xim_srv_tmpl, G_TYPE_XIM_CORE);

/*
 * Private functions
 */
static void
_weak_notify_conn_cb(gpointer  data,
		     GObject  *object)
{
	GXimServerTemplate *server = G_XIM_SRV_TMPL (data);
	GXimCore *core = G_XIM_CORE (data);
	GdkDisplay *dpy = g_xim_core_get_display(core);
	GdkNativeWindow nw;
	GdkWindow *w;

	nw = g_xim_transport_get_native_channel(G_XIM_TRANSPORT (object));
	if (nw) {
		g_xim_messages_debug(core->message, "server/conn",
				     "Removing a connection from the table for %p",
				     G_XIM_NATIVE_WINDOW_TO_POINTER (nw));
		w = g_xim_get_window(dpy, nw);
		g_xim_core_unwatch_event(core, w);
		g_object_unref(w);
		g_hash_table_remove(server->conn_table,
				    G_XIM_NATIVE_WINDOW_TO_POINTER (nw));
	}
	nw = g_xim_transport_get_client_window(G_XIM_TRANSPORT (object));
	if (nw) {
		g_xim_messages_debug(core->message, "server/conn",
				     "Removing a connection from the table for %p",
				     G_XIM_NATIVE_WINDOW_TO_POINTER (nw));
		w = g_xim_get_window(dpy, nw);
		g_xim_core_unwatch_event(core, w);
		g_object_unref(w);
		g_hash_table_remove(server->conn_table,
				    G_XIM_NATIVE_WINDOW_TO_POINTER (nw));
	}
#ifdef GNOME_ENABLE_DEBUG
	g_print("live connection: %d\n", g_hash_table_size(server->conn_table));
#endif
}

static GObject *
g_xim_srv_tmpl_real_constructor(GType                  type,
				guint                  n_construct_properties,
				GObjectConstructParam *construct_properties)
{
	GObject *object, *retval = NULL;
	GXimServerTemplate *srv;
	GXimCore *core;

	object = G_OBJECT_CLASS (g_xim_srv_tmpl_parent_class)->constructor(type,
									   n_construct_properties,
									   construct_properties);
	if (object) {
		gchar *s;

		core = G_XIM_CORE (object);
		srv = G_XIM_SRV_TMPL (object);
		if (srv->server_name == NULL) {
			g_xim_messages_critical(core->message,
						"Property \"server_name\" has to be given.\n");
			goto end;
		}
		s = g_strdup_printf("@server=%s", srv->server_name);
		srv->atom_server = gdk_atom_intern(s, FALSE);
		gdk_flush();
		g_free(s);
		retval = object;
	  end:
		if (retval == NULL)
			g_object_unref(object);
	}

	return retval;
}

static void
g_xim_srv_tmpl_real_set_property(GObject      *object,
				 guint         prop_id,
				 const GValue *value,
				 GParamSpec   *pspec)
{
	GXimServerTemplate *srv = G_XIM_SRV_TMPL (object);

	switch (prop_id) {
	    case PROP_SERVER_NAME:
		    srv->server_name = g_strdup(g_value_get_string(value));
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_srv_tmpl_real_get_property(GObject    *object,
				 guint       prop_id,
				 GValue     *value,
				 GParamSpec *pspec)
{
	GXimServerTemplate *srv = G_XIM_SRV_TMPL (object);

	switch (prop_id) {
	    case PROP_SERVER_NAME:
		    g_value_set_string(value, srv->server_name);
		    break;
	    default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		    break;
	}
}

static void
g_xim_srv_tmpl_real_dispose(GObject *object)
{
	GXimCore *core = G_XIM_CORE (object);
	GdkWindow *w;

	w = g_xim_core_get_selection_window(core);
	if (w)
		g_xim_core_unwatch_event(core, w);

	if (G_OBJECT_CLASS (g_xim_srv_tmpl_parent_class)->dispose)
		(* G_OBJECT_CLASS (g_xim_srv_tmpl_parent_class)->dispose) (object);
}

static void
g_xim_srv_tmpl_real_finalize(GObject *object)
{
	GXimServerTemplate *srv = G_XIM_SRV_TMPL (object);

	g_free(srv->server_name);
	g_hash_table_destroy(srv->conn_table);

	if (G_OBJECT_CLASS (g_xim_srv_tmpl_parent_class)->finalize)
		(* G_OBJECT_CLASS (g_xim_srv_tmpl_parent_class)->finalize) (object);
}

static GdkFilterReturn
g_xim_srv_tmpl_real_translate_events(GXimCore  *core,
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
		    g_xim_srv_tmpl_remove_connection(G_XIM_SRV_TMPL (core),
						     xev->xdestroywindow.window);
#ifdef GNOME_ENABLE_DEBUG
		    g_print("destroyed a server connection %p\n", (gpointer)(gulong)xev->xdestroywindow.window);
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
g_xim_srv_tmpl_real_selection_request_event(GXimCore          *core,
					    GdkEventSelection *event)
{
	GXimServerTemplate *srv = G_XIM_SRV_TMPL (core);
	gchar *ret = NULL, *p = NULL;
	GError *error = NULL;

	if (event->property == core->atom_locales) {
		g_xim_messages_debug(core->message, "server/event",
				     "%p -> SelectionRequest[LOCALES]",
				     G_XIM_NATIVE_WINDOW_TO_POINTER (event->requestor));
		g_signal_emit(core, signals[GET_SUPPORTED_LOCALES], 0, &ret);
		p = g_strdup_printf("@locale=%s", ret);
	} else if (event->property == core->atom_transport) {
		g_xim_messages_debug(core->message, "server/event",
				     "%p -> SelectionRequest[TRANSPORT]",
				     G_XIM_NATIVE_WINDOW_TO_POINTER (event->requestor));
		g_signal_emit(core, signals[GET_SUPPORTED_TRANSPORT], 0, &ret);
		p = g_strdup_printf("@transport=%s", ret);
	} else {
		gchar *s = gdk_atom_name(event->property);

		g_xim_messages_warning(core->message,
				       "%p -> Unsupported property `%s' requested",
				       G_XIM_NATIVE_WINDOW_TO_POINTER (event->requestor), s);
		g_free(s);

		return FALSE;
	}

	g_xim_srv_tmpl_send_selection_notify(srv, event, p, strlen(p) + 1, &error);
	g_xim_messages_debug(core->message, "server/event",
			     "%p <- SelectionNotify[%s]\n  result: %s",
			     G_XIM_NATIVE_WINDOW_TO_POINTER (event->requestor),
			     p, error ? error->message : "success");
	if (error)
		g_error_free(error);
	g_free(ret);
	g_free(p);

	return TRUE;
}

static gboolean
g_xim_srv_tmpl_real_selection_clear_event(GXimCore          *core,
					  GdkEventSelection *event)
{
	GXimServerTemplate *srv = G_XIM_SRV_TMPL (core);

	if (event->selection == srv->atom_server) {
		g_signal_emit(core, signals[DESTROY], 0);

		return TRUE;
	}

	return FALSE;
}

static gboolean
g_xim_srv_tmpl_real_selection_notify_event(GXimCore          *core,
					   GdkEventSelection *event)
{
	g_xim_messages_bug(core->message,
			   "SelectionNotify is unlikely to appear in the server instance.");

	return FALSE;
}

static gboolean
g_xim_srv_tmpl_real_expose_event(GXimCore       *core,
				 GdkEventExpose *event)
{
	g_xim_messages_warning(core->message, "FIXME!! received a Expose event.");

	return FALSE;
}

static gboolean
g_xim_srv_tmpl_real_destroy_event(GXimCore    *core,
				  GdkEventAny *event)
{
	/* Nothing we can do */

	return FALSE;
}

static gboolean
g_xim_srv_tmpl_real_client_event(GXimCore       *core,
				 GdkEventClient *event)
{
	static gboolean is_atom_registered = FALSE;
	gboolean retval = TRUE;

	if (event->message_type == core->atom_xim_xconnect) {
		GdkNativeWindow w;
		GXimConnection *conn = NULL;
		GXimProtocolIface *iface;

		g_signal_emit(core, signals[XCONNECT], 0,
			      event, &conn);
		if (conn == NULL) {
			g_xim_messages_error(core->message,
					     "Unable to create a connection instance.");
			return TRUE;
		}
		if (!is_atom_registered) {
			iface = G_XIM_PROTOCOL_GET_IFACE (conn);
			g_xim_core_add_client_message_filter(core, iface->atom_xim_protocol);
			g_xim_core_add_client_message_filter(core, iface->atom_xim_moredata);
			is_atom_registered = TRUE;
		}
		/* Add a proc to the hash table here,
		 * because comm_window would be setup in
		 * xim_xconnect signal.
		 */
		w = g_xim_transport_get_native_channel(G_XIM_TRANSPORT (conn));
		g_xim_srv_tmpl_add_connection(G_XIM_SRV_TMPL (core), conn, w);
	} else {
		GXimConnection *conn = g_xim_srv_tmpl_lookup_connection(G_XIM_SRV_TMPL (core),
									event->window);

		if (conn == NULL) {
			if (g_xim_core_lookup_client_message_filter(core, event->message_type)) {
				gchar *s = gdk_atom_name(event->message_type);

				g_xim_messages_warning(core->message,
						       "Event was sent from the unknown client: message_type: %s",
						       s);
				g_free(s);
			}
		} else {
			GError *error = NULL;

			retval = g_xim_protocol_process_event(G_XIM_PROTOCOL (conn),
							      event, &error);
			if (error) {
				g_xim_messages_gerror(core->message, error);
				g_error_free(error);
			}
		}
	}

	return retval;
}

static gchar *
g_xim_srv_tmpl_real_get_supported_locales(GXimServerTemplate *server)
{
	gchar *supported_locales;

	supported_locales = g_strjoinv(",", (gchar **)locales);

	return supported_locales;
}

static gchar *
g_xim_srv_tmpl_real_get_supported_transport(GXimServerTemplate *server)
{
	static const gchar *supported_transport = "X/";

	return g_strdup(supported_transport);
}

static GXimConnection *
g_xim_srv_tmpl_real_xconnect(GXimServerTemplate *server,
			     GdkEventClient     *event)
{
	GdkNativeWindow w = event->data.l[0], comm_window;
	GXimConnection *conn = g_xim_srv_tmpl_lookup_connection_with_native_window(server, w);
	GXimCore *core = G_XIM_CORE (server);
	GXimTransport *trans;
	GdkDisplay *dpy = g_xim_core_get_display(core);
	GType gtype;
	GdkWindow *window;
	GdkEvent *ev;

	if (conn) {
		g_xim_messages_warning(core->message,
				       "Received XIM_XCONNECT event from %p again",
				       G_XIM_NATIVE_WINDOW_TO_POINTER (w));
		return g_object_ref(conn);
	}
	gtype = g_xim_core_get_connection_gtype(core);
	if (!g_type_is_a(gtype, G_TYPE_XIM_CONNECTION)) {
		g_xim_messages_bug(core->message,
				   "given GObject type isn't inherited from GXimConnection");
		return NULL;
	}
	conn = G_XIM_CONNECTION (g_object_new(gtype,
					      "proto_signals", g_xim_core_get_protocol_signal_connector(core),
					      NULL));
	g_xim_core_setup_connection(core, conn);
	trans = G_XIM_TRANSPORT (conn);
	g_xim_transport_set_display(trans, dpy);
	g_xim_transport_set_client_window(trans, w);
	/* supposed to do create a comm_window.
	 * but what we really want here is GdkNativeWindow.
	 * so the return value of g_xim_transport_get_channel
	 * won't be used then.
	 */
	g_xim_transport_get_channel(trans, event->window);
	g_xim_core_add_client_message_filter(core,
					     g_xim_transport_get_atom(G_XIM_TRANSPORT (conn)));
	g_xim_srv_tmpl_add_connection(server, conn, w);
	g_object_weak_ref(G_OBJECT (conn),
			  _weak_notify_conn_cb,
			  server);

	comm_window = g_xim_transport_get_native_channel(trans);
	g_xim_messages_debug(core->message, "server/event",
			     "%p -> XIM_XCONNECT",
			     G_XIM_NATIVE_WINDOW_TO_POINTER (w));
	window = g_xim_get_window(dpy, w);
	ev = gdk_event_new(GDK_CLIENT_EVENT);
	G_XIM_CHECK_ALLOC_WITH_CODE (ev, g_object_unref(conn), NULL);

	ev->client.window = window;
	ev->client.message_type = event->message_type;
	ev->client.data_format = 32;
	ev->client.data.l[0] = (long)comm_window;
	/* We support the all of transport methods.
	 * so just following what the client is requesting.
	 */
	g_xim_transport_set_version(trans, event->data.l[1], event->data.l[2]);
	ev->client.data.l[1] = event->data.l[1];
	ev->client.data.l[2] = event->data.l[2];
	ev->client.data.l[3] = g_xim_transport_get_transport_max(trans);

	g_xim_messages_debug(core->message, "server/event",
			     "%p <- XIM_XCONNECT[%p]",
			     G_XIM_NATIVE_WINDOW_TO_POINTER (w),
			     G_XIM_NATIVE_WINDOW_TO_POINTER (comm_window));
	gdk_event_send_client_message_for_display(dpy, ev, w);

	/* gdk_event_free does unref GdkWindow.
	 * So we don't need to invoke gdk_window_destroy explicitly.
	 */
	gdk_event_free(ev);

	return conn;
}

static void
g_xim_srv_tmpl_class_init(GXimServerTemplateClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GXimCoreClass *core_class = G_XIM_CORE_CLASS (klass);

	object_class->constructor  = g_xim_srv_tmpl_real_constructor;
	object_class->set_property = g_xim_srv_tmpl_real_set_property;
	object_class->get_property = g_xim_srv_tmpl_real_get_property;
	object_class->dispose      = g_xim_srv_tmpl_real_dispose;
	object_class->finalize     = g_xim_srv_tmpl_real_finalize;

	core_class->translate_events        = g_xim_srv_tmpl_real_translate_events;
	core_class->selection_request_event = g_xim_srv_tmpl_real_selection_request_event;
	core_class->selection_clear_event   = g_xim_srv_tmpl_real_selection_clear_event;
	core_class->selection_notify_event  = g_xim_srv_tmpl_real_selection_notify_event;
	core_class->expose_event            = g_xim_srv_tmpl_real_expose_event;
	core_class->destroy_event           = g_xim_srv_tmpl_real_destroy_event;
	core_class->client_event            = g_xim_srv_tmpl_real_client_event;

	klass->get_supported_locales   = g_xim_srv_tmpl_real_get_supported_locales;
	klass->get_supported_transport = g_xim_srv_tmpl_real_get_supported_transport;
	klass->xconnect                = g_xim_srv_tmpl_real_xconnect;

	/* properties */
	/**
	 * GXimServerTemplate:server-name:
	 *
	 * XIM server name that is used to identify the XIM server through
	 * X property, such as XIM_SERVERS. also it's used to identify for
	 * client applications with XMODIFIERS.
	 */
	g_object_class_install_property(object_class, PROP_SERVER_NAME,
					g_param_spec_string("server_name",
							    _("XIM server name"),
							    _("XIM server name that is used to identify the server through X property"),
							    NULL,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	/* signals */
	/**
	 * GXimServerTemplate::destroy:
	 * @srvtmpl: the object which received the signal.
	 *
	 * The ::destroy signal will be emitted when the instance is going to
	 * be destroyed. you can do something with it if you want.
	 */
	signals[DESTROY] = g_signal_new("destroy",
					G_OBJECT_CLASS_TYPE (klass),
					G_SIGNAL_RUN_FIRST,
					G_STRUCT_OFFSET (GXimServerTemplateClass, destroy),
					NULL, NULL,
					gxim_marshal_VOID__VOID,
					G_TYPE_NONE, 0);
	/**
	 * GXimServerTemplate::get-supported-locales:
	 * @srvtmpl: the object which received the signal.
	 *
	 * The ::get-supported-locales signal will be emitted when the client
	 * application is connecting and requesting which locales XIM server
	 * would supports.
	 *
	 * This is a convenience signal to deal with
	 * #GXimCore::selection-request-event for %LOCALES request.
	 *
	 * Returns: a locales string that XIM server would supports. multiple
	 * locales would be separated with comma.
	 */
	signals[GET_SUPPORTED_LOCALES] = g_signal_new("get_supported_locales",
						      G_OBJECT_CLASS_TYPE (klass),
						      G_SIGNAL_RUN_LAST,
						      G_STRUCT_OFFSET (GXimServerTemplateClass, get_supported_locales),
						      _gxim_acc_signal_accumulator__STRING,
						      NULL,
						      gxim_marshal_STRING__VOID,
						      G_TYPE_STRING, 0);
	/**
	 * GXimServerTemplate::get-supported-transport:
	 * @srvtmpl: the object which received the signal.
	 *
	 * The ::get-supported-transport signal will be emitted when the client
	 * application is connecting and requesting which transport-specific names
	 * XIM server would supports.
	 *
	 * This is a convenience signal to deal with
	 * #GXimCore::selection-request-event for %TRANSPORT request.
	 *
	 * Returns: a transport-specific names that XIM server would supports.
	 * In general, you may just need to support "X/" for X Window System.
	 *
	 * Multiple transport-specific names would be separated with comma.
	 * and evaluated in order of the list.
	 */
	signals[GET_SUPPORTED_TRANSPORT] = g_signal_new("get_supported_transport",
							G_OBJECT_CLASS_TYPE (klass),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET (GXimServerTemplateClass, get_supported_transport),
							_gxim_acc_signal_accumulator__STRING,
							NULL,
							gxim_marshal_STRING__VOID,
							G_TYPE_STRING, 0);
	/**
	 * GXimServerTemplate::xconnect:
	 * @srvtmpl: the object which received the signal.
	 * @event: the #GdkEventClient which triggered this signal.
	 *
	 * The ::xconnect signal will be emitted when %XIM_XCONNECT event is
	 * dispatched from the client applications in order to establish
	 * a connection between XIM server and a client.
	 * During processing this signal, it would be supposed to prepare
	 * a #GXimConnection instance and sends back %XIM_XCONNECT reply to
	 * the client. See The Input Method Protocol, XIM specification document
	 * for more details about %XIM_XCONNECT.
	 *
	 * Returns: a #GXimConnection for this session.
	 */
	signals[XCONNECT] = g_signal_new("xconnect",
					 G_OBJECT_CLASS_TYPE (klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET (GXimServerTemplateClass, xconnect),
					 NULL, NULL,
					 gxim_marshal_OBJECT__BOXED,
					 G_TYPE_XIM_CONNECTION, 1,
					 GDK_TYPE_EVENT);
}

static void
g_xim_srv_tmpl_init(GXimServerTemplate *server)
{
	server->conn_table = g_hash_table_new(g_direct_hash, g_direct_equal);
}

/*
 * Public functions
 */
GQuark
g_xim_srv_tmpl_get_error_quark(void)
{
	static GQuark quark = 0;

	if (!quark)
		quark = g_quark_from_static_string("g-xim-srv-tmpl-error");

	return quark;
}

/**
 * g_xim_srv_tmpl_is_running:
 * @srvtmpl: a #GXimServerTemplate.
 * @error: a location to store error, or %NULL.
 *
 * Checks if XIM server process is already running at @srvtmpl.
 *
 * Returns: %TRUE when XIM server process is already running at @srvtmpl.
 *  Otherwise %FALSE.
 */
gboolean
g_xim_srv_tmpl_is_running(GXimServerTemplate  *srvtmpl,
			  GError             **error)
{
	GXimCore *core;
	GdkDisplay *dpy;
	GdkWindow *owner, *selection_window;

	/* this function would be mainly used to bring the server up.
	 * so returning TRUE would be reasonable for any unexpected errors
	 * to avoid next step.
	 */
	g_return_val_if_fail (G_IS_XIM_SRV_TMPL (srvtmpl), TRUE);

	core = G_XIM_CORE (srvtmpl);
	dpy = g_xim_core_get_display(core);
	owner = g_xim_get_selection_owner(dpy, srvtmpl->atom_server);
	selection_window = g_xim_core_get_selection_window(core);

	if (error) {
		if (owner == selection_window)
			g_set_error(error, G_XIM_SRV_TMPL_ERROR,
				    G_XIM_SRV_TMPL_ERROR_SAME_SERVER_IS_RUNNING,
				    _("This server instance is already running"));
		else if (owner != NULL)
			g_set_error(error, G_XIM_SRV_TMPL_ERROR,
				    G_XIM_SRV_TMPL_ERROR_ANOTHER_SERVER_IS_RUNNING,
				    _("Another server instance is already running"));
	}

	return owner != NULL;
}

/**
 * g_xim_srv_tmpl_take_ownership:
 * @srvtmpl: a #GXimServerTemplate.
 * @force: %TRUE to take an ownership forcibly. %FALSE to fail if the XIM server
 *  is already running.
 * @error: a location to store error, or %NULL.
 *
 * Starts XIM server if able to take an ownership for
 * #GXimServerTemplate:server_name in X property.
 *
 * Returns: %TRUE when XIM serrver could be started.
 */
gboolean
g_xim_srv_tmpl_take_ownership(GXimServerTemplate  *srvtmpl,
			      gboolean             force,
			      GError             **error)
{
	GXimCore *core;
	GdkDisplay *dpy;
	GdkWindow *owner, *selection_window, *root;
	GdkEvent *ev;
	GdkPropMode mode = GDK_PROP_MODE_PREPEND;
	gboolean changed = TRUE, is_valid;
	guint32 error_code;

	g_return_val_if_fail (G_IS_XIM_SRV_TMPL (srvtmpl), FALSE);

	if (g_xim_srv_tmpl_is_running(srvtmpl, error) &&
	    force == FALSE)
		return FALSE;

	core = G_XIM_CORE (srvtmpl);
	dpy = g_xim_core_get_display(core);
	owner = g_xim_get_selection_owner(dpy, srvtmpl->atom_server);
	if (owner != NULL) {
		/* taking over the ownership */
		g_xim_error_push();
		gdk_window_set_events(owner, GDK_STRUCTURE_MASK);
		error_code = g_xim_error_pop();
		if (G_XIM_ERROR_DECODE_X_ERROR_CODE (error_code) != 0)
			owner = NULL;
	}

	selection_window = g_xim_core_get_selection_window(core);
	gdk_selection_owner_set_for_display(dpy, selection_window,
					    srvtmpl->atom_server, 0L, TRUE);
	gdk_window_set_events(selection_window, GDK_ALL_EVENTS_MASK);
	gdk_display_sync(dpy);

	if (g_xim_get_selection_owner(dpy, srvtmpl->atom_server) != selection_window) {
		g_set_error(error, G_XIM_SRV_TMPL_ERROR,
			    G_XIM_SRV_TMPL_ERROR_UNABLE_TO_ACQUIRE_SERVER_OWNER,
			    _("Unable to acquire the XIM server owner for `%s'"),
			    srvtmpl->server_name);
		return FALSE;
	}

	/* notify taking an ownership to previous owner */
	ev = gdk_event_new(GDK_CLIENT_EVENT);
	G_XIM_CHECK_ALLOC (ev, FALSE);

	root = gdk_screen_get_root_window(gdk_display_get_default_screen(dpy));
	ev->client.window = g_object_ref(root);
	ev->client.message_type = srvtmpl->atom_server;
	ev->client.data_format = 32;
	ev->client.data.l[0] = 0L;
	ev->client.data.l[1] = (long)srvtmpl->atom_server;
	gdk_event_send_client_message_for_display(dpy, ev, GDK_WINDOW_XID (ev->client.window));
	gdk_event_free(ev);

	/* Add XIM server */
	gdk_x11_display_grab(dpy);

	if (g_xim_lookup_xim_server(dpy, srvtmpl->atom_server, &is_valid)) {
		mode = GDK_PROP_MODE_APPEND;
		changed = FALSE;
	} else {
		if (!is_valid)
			mode = GDK_PROP_MODE_REPLACE;
	}
	g_xim_error_push();
	gdk_property_change(gdk_screen_get_root_window(gdk_display_get_default_screen(dpy)),
			    core->atom_xim_servers, GDK_SELECTION_TYPE_ATOM,
			    32, mode, (const guchar *)&srvtmpl->atom_server, changed ? 1 : 0);
	error_code = g_xim_error_pop();

	gdk_x11_display_ungrab(dpy);

	if (error_code != 0) {
		gchar *s = gdk_atom_name(srvtmpl->atom_server);

		g_set_error(error, G_XIM_SRV_TMPL_ERROR,
			    G_XIM_SRV_TMPL_ERROR_UNABLE_TO_ADD_SERVER,
			    _("Unable to add a server atom `%s'."),
			    s);
		g_free(s);

		return FALSE;
	}

	g_xim_core_watch_event(core, selection_window);

	return TRUE;
}

/**
 * g_xim_srv_tmpl_send_selection_notify:
 * @srvtmpl: a #GXimServerTemplate.
 * @event: the #GdkEventSelection to send.
 * @data: the chunks of data to send with %SelectionNotify.
 * @length: the number of bytes of @data to send.
 * @error: a location to store error, or %NULL.
 *
 * Sends @data with %SelectionNotify. which mainly be used to deliver the result
 * of %LOCALES and %TRANSPORT request.
 *
 * Returns: %TRUE if @data is successfully sent.
 */
gboolean
g_xim_srv_tmpl_send_selection_notify(GXimServerTemplate  *srvtmpl,
				     GdkEventSelection   *event,
				     const gchar         *data,
				     gsize                length,
				     GError             **error)
{
	GdkWindow *window;
	guint32 error_code;
	GdkDisplay *dpy = g_xim_core_get_display(G_XIM_CORE (srvtmpl));

	g_xim_error_push();
	window = g_xim_get_window(dpy, event->requestor);
	gdk_property_change(window, event->property, event->target,
			    8, GDK_PROP_MODE_REPLACE, (const guchar *)data, length);
	g_object_unref(window);
	gdk_selection_send_notify_for_display(dpy,
					      event->requestor,
					      event->selection,
					      event->target,
					      event->property,
					      0L);

	error_code = g_xim_error_pop();
	if (error_code != 0) {
		if (error) {
			/* XXX: how to get detailed message without X deps? */
			g_set_error(error, G_XIM_SRV_TMPL_ERROR,
				    G_XIM_SRV_TMPL_ERROR_UNABLE_TO_SEND_PROPERTY_NOTIFY,
				    "Unable to send a PropertyNotify:\n"
				    " Details: request_code: %d minor_code: %d",
				    G_XIM_ERROR_DECODE_X_REQUEST_CODE (error_code),
				    G_XIM_ERROR_DECODE_X_MINOR_CODE (error_code));
		}

		return FALSE;
	}

	return TRUE;
}

/**
 * g_xim_srv_tmpl_add_connection:
 * @srvtmpl: a #GXimServerTemplate.
 * @conn: the #GXimConnection to get it managed under @srvtmpl.
 * @window: the #GdkNativeWindow to make reference.
 *
 * Puts @conn to get it managed under @srvtmpl. @window will be referred to
 * deliver events to the right place. i.e. @conn to deal with events for @window.
 */
void
g_xim_srv_tmpl_add_connection(GXimServerTemplate *srvtmpl,
			      GXimConnection     *conn,
			      GdkNativeWindow     window)
{
	gpointer p;
	GdkWindow *w;
	GdkDisplay *dpy;
	GdkEventMask mask;
	GXimCore *core;

	g_return_if_fail (G_IS_XIM_SRV_TMPL (srvtmpl));
	g_return_if_fail (G_IS_XIM_CONNECTION (conn));
	g_return_if_fail (window != 0);

	core = G_XIM_CORE (srvtmpl);
	dpy = g_xim_core_get_display(core);
	p = G_XIM_NATIVE_WINDOW_TO_POINTER (window);
	g_xim_messages_debug(core->message, "server/conn",
			     "Inserting a connection %p to the table with %p",
			     conn, p);
	g_hash_table_insert(srvtmpl->conn_table, p, conn);
	w = g_xim_get_window(dpy, window);
	mask = gdk_window_get_events(w);
	gdk_window_set_events(w, mask | GDK_STRUCTURE_MASK);
	g_xim_core_watch_event(core, w);
	g_object_unref(w);
}

/**
 * g_xim_srv_tmpl_remove_connection:
 * @srvtmpl: a #GXimServerTemplate.
 * @window: the #GdkNativeWindow to get rid of the connection. which should be
 *  added with g_xim_srv_tmpl_add_connection().
 *
 * Gets rid of the connection, which managed under @srvtmpl. @window will be
 * used as the key to remove it.
 */
void
g_xim_srv_tmpl_remove_connection(GXimServerTemplate *srvtmpl,
				 GdkNativeWindow     window)
{
	GXimConnection *conn;

	g_return_if_fail (G_IS_XIM_SRV_TMPL (srvtmpl));
	g_return_if_fail (window != 0);

	conn = g_hash_table_lookup(srvtmpl->conn_table,
				   G_XIM_NATIVE_WINDOW_TO_POINTER (window));
	/* We don't need to remove an instance from the hash.
	 * That should be done in _weak_notify_conn_cb.
	 */
	if (conn) {
		g_xim_messages_debug(G_XIM_CORE (srvtmpl)->message, "server/conn",
				     "Unreferencing a server connection %p",
				     G_XIM_NATIVE_WINDOW_TO_POINTER (window));
		g_object_unref(conn);
	}
}

/**
 * g_xim_srv_tmpl_lookup_connection:
 * @srvtmpl: a #GXimServerTemplate.
 * @window: the #GdkWindow to look up the connection.
 *
 * Looks up the connection which is reference with @window. this is
 * a convenience function to look up with #GdkWindow. use
 * g_xim_srv_tmpl_lookup_connection_with_native_window() for #GdkNativeWindow
 * if you like.
 *
 * Returns: a #GXimConnection, or %NULL if the key is not found.
 */
GXimConnection *
g_xim_srv_tmpl_lookup_connection(GXimServerTemplate *srvtmpl,
				 GdkWindow          *window)
{
	GdkNativeWindow w;

	g_return_val_if_fail (G_IS_XIM_SRV_TMPL (srvtmpl), NULL);
	g_return_val_if_fail (GDK_IS_WINDOW (window), NULL);

	w = GDK_WINDOW_XID (window);

	return g_xim_srv_tmpl_lookup_connection_with_native_window(srvtmpl, w);
}

/**
 * g_xim_srv_tmpl_lookup_connection_with_native_window:
 * @srvtmpl: a #GXimServerTemplate.
 * @window: the #GdkNativeWindow to look up the connection.
 *
 * Looks up the connection which is reference with @window.
 *
 * Returns: a #GXimConnection, or %NULL if the key is not found.
 */
GXimConnection *
g_xim_srv_tmpl_lookup_connection_with_native_window(GXimServerTemplate *srvtmpl,
						    GdkNativeWindow     window)
{
	g_return_val_if_fail (G_IS_XIM_SRV_TMPL (srvtmpl), NULL);
	g_return_val_if_fail (window != 0, NULL);

	return g_hash_table_lookup(srvtmpl->conn_table,
				   G_XIM_NATIVE_WINDOW_TO_POINTER (window));
}
