/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximmisc.c
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

#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <gdk/gdkprivate.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include "gximattr.h"
#include "gximerror.h"
#include "gximmessage.h"
#include "gximmisc.h"
#include "gximprotocol.h"

#define N_REALLOC_NESTED_LIST	16

static GXimMessage *master_message = NULL;

/*
 * Private functions
 */
static gint
g_xim_gdkevent_translate_event_type(GdkEvent *event)
{
	switch (event->type) {
	    case GDK_NOTHING:
	    case GDK_DELETE:
		    goto unsupported;
	    case GDK_DESTROY:
		    return DestroyNotify;
	    case GDK_EXPOSE:
		    return Expose;
	    case GDK_MOTION_NOTIFY:
		    return MotionNotify;
	    case GDK_BUTTON_PRESS:
	    case GDK_2BUTTON_PRESS:
	    case GDK_3BUTTON_PRESS:
		    return ButtonPress;
	    case GDK_BUTTON_RELEASE:
		    return ButtonRelease;
	    case GDK_KEY_PRESS:
		    return KeyPress;
	    case GDK_KEY_RELEASE:
		    return KeyRelease;
	    case GDK_ENTER_NOTIFY:
		    return EnterNotify;
	    case GDK_LEAVE_NOTIFY:
		    return LeaveNotify;
	    case GDK_FOCUS_CHANGE:
		    if (event->focus_change.in)
			    return FocusIn;
		    else
			    return FocusOut;
	    case GDK_CONFIGURE:
		    return ConfigureNotify;
	    case GDK_MAP:
		    return MapNotify;
	    case GDK_UNMAP:
		    return UnmapNotify;
	    case GDK_PROPERTY_NOTIFY:
		    return PropertyNotify;
	    case GDK_SELECTION_CLEAR:
		    return SelectionClear;
	    case GDK_SELECTION_REQUEST:
		    return SelectionRequest;
	    case GDK_SELECTION_NOTIFY:
		    return SelectionNotify;
	    case GDK_PROXIMITY_IN:
	    case GDK_PROXIMITY_OUT:
	    case GDK_DRAG_ENTER:
	    case GDK_DRAG_LEAVE:
	    case GDK_DRAG_MOTION:
	    case GDK_DRAG_STATUS:
	    case GDK_DROP_START:
	    case GDK_DROP_FINISHED:
		    goto unsupported;
	    case GDK_CLIENT_EVENT:
		    return ClientMessage;
	    case GDK_VISIBILITY_NOTIFY:
		    return VisibilityNotify;
	    case GDK_NO_EXPOSE:
		    return NoExpose;
	    case GDK_SCROLL:
		    return ButtonPress;
	    case GDK_WINDOW_STATE:
	    case GDK_SETTING:
	    case GDK_OWNER_CHANGE:
	    case GDK_GRAB_BROKEN:
	    default:
	    unsupported:
		    break;
	}

	return 0;
}

static GdkEventType
g_xim_xevent_translate_event_type(gint type)
{
	switch (type & 0x7f) {
	    case KeyPress:
		    return GDK_KEY_PRESS;
	    case KeyRelease:
		    return GDK_KEY_RELEASE;
	    case ButtonPress:
		    return GDK_BUTTON_PRESS;
	    case ButtonRelease:
		    return GDK_BUTTON_RELEASE;
	    case MotionNotify:
		    return GDK_MOTION_NOTIFY;
	    case EnterNotify:
		    return GDK_ENTER_NOTIFY;
	    case LeaveNotify:
		    return GDK_LEAVE_NOTIFY;
	    case FocusIn:
	    case FocusOut:
		    return GDK_FOCUS_CHANGE;
	    case KeymapNotify:
		    goto unsupported;
	    case Expose:
	    case GraphicsExpose:
		    return GDK_EXPOSE;
	    case NoExpose:
		    return GDK_NO_EXPOSE;
	    case VisibilityNotify:
		    return GDK_VISIBILITY_NOTIFY;
	    case CreateNotify:
		    goto unsupported;
	    case DestroyNotify:
		    return GDK_DESTROY;
	    case UnmapNotify:
		    return GDK_UNMAP;
	    case MapNotify:
		    return GDK_MAP;
	    case MapRequest:
	    case ReparentNotify:
		    goto unsupported;
	    case ConfigureNotify:
		    return GDK_CONFIGURE;
	    case ConfigureRequest:
	    case GravityNotify:
	    case ResizeRequest:
	    case CirculateNotify:
	    case CirculateRequest:
		    goto unsupported;
	    case PropertyNotify:
		    return GDK_PROPERTY_NOTIFY;
	    case SelectionClear:
		    return GDK_SELECTION_CLEAR;
	    case SelectionRequest:
		    return GDK_SELECTION_REQUEST;
	    case SelectionNotify:
		    return GDK_SELECTION_NOTIFY;
	    case ColormapNotify:
		    goto unsupported;
	    case ClientMessage:
		    return GDK_CLIENT_EVENT;
	    case MappingNotify:
		    goto unsupported;
	    default:
	    unsupported:
		    break;
	}

	return GDK_NOTHING;
}

static const gchar *
g_xim_gdkevent_type_name(gint type)
{
	static const gchar *event_names =
		"\0"
		"KeyPress\0"
		"KeyRelease\0"
		"ButtonPress\0"
		"ButtonRelease\0"
		"MotionNotify\0"
		"EnterNotify\0"
		"LeaveNotify\0"
		"FocusIn\0"
		"FocusOut\0"
		"KeymapNotify\0"
		"Expose\0"
		"GraphicsExpose\0"
		"NoExpose\0"
		"VisibilityNotify\0"
		"CreateNotify\0"
		"DestroyNotify\0"
		"UnmapNotify\0"
		"MapNotify\0"
		"ReparentNotify\0"
		"ConfigureNotify\0"
		"ConfigureRequest\0"
		"GravityNotify\0"
		"ResizeRequest\0"
		"CirculateNotify\0"
		"CirculateRequest\0"
		"PropertyNotify\0"
		"SelectionClear\0"
		"SelectionRequest\0"
		"SelectionNotify\0"
		"ColormapNotify\0"
		"ClientMessage\0"
		"MappingNotify\0";
	static guint type2pos[LASTEvent];
	static gboolean initialized = FALSE;

	if (!initialized) {
		memset(type2pos, 0, sizeof (guint) * (LASTEvent - 1));
		type2pos[KeyPress] = 1;
		type2pos[KeyRelease] = 10;
		type2pos[ButtonPress] = 21;
		type2pos[ButtonRelease] = 33;
		type2pos[MotionNotify] = 47;
		type2pos[EnterNotify] = 60;
		type2pos[LeaveNotify] = 72;
		type2pos[FocusIn] = 84;
		type2pos[FocusOut] = 92;
		type2pos[KeymapNotify] = 101;
		type2pos[Expose] = 114;
		type2pos[GraphicsExpose] = 121;
		type2pos[NoExpose] = 136;
		type2pos[VisibilityNotify] = 145;
		type2pos[CreateNotify] = 162;
		type2pos[DestroyNotify] = 175;
		type2pos[UnmapNotify] = 189;
		type2pos[MapNotify] = 201;
		type2pos[MapRequest] = 211;
		type2pos[ReparentNotify] = 226;
		type2pos[ConfigureNotify] = 242;
		type2pos[ConfigureRequest] = 259;
		type2pos[GravityNotify] = 273;
		type2pos[ResizeRequest] = 287;
		type2pos[CirculateNotify] = 303;
		type2pos[CirculateRequest] = 320;
		type2pos[PropertyNotify] = 335;
		type2pos[SelectionClear] = 350;
		type2pos[SelectionRequest] = 367;
		type2pos[SelectionNotify] = 383;
		type2pos[ColormapNotify] = 398;
		type2pos[ClientMessage] = 412;
		type2pos[MappingNotify] = 426;

		initialized = TRUE;
	}

	return (&event_names[type2pos[type]])[0] == 0 ? "<UnknownEvent>" : &event_names[type2pos[type]];
}

static GXimNestedListNode *
g_xim_nested_list_node_new(void)
{
	GXimNestedListNode *retval;

	retval = g_new0(GXimNestedListNode, 1);
	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->vtype = G_XIM_TYPE_INVALID;

	return retval;
}

static GXimNestedListNode *
g_xim_nested_list_node_copy(GXimNestedListNode *node)
{
	GXimNestedListNode *retval;
	GType gtype;

	g_return_val_if_fail (node != NULL, NULL);

	gtype = g_xim_value_type_to_gtype(node->vtype);
	g_return_val_if_fail (gtype != G_TYPE_INVALID, NULL);

	retval = g_xim_nested_list_node_new();
	retval->vtype = node->vtype;
	retval->name = g_strdup(node->name);
	retval->value = g_xim_copy_by_gtype(gtype, node->value);

	return retval;
}

static void
g_xim_nested_list_node_free(GXimNestedListNode *node)
{
	GType gtype;

	if (node == NULL)
		return;

	gtype = g_xim_value_type_to_gtype(node->vtype);
	if (gtype == G_TYPE_INVALID)
		g_warning("Invalid NestedList node for %s to be freed", node->name);
	g_free(node->name);
	g_xim_free_by_gtype(gtype, node->value);

	g_free(node);
}

/*
 * Public functions
 */
void
g_xim_init(void)
{
	const gchar *env;
	gchar **tokens;
	gint i;

	if (master_message == NULL) {
		master_message = g_xim_message_new();
		G_XIM_CHECK_ALLOC_WITH_NO_RET (master_message);

		g_object_set(G_OBJECT (master_message), "master", TRUE, NULL);
	}
	env = g_getenv("LIBGXIM_DEBUG");
	if (env) {
		tokens = g_strsplit(env, ",", 0);
		for (i = 0; tokens && tokens[i] != NULL; i++) {
			g_xim_message_enable_filter(master_message, tokens[i]);
		}
		g_xim_message_activate(master_message, TRUE);
	}
}

void
g_xim_finalize(void)
{
	g_object_unref(master_message);
}

GDataStreamByteOrder
g_xim_get_byte_order(void)
{
	gint i = 1;
	gchar *p = (gchar *)&i;

	if (*p == 1)
		return G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN;

	return G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN;
}

GXimValueType
g_xim_gtype_to_value_type(GType gtype)
{
	switch (gtype) {
	    case G_TYPE_CHAR:
	    case G_TYPE_UCHAR:
		    return G_XIM_TYPE_BYTE;
	    case G_TYPE_INT:
	    case G_TYPE_UINT:
		    return G_XIM_TYPE_WORD;
	    case G_TYPE_LONG:
	    case G_TYPE_ULONG:
		    return G_XIM_TYPE_LONG;
	    case G_TYPE_STRING:
		    return G_XIM_TYPE_CHAR;
	    case G_TYPE_BOXED:
	    default:
		    if (g_type_is_a(gtype, G_TYPE_XIM_STYLES)) {
			    return G_XIM_TYPE_XIMSTYLES;
		    } else if (g_type_is_a(gtype, GDK_TYPE_WINDOW)) {
			    return G_XIM_TYPE_WINDOW;
		    } else if (g_type_is_a(gtype, G_TYPE_XIM_NESTEDLIST)) {
			    return G_XIM_TYPE_NESTEDLIST;
		    } else if (g_type_is_a(gtype, G_TYPE_XIM_RECTANGLE)) {
			    return G_XIM_TYPE_XRECTANGLE;
		    } else if (g_type_is_a(gtype, G_TYPE_XIM_POINT)) {
			    return G_XIM_TYPE_XPOINT;
		    } else if (g_type_is_a(gtype, G_TYPE_XIM_FONTSET)) {
			    return G_XIM_TYPE_XFONTSET;
		    } else if (g_type_is_a(gtype, G_TYPE_XIM_SEP_NESTEDLIST)) {
			    return G_XIM_TYPE_SEPARATOR;
		    }

		    g_warning("Unsupported object type: %s",
			      g_type_name(gtype));
		    break;
	}

	return G_XIM_TYPE_INVALID;
}

