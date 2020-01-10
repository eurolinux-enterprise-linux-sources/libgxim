/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximtypes.h
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
#ifndef __G_XIM_TYPES_H__
#define __G_XIM_TYPES_H__

#include <glib.h>
#include <glib-object.h>

#ifdef GNOME_ENABLE_DEBUG
#define d(e)	e
#else
#define d(e)
#endif

G_BEGIN_DECLS

#define G_XIM_DEFAULT_TRANSPORT_MAJOR	0
#define G_XIM_DEFAULT_TRANSPORT_MINOR	2

#define G_XIM_NATIVE_WINDOW_TO_POINTER(_o_)	((gpointer)(gulong)(_o_))
#define G_XIM_POINTER_TO_NATIVE_WINDOW(_o_)	((GdkNativeWindow)(gulong)(_o_))

#define G_XIM_NOTICE_MASK		0xf0000000

typedef struct _GXimAttr		GXimAttr;
typedef struct _GXimICAttr		GXimICAttr;
typedef struct _GXimIMAttr		GXimIMAttr;
typedef struct _GXimClient		GXimClient;
typedef struct _GXimClientConnection	GXimClientConnection;
typedef struct _GXimClientTemplate	GXimClientTemplate;
typedef struct _GXimConnection		GXimConnection;
typedef struct _GXimCore		GXimCore;
typedef struct _GXimMessages		GXimMessages;
typedef struct _GXimProtocol		GXimProtocol;
typedef struct _GXimProtocolClosure	GXimProtocolClosure;
typedef struct _GXimServer		GXimServer;
typedef struct _GXimServerConnection	GXimServerConnection;
typedef struct _GXimServerTemplate	GXimServerTemplate;
typedef struct _GXimTransport		GXimTransport;

typedef struct _GXimStrConvText		GXimStrConvText;
typedef struct _GXimStyles		GXimStyles;
typedef struct _GXimRectangle		GXimRectangle;
typedef struct _GXimPoint		GXimPoint;
typedef struct _GXimFontSet		GXimFontSet;
typedef struct _GXimString		GXimString;
typedef struct _GXimStr			GXimStr;
typedef struct _GXimEncodingInfo	GXimEncodingInfo;
typedef struct _GXimNestedList		GXimNestedList;
typedef struct _GXimSepNestedList	GXimSepNestedList;
typedef struct _GXimExt			GXimExt;
typedef struct _GXimAttribute		GXimAttribute;
typedef struct _GXimRawAttr		GXimRawAttr;
typedef struct _GXimText		GXimText;
typedef struct _GXimPreeditCaret	GXimPreeditCaret;
typedef struct _GXimPreeditDraw		GXimPreeditDraw;
typedef struct _GXimStatusDraw		GXimStatusDraw;
typedef struct _GXimHotkeyTrigger	GXimHotkeyTrigger;

typedef struct _GXimLazySignalConnector	GXimLazySignalConnector;

typedef enum {
	G_XIM_ERR_BadAlloc = 1,
	G_XIM_ERR_BadStyle,
	G_XIM_ERR_BadClientWindow,
	G_XIM_ERR_BadFocusWindow,
	G_XIM_ERR_BadArea,
	G_XIM_ERR_BadSpotLocation,
	G_XIM_ERR_BadColormap,
	G_XIM_ERR_BadAtom,
	G_XIM_ERR_BadPixel,
	G_XIM_ERR_BadPixmap,
	G_XIM_ERR_BadName,
	G_XIM_ERR_BadCursor,
	G_XIM_ERR_BadProtocol,
	G_XIM_ERR_BadForeground,
	G_XIM_ERR_BadBackground,
	G_XIM_ERR_LocaleNotSupported,
	G_XIM_ERR_BadSomething = 999
} GXimErrorCode;

typedef enum {
	G_XIM_EMASK_NO_VALID_ID = 0,
	G_XIM_EMASK_VALID_IMID = 1,
	G_XIM_EMASK_VALID_ICID = 2
} GXimErrorMask;