GType
g_xim_value_type_to_gtype(GXimValueType vtype)
{
	switch (vtype) {
	    case G_XIM_TYPE_SEPARATOR:
		    return G_TYPE_XIM_SEP_NESTEDLIST;
	    case G_XIM_TYPE_BYTE:
		    return G_TYPE_UCHAR;
	    case G_XIM_TYPE_WORD:
		    return G_TYPE_UINT;
	    case G_XIM_TYPE_LONG:
		    return G_TYPE_ULONG;
	    case G_XIM_TYPE_CHAR:
		    return G_TYPE_STRING;
	    case G_XIM_TYPE_WINDOW:
		    return GDK_TYPE_WINDOW;
	    case G_XIM_TYPE_XIMSTYLES:
		    return G_TYPE_XIM_STYLES;
	    case G_XIM_TYPE_XRECTANGLE:
		    return G_TYPE_XIM_RECTANGLE;
	    case G_XIM_TYPE_XPOINT:
		    return G_TYPE_XIM_POINT;
	    case G_XIM_TYPE_XFONTSET:
		    return G_TYPE_XIM_FONTSET;
	    case G_XIM_TYPE_HOTKEYTRIGGERS:
	    case G_XIM_TYPE_HOTKEYSTATE:
	    case G_XIM_TYPE_STRINGCONVERSION:
	    case G_XIM_TYPE_PREEDITSTATE:
	    case G_XIM_TYPE_RESETSTATE:
		    break;
	    case G_XIM_TYPE_NESTEDLIST:
		    return G_TYPE_XIM_NESTEDLIST;
	    case G_XIM_TYPE_INVALID:
		    break;
	    case G_XIM_TYPE_PADDING:
	    case G_XIM_TYPE_AUTO_PADDING:
	    case G_XIM_TYPE_MARKER_N_BYTES_1:
	    case G_XIM_TYPE_MARKER_N_BYTES_2:
	    case G_XIM_TYPE_MARKER_N_BYTES_4:
	    case G_XIM_TYPE_MARKER_N_ITEMS_2:
		    break;
	    case G_XIM_TYPE_STR:
		    return G_TYPE_XIM_STR;
	    case G_XIM_TYPE_GSTRING:
		    return G_TYPE_GSTRING;
	    case G_XIM_TYPE_PREEDIT_CARET:
		    return G_TYPE_XIM_PREEDIT_CARET;
	    case G_XIM_TYPE_PREEDIT_DRAW:
		    return G_TYPE_XIM_PREEDIT_DRAW;
	    case G_XIM_TYPE_GDKEVENT:
		    return GDK_TYPE_EVENT;
	    case G_XIM_TYPE_XIMTEXT:
		    return G_TYPE_XIM_TEXT;
	    case G_XIM_TYPE_HOTKEY_TRIGGER:
		    return G_TYPE_XIM_HOTKEY_TRIGGER;
	    case G_XIM_TYPE_PIXMAP:
		    return GDK_TYPE_PIXMAP;
	    case G_XIM_TYPE_STATUS_DRAW:
		    return G_TYPE_XIM_STATUS_DRAW;
	    case G_XIM_TYPE_LIST_OF_IMATTR:
	    case G_XIM_TYPE_LIST_OF_ICATTR:
	    case G_XIM_TYPE_LIST_OF_IMATTRIBUTE:
	    case G_XIM_TYPE_LIST_OF_ICATTRIBUTE:
	    case G_XIM_TYPE_LIST_OF_EXT:
	    case G_XIM_TYPE_LIST_OF_STRING:
	    case G_XIM_TYPE_LIST_OF_STR:
	    case G_XIM_TYPE_LIST_OF_ENCODINGINFO:
	    case G_XIM_TYPE_LIST_OF_BYTE:
	    case G_XIM_TYPE_LIST_OF_CARD16:
	    case G_XIM_TYPE_LIST_OF_HOTKEY_TRIGGER:
	    default:
		    break;
	}

	return G_TYPE_INVALID;
}

const gchar *
g_xim_value_type_name(GXimValueType vtype)
{
	static const gchar vtype_name[] =
		"\0"
		"SeparatorofNestedList\0"
		"CARD8\0"
		"CARD16\0"
		"CARD32\0"
		"STRING8\0"
		"Window\0"
		"XIMStyles\0"
		"XRectangle\0"
		"XPoint\0"
		"XFontSet\0"
		"XIMHotKeyTriggers\0"
		"XIMHotKeyState\0"
		"XIMStringConversion\0"
		"XIMPreeditState\0"
		"XIMResetState\0"
		"NestedList\0"
		"<Invalid>\0"
		"Padding\0"
		"AutoPadding\0"
		"n-bytes-marker[CARD8]\0"
		"n-bytes-marker[CARD16]\0"
		"n-bytes-marker[CARD32]\0"
		"n-items-marker[CARD16]\0"
		"STR\0"
		"GString\0"
		"GXimPreeditCaret\0"
		"GXimPreeditDraw\0"
		"GdkEvent\0"
		"GXimText\0"
		"GXimHotkeyTrigger\0"
		"Pixmap\0"
		"GXimStatusDraw\0"
		"LISTofXIMATTR\0"
		"LISTofXICATTR\0"
		"LISTofXIMATTRIBUTE\0"
		"LISTofXICATTRIBUTE\0"
		"LISTofEXT\0"
		"LISTofSTRING\0"
		"LISTofSTR\0"
		"LISTofENCODINGINFO\0"
		"LISTofBYTE\0"
		"LISTofCARD16\0"
		"LISTofXIMTRIGGERKEY\0";
	static guint type2pos[LAST_VALUE_TYPE];
	static gboolean initialized = FALSE;
	static gchar tmp[256];

	if (!initialized) {
		memset(type2pos, 0, sizeof (guint) * (LAST_VALUE_TYPE - 1));
		type2pos[G_XIM_TYPE_SEPARATOR] = 1;
		type2pos[G_XIM_TYPE_BYTE] = 23;
		type2pos[G_XIM_TYPE_WORD] = 29;
		type2pos[G_XIM_TYPE_LONG] = 36;
		type2pos[G_XIM_TYPE_CHAR] = 43;
		type2pos[G_XIM_TYPE_WINDOW] = 51;
		type2pos[G_XIM_TYPE_XIMSTYLES] = 58;
		type2pos[G_XIM_TYPE_XRECTANGLE] = 68;
		type2pos[G_XIM_TYPE_XPOINT] = 79;
		type2pos[G_XIM_TYPE_XFONTSET] = 86;
		type2pos[G_XIM_TYPE_HOTKEYTRIGGERS] = 95;
		type2pos[G_XIM_TYPE_HOTKEYSTATE] = 113;
		type2pos[G_XIM_TYPE_STRINGCONVERSION] = 128;
		type2pos[G_XIM_TYPE_PREEDITSTATE] = 148;
		type2pos[G_XIM_TYPE_RESETSTATE] = 164;
		type2pos[G_XIM_TYPE_NESTEDLIST] = 178;
		type2pos[G_XIM_TYPE_INVALID] = 189;
		type2pos[G_XIM_TYPE_PADDING] = 199;
		type2pos[G_XIM_TYPE_AUTO_PADDING] = 207;
		type2pos[G_XIM_TYPE_MARKER_N_BYTES_1] = 219;
		type2pos[G_XIM_TYPE_MARKER_N_BYTES_2] = 241;
		type2pos[G_XIM_TYPE_MARKER_N_BYTES_4] = 264;
		type2pos[G_XIM_TYPE_MARKER_N_ITEMS_2] = 287;
		type2pos[G_XIM_TYPE_STR] = 310;
		type2pos[G_XIM_TYPE_GSTRING] = 314;
		type2pos[G_XIM_TYPE_PREEDIT_CARET] = 322;
		type2pos[G_XIM_TYPE_PREEDIT_DRAW] = 339;
		type2pos[G_XIM_TYPE_GDKEVENT] = 355;
		type2pos[G_XIM_TYPE_XIMTEXT] = 364;
		type2pos[G_XIM_TYPE_HOTKEY_TRIGGER] = 373;
		type2pos[G_XIM_TYPE_PIXMAP] = 391;
		type2pos[G_XIM_TYPE_STATUS_DRAW] = 398;
		type2pos[G_XIM_TYPE_LIST_OF_IMATTR] = 413;
		type2pos[G_XIM_TYPE_LIST_OF_ICATTR] = 427;
		type2pos[G_XIM_TYPE_LIST_OF_IMATTRIBUTE] = 441;
		type2pos[G_XIM_TYPE_LIST_OF_ICATTRIBUTE] = 460;
		type2pos[G_XIM_TYPE_LIST_OF_EXT] = 479;
		type2pos[G_XIM_TYPE_LIST_OF_STRING] = 489;
		type2pos[G_XIM_TYPE_LIST_OF_STR] = 502;
		type2pos[G_XIM_TYPE_LIST_OF_ENCODINGINFO] = 512;
		type2pos[G_XIM_TYPE_LIST_OF_BYTE] = 531;
		type2pos[G_XIM_TYPE_LIST_OF_CARD16] = 542;
		type2pos[G_XIM_TYPE_LIST_OF_HOTKEY_TRIGGER] = 555;
	}

	if (vtype > LAST_VALUE_TYPE ||
	    type2pos[vtype] == 0) {
		snprintf(tmp, 255, "0x%x", vtype);

		return tmp;
	}

	return &vtype_name[type2pos[vtype]];
}

const gchar *
g_xim_protocol_name(guint16 major_opcode)
{
	static const gchar op2str[] = {
		"\0"
		"XIM_CONNECT\0"
		"XIM_CONNECT_REPLY\0"
		"XIM_DISCONNECT\0"
		"XIM_DISCONNECT_REPLY\0"
		"\0"
		"\0"
		"\0"
		"\0"
		"\0"
		"XIM_AUTH_REQUIRED\0"
		"XIM_AUTH_REPLY\0"
		"XIM_AUTH_NEXT\0"
		"XIM_AUTH_SETUP\0"
		"XIM_AUTH_NG\0"
		"\0"
		"\0"
		"\0"
		"\0"
		"\0"
		"XIM_ERROR\0"
		"\0"
		"\0"
		"\0"
		"\0"
		"\0"
		"\0"
		"\0"
		"\0"
		"\0"
		"XIM_OPEN\0"
		"XIM_OPEN_REPLY\0"
		"XIM_CLOSE\0"
		"XIM_CLOSE_REPLY\0"
		"XIM_REGISTER_TRIGGERKEYS\0"
		"XIM_TRIGGER_NOTIFY\0"
		"XIM_TRIGGER_NOTIFY_REPLY\0"
		"XIM_SET_EVENT_MASK\0"
		"XIM_ENCODING_NEGOTIATION\0"
		"XIM_ENCODING_NEGOTIATION_REPLY\0"
		"XIM_QUERY_EXTENSION\0"
		"XIM_QUERY_EXTENSION_REPLY\0"
		"XIM_SET_IM_VALUES\0"
		"XIM_SET_IM_VALUES_REPLY\0"
		"XIM_GET_IM_VALUES\0"
		"XIM_GET_IM_VALUES_REPLY\0"
		"\0"
		"\0"
		"\0"
		"\0"
		"XIM_CREATE_IC\0"
		"XIM_CREATE_IC_REPLY\0"
		"XIM_DESTROY_IC\0"
		"XIM_DESTROY_IC_REPLY\0"
		"XIM_SET_IC_VALUES\0"
		"XIM_SET_IC_VALUES_REPLY\0"
		"XIM_GET_IC_VALUES\0"
		"XIM_GET_IC_VALUES_REPLY\0"
		"XIM_SET_IC_FOCUS\0"
		"XIM_UNSET_IC_FOCUS\0"
		"XIM_FORWARD_EVENT\0"
		"XIM_SYNC\0"
		"XIM_SYNC_REPLY\0"
		"XIM_COMMIT\0"
		"XIM_RESET_IC\0"
		"XIM_RESET_IC_REPLY\0"
		"\0"
		"\0"
		"\0"
		"\0"
		"XIM_GEOMETRY\0"
		"XIM_STR_CONVERSION\0"
		"XIM_STR_CONVERSION_REPLY\0"
		"XIM_PREEDIT_START\0"
		"XIM_PREEDIT_START_REPLY\0"
		"XIM_PREEDIT_DRAW\0"
		"XIM_PREEDIT_CARET\0"
		"XIM_PREEDIT_CARET_REPLY\0"
		"XIM_PREEDIT_DONE\0"
		"XIM_STATUS_START\0"
		"XIM_STATUS_DRAW\0"
		"XIM_STATUS_DONE\0"
		"XIM_PREEDITSTATE\0"
	};
	static gsize opindexes[] = {
		0, 1, 13, 31, 46, 67, 68, 69, 70, 71,
		72, 90, 105, 119, 134, 146, 147, 148, 149, 150,
		151, 161, 162, 163, 164, 165, 166, 167, 168, 169,
		170, 179, 194, 204, 220, 245, 264, 289, 308, 333,
		364, 384, 410, 428, 452, 470, 494, 495, 496, 497,
		498, 512, 532, 547, 568, 586, 610, 628, 652, 669,
		688, 706, 715, 730, 741, 754, 773, 774, 775, 776,
		777, 790, 809, 834, 852, 876, 893, 911, 935, 952,
		969, 985, 1001, 1018,
	};

	if (major_opcode >= LAST_XIM_EVENTS)
		return NULL;

	return &op2str[opindexes[major_opcode]];
}

GdkWindow *
g_xim_get_window(GdkDisplay      *dpy,
		 GdkNativeWindow  window)
{
	GdkWindow *retval;
	guint32 error_code;

	g_xim_error_push();

	retval = gdk_window_lookup_for_display(dpy, window);
	if (retval == NULL ||
	    !GDK_IS_WINDOW (retval) ||
	    GDK_WINDOW_DESTROYED (retval)) {
		if (retval)
			gdk_window_destroy(retval);
		retval = gdk_window_foreign_new_for_display(dpy, window);
	}

	error_code = g_xim_error_pop();
	if (G_XIM_ERROR_DECODE_X_ERROR_CODE (error_code) != 0) {
		g_printerr("Unable to convert the native window to GdkWindow: %p",
			   G_XIM_NATIVE_WINDOW_TO_POINTER (window));
	}

	return retval != NULL ? g_object_ref(retval) : NULL;
}

GdkPixmap *
g_xim_get_pixmap(GdkDisplay      *dpy,
		 GdkNativeWindow  window)
{
	GdkPixmap *retval;

	retval = gdk_pixmap_lookup_for_display(dpy, window);
	if (retval == NULL ||
	    !GDK_IS_PIXMAP (retval)) {
		if (retval)
			g_object_unref(retval);
		retval = gdk_pixmap_foreign_new_for_display(dpy, window);
	}

	return retval;
}

GdkWindow *
g_xim_get_selection_owner(GdkDisplay *display,
			  GdkAtom     selection)
{
	Window xwindow;
	GdkWindow *retval;

	g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);
	g_return_val_if_fail (selection != GDK_NONE, NULL);

	if (display->closed)
		return NULL;

	xwindow = XGetSelectionOwner(GDK_DISPLAY_XDISPLAY (display),
				     gdk_x11_atom_to_xatom_for_display(display,
								       selection));
	if (xwindow == None)
		return NULL;

	retval = g_xim_get_window(display, (GdkNativeWindow)xwindow);
	/* just decrease a counter to not mind unref outside this function */
	g_object_unref(retval);

	return retval;
}

GdkWindow *
g_xim_lookup_xim_server(GdkDisplay *dpy,
			GdkAtom     atom_server,
			gboolean   *is_valid)
{
	GdkAtom atom_xim_servers, atom_type, *atom_prop = NULL;
	gint format, bytes, i;
	GdkWindow *retval = NULL;
	guint32 error_code;

	g_return_val_if_fail (atom_server != GDK_NONE, NULL);
	g_return_val_if_fail (is_valid != NULL, NULL);

	atom_xim_servers = gdk_atom_intern_static_string("XIM_SERVERS");

	g_xim_error_push();
	gdk_property_get(gdk_screen_get_root_window(gdk_display_get_default_screen(dpy)),
			 atom_xim_servers, GDK_SELECTION_TYPE_ATOM,
			 0, 8192, FALSE,
			 &atom_type, &format, &bytes,
			 (guchar **)(uintptr_t)&atom_prop);
	error_code = g_xim_error_pop();
	if (error_code != 0) {
		*is_valid = FALSE;
		return NULL;
	}
	if (atom_type != GDK_SELECTION_TYPE_ATOM ||
	    format != 32) {
		*is_valid = FALSE;
		return NULL;
	}

	for (i = 0; i < (bytes / sizeof (gulong)); i++) {
		if (atom_prop[i] == atom_server) {
			retval = g_xim_get_selection_owner(dpy, atom_server);
			break;
		}
	}
	g_free(atom_prop);
	*is_valid = TRUE;

	return retval;
}

GdkWindow *
g_xim_lookup_xim_server_from_string(GdkDisplay  *dpy,
				    const gchar *server_name,
				    gboolean    *is_valid)
{
	GdkWindow *retval;
	GdkAtom atom_server;

	g_return_val_if_fail (server_name != NULL, NULL);

	atom_server = g_xim_get_server_atom(server_name);
	retval = g_xim_lookup_xim_server(dpy, atom_server, is_valid);

	return retval;
}

GdkAtom
g_xim_get_server_atom(const gchar *server_name)
{
	gchar *s;
	GdkAtom retval;

	s = g_strdup_printf("@server=%s", server_name);
	retval = gdk_atom_intern(s, FALSE);
	g_free(s);

	return retval;
}

gpointer
g_xim_copy_by_gtype(GType    gtype,
		    gpointer value)
{
	gpointer retval;

	if (G_TYPE_IS_BOXED (gtype))
		retval = g_boxed_copy(gtype, value);
	else if (G_TYPE_IS_OBJECT (gtype))
		retval = g_object_ref(value);
	else if (gtype == G_TYPE_STRING)
		retval = g_strdup(value);
	else
		retval = value;

	return retval;
}

void
g_xim_free_by_gtype(GType    gtype,
		    gpointer value)
{
	if (G_TYPE_IS_BOXED (gtype))
		g_boxed_free(gtype, value);
	else if (G_TYPE_IS_OBJECT (gtype))
		g_object_unref(value);
	else if (gtype == G_TYPE_STRING)
		g_free(value);
}

#if 0
/* GXimStrConv */
GXimStrConvText *
g_xim_str_conv_text_new(const gchar *string,
			guint16      length)
{
	GXimStrConvText *retval;

	g_return_val_if_fail (string != NULL, NULL);

	retval = g_new0(GXimStrConvText, 1);
	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->string = g_strndup(string, length);
	retval->length = length;

	return retval;
}

GXimStrConvText *
g_xim_str_conv_text_new_full(const gchar *string,
			     guint16      length,
			     guint16      feedback)
{
	GXimStrConvText *retval = g_xim_str_conv_text_new(string, length);

	if (retval)
		g_xim_str_conv_set_feedback(retval, feedback);

	return retval;
}

void
g_xim_str_conv_text_free(GXimStrConvText *text)
{
	g_return_if_fail (text != NULL);

	g_free(text->string);
	g_free(text);
}

void
g_xim_str_conv_set_feedback(GXimStrConvText *text,
			    guint16          feedback)
{
	g_return_if_fail (text != NULL);

	text->feedback = feedback;
}

guint16
g_xim_str_conv_get_feedback(GXimStrConvText *text)
{
	g_return_val_if_fail (text != NULL, 0);

	return text->feedback;
}
#endif

/* XIMStyles */
GType
g_xim_styles_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimStyles"),
						       g_xim_styles_copy,
						       g_xim_styles_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

GQuark
g_xim_styles_get_error_quark(void)
{
	static GQuark quark = 0;

	if (!quark)
		quark = g_quark_from_static_string("g-xim-styles-error");

	return quark;
}

GXimStyles *
g_xim_styles_new(void)
{
	GXimStyles *retval = g_new0(GXimStyles, 1);

	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->count_styles = 0;
	retval->supported_styles = NULL;

	return retval;
}

gboolean
g_xim_styles_append(GXimStyles  *styles,
		    GXimStyle    style,
		    GError     **error)
{
	g_return_val_if_fail (styles != NULL, FALSE);

	return g_xim_styles_insert(styles, styles->count_styles, style, error);
}

gboolean
g_xim_styles_insert(GXimStyles  *styles,
		    guint        index_,
		    GXimStyle    style,
		    GError     **error)
{
	gboolean retval = TRUE;

	g_return_val_if_fail (styles != NULL, FALSE);
	g_return_val_if_fail (error != NULL, FALSE);

	if (styles->count_styles <= index_) {
		gpointer data;

		data = g_realloc(styles->supported_styles,
				 sizeof (GXimStyle) * (index_ + 1));
		G_XIM_GERROR_CHECK_ALLOC (data,
					  error, G_XIM_STYLES_ERROR,
					  FALSE);

		styles->supported_styles = data;
		styles->count_styles = index_ + 1;
	}
	styles->supported_styles[index_] = style;

	return retval;
}

gpointer
g_xim_styles_copy(gpointer boxed)
{
	GXimStyles *retval, *orig = boxed;

	if (boxed == NULL)
		return NULL;

	retval = g_new(GXimStyles, 1);
	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->count_styles = orig->count_styles;
	retval->supported_styles = g_new(GXimStyle, sizeof (GXimStyle) * retval->count_styles);
	G_XIM_CHECK_ALLOC (retval->supported_styles, NULL);

	memcpy(retval->supported_styles,
	       orig->supported_styles,
	       sizeof (GXimStyle) * retval->count_styles);

	return retval;
}

void
g_xim_styles_free(gpointer boxed)
{
	GXimStyles *s = boxed;

	if (boxed == NULL)
		return;

	g_free(s->supported_styles);
	g_free(s);
}

gsize
g_xim_styles_put_to_stream(GXimStyles    *styles,
			   GXimProtocol  *proto,
			   GCancellable  *cancellable,
			   GError       **error)
{
	gsize retval = 0;
	guint16 i;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (styles != NULL, 0);
	g_return_val_if_fail (error != NULL, 0);

	retval = g_xim_protocol_send_format(proto, cancellable, error, 2,
					    G_XIM_TYPE_WORD, styles->count_styles,
					    G_XIM_TYPE_PADDING, 2);
	if (*error)
		return 0;
	/* XIMStyle */
	for (i = 0; i < styles->count_styles; i++) {
		retval += g_xim_protocol_send_format(proto, cancellable, error, 1,
						     G_XIM_TYPE_LONG, styles->supported_styles[i]);
		if (*error)
			return 0;
	}

	return retval;
}

gpointer
g_xim_styles_get_from_stream(GXimProtocol      *proto,
			     GDataInputStream  *stream,
			     GCancellable      *cancellable,
			     GError           **error)
{
	GXimStyles *retval;
	guint16 i;
	GXimStyle style;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	retval = g_xim_styles_new();
	G_XIM_CHECK_ALLOC (retval, NULL);

	if (!g_xim_protocol_read_format(proto, stream, cancellable, error,
					2,
					G_XIM_TYPE_WORD, &retval->count_styles,
					G_XIM_TYPE_PADDING, 2)) {
		g_xim_styles_free(retval);
		return NULL;
	}
	retval->supported_styles = g_new0(GXimStyle, retval->count_styles);
	G_XIM_GERROR_CHECK_ALLOC (retval->supported_styles, error,
				  G_XIM_PROTOCOL_ERROR, NULL);

	for (i = 0; i < retval->count_styles; i++) {
		if (!g_xim_protocol_read_format(proto, stream, cancellable, error,
						1,
						G_XIM_TYPE_LONG, &style)) {
			/* keep the values as much as possible */
			retval->count_styles = i;
			G_XIM_GERROR_RESET_NOTICE_FLAG (*error);
			G_XIM_GERROR_SET_NOTICE_FLAG (*error,
						      G_XIM_NOTICE_WARNING);
			break;
		}
		if (!g_xim_styles_insert(retval, i, style, error)) {
			retval->count_styles = i;
			G_XIM_GERROR_RESET_NOTICE_FLAG (*error);
			G_XIM_GERROR_SET_NOTICE_FLAG (*error,
						      G_XIM_NOTICE_WARNING);
			break;
		}
	}

	return retval;
}

/* XRectangle */
GType
g_xim_rectangle_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimRectangle"),
						       g_xim_rectangle_copy,
						       g_xim_rectangle_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_rectangle_new(void)
{
	return g_new0(GXimRectangle, 1);
}

gpointer
g_xim_rectangle_copy(gpointer boxed)
{
	GXimRectangle *retval, *val = boxed;

	if (boxed == NULL)
		return NULL;

	retval = g_xim_rectangle_new();
	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->x = val->x;
	retval->y = val->y;
	retval->width = val->width;
	retval->height = val->height;

	return retval;
}

void
g_xim_rectangle_free(gpointer boxed)
{
	g_free(boxed);
}

gsize
g_xim_rectangle_put_to_stream(GXimRectangle  *rectangle,
			      GXimProtocol   *proto,
			      GCancellable   *cancellable,
			      GError        **error)
{
	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (rectangle != NULL, 0);
	g_return_val_if_fail (error != NULL, 0);

	return g_xim_protocol_send_format(proto, cancellable, error, 4,
					  G_XIM_TYPE_WORD, rectangle->x,
					  G_XIM_TYPE_WORD, rectangle->y,
					  G_XIM_TYPE_WORD, rectangle->width,
					  G_XIM_TYPE_WORD, rectangle->height);
}

gpointer
g_xim_rectangle_get_from_stream(GXimProtocol      *proto,
				GDataInputStream  *stream,
				GCancellable      *cancellable,
				GError           **error)
{
	GXimRectangle *retval;
	guint16 x, y, width, height;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	if (!g_xim_protocol_read_format(proto, stream, cancellable, error,
					4,
					G_XIM_TYPE_WORD, &x,
					G_XIM_TYPE_WORD, &y,
					G_XIM_TYPE_WORD, &width,
					G_XIM_TYPE_WORD, &height))
		return NULL;

	retval = g_xim_rectangle_new();
	G_XIM_GERROR_CHECK_ALLOC (retval, error,
				  G_XIM_PROTOCOL_ERROR, NULL);

	retval->x = x;
	retval->y = y;
	retval->width = width;
	retval->height = height;

	return retval;
}

/* XPoint */
GType
g_xim_point_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimPoint"),
						       g_xim_point_copy,
						       g_xim_point_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_point_new(void)
{
	return g_new0(GXimPoint, 1);
}

gpointer
g_xim_point_copy(gpointer boxed)
{
	GXimPoint *retval, *val = boxed;

	if (boxed == NULL)
		return NULL;

	retval = g_xim_point_new();
	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->x = val->x;
	retval->y = val->y;

	return retval;
}

void
g_xim_point_free(gpointer boxed)
{
	g_free(boxed);
}

gsize
g_xim_point_put_to_stream(GXimPoint     *point,
			  GXimProtocol  *proto,
			  GCancellable  *cancellable,
			  GError       **error)
{
	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (point != NULL, 0);
	g_return_val_if_fail (error != NULL, 0);

	return g_xim_protocol_send_format(proto, cancellable, error, 2,
					  G_XIM_TYPE_WORD, point->x,
					  G_XIM_TYPE_WORD, point->y);
}

gpointer
g_xim_point_get_from_stream(GXimProtocol      *proto,
			    GDataInputStream  *stream,
			    GCancellable      *cancellable,
			    GError           **error)
{
	GXimPoint *retval;
	guint16 x, y;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	if (!g_xim_protocol_read_format(proto, stream, cancellable, error,
					2,
					G_XIM_TYPE_WORD, &x,
					G_XIM_TYPE_WORD, &y))
		return NULL;

	retval = g_xim_point_new();
	G_XIM_GERROR_CHECK_ALLOC (retval, error,
				  G_XIM_PROTOCOL_ERROR, NULL);

	retval->x = x;
	retval->y = y;

	return retval;
}