typedef enum {
	G_XIM_CONNECT = 1,
	G_XIM_CONNECT_REPLY,
	G_XIM_DISCONNECT,
	G_XIM_DISCONNECT_REPLY,
	G_XIM_AUTH_REQUIRED = 10,
	G_XIM_AUTH_REPLY,
	G_XIM_AUTH_NEXT,
	G_XIM_AUTH_SETUP,
	G_XIM_AUTH_NG,
	G_XIM_ERROR = 20,
	G_XIM_OPEN = 30,
	G_XIM_OPEN_REPLY,
	G_XIM_CLOSE,
	G_XIM_CLOSE_REPLY,
	G_XIM_REGISTER_TRIGGERKEYS,
	G_XIM_TRIGGER_NOTIFY,
	G_XIM_TRIGGER_NOTIFY_REPLY,
	G_XIM_SET_EVENT_MASK,
	G_XIM_ENCODING_NEGOTIATION,
	G_XIM_ENCODING_NEGOTIATION_REPLY,
	G_XIM_QUERY_EXTENSION,
	G_XIM_QUERY_EXTENSION_REPLY,
	G_XIM_SET_IM_VALUES,
	G_XIM_SET_IM_VALUES_REPLY,
	G_XIM_GET_IM_VALUES,
	G_XIM_GET_IM_VALUES_REPLY,
	G_XIM_CREATE_IC = 50,
	G_XIM_CREATE_IC_REPLY,
	G_XIM_DESTROY_IC,
	G_XIM_DESTROY_IC_REPLY,
	G_XIM_SET_IC_VALUES,
	G_XIM_SET_IC_VALUES_REPLY,
	G_XIM_GET_IC_VALUES,
	G_XIM_GET_IC_VALUES_REPLY,
	G_XIM_SET_IC_FOCUS,
	G_XIM_UNSET_IC_FOCUS,
	G_XIM_FORWARD_EVENT,
	G_XIM_SYNC,
	G_XIM_SYNC_REPLY,
	G_XIM_COMMIT,
	G_XIM_RESET_IC,
	G_XIM_RESET_IC_REPLY,
	G_XIM_GEOMETRY = 70,
	G_XIM_STR_CONVERSION,
	G_XIM_STR_CONVERSION_REPLY,
	G_XIM_PREEDIT_START,
	G_XIM_PREEDIT_START_REPLY,
	G_XIM_PREEDIT_DRAW,
	G_XIM_PREEDIT_CARET,
	G_XIM_PREEDIT_CARET_REPLY,
	G_XIM_PREEDIT_DONE,
	G_XIM_STATUS_START,
	G_XIM_STATUS_DRAW,
	G_XIM_STATUS_DONE,
	G_XIM_PREEDITSTATE,
	LAST_XIM_EVENTS
} GXimOpcode;

typedef enum {
	G_XIM_Event_Synchronous = 1 << 0,
	G_XIM_Event_FilterRequest = 1 << 1,
	G_XIM_Event_LookupRequest = 1 << 2
} GXimEventFlags;

typedef enum {
	G_XIM_XIMReverse           = (1L << 0),
	G_XIM_XIMUnderline         = (1L << 1),
	G_XIM_XIMHighlight         = (1L << 2),
	G_XIM_XIMPrimary           = (1L << 5),
	G_XIM_XIMSecondary         = (1L << 6),
	G_XIM_XIMTertiary          = (1L << 7),
	G_XIM_XIMVisibleToForward  = (1L << 8),
	G_XIM_XIMVisibleToBackward = (1L << 9),
	G_XIM_XIMVisibleToCenter   = (1L << 10)
} GXimFeedback;

typedef enum {
	G_XIM_XIMForwardChar,
	G_XIM_XIMBackwardChar,
	G_XIM_XIMForwardWord,
	G_XIM_XIMBackwardWord,
	G_XIM_XIMCaretUp,
	G_XIM_XIMCaretDown,
	G_XIM_XIMNextLine,
	G_XIM_XIMPreviousLine,
	G_XIM_XIMLineStart,
	G_XIM_XIMLineEnd,
	G_XIM_XIMAbsolutePosition,
	G_XIM_XIMDontChange
} GXimCaretDirection;

typedef enum {
	G_XIM_XIMIsInvisible,
	G_XIM_XIMIsPrimary,
	G_XIM_XIMIsSecondary
} GXimCaretStyle;