/* XFontSet */
GType
g_xim_fontset_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimFontSet"),
						       g_xim_fontset_copy,
						       g_xim_fontset_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_fontset_copy(gpointer boxed)
{
	GString *retval, *val = boxed;

	if (boxed == NULL)
		return NULL;

	retval = g_string_sized_new(val->len);
	G_XIM_CHECK_ALLOC (retval, NULL);

	g_string_append(retval, val->str);

	return retval;
}

void
g_xim_fontset_free(gpointer boxed)
{
	if (boxed == NULL)
		return;

	g_string_free(boxed, TRUE);
}

gsize
g_xim_fontset_put_to_stream(GXimFontSet   *fontset,
			    GXimProtocol  *proto,
			    GCancellable  *cancellable,
			    GError       **error)
{
	g_return_val_if_fail (fontset != NULL, 0);
	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (error != NULL, 0);

	return g_xim_protocol_send_format(proto, cancellable, error, 2,
					  G_XIM_TYPE_GSTRING, fontset,
					  G_XIM_TYPE_AUTO_PADDING, 2);
}

gpointer
g_xim_fontset_get_from_stream(GXimProtocol      *proto,
			      GDataInputStream  *stream,
			      GCancellable      *cancellable,
			      GError           **error)
{
	GXimFontSet *retval = NULL;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	if (!g_xim_protocol_read_format(proto, stream, cancellable, error,
					2,
					G_XIM_TYPE_GSTRING, &retval,
					G_XIM_TYPE_AUTO_PADDING, 2))
		return NULL;

	return retval;
}

/* GString */
gsize
g_xim_gstring_put_to_stream(GString            *string,
			    GDataOutputStream  *stream,
			    GCancellable       *cancellable,
			    GError            **error)
{
	gint i;
	gsize retval = 2;

	g_return_val_if_fail (G_IS_DATA_OUTPUT_STREAM (stream), 0);
	g_return_val_if_fail (string != NULL, 0);
	g_return_val_if_fail (string->len <= 0xffff, 0);
	g_return_val_if_fail (error != NULL, 0);

	g_data_output_stream_put_uint16(stream, string->len, cancellable, error);
	if (*error)
		return 0;
	for (i = 0; i < string->len; i++) {
		g_data_output_stream_put_byte(stream, string->str[i], cancellable, error);
		if (*error)
			return 0;
		retval++;
	}

	return retval;
}

gpointer
g_xim_gstring_get_from_stream(GDataInputStream  *stream,
			      GCancellable      *cancellable,
			      GError           **error)
{
	GString *retval = NULL;
	gsize size, avail, read = 2, i;

	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	size = g_data_input_stream_read_uint16(stream, cancellable, error);
	if (*error)
		return NULL;
	if ((avail = g_buffered_input_stream_get_buffer_size(G_BUFFERED_INPUT_STREAM (stream))) < size) {
		g_set_error(error, G_XIM_PROTOCOL_ERROR,
			    G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
			    "Unable to compose a string with the remaining packets: %" G_GSIZE_FORMAT " for %" G_GSIZE_FORMAT,
			    size, avail);
		return NULL;
	}
	retval = g_string_sized_new(size);
	G_XIM_GERROR_CHECK_ALLOC (retval, error,
				  G_XIM_PROTOCOL_ERROR, NULL);

	for (i = 0; i < size; i++) {
		gchar c;

		c = g_data_input_stream_read_byte(stream, cancellable, error);
		if (*error)
			goto fail;
		g_string_append_c(retval, c);
		read++;
	}

	return retval;
  fail:
	if (retval)
		g_string_free(retval, TRUE);

	return NULL;
}

/* STRING */
GType
g_xim_string_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimString"),
						       g_xim_string_copy,
						       g_xim_string_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_string_new(void)
{
	GString *retval = g_string_new(NULL);

	return retval;
}

gpointer
g_xim_string_copy(gpointer boxed)
{
	GString *retval, *orig = boxed;

	if (boxed == NULL)
		return NULL;

	retval = g_string_sized_new(orig->len);
	G_XIM_CHECK_ALLOC (retval, NULL);

	g_string_append(retval, orig->str);

	return retval;
}

void
g_xim_string_free(gpointer boxed)
{
	if (boxed == NULL)
		return;

	g_string_free(boxed, TRUE);
}

const gchar *
g_xim_string_get_string(const GXimString *string)
{
	g_return_val_if_fail (string != NULL, NULL);

	return ((GString *)string)->str;
}

gsize
g_xim_string_get_length(GXimString *string)
{
	GString *str = (GString *)string;

	g_return_val_if_fail (string != NULL, 0);

	return str->len;
}

gsize
g_xim_string_put_to_stream(GXimString         *string,
			   GDataOutputStream  *stream,
			   GCancellable       *cancellable,
			   GError            **error)
{
	gsize retval = 2, i;
	GString *str = (GString *)string;

	g_return_val_if_fail (G_IS_DATA_OUTPUT_STREAM (stream), 0);
	g_return_val_if_fail (string != NULL, 0);
	g_return_val_if_fail (error != NULL, 0);

	/* length of string in bytes */
	g_data_output_stream_put_uint16(stream,
					str->len,
					cancellable,
					error);
	if (*error)
		return 0;
	/* LIST of LPCE */
	for (i = 0; i < str->len; i++) {
		g_data_output_stream_put_byte(stream,
					      str->str[i],
					      cancellable,
					      error);
		if (*error)
			return 0;

		retval++;
	}
	/* padding */
	for (i = 0; i < g_xim_protocol_n_pad4 (2 + str->len); i++) {
		g_data_output_stream_put_byte(stream, 0, cancellable, error);
		if (*error)
			return 0;
		retval++;
	}

	return retval;
}

gpointer
g_xim_string_get_from_stream(GDataInputStream  *stream,
			     GCancellable      *cancellable,
			     GError           **error)
{
	guint16 n, i;
	GString *str = NULL;
	GInputStream *base_stream;

	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	n = g_data_input_stream_read_uint16(stream, cancellable, error);
	if (*error)
		return NULL;
	str = g_string_sized_new(n);
	G_XIM_GERROR_CHECK_ALLOC (str, error,
				  G_XIM_PROTOCOL_ERROR, NULL);

	for (i = 0; i < n; i++) {
		guint8 c;

		c = g_data_input_stream_read_byte(stream, cancellable, error);
		if (*error)
			goto fail;
		g_string_append_c(str, c);
	}
	/* skip padding */
	base_stream = g_filter_input_stream_get_base_stream(G_FILTER_INPUT_STREAM (stream));
	g_seekable_seek(G_SEEKABLE (base_stream),
			g_xim_protocol_n_pad4 (2 + n),
			G_SEEK_CUR, cancellable, error);

	return str;
  fail:
	if (str)
		g_string_free(str, TRUE);

	return NULL;
}

/* STR */
GType
g_xim_str_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimStr"),
						       g_xim_str_copy,
						       g_xim_str_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_str_new(void)
{
	GString *retval = g_string_new(NULL);

	return retval;
}

gpointer
g_xim_str_copy(gpointer boxed)
{
	GString *retval, *orig = boxed;

	g_return_val_if_fail (boxed != NULL, NULL);

	retval = g_string_sized_new(orig->len);
	G_XIM_CHECK_ALLOC (retval, NULL);

	g_string_append(retval, orig->str);

	return retval;
}

void
g_xim_str_free(gpointer boxed)
{
	if (boxed == NULL)
		return;

	g_string_free(boxed, TRUE);
}

const gchar *
g_xim_str_get_string(const GXimStr *string)
{
	g_return_val_if_fail (string != NULL, NULL);

	return ((GString *)string)->str;
}

gsize
g_xim_str_get_length(const GXimStr *string)
{
	const GString *str = (const GString *)string;

	g_return_val_if_fail (string != NULL, 0);

	return str->len;
}

GXimStr *
g_xim_str_append(GXimStr     *string,
		 const gchar *val)
{
	g_return_val_if_fail (string != NULL, NULL);
	g_return_val_if_fail (val != NULL, string);

	return g_xim_str_append_len(string, val, strlen(val));
}

GXimStr *
g_xim_str_append_len(GXimStr     *string,
		     const gchar *val,
		     gssize       len)
{
	g_return_val_if_fail (string != NULL, NULL);
	g_return_val_if_fail (val != NULL, string);

	if (len < 0)
		len = strlen(val);

	return (GXimStr *)g_string_append_len((GString *)string,
					      val,
					      len);
}

gsize
g_xim_str_put_to_stream(GXimStr            *string,
			GDataOutputStream  *stream,
			GCancellable       *cancellable,
			GError            **error)
{
	gsize retval = 1, i;
	GString *str = (GString *)string;

	g_return_val_if_fail (G_IS_DATA_OUTPUT_STREAM (stream), 0);
	g_return_val_if_fail (string != NULL, 0);
	g_return_val_if_fail (str->len < 0x100, 0);
	g_return_val_if_fail (error != NULL, 0);

	/* length of string in bytes */
	g_data_output_stream_put_byte(stream,
				      str->len,
				      cancellable,
				      error);
	if (*error)
		return 0;
	/* STRING8 */
	for (i = 0; i < str->len; i++) {
		g_data_output_stream_put_byte(stream,
					      str->str[i],
					      cancellable,
					      error);
		if (*error)
			return 0;

		retval++;
	}

	return retval;
}

gpointer
g_xim_str_get_from_stream(GDataInputStream  *stream,
			  GCancellable      *cancellable,
			  GError           **error)
{
	guint8 n, i;
	GString *str = NULL;

	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	n = g_data_input_stream_read_byte(stream, cancellable, error);
	if (*error)
		return NULL;
	str = g_string_sized_new(n);
	G_XIM_GERROR_CHECK_ALLOC (str, error,
				  G_XIM_PROTOCOL_ERROR, NULL);

	for (i = 0; i < n; i++) {
		guint8 c;

		c = g_data_input_stream_read_byte(stream, cancellable, error);
		if (*error)
			goto fail;
		g_string_append_c(str, c);
	}

	return str;
  fail:
	if (str)
		g_string_free(str, TRUE);

	return NULL;
}

/* ENCODINGINFO */
GType
g_xim_encodinginfo_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("ENCODINGINFO"),
						       g_xim_encodinginfo_copy,
						       g_xim_encodinginfo_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_encodinginfo_new(void)
{
	GString *retval = g_string_new(NULL);

	return retval;
}

gpointer
g_xim_encodinginfo_copy(gpointer boxed)
{
	GString *retval, *orig = boxed;

	g_return_val_if_fail (boxed != NULL, NULL);

	retval = g_string_sized_new(orig->len);
	G_XIM_CHECK_ALLOC (retval, NULL);

	g_string_append(retval, orig->str);

	return retval;
}

void
g_xim_encodinginfo_free(gpointer boxed)
{
	if (boxed == NULL)
		return;

	g_string_free(boxed, TRUE);
}

const gchar *
g_xim_encodinginfo_get_string(const GXimEncodingInfo *encoding)
{
	g_return_val_if_fail (encoding != NULL, NULL);

	return ((GString *)encoding)->str;
}

gsize
g_xim_encodinginfo_get_length(GXimEncodingInfo *encoding)
{
	GString *str = (GString *)encoding;

	g_return_val_if_fail (encoding != NULL, 0);

	return str->len;
}

gsize
g_xim_encodinginfo_put_to_stream(GXimEncodingInfo   *encoding,
				 GDataOutputStream  *stream,
				 GCancellable       *cancellable,
				 GError            **error)
{
	gsize retval = 2, i;
	GString *str = (GString *)encoding;

	g_return_val_if_fail (G_IS_DATA_OUTPUT_STREAM (stream), 0);
	g_return_val_if_fail (encoding != NULL, 0);
	g_return_val_if_fail (error != NULL, 0);

	/* length of string in bytes */
	g_data_output_stream_put_uint16(stream,
					str->len,
					cancellable,
					error);
	if (*error)
		return 0;
	/* STRING8 */
	for (i = 0; i < str->len; i++) {
		g_data_output_stream_put_byte(stream,
					      str->str[i],
					      cancellable,
					      error);
		if (*error)
			return 0;

		retval++;
	}
	/* padding */
	for (i = 0; i < g_xim_protocol_n_pad4 (2 + str->len); i++) {
		g_data_output_stream_put_byte(stream, 0, cancellable, error);
		if (*error)
			return 0;
		retval++;
	}

	return retval;
}

gpointer
g_xim_encodinginfo_get_from_stream(GDataInputStream  *stream,
				   GCancellable      *cancellable,
				   GError           **error)
{
	guint16 n, i;
	GString *str = NULL;
	GInputStream *base_stream;

	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	n = g_data_input_stream_read_uint16(stream, cancellable, error);
	if (*error)
		return NULL;
	str = g_string_sized_new(n);
	G_XIM_GERROR_CHECK_ALLOC (str, error,
				  G_XIM_PROTOCOL_ERROR, NULL);

	for (i = 0; i < n; i++) {
		guint8 c;

		c = g_data_input_stream_read_byte(stream, cancellable, error);
		if (*error)
			goto fail;
		g_string_append_c(str, c);
	}
	/* skip padding */
	base_stream = g_filter_input_stream_get_base_stream(G_FILTER_INPUT_STREAM (stream));
	g_seekable_seek(G_SEEKABLE (base_stream),
			g_xim_protocol_n_pad4 (2 + n),
			G_SEEK_CUR, cancellable, error);

	return str;
  fail:
	if (str)
		g_string_free(str, TRUE);

	return NULL;
}

/* ATTR */
GType
g_xim_raw_attr_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimRawAttr"),
						       g_xim_raw_attr_copy,
						       g_xim_raw_attr_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_raw_attr_new(void)
{
	GXimRawAttr *retval;

	retval = g_new0(GXimRawAttr, 1);
	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->base.vtype = G_XIM_TYPE_INVALID;

	return retval;
}

gpointer
g_xim_raw_attr_new_with_value(guint16        id,
			      GString       *name,
			      GXimValueType  vtype)
{
	GXimRawAttr *retval;

	g_return_val_if_fail (vtype != G_XIM_TYPE_INVALID, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	retval = g_xim_raw_attr_new();
	G_XIM_CHECK_ALLOC (retval, NULL);

	g_xim_raw_attr_set_name(retval, name);
	retval->base.id = id;
	retval->base.vtype = vtype;

	return retval;
}

void
g_xim_raw_attr_set_name(GXimRawAttr *attr,
			GString     *name)
{
	g_return_if_fail (attr != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (name->len > 0);

	if (attr->attribute_name)
		g_string_free(attr->attribute_name, TRUE);

	attr->attribute_name = g_string_sized_new(name->len);
	G_XIM_CHECK_ALLOC_WITH_NO_RET (attr->attribute_name);

	g_string_append(attr->attribute_name, name->str);
}

void
g_xim_raw_attr_clear(GXimRawAttr *attr)
{
	g_return_if_fail (attr != NULL);

	attr->base.vtype = G_XIM_TYPE_INVALID;
	g_string_free(attr->attribute_name, TRUE);
	attr->attribute_name = NULL;
}

gpointer
g_xim_raw_attr_copy(gpointer boxed)
{
	GXimRawAttr *retval, *orig;

	if (boxed == NULL)
		return NULL;

	orig = boxed;
	retval = g_xim_raw_attr_new_with_value(orig->base.id,
					       orig->attribute_name,
					       orig->base.vtype);

	return retval;
}

void
g_xim_raw_attr_free(gpointer boxed)
{
	if (boxed == NULL)
		return;

	g_xim_raw_attr_clear(boxed);
	g_free(boxed);
}

gsize
g_xim_raw_attr_put_to_stream(GXimRawAttr   *attr,
			     GXimProtocol  *proto,
			     GCancellable  *cancellable,
			     GError       **error)
{
	g_return_val_if_fail (attr != NULL, 0);
	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (error != NULL, 0);

	return g_xim_protocol_send_format(proto, cancellable, error, 4,
					  G_XIM_TYPE_WORD, attr->base.id,
					  G_XIM_TYPE_WORD, attr->base.vtype,
					  /* G_XIM_TYPE_GSTRING puts the size as CARD16 first then STRING8 */
					  G_XIM_TYPE_GSTRING, attr->attribute_name,
					  G_XIM_TYPE_AUTO_PADDING, 2);
}

gpointer
g_xim_raw_attr_get_from_stream(GXimProtocol      *proto,
			       GDataInputStream  *stream,
			       GCancellable      *cancellable,
			       GError           **error)
{
	GXimRawAttr *retval;
	guint16 id;
	GXimValueType vtype = 0;
	GString *name = NULL;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	if (!g_xim_protocol_read_format(proto, stream, cancellable, error,
					4,
					G_XIM_TYPE_WORD, &id,
					G_XIM_TYPE_WORD, &vtype,
					G_XIM_TYPE_GSTRING, &name,
					G_XIM_TYPE_AUTO_PADDING, 2))
		return NULL;
	retval = g_xim_raw_attr_new_with_value(id, name, vtype);

	return retval;
}

/* ATTRIBUTE */
GType
g_xim_attribute_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimAttribute"),
						       g_xim_attribute_copy,
						       g_xim_attribute_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_attribute_new(void)
{
	GXimAttribute *retval;

	retval = g_new0(GXimAttribute, 1);
	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->vtype = G_XIM_TYPE_INVALID;

	return retval;
}

gpointer
g_xim_attribute_new_with_value(guint16       id,
			       GXimValueType vtype,
			       gpointer      value)
{
	GXimAttribute *retval;

	g_return_val_if_fail (vtype != G_XIM_TYPE_INVALID, NULL);

	retval = g_xim_attribute_new();
	G_XIM_CHECK_ALLOC (retval, NULL);

	g_xim_attribute_set(retval, id, vtype, value);

	return retval;
}

void
g_xim_attribute_set(GXimAttribute *attr,
		    guint16        id,
		    GXimValueType  vtype,
		    gpointer       value)
{
	GType gtype;

	g_return_if_fail (attr != NULL);
	g_return_if_fail (vtype != G_XIM_TYPE_INVALID);

	gtype = g_xim_value_type_to_gtype(vtype);
	g_return_if_fail (gtype != G_TYPE_INVALID);

	g_xim_attribute_clear(attr);
	attr->id = id;
	attr->vtype = vtype;
	if (vtype == G_XIM_TYPE_WORD)
		attr->v.i = GPOINTER_TO_UINT (value);
	else if (vtype == G_XIM_TYPE_LONG)
		attr->v.l = (gulong)value;
	else
		attr->v.pointer = g_xim_copy_by_gtype(gtype, value);
}

void
g_xim_attribute_clear(GXimAttribute *attr)
{
	GType gtype;

	g_return_if_fail (attr != NULL);

	if (attr->vtype == G_XIM_TYPE_INVALID)
		return;

	gtype = g_xim_value_type_to_gtype(attr->vtype);
	g_return_if_fail (gtype != G_TYPE_INVALID);

	g_xim_free_by_gtype(gtype, attr->v.pointer);
	attr->v.pointer = NULL;
	attr->vtype = G_XIM_TYPE_INVALID;
}

gpointer
g_xim_attribute_copy(gpointer boxed)
{
	GXimAttribute *retval, *orig;

	if (boxed == NULL)
		return NULL;

	orig = boxed;
	retval = g_xim_attribute_new();
	G_XIM_CHECK_ALLOC (retval, NULL);

	g_xim_attribute_set(retval, orig->id, orig->vtype, orig->v.pointer);

	return retval;
}

void
g_xim_attribute_free(gpointer boxed)
{
	if (boxed == NULL)
		return;

	g_xim_attribute_clear(boxed);
	g_free(boxed);
}

gsize
g_xim_attribute_put_to_stream(GXimAttribute *attr,
			      GXimProtocol  *proto,
			      GCancellable  *cancellable,
			      GError       **error)
{
	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (attr != NULL, 0);
	g_return_val_if_fail (error != NULL, 0);

	return g_xim_protocol_send_format(proto, cancellable, error, 3,
					  G_XIM_TYPE_WORD, attr->id,
					  G_XIM_TYPE_MARKER_N_BYTES_2, attr->vtype,
					  attr->vtype, attr->v.pointer);
}

/* NESTEDLIST */
GType
g_xim_nested_list_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("NESTEDLIST"),
						       g_xim_nested_list_copy,
						       g_xim_nested_list_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_nested_list_new(GXimAttr *attr,
		      guint     n_nodes)
{
	GXimNestedList *retval;

	retval = g_new0(GXimNestedList, 1);
	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->attr = g_object_ref(attr);
	if (n_nodes > 0) {
		retval->nodes = g_new0(GXimNestedListNode *, n_nodes);
		G_XIM_CHECK_ALLOC (retval->nodes, NULL);
	}
	retval->allocated_len = n_nodes;

	return retval;
}

gpointer
g_xim_nested_list_copy(gpointer boxed)
{
	GXimNestedList *retval, *orig = boxed;
	guint16 i;

	if (boxed == NULL)
		return NULL;

	retval = g_xim_nested_list_new(orig->attr, orig->n_nodes);
	G_XIM_CHECK_ALLOC (retval, NULL);

	for (i = 0; i < orig->n_nodes; i++) {
		retval->nodes[i] = g_xim_nested_list_node_copy(orig->nodes[i]);
		G_XIM_CHECK_ALLOC_WITH_CODE (retval->nodes[i],
					     {retval->n_nodes = i; g_xim_nested_list_free(retval);},
					     NULL);
	}
	retval->n_nodes = orig->n_nodes;

	return retval;
}

void
g_xim_nested_list_free(gpointer boxed)
{
	GXimNestedList *list = boxed;
	guint16 i;

	if (boxed == NULL)
		return;

	for (i = 0; i < list->n_nodes; i++) {
		GXimNestedListNode *node;

		node = list->nodes[i];
		g_xim_nested_list_node_free(node);
	}
	g_object_unref(list->attr);
	g_free(list->nodes);
	g_free(list);
}

gboolean
g_xim_nested_list_append(GXimNestedList *list,
			 const gchar    *name,
			 gpointer        value)
{
	GXimNestedListNode *node, *tmp;
	GXimValueType vtype;
	GType gtype;

	g_return_val_if_fail (list != NULL, FALSE);
	g_return_val_if_fail (name != NULL, FALSE);

	if (list->n_nodes == list->allocated_len) {
		gpointer data;

		data = g_realloc(list->nodes,
				 sizeof (gpointer) * (list->allocated_len + N_REALLOC_NESTED_LIST));
		G_XIM_CHECK_ALLOC (data, FALSE);

		list->nodes = data;
		list->allocated_len += N_REALLOC_NESTED_LIST;
	}
	gtype = g_xim_attr_get_gtype_by_name(list->attr, name);
	g_return_val_if_fail (gtype != G_TYPE_INVALID, FALSE);

	vtype = g_xim_gtype_to_value_type(gtype);
	g_return_val_if_fail (vtype != G_XIM_TYPE_INVALID, FALSE);

	tmp = g_xim_nested_list_node_new();
	G_XIM_CHECK_ALLOC (tmp, FALSE);

	tmp->vtype = vtype;
	tmp->name = (gchar *)name;
	tmp->value = value;

	node = g_xim_nested_list_node_copy(tmp);
	G_XIM_CHECK_ALLOC_WITH_CODE (node, g_free(tmp), FALSE);
	g_free(tmp);

	list->nodes[list->n_nodes] = node;
	list->n_nodes++;

	return TRUE;
}

gboolean
g_xim_nested_list_foreach(GXimNestedList *list,
			  GXimNestedFunc  func,
			  gpointer        data)
{
	guint16 i;

	g_return_val_if_fail (list != NULL, FALSE);
	g_return_val_if_fail (func != NULL, FALSE);

	for (i = 0; i < list->n_nodes; i++) {
		GXimNestedListNode *node = list->nodes[i];

		if (func(node, data))
			break;
	}

	return TRUE;
}

gsize
g_xim_nested_list_put_to_stream(GXimNestedList  *list,
				GXimProtocol    *proto,
				GCancellable    *cancellable,
				GError         **error)
{
	gsize retval = 0;
	guint16 i;

	g_return_val_if_fail (list != NULL, 0);
	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (error != NULL, 0);

	for (i = 0; i < list->n_nodes; i++) {
		GXimNestedListNode *node;
		gint16 attr_id;

		node = list->nodes[i];
		attr_id = g_xim_attr_get_attribute_id(list->attr, node->name);
		if (attr_id < 0) {
			g_xim_message_warning(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      "No attribute id available for %s in %s",
					      node->name, g_type_name(G_TYPE_FROM_INSTANCE (list->attr)));
			continue;
		}
		retval += g_xim_protocol_send_format(proto, cancellable, error, 3,
						     G_XIM_TYPE_WORD, attr_id,
						     G_XIM_TYPE_MARKER_N_BYTES_2, node->vtype,
						     node->vtype, node->value);
	}

	return retval;
}

/* SeparatorofNestedList */
GType
g_xim_sep_nested_list_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("SeparatorofNestedList"),
						       g_xim_sep_nested_list_copy,
						       g_xim_sep_nested_list_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_sep_nested_list_new(void)
{
	return g_new0(GXimSepNestedList, 1);
}

gpointer
g_xim_sep_nested_list_copy(gpointer boxed)
{
	if (boxed == NULL)
		return NULL;

	return g_xim_sep_nested_list_new();
}

void
g_xim_sep_nested_list_free(gpointer boxed)
{
	g_free(boxed);
}

/* EXT */
GType
g_xim_ext_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimExt"),
						       g_xim_ext_copy,
						       g_xim_ext_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_ext_new(guint8       major_opcode,
	      guint8       minor_opcode,
	      const gchar *name)
{
	GXimExt *retval;

	retval = g_new0(GXimExt, 1);
	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->major_opcode = major_opcode;
	retval->minor_opcode = minor_opcode;
	retval->name = g_string_new(name);

	return retval;
}

gpointer
g_xim_ext_copy(gpointer boxed)
{
	GXimExt *orig = boxed;

	if (boxed == NULL)
		return NULL;

	return g_xim_ext_new(orig->major_opcode,
			     orig->minor_opcode,
			     orig->name->str);
}

void
g_xim_ext_free(gpointer boxed)
{
	GXimExt *ext = boxed;

	if (boxed == NULL)
		return;

	g_string_free(ext->name, TRUE);
	g_free(ext);
}

gsize
g_xim_ext_put_to_stream(GXimExt       *ext,
			GXimProtocol  *proto,
			GCancellable  *cancellable,
			GError       **error)
{
	g_return_val_if_fail (ext != NULL, 0);
	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (error != NULL, 0);

	return g_xim_protocol_send_format(proto, cancellable, error, 4,
					  G_XIM_TYPE_BYTE, ext->major_opcode,
					  G_XIM_TYPE_BYTE, ext->minor_opcode,
					  G_XIM_TYPE_GSTRING, ext->name,
					  G_XIM_TYPE_AUTO_PADDING, 0);
}

gpointer
g_xim_ext_get_from_stream(GXimProtocol      *proto,
			  GDataInputStream  *stream,
			  GCancellable      *cancellable,
			  GError           **error)
{
	GXimExt *retval;
	guint8 major, minor;
	GString *name;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	if (!g_xim_protocol_read_format(proto, stream, cancellable, error,
					4,
					G_XIM_TYPE_BYTE, &major,
					G_XIM_TYPE_BYTE, &minor,
					G_XIM_TYPE_GSTRING, &name,
					G_XIM_TYPE_AUTO_PADDING, 0))
		return NULL;

	retval = g_xim_ext_new(major, minor, name->str);
	g_string_free(name, TRUE);

	return retval;
}

/* GXimText */
GType
g_xim_text_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimText"),
						       g_xim_text_copy,
						       g_xim_text_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_text_new(void)
{
	return g_new0(GXimText, 1);
}

gboolean
g_xim_text_set_mbstring(GXimText    *text,
			const gchar *string,
			gssize       length)
{
	g_return_val_if_fail (text != NULL, FALSE);
	g_return_val_if_fail (string != NULL, FALSE);

	g_xim_text_clear_string(text);
	text->encoding_is_wchar = FALSE;
	if (length < 0)
		length = strlen(string);
	g_return_val_if_fail (length < 65536, FALSE);
	text->length = length;
	text->string.multi_byte = g_strdup(string);
	text->feedback = g_new0(GXimFeedback, length + 1);

	return TRUE;
}

gboolean
g_xim_text_set_wcstring(GXimText        *text,
			const gunichar2 *string,
			gssize           length)
{
	g_return_val_if_fail (text != NULL, FALSE);
	g_return_val_if_fail (string != NULL, FALSE);
	g_return_val_if_fail (length > 0 && length < 65536, FALSE);

	g_xim_text_clear_string(text);
	text->encoding_is_wchar = TRUE;
	text->length = length;
	text->string.wide_char = g_new0(gunichar2, length + 1);
	G_XIM_CHECK_ALLOC (text->string.wide_char, FALSE);

	memcpy(text->string.wide_char, string, sizeof(gunichar2) * length);
	text->string.wide_char[length] = 0;
	text->feedback = g_new0(GXimFeedback, length + 1);

	return TRUE;
}

void
g_xim_text_clear_string(GXimText *text)
{
	g_return_if_fail (text != NULL);

	if (text->encoding_is_wchar) {
		g_free(text->string.wide_char);
	} else {
		g_free(text->string.multi_byte);
	}
	g_free(text->feedback);
}

void
g_xim_text_set_feedback(GXimText     *text,
			GXimFeedback  feedback,
			gshort        position)
{
	g_return_if_fail (text != NULL);
	g_return_if_fail (text->length > position);

	text->feedback[position] = feedback;
}

GXimFeedback
g_xim_text_get_feedback(GXimText *text,
			gshort    position)
{
	g_return_val_if_fail (text != NULL, 0);
	g_return_val_if_fail (text->length > position, 0);
	g_return_val_if_fail (text->feedback != NULL, 0);

	return text->feedback[position];
}

gpointer
g_xim_text_copy(gpointer boxed)
{
	GXimText *retval, *orig;

	if (boxed == NULL)
		return NULL;
	orig = boxed;
	retval = g_xim_text_new();
	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->length = orig->length;
	retval->encoding_is_wchar = orig->encoding_is_wchar;
	if (orig->encoding_is_wchar) {
		g_xim_text_set_wcstring(retval,
					orig->string.wide_char,
					orig->length);
	} else {
		g_xim_text_set_mbstring(retval,
					orig->string.multi_byte,
					orig->length);
	}
	g_return_val_if_fail (retval->feedback != NULL, NULL);
	g_return_val_if_fail (orig->feedback != NULL, NULL);

	memcpy(retval->feedback, orig->feedback, sizeof (GXimFeedback) * orig->length);

	return retval;
}

void
g_xim_text_free(gpointer boxed)
{
	GXimText *text = boxed;

	if (boxed == NULL)
		return;
	if (text->encoding_is_wchar)
		g_free(text->string.wide_char);
	else
		g_free(text->string.multi_byte);
	g_free(text->feedback);

	g_free(text);
}

gsize
g_xim_text_put_to_stream(GXimText      *text,
			 GXimProtocol  *proto,
			 GCancellable  *cancellable,
			 GError       **error)
{
	guint32 mask = 0;
	gsize retval;
	gboolean no_feedback = FALSE, no_string = FALSE;
	gint i, len;

	g_return_val_if_fail (text != NULL, 0);
	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (error != NULL, 0);

	if (text->length == 0 ||
	    (text->encoding_is_wchar && text->string.wide_char[0] == 0) ||
	    (!text->encoding_is_wchar && text->string.multi_byte[0] == 0))
		no_string = TRUE;
	if (text->feedback[0] == 0)
		no_feedback = TRUE;

	if (no_string)
		mask |= (1L << 0);
	if (no_feedback)
		mask |= (1L << 1);
	retval = g_xim_protocol_send_format(proto, cancellable, error, 1,
					    G_XIM_TYPE_LONG, mask);
	if (*error)
		return 0;
	if (no_string)
		retval += g_xim_protocol_send_format(proto, cancellable, error, 2,
						     G_XIM_TYPE_WORD, 0,
						     G_XIM_TYPE_PADDING, 2);
	else
		retval += g_xim_protocol_send_format(proto, cancellable, error, 3,
						     G_XIM_TYPE_MARKER_N_BYTES_2, G_XIM_TYPE_CHAR,
						     G_XIM_TYPE_CHAR, text->string.multi_byte,
						     G_XIM_TYPE_AUTO_PADDING, 2);
	if (*error)
		return 0;
	if (no_feedback) {
		retval += g_xim_protocol_send_format(proto, cancellable, error, 2,
						     G_XIM_TYPE_WORD, 0,
						     G_XIM_TYPE_PADDING, 2);
	} else {
		for (len = 0; len < text->length; len++)
			if (text->feedback[len] == 0)
				break;
		retval += g_xim_protocol_send_format(proto, cancellable, error, 2,
						     G_XIM_TYPE_WORD, len * 4,
						     G_XIM_TYPE_PADDING, 2);
		if (*error)
			return 0;
		for (i = 0; i < len; i++) {
			guint32 val = 0;

			if (text->feedback[i] & G_XIM_XIMReverse)
				val |= (1L << 0);
			if (text->feedback[i] & G_XIM_XIMUnderline)
				val |= (1L << 1);
			if (text->feedback[i] & G_XIM_XIMHighlight)
				val |= (1L << 2);
			if (text->feedback[i] & G_XIM_XIMPrimary)
				val |= (1L << 3);
			if (text->feedback[i] & G_XIM_XIMSecondary)
				val |= (1L << 4);
			if (text->feedback[i] & G_XIM_XIMTertiary)
				val |= (1L << 5);
			if (text->feedback[i] & G_XIM_XIMVisibleToForward)
				val |= (1L << 6);
			if (text->feedback[i] & G_XIM_XIMVisibleToBackward)
				val |= (1L << 7);
			if (text->feedback[i] & G_XIM_XIMVisibleToCenter)
				val |= (1L << 8);
			retval += g_xim_protocol_send_format(proto, cancellable, error, 1,
							     G_XIM_TYPE_LONG, val);
			if (*error)
				return 0;
		}
	}

	return retval;
}

gpointer
g_xim_text_get_from_stream(GXimProtocol      *proto,
			   GDataInputStream  *stream,
			   GCancellable      *cancellable,
			   GError           **error)
{
	GXimText *retval;
	guint32 mask;
	GString *string;
	guint16 length, i;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 5,
					G_XIM_TYPE_LONG, &mask,
					G_XIM_TYPE_GSTRING, &string,
					G_XIM_TYPE_AUTO_PADDING, 2,
					G_XIM_TYPE_WORD, &length,
					G_XIM_TYPE_PADDING, 2))
		return NULL;

	retval = g_xim_text_new();
	G_XIM_GERROR_CHECK_ALLOC (retval, error,
				  G_XIM_PROTOCOL_ERROR, NULL);

	g_xim_text_set_mbstring(retval, string->str, string->len);
	g_string_free(string, TRUE);
	if ((mask & (1L << 1)) == 0) {
		if (length < 4 ||
		    (length % 4) != 0) {
			g_xim_message_warning(G_XIM_PROTOCOL_GET_IFACE (proto)->message,
					      "The length of feedback isn't aligned to 4 bytes: %" G_GUINT16_FORMAT,
					      length);
		}
		length /= 4;
	} else {
		length = 0;
	}
	for (i = 0; i < length; i++) {
		guint32 val = 0, xval = 0;

		if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 1,
						G_XIM_TYPE_LONG, &val)) {
			length = i;
			G_XIM_GERROR_RESET_NOTICE_FLAG (*error);
			G_XIM_GERROR_SET_NOTICE_FLAG (*error,
						      G_XIM_NOTICE_WARNING);
			break;
		}
		if (val & (1L << 0))
			xval |= G_XIM_XIMReverse;
		if (val & (1L << 1))
			xval |= G_XIM_XIMUnderline;
		if (val & (1L << 2))
			xval |= G_XIM_XIMHighlight;
		if (val & (1L << 3))
			xval |= G_XIM_XIMPrimary;
		if (val & (1L << 4))
			xval |= G_XIM_XIMSecondary;
		if (val & (1L << 5))
			xval |= G_XIM_XIMTertiary;
		if (val & (1L << 6))
			xval |= G_XIM_XIMVisibleToForward;
		if (val & (1L << 7))
			xval |= G_XIM_XIMVisibleToBackward;
		if (val & (1L << 8))
			xval |= G_XIM_XIMVisibleToCenter;
		g_xim_text_set_feedback(retval, xval, i);
	}
	if (mask & (1L << 0))
		retval->length = length;

	return retval;
}
			 