typedef enum {
	G_XIM_XLookupSynchronous = 1 << 0,
	G_XIM_XLookupChars = 1 << 1,
	G_XIM_XLookupKeySym = 1 << 2
} GXimLookupType;

typedef enum {
	G_XIM_STRCONV_LEFTEDGE   = 1 << 0,
	G_XIM_STRCONV_RIGHTEDGE  = 1 << 1,
	G_XIM_STRCONV_TOPEDGE    = 1 << 2,
	G_XIM_STRCONV_BOTTOMEDGE = 1 << 3,
	G_XIM_STRCONV_CONVEALED  = 1 << 4,
	G_XIM_STRCONV_WRAPPED    = 1 << 5,
	G_XIM_STRCONV_LAST       = 1 << 15
} GXimStrConvFeedbacks;

typedef enum {
	G_XIM_TYPE_SEPARATOR = 0,
	G_XIM_TYPE_BYTE,
	G_XIM_TYPE_WORD,
	G_XIM_TYPE_LONG,
	G_XIM_TYPE_CHAR,
	G_XIM_TYPE_WINDOW,
	G_XIM_TYPE_XIMSTYLES = 10,
	G_XIM_TYPE_XRECTANGLE,
	G_XIM_TYPE_XPOINT,
	G_XIM_TYPE_XFONTSET,
	G_XIM_TYPE_HOTKEYTRIGGERS = 15,
	G_XIM_TYPE_HOTKEYSTATE,
	G_XIM_TYPE_STRINGCONVERSION,
	G_XIM_TYPE_PREEDITSTATE,
	G_XIM_TYPE_RESETSTATE,
	G_XIM_TYPE_NESTEDLIST = 0x7fff,
	/* no official support in XIM protocol */
	G_XIM_TYPE_INVALID,
	G_XIM_TYPE_PADDING,
	G_XIM_TYPE_AUTO_PADDING,
	G_XIM_TYPE_MARKER_N_BYTES_1,
	G_XIM_TYPE_MARKER_N_BYTES_2,
	G_XIM_TYPE_MARKER_N_BYTES_4,
	G_XIM_TYPE_MARKER_N_ITEMS_2,
	G_XIM_TYPE_STR,
	G_XIM_TYPE_GSTRING,
	G_XIM_TYPE_PREEDIT_CARET,
	G_XIM_TYPE_PREEDIT_DRAW,
	G_XIM_TYPE_GDKEVENT,
	G_XIM_TYPE_XIMTEXT,
	G_XIM_TYPE_HOTKEY_TRIGGER,
	G_XIM_TYPE_PIXMAP,
	G_XIM_TYPE_STATUS_DRAW,
	G_XIM_TYPE_LIST_OF_IMATTR,
	G_XIM_TYPE_LIST_OF_ICATTR,
	G_XIM_TYPE_LIST_OF_IMATTRIBUTE,
	G_XIM_TYPE_LIST_OF_ICATTRIBUTE,
	G_XIM_TYPE_LIST_OF_EXT,
	G_XIM_TYPE_LIST_OF_STRING,
	G_XIM_TYPE_LIST_OF_STR,
	G_XIM_TYPE_LIST_OF_ENCODINGINFO,
	G_XIM_TYPE_LIST_OF_BYTE,
	G_XIM_TYPE_LIST_OF_CARD16,
	G_XIM_TYPE_LIST_OF_HOTKEY_TRIGGER,
	LAST_VALUE_TYPE
} GXimValueType;

typedef enum {
	G_XIM_STD_ERROR_NO_MEMORY,
	G_XIM_STD_ERROR_NO_REAL_METHODS,
	G_XIM_STD_ERROR_INVALID_ARGUMENT,
	G_XIM_STD_ERROR_UNSUPPORTED,
} GXimStandardError;

typedef enum {
	G_XIM_NOTICE_WARNING = 1 << 28,
	G_XIM_NOTICE_ERROR = 1 << 29,
	G_XIM_NOTICE_CRITICAL = 1 << 30,
	G_XIM_NOTICE_BUG = 1 << 31
} GXimErrorType;

struct _GXimLazySignalConnector {
	gchar     *signal_name;
	GCallback  function;
	gpointer   data;
	gulong     id;
};


G_END_DECLS

#endif /* __G_XIM_TYPES_H__ */