/* GXimPreeditCaret */
GType
g_xim_preedit_caret_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimPreeditCaret"),
						       g_xim_preedit_caret_copy,
						       g_xim_preedit_caret_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_preedit_caret_new(void)
{
	return g_new0(GXimPreeditCaret, 1);
}

gpointer
g_xim_preedit_caret_copy(gpointer boxed)
{
	GXimPreeditCaret *retval, *orig;

	if (boxed == NULL)
		return NULL;
	orig = boxed;

	retval = g_xim_preedit_caret_new();
	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->position = orig->position;
	retval->direction = orig->direction;
	retval->style = orig->style;

	return retval;
}

void
g_xim_preedit_caret_free(gpointer boxed)
{
	g_free(boxed);
}

gsize
g_xim_preedit_caret_put_to_stream(GXimPreeditCaret  *caret,
				  GXimProtocol      *proto,
				  GCancellable      *cancellable,
				  GError           **error)
{
	g_return_val_if_fail (caret != NULL, 0);
	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (error != NULL, 0);

	return g_xim_protocol_send_format(proto, cancellable, error, 3,
					  G_XIM_TYPE_LONG, caret->position,
					  G_XIM_TYPE_LONG, caret->direction,
					  G_XIM_TYPE_LONG, caret->style);
}

gpointer
g_xim_preedit_caret_get_from_stream(GXimProtocol      *proto,
				    GDataInputStream  *stream,
				    GCancellable      *cancellable,
				    GError           **error)
{
	GXimPreeditCaret *retval;
	gint32 position;
	GXimCaretDirection direction;
	GXimCaretStyle style;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	if (!g_xim_protocol_read_format(proto, stream, cancellable, error,
					3,
					G_XIM_TYPE_LONG, &position,
					G_XIM_TYPE_LONG, &direction,
					G_XIM_TYPE_LONG, &style))
		return NULL;

	retval = g_xim_preedit_caret_new();
	G_XIM_GERROR_CHECK_ALLOC (retval, error,
				  G_XIM_PROTOCOL_ERROR, NULL);

	retval->position = position;
	retval->direction = direction;
	retval->style = style;

	return retval;
}

/* GXimPreeditDraw */
GType
g_xim_preedit_draw_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimPreeditDraw"),
						       g_xim_preedit_draw_copy,
						       g_xim_preedit_draw_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_preedit_draw_new(void)
{
	return g_new0(GXimPreeditDraw, 1);
}

gpointer
g_xim_preedit_draw_copy(gpointer boxed)
{
	GXimPreeditDraw *retval, *orig = boxed;

	if (boxed == NULL)
		return NULL;

	retval = g_xim_preedit_draw_new();
	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->text = g_xim_text_copy(orig->text);
	retval->caret = orig->caret;
	retval->chg_first = orig->chg_first;
	retval->chg_length = orig->chg_length;

	return retval;
}

void
g_xim_preedit_draw_free(gpointer boxed)
{
	GXimPreeditDraw *draw = boxed;

	if (boxed == NULL)
		return;

	g_xim_text_free(draw->text);
	g_free(draw);
}

gsize
g_xim_preedit_draw_put_to_stream(GXimPreeditDraw  *draw,
				 GXimProtocol     *proto,
				 GCancellable     *cancellable,
				 GError          **error)
{
	g_return_val_if_fail (draw != NULL, 0);
	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (error != NULL, 0);

	return g_xim_protocol_send_format(proto, cancellable, error, 4,
					  G_XIM_TYPE_LONG, draw->caret,
					  G_XIM_TYPE_LONG, draw->chg_first,
					  G_XIM_TYPE_LONG, draw->chg_length,
					  G_XIM_TYPE_XIMTEXT, draw->text);
}

gpointer
g_xim_preedit_draw_get_from_stream(GXimProtocol      *proto,
				   GDataInputStream  *stream,
				   GCancellable      *cancellable,
				   GError           **error)
{
	GXimPreeditDraw *retval;
	gint32 caret, chg_first, chg_length;
	GXimText *text;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 4,
					G_XIM_TYPE_LONG, &caret,
					G_XIM_TYPE_LONG, &chg_first,
					G_XIM_TYPE_LONG, &chg_length,
					G_XIM_TYPE_XIMTEXT, &text))
		return NULL;

	retval = g_xim_preedit_draw_new();
	G_XIM_GERROR_CHECK_ALLOC (retval, error,
				  G_XIM_PROTOCOL_ERROR, NULL);

	retval->caret = caret;
	retval->chg_first = chg_first;
	retval->chg_length = chg_length;
	retval->text = text;

	return retval;
}

/* GXimStatusDraw */
GType
g_xim_status_draw_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimStatusDraw"),
						       g_xim_status_draw_copy,
						       g_xim_status_draw_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_status_draw_new(void)
{
	GXimStatusDraw *retval;

	retval = g_new0(GXimStatusDraw, 1);

	return retval;
}

gpointer
g_xim_status_draw_copy(gpointer boxed)
{
	GXimStatusDraw *retval, *orig = boxed;

	if (boxed == NULL)
		return NULL;

	retval = g_xim_status_draw_new();
	G_XIM_CHECK_ALLOC (retval, NULL);

	switch (orig->type) {
	    case G_XIM_XIMTextType:
		    retval->data.text = g_xim_text_copy(orig->data.text);
		    break;
	    case G_XIM_XIMBitmapType:
		    retval->data.bitmap = g_object_ref(orig->data.bitmap);
		    break;
	    default:
		    g_xim_status_draw_free(retval);
		    return NULL;
	}

	return retval;
}

void
g_xim_status_draw_free(gpointer boxed)
{
	GXimStatusDraw *draw = boxed;

	if (boxed == NULL)
		return;

	switch (draw->type) {
	    case G_XIM_XIMTextType:
		    g_xim_text_free(draw->data.text);
		    break;
	    case G_XIM_XIMBitmapType:
		    if (draw->data.bitmap)
			    g_object_unref(draw->data.bitmap);
	    default:
		    break;
	}
	g_free(draw);
}

gsize
g_xim_status_draw_put_to_stream(GXimStatusDraw  *draw,
				GXimProtocol    *proto,
				GCancellable    *cancellable,
				GError         **error)
{
	gsize retval;

	g_return_val_if_fail (draw != NULL, 0);
	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (error != NULL, 0);

	retval = g_xim_protocol_send_format(proto, cancellable, error, 1,
					    G_XIM_TYPE_LONG, draw->type);
	if (*error)
		return 0;

	switch (draw->type) {
	    case G_XIM_XIMTextType:
		    retval += g_xim_protocol_send_format(proto, cancellable, error, 1,
							 G_XIM_TYPE_XIMTEXT, draw->data.text);
		    break;
	    case G_XIM_XIMBitmapType:
		    retval += g_xim_protocol_send_format(proto, cancellable, error, 1,
							 G_XIM_TYPE_PIXMAP, draw->data.bitmap);
		    break;
	    default:
		    g_set_error(error, G_XIM_PROTOCOL_ERROR,
				G_XIM_STD_ERROR_INVALID_ARGUMENT | G_XIM_NOTICE_BUG,
				"sending an Incomplete status draw object.");
		    return 0;
	}

	return retval;
}

gpointer
g_xim_status_draw_get_from_stream(GXimProtocol      *proto,
				  GDataInputStream  *stream,
				  GCancellable      *cancellable,
				  GError           **error)
{
	GXimStatusDraw *retval;
	GXimStatusDataType type = 0;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 1,
					G_XIM_TYPE_LONG, &type))
		return NULL;

	retval = g_xim_status_draw_new();
	G_XIM_GERROR_CHECK_ALLOC (retval, error, G_XIM_PROTOCOL_ERROR, NULL);

	retval->type = type;
	switch (type) {
	    case G_XIM_XIMTextType:
		    if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 1,
						    G_XIM_TYPE_XIMTEXT, &retval->data.text))
			    goto fail;
		    break;
	    case G_XIM_XIMBitmapType:
		    if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 1,
						    G_XIM_TYPE_PIXMAP, &retval->data.bitmap))
			    goto fail;
		    break;
	    default:
		    g_set_error(error, G_XIM_PROTOCOL_ERROR,
				G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
				"Unknown drawable type %d for status", type);
	    fail:
		    g_xim_status_draw_free(retval);

		    return NULL;
	}

	return retval;
}

/* GXimHotkeyTrigger */
GType
g_xim_hotkey_trigger_get_type(void)
{
	static volatile gsize type_id_volatile = 0;

	if (g_once_init_enter(&type_id_volatile)) {
		GType type_id;

		type_id = g_boxed_type_register_static(g_intern_static_string("GXimHotkeyTrigger"),
						       g_xim_hotkey_trigger_copy,
						       g_xim_hotkey_trigger_free);
		g_once_init_leave(&type_id_volatile, type_id);
	}

	return type_id_volatile;
}

gpointer
g_xim_hotkey_trigger_new(guint32 keysym,
			 guint32 modifier,
			 guint32 modifier_mask)
{
	GXimHotkeyTrigger *retval;

	retval = g_new0(GXimHotkeyTrigger, 1);
	G_XIM_CHECK_ALLOC (retval, NULL);

	retval->keysym = keysym;
	retval->modifier = modifier;
	retval->modifier_mask = modifier_mask;

	return retval;
}

gpointer
g_xim_hotkey_trigger_copy(gpointer boxed)
{
	GXimHotkeyTrigger *orig = boxed;

	if (boxed == NULL)
		return NULL;

	return g_xim_hotkey_trigger_new(orig->keysym,
					orig->modifier,
					orig->modifier_mask);
}

void
g_xim_hotkey_trigger_free(gpointer boxed)
{
	if (boxed == NULL)
		return;

	g_free(boxed);
}

gsize
g_xim_hotkey_trigger_put_to_stream(GXimHotkeyTrigger  *hotkey,
				   GXimProtocol       *proto,
				   GCancellable       *cancellable,
				   GError            **error)
{
	g_return_val_if_fail (hotkey != NULL, 0);
	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (error != NULL, 0);

	return g_xim_protocol_send_format(proto, cancellable, error, 3,
					  G_XIM_TYPE_LONG, hotkey->keysym,
					  G_XIM_TYPE_LONG, hotkey->modifier,
					  G_XIM_TYPE_LONG, hotkey->modifier_mask);
}

gpointer
g_xim_hotkey_trigger_get_from_stream(GXimProtocol      *proto,
				     GDataInputStream  *stream,
				     GCancellable      *cancellable,
				     GError           **error)
{
	guint32 keysym, modifier, modifier_mask;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 3,
					G_XIM_TYPE_LONG, &keysym,
					G_XIM_TYPE_LONG, &modifier,
					G_XIM_TYPE_LONG, &modifier_mask))
		return NULL;

	return g_xim_hotkey_trigger_new(keysym, modifier, modifier_mask);
}

/* GdkEvent */
GQuark
g_xim_gdkevent_get_error_quark(void)
{
	static GQuark quark = 0;

	if (!quark)
		quark = g_quark_from_static_string("g-xim-gdkevent-error");

	return quark;
}

gsize
g_xim_gdkevent_put_to_stream(GdkEvent      *event,
			     GXimProtocol  *proto,
			     GCancellable  *cancellable,
			     GError       **error)
{
	gint xtype;
	gsize size = 0;

	g_return_val_if_fail (event != NULL, 0);
	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), 0);
	g_return_val_if_fail (error != NULL, 0);

	/* XXX: better move this into GXimTransport? */
	xtype = g_xim_gdkevent_translate_event_type(event);
	if (xtype == 0) {
		g_set_error(error, G_XIM_GDKEVENT_ERROR,
			    G_XIM_GDKEVENT_ERROR_UNSUPPORTED_EVENT | G_XIM_NOTICE_BUG,
			    "Unable to convert GdkEvent %d to any X event. this may be a libgxim bug. please report a bug.",
			    event->type);
		return 0;
	}
	/* No serial number provided in GdkEvent */
	size = g_xim_protocol_send_format(proto, cancellable, error, 2,
					  G_XIM_TYPE_WORD, 0,
					  G_XIM_TYPE_BYTE, xtype);
	if (*error)
		return 0;

	switch (event->type) {
	    case GDK_DESTROY:
		    size += g_xim_protocol_send_format(proto, cancellable, error, 4,
						       G_XIM_TYPE_BYTE, 0, /* detail */
						       G_XIM_TYPE_WORD, 0, /* serial */
						       G_XIM_TYPE_WINDOW, event->any.window,
						       G_XIM_TYPE_WINDOW, event->any.window);
		    break;
	    case GDK_EXPOSE:
		    /* XXX: We may need to correct the rectangle area.
		     * because GTK+ modifies them.
		     */
		    size += g_xim_protocol_send_format(proto, cancellable, error, 9,
						       G_XIM_TYPE_BYTE, 0, /* detail */
						       G_XIM_TYPE_WORD, 0, /* serial */
						       G_XIM_TYPE_WINDOW, event->expose.window,
						       G_XIM_TYPE_WORD, event->expose.area.x,
						       G_XIM_TYPE_WORD, event->expose.area.y,
						       G_XIM_TYPE_WORD, event->expose.area.width,
						       G_XIM_TYPE_WORD, event->expose.area.height,
						       G_XIM_TYPE_WORD, event->expose.count,
						       G_XIM_TYPE_PADDING, 2);
		    break;
	    case GDK_MOTION_NOTIFY:
		    /* XXX: GTK+ doesn't deal with a child window. */
		    size += g_xim_protocol_send_format(proto, cancellable, error, 13,
						       G_XIM_TYPE_BYTE, event->motion.is_hint, /* detail */
						       G_XIM_TYPE_WORD, 0, /* serial */
						       G_XIM_TYPE_LONG, event->motion.time,
						       G_XIM_TYPE_WINDOW, gdk_screen_get_root_window(gdk_display_get_default_screen(gdk_drawable_get_display(event->motion.window))),
						       G_XIM_TYPE_WINDOW, event->motion.window,
						       G_XIM_TYPE_WINDOW, event->motion.window,
						       G_XIM_TYPE_WORD, (gint)event->motion.x_root,
						       G_XIM_TYPE_WORD, (gint)event->motion.y_root,
						       G_XIM_TYPE_WORD, (gint)event->motion.x,
						       G_XIM_TYPE_WORD, (gint)event->motion.y,
						       G_XIM_TYPE_WORD, event->motion.state,
						       G_XIM_TYPE_BYTE, 1, /* sameScreen */
						       G_XIM_TYPE_PADDING, 1);
		    break;
	    case GDK_BUTTON_PRESS:
	    case GDK_2BUTTON_PRESS:
	    case GDK_3BUTTON_PRESS:
	    case GDK_BUTTON_RELEASE:
		    /* XXX: GTK+ doesn't deal with a child window. */
		    size += g_xim_protocol_send_format(proto, cancellable, error, 13,
						       G_XIM_TYPE_BYTE, event->button.button, /* detail */
						       G_XIM_TYPE_WORD, 0, /* serial */
						       G_XIM_TYPE_LONG, event->button.time,
						       G_XIM_TYPE_WINDOW, gdk_screen_get_root_window(gdk_display_get_default_screen(gdk_drawable_get_display(event->button.window))),
						       G_XIM_TYPE_WINDOW, event->button.window,
						       G_XIM_TYPE_WINDOW, event->button.window,
						       G_XIM_TYPE_WORD, (gint)event->button.x_root,
						       G_XIM_TYPE_WORD, (gint)event->button.y_root,
						       G_XIM_TYPE_WORD, (gint)event->button.x,
						       G_XIM_TYPE_WORD, (gint)event->button.y,
						       G_XIM_TYPE_WORD, event->button.state,
						       G_XIM_TYPE_BYTE, 1, /* sameScreen */
						       G_XIM_TYPE_PADDING, 1);
		    break;
	    case GDK_KEY_PRESS:
	    case GDK_KEY_RELEASE:
		    /* XXX: GTK+ doesn't deal with a child window nor positions the event occurred on. */
		    size += g_xim_protocol_send_format(proto, cancellable, error, 13,
						       G_XIM_TYPE_BYTE, event->key.hardware_keycode, /* detail */
						       G_XIM_TYPE_WORD, 0, /* serial */
						       G_XIM_TYPE_LONG, event->key.time,
						       G_XIM_TYPE_WINDOW, gdk_screen_get_root_window(gdk_display_get_default_screen(gdk_drawable_get_display(event->key.window))),
						       G_XIM_TYPE_WINDOW, event->key.window,
						       G_XIM_TYPE_WINDOW, event->key.window,
						       G_XIM_TYPE_WORD, 0,
						       G_XIM_TYPE_WORD, 0,
						       G_XIM_TYPE_WORD, 0,
						       G_XIM_TYPE_WORD, 0,
						       G_XIM_TYPE_WORD, event->key.state,
						       G_XIM_TYPE_BYTE, 1, /* sameScreen */
						       G_XIM_TYPE_PADDING, 1);
		    break;
	    case GDK_ENTER_NOTIFY:
//		    return EnterNotify;
	    case GDK_LEAVE_NOTIFY:
//		    return LeaveNotify;
	    case GDK_FOCUS_CHANGE:
//		    if (event->focus_change.in)
//			    return FocusIn;
//		    else
//			    return FocusOut;
	    case GDK_CONFIGURE:
//		    return ConfigureNotify;
	    case GDK_MAP:
//		    return MapNotify;
	    case GDK_UNMAP:
//		    return UnmapNotify;
	    case GDK_PROPERTY_NOTIFY:
//		    return PropertyNotify;
	    case GDK_SELECTION_CLEAR:
//		    return SelectionClear;
	    case GDK_SELECTION_REQUEST:
//		    return SelectionRequest;
	    case GDK_SELECTION_NOTIFY:
//		    return SelectionNotify;
	    case GDK_CLIENT_EVENT:
//		    return ClientMessage;
	    case GDK_VISIBILITY_NOTIFY:
//		    return VisibilityNotify;
	    case GDK_NO_EXPOSE:
//		    return NoExpose;
	    case GDK_SCROLL:
//		    return ButtonPress;
	    default:
		    g_set_error(error, G_XIM_GDKEVENT_ERROR,
				G_XIM_GDKEVENT_ERROR_UNSUPPORTED_EVENT | G_XIM_NOTICE_BUG,
				"Not yet implemented for the event type %d",
				event->type);
		    break;
	}

	if (*error)
		size = 0;

	return size;
}

gpointer
g_xim_gdkevent_get_from_stream(GXimProtocol      *proto,
			       GDataInputStream  *stream,
			       GCancellable      *cancellable,
			       GError           **error)
{
	GXimProtocolIface *iface;
	guint16 msb_serial, lsb_serial;
	gint xtype = 0;
	GdkEventType type;
	GdkEvent *retval;

	g_return_val_if_fail (G_IS_XIM_PROTOCOL (proto), NULL);
	g_return_val_if_fail (G_IS_DATA_INPUT_STREAM (stream), NULL);
	g_return_val_if_fail (error != NULL, NULL);

	if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 2,
					G_XIM_TYPE_WORD, &msb_serial,
					G_XIM_TYPE_BYTE, &xtype))
		return NULL;

	iface = G_XIM_PROTOCOL_GET_IFACE (proto);
	g_xim_message_debug(iface->message, "proto/gdkevent",
			    "Event type: %s", g_xim_gdkevent_type_name(xtype));

	type = g_xim_xevent_translate_event_type(xtype);
	retval = gdk_event_new(type);
	G_XIM_GERROR_CHECK_ALLOC (retval, error,
				  G_XIM_PROTOCOL_ERROR, NULL);

	switch (type) {
	    case GDK_DESTROY:
		    G_STMT_START {
			    GdkWindow *ev, *window;

			    if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 4,
							    G_XIM_TYPE_PADDING, 1, /* detail(unused) */
							    G_XIM_TYPE_WORD, &lsb_serial, /* serial */
							    G_XIM_TYPE_WINDOW, &ev,
							    G_XIM_TYPE_WINDOW, &window))
				    goto end;
			    if (ev != window) {
				    /* we don't deal with it */
				    g_set_error(error, G_XIM_PROTOCOL_ERROR,
						G_XIM_PROTOCOL_ERROR_INVALID_PACKETS_RECEIVED | G_XIM_NOTICE_ERROR,
						"Received different window for DestroyNotify: event: %p, window: %p",
						G_XIM_NATIVE_WINDOW_TO_POINTER (GDK_WINDOW_XID (ev)),
						G_XIM_NATIVE_WINDOW_TO_POINTER (GDK_WINDOW_XID (window)));
				    goto end;
			    }
			    retval->any.window = g_object_ref(ev);
		    } G_STMT_END;
		    break;
	    case GDK_EXPOSE:
		    G_STMT_START {
			    GdkWindow *window;
			    gint16 x, y, width, height, count;

			    if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 9,
							    G_XIM_TYPE_PADDING, 1, /* detail(unused) */
							    G_XIM_TYPE_WORD, &lsb_serial, /* serial */
							    G_XIM_TYPE_WINDOW, &window,
							    G_XIM_TYPE_WORD, &x,
							    G_XIM_TYPE_WORD, &y,
							    G_XIM_TYPE_WORD, &width,
							    G_XIM_TYPE_WORD, &height,
							    G_XIM_TYPE_WORD, &count,
							    G_XIM_TYPE_PADDING, 2))
				    goto end;
			    retval->expose.window = g_object_ref(window);
			    retval->expose.area.x = x;
			    retval->expose.area.y = y;
			    retval->expose.area.width = width;
			    retval->expose.area.height = height;
			    retval->expose.count = count;
		    } G_STMT_END;
		    break;
	    case GDK_MOTION_NOTIFY:
		    G_STMT_START {
			    gint16 is_hint = 0, x_root, y_root, x, y;
			    guint16 state;
			    guint32 time;
			    GdkWindow *root, *ev, *window;
			    gboolean same_screen = FALSE;

			    if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 13,
							    G_XIM_TYPE_BYTE, &is_hint, /* detail */
							    G_XIM_TYPE_WORD, &lsb_serial, /* serial */
							    G_XIM_TYPE_LONG, &time,
							    G_XIM_TYPE_WINDOW, &root,
							    G_XIM_TYPE_WINDOW, &ev,
							    G_XIM_TYPE_WINDOW, &window,
							    G_XIM_TYPE_WORD, &x_root,
							    G_XIM_TYPE_WORD, &y_root,
							    G_XIM_TYPE_WORD, &x,
							    G_XIM_TYPE_WORD, &y,
							    G_XIM_TYPE_WORD, &state,
							    G_XIM_TYPE_BYTE, &same_screen,
							    G_XIM_TYPE_PADDING, 1))
				    goto end;
			    if (!same_screen) {
				    g_set_error(error, G_XIM_GDKEVENT_ERROR,
						G_XIM_GDKEVENT_ERROR_UNSUPPORTED_EVENT | G_XIM_NOTICE_ERROR,
						"Event %d occurred on the different screen.",
						type);
				    goto end;
			    }
			    retval->motion.window = g_object_ref(ev);
			    retval->motion.time = time;
			    retval->motion.x = x;
			    retval->motion.y = y;
			    retval->motion.state = state;
			    retval->motion.is_hint = is_hint;
			    retval->motion.x_root = x_root;
			    retval->motion.y_root = y_root;
			    retval->motion.axes = NULL;
			    retval->motion.device = gdk_drawable_get_display(ev)->core_pointer;
		    } G_STMT_END;
		    break;
	    case GDK_BUTTON_PRESS:
	    case GDK_BUTTON_RELEASE:
		    G_STMT_START {
			    guint16 button = 0, state;
			    gint16 x_root, y_root, x, y;
			    guint32 time;
			    GdkWindow *root, *ev, *window;
			    gboolean same_screen = FALSE;

			    if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 13,
							    G_XIM_TYPE_BYTE, &button, /* detail */
							    G_XIM_TYPE_WORD, &lsb_serial, /* serial */
							    G_XIM_TYPE_LONG, &time,
							    G_XIM_TYPE_WINDOW, &root,
							    G_XIM_TYPE_WINDOW, &ev,
							    G_XIM_TYPE_WINDOW, &window,
							    G_XIM_TYPE_WORD, &x_root,
							    G_XIM_TYPE_WORD, &y_root,
							    G_XIM_TYPE_WORD, &x,
							    G_XIM_TYPE_WORD, &y,
							    G_XIM_TYPE_WORD, &state,
							    G_XIM_TYPE_BYTE, &same_screen,
							    G_XIM_TYPE_PADDING, 1))
				    goto end;
			    if (!same_screen) {
				    g_set_error(error, G_XIM_GDKEVENT_ERROR,
						G_XIM_GDKEVENT_ERROR_UNSUPPORTED_EVENT | G_XIM_NOTICE_ERROR,
						"Event %d occurred on the different screen.",
						type);
				    goto end;
			    }
			    retval->button.window = g_object_ref(ev);
			    retval->button.time = time;
			    retval->button.x = x;
			    retval->button.y = y;
			    retval->button.state = state;
			    retval->button.button = button;
			    retval->button.x_root = x_root;
			    retval->button.y_root = y_root;
			    retval->button.axes = NULL;
			    retval->button.device = gdk_drawable_get_display(ev)->core_pointer;
		    } G_STMT_END;
		    break;
	    case GDK_KEY_PRESS:
	    case GDK_KEY_RELEASE:
		    G_STMT_START {
			    guint16 keycode = 0, state;
			    gint16 x_root, y_root, x, y;
			    guint32 time;
			    GdkWindow *root, *ev, *window;
			    gboolean same_screen = FALSE;
			    GdkDisplay *dpy;
			    GdkKeymap *keymap;
			    gunichar c = 0;
			    gchar buf[7];

			    if (!g_xim_protocol_read_format(proto, stream, cancellable, error, 13,
							    G_XIM_TYPE_BYTE, &keycode, /* detail */
							    G_XIM_TYPE_WORD, &lsb_serial, /* serial */
							    G_XIM_TYPE_LONG, &time,
							    G_XIM_TYPE_WINDOW, &root,
							    G_XIM_TYPE_WINDOW, &ev,
							    G_XIM_TYPE_WINDOW, &window,
							    G_XIM_TYPE_WORD, &x_root,
							    G_XIM_TYPE_WORD, &y_root,
							    G_XIM_TYPE_WORD, &x,
							    G_XIM_TYPE_WORD, &y,
							    G_XIM_TYPE_WORD, &state,
							    G_XIM_TYPE_BYTE, &same_screen,
							    G_XIM_TYPE_PADDING, 1))
				    goto end;
			    if (!same_screen) {
				    g_set_error(error, G_XIM_GDKEVENT_ERROR,
						G_XIM_GDKEVENT_ERROR_UNSUPPORTED_EVENT | G_XIM_NOTICE_ERROR,
						"Event %d occurred on the different screen.",
						type);
				    goto end;
			    }
			    dpy = gdk_drawable_get_display(ev);
			    keymap = gdk_keymap_get_for_display(dpy);
			    retval->key.window = g_object_ref(ev);
			    retval->key.time = time;
			    retval->key.state = state;
			    retval->key.hardware_keycode = keycode;
			    // retval->key.group =;
			    retval->key.keyval = GDK_VoidSymbol;

			    /* borrow code from gdkevents-x11.c */
			    gdk_keymap_translate_keyboard_state(keymap,
								retval->key.hardware_keycode,
								retval->key.state,
								retval->key.group,
								&retval->key.keyval,
								NULL, NULL, NULL);
			    // retval->key.is_modifier =;
			    retval->key.string = NULL;
			    if (retval->key.keyval != GDK_VoidSymbol)
				    c = gdk_keyval_to_unicode(retval->key.keyval);
			    if (c) {
				    gsize bytes_written;
				    gint len;

				    /* Apply the control key - Taken from Xlib
				     */
				    if (retval->key.state & GDK_CONTROL_MASK) {
					    if ((c >= '@' && c < '\177') || c == ' ') {
						    c &= 0x1F;
					    } else if (c == '2') {
						    retval->key.string = g_memdup ("\0\0", 2);
						    retval->key.length = 1;
						    buf[0] = '\0';
						    goto end;
					    } else if (c >= '3' && c <= '7') {
						    c -= ('3' - '\033');
					    } else if (c == '8') {
						    c = '\177';
					    } else if (c == '/') {
						    c = '_' & 0x1F;
					    }
				    }
      
				    len = g_unichar_to_utf8 (c, buf);
				    buf[len] = '\0';
      
				    retval->key.string = g_locale_from_utf8 (buf, len,
									     NULL, &bytes_written,
									     NULL);
				    if (retval->key.string)
					    retval->key.length = bytes_written;
			    } else if (retval->key.keyval == GDK_Escape) {
				    retval->key.length = 1;
				    retval->key.string = g_strdup ("\033");
			    } else if (retval->key.keyval == GDK_Return ||
				       retval->key.keyval == GDK_KP_Enter) {
				    retval->key.length = 1;
				    retval->key.string = g_strdup ("\r");
			    }
			    if (!retval->key.string) {
				    retval->key.length = 0;
				    retval->key.string = g_strdup ("");
			    }
		    } G_STMT_END;
		    break;
	    default:
		    g_set_error(error, G_XIM_GDKEVENT_ERROR,
				G_XIM_GDKEVENT_ERROR_UNSUPPORTED_EVENT | G_XIM_NOTICE_BUG,
				"Noet yet implemented for the event type %d",
				type);
		    break;
	}

  end:
	if (*error) {
		if (!G_XIM_GERROR_IS_RECOVERABLE (*error)) {
			gdk_event_free(retval);
			retval = NULL;
		}
	}

	return retval;
}
