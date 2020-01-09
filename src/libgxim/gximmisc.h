/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximmisc.h
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
#ifndef __G_XIM_MISC_H__
#define __G_XIM_MISC_H__

#include <glib.h>
#include <gio/gio.h>
#include <gdk/gdk.h>
#include <libgxim/gximtypes.h>

G_BEGIN_DECLS

#define G_XIM_GERROR_IS_RECOVERABLE(_e_)		\
	(!(((_e_)->code & G_XIM_NOTICE_ERROR) ||	\
	   ((_e_)->code & G_XIM_NOTICE_CRITICAL) ||	\
	   ((_e_)->code & G_XIM_NOTICE_MASK) == 0))
#define G_XIM_GERROR_RESET_NOTICE_FLAG(_e_)	\
	((_e_)->code &= ~G_XIM_NOTICE_MASK)
#define G_XIM_GERROR_SET_NOTICE_FLAG(_e_,_f_)	\
	((_e_)->code |= (_f_))
#define G_XIM_CHECK_ALLOC_WITH_CODE(_v_,_c_,_r_)		\
	if ((_v_) == NULL) {					\
		g_warning("%s: Unable to allocate a memory.",	\
			  __FUNCTION__);			\
		(_c_);						\
		return (_r_);					\
	}
#define G_XIM_CHECK_ALLOC(_v_,_r_)				\
	if ((_v_) == NULL) {					\
		g_warning("%s: Unable to allocate a memory.",	\
			  __FUNCTION__);			\
		return (_r_);					\
	}
#define G_XIM_CHECK_ALLOC_WITH_NO_RET(_v_)			\
	if ((_v_) == NULL) {					\
		g_warning("%s: Unable to allocate a memory.",	\
			  __FUNCTION__);			\
		return;						\
	}
#define G_XIM_GERROR_CHECK_ALLOC_WITH_CODE(_v_,_e_,_t_,_c_,_r_)		\
	if ((_v_) == NULL) {						\
		g_set_error((_e_), (_t_),				\
			    G_XIM_STD_ERROR_NO_MEMORY | G_XIM_NOTICE_ERROR, \
			    "%s: Unable to allocate a memory.",		\
			    __FUNCTION__);				\
		(_c_);							\
		return (_r_);						\
	}
#define G_XIM_GERROR_CHECK_ALLOC(_v_,_e_,_t_,_r_)			\
	if ((_v_) == NULL) {						\
		g_set_error((_e_), (_t_),				\
			    G_XIM_STD_ERROR_NO_MEMORY | G_XIM_NOTICE_ERROR, \
			    "%s: Unable to allocate a memory.",		\
			    __FUNCTION__);				\
		return (_r_);						\
	}

void                  g_xim_init                         (void);
void                  g_xim_finalize                     (void);
GDataStreamByteOrder  g_xim_get_byte_order               (void);
GXimValueType         g_xim_gtype_to_value_type          (GType            gtype);
GType                 g_xim_value_type_to_gtype          (GXimValueType    vtype);
const gchar          *g_xim_value_type_name              (GXimValueType    vtype);
const gchar          *g_xim_protocol_name                (guint16          major_opcode);
GdkWindow            *g_xim_get_window                   (GdkDisplay      *dpy,
                                                          GdkNativeWindow  window);
GdkPixmap            *g_xim_get_pixmap                   (GdkDisplay      *dpy,
							  GdkNativeWindow  window);
GdkWindow            *g_xim_get_selection_owner          (GdkDisplay      *display,
                                                          GdkAtom          selection);
GdkWindow            *g_xim_lookup_xim_server            (GdkDisplay      *dpy,
                                                          GdkAtom          atom_server,
							  gboolean        *is_valid);
GdkWindow            *g_xim_lookup_xim_server_from_string(GdkDisplay      *dpy,
                                                          const gchar     *server_name,
							  gboolean        *is_valid);
GdkAtom               g_xim_get_server_atom              (const gchar     *server_name);
gpointer              g_xim_copy_by_gtype                (GType            gtype,
							  gpointer         value);
void                  g_xim_free_by_gtype                (GType            gtype,
							  gpointer         value);

#if 0
/**
 * GXimStrConvText:
 *
 * A structure to deal with XIM String Conversion Text.
 **/
struct _GXimStrConvText {
	GXimStrConvFeedbacks  feedback;
	guint16               length;
	gchar                *string;
};

GXimStrConvText *g_xim_str_conv_text_new     (const gchar     *string,
                                              guint16          length);
GXimStrConvText *g_xim_str_conv_text_new_full(const gchar     *string,
                                              guint16          length,
                                              guint16          feedback);
void             g_xim_str_conv_text_free    (GXimStrConvText *text);
void             g_xim_str_conv_set_feedback (GXimStrConvText *text,
					      guint16          feedback);
guint16          g_xim_str_conv_get_feedback (GXimStrConvText *text);
#endif


/**
 * GXimStyles:
 *
 **/
#define G_TYPE_XIM_STYLES		(g_xim_styles_get_type())
#define G_XIM_STYLES_ERROR		(g_xim_styles_get_error_quark())

typedef enum {
	G_XIM_PreeditArea	= 1 << 0,
	G_XIM_PreeditCallbacks	= 1 << 1,
	G_XIM_PreeditPosition	= 1 << 2,
	G_XIM_PreeditNothing	= 1 << 3,
	G_XIM_PreeditNone	= 1 << 4,
	G_XIM_StatusArea	= 1 << 8,
	G_XIM_StatusCallbacks	= 1 << 9,
	G_XIM_StatusNothing	= 1 << 10,
	G_XIM_StatusNone	= 1 << 11,
} GXimStyle;

/* GXimStyles has to be compatible with XIMStyles.
 * but we don't include Xlib.h here.
 */
struct _GXimStyles {
	guint16    count_styles;
	GXimStyle *supported_styles;
};

GType       g_xim_styles_get_type                (void) G_GNUC_CONST;
GQuark      g_xim_styles_get_error_quark         (void);
GXimStyles *g_xim_styles_new                     (void);
gboolean    g_xim_styles_append                  (GXimStyles         *styles,
                                                  GXimStyle           style,
                                                  GError            **error);
gboolean    g_xim_styles_insert                  (GXimStyles         *styles,
						  guint               index_,
						  GXimStyle           style,
						  GError            **error);
gpointer    g_xim_styles_copy                    (gpointer            boxed);
void        g_xim_styles_free                    (gpointer            boxed);
gsize       g_xim_styles_put_to_stream           (GXimStyles         *styles,
						  GXimProtocol       *proto,
                                                  GCancellable       *cancellable,
                                                  GError            **error);
gpointer    g_xim_styles_get_from_stream         (GXimProtocol       *proto,
						  GDataInputStream   *stream,
						  GCancellable       *cancellable,
						  GError            **error);

/**
 * GXimRectangle:
 *
 **/
#define G_TYPE_XIM_RECTANGLE		(g_xim_rectangle_get_type())

/* GXimRectangle has to be compatible with XRectangle.
 * but we don't include Xlib.h here.
 */
struct _GXimRectangle {
	short x;
	short y;
	unsigned short width;
	unsigned short height;
};

GType    g_xim_rectangle_get_type       (void) G_GNUC_CONST;
gpointer g_xim_rectangle_new            (void);
gpointer g_xim_rectangle_copy           (gpointer            boxed);
void     g_xim_rectangle_free           (gpointer            boxed);
gsize    g_xim_rectangle_put_to_stream  (GXimRectangle      *rectangle,
					 GXimProtocol       *proto,
                                         GCancellable       *cancellable,
                                         GError            **error);
gpointer g_xim_rectangle_get_from_stream(GXimProtocol       *proto,
					 GDataInputStream   *stream,
                                         GCancellable       *cancellable,
                                         GError            **error);

/**
 * GXimPoint:
 *
 **/
#define G_TYPE_XIM_POINT		(g_xim_point_get_type())

/* GXimPoint has to be compatible with XPoint.
 * but we don't include Xlib.h here.
 */
struct _GXimPoint {
	short x;
	short y;
};

GType    g_xim_point_get_type       (void) G_GNUC_CONST;
gpointer g_xim_point_new            (void);
gpointer g_xim_point_copy           (gpointer            boxed);
void     g_xim_point_free           (gpointer            boxed);
gsize    g_xim_point_put_to_stream  (GXimPoint          *point,
				     GXimProtocol       *proto,
                                     GCancellable       *cancellable,
                                     GError            **error);
gpointer g_xim_point_get_from_stream(GXimProtocol       *proto,
				     GDataInputStream   *stream,
                                     GCancellable       *cancellable,
                                     GError            **error);

/**
 * GXimFontSet:
 *
 **/
#define G_TYPE_XIM_FONTSET		(g_xim_fontset_get_type())

/* We don't have entities for this really.
 * on the protocol, it doesn't send out the entire structure of XFontSet.
 * but sending out the font name only instead.
 * So we use GString as alternative to deal with it.
 */
GType    g_xim_fontset_get_type       (void) G_GNUC_CONST;
gpointer g_xim_fontset_copy           (gpointer           boxed);
void     g_xim_fontset_free           (gpointer           boxed);
gsize    g_xim_fontset_put_to_stream  (GXimFontSet       *fontset,
                                       GXimProtocol      *proto,
                                       GCancellable      *cancellable,
                                       GError           **error);
gpointer g_xim_fontset_get_from_stream(GXimProtocol      *proto,
                                       GDataInputStream  *stream,
                                       GCancellable      *cancellable,
                                       GError           **error);

/**
 * GString:
 *
 **/
/* just for convenience */
gsize    g_xim_gstring_put_to_stream  (GString           *string,
                                       GDataOutputStream *stream,
                                       GCancellable      *cancellable,
                                       GError            **error);
gpointer g_xim_gstring_get_from_stream(GDataInputStream  *stream,
                                       GCancellable      *cancellable,
                                       GError            **error);

/**
 * GXimString:
 *
 **/
#define G_TYPE_XIM_STRING		(g_xim_string_get_type())

GType        g_xim_string_get_type       (void) G_GNUC_CONST;
gpointer     g_xim_string_new            (void);
gpointer     g_xim_string_copy           (gpointer            boxed);
void         g_xim_string_free           (gpointer            boxed);
const gchar *g_xim_string_get_string     (const GXimString   *string);
gsize        g_xim_string_get_length     (GXimString         *string);
gsize        g_xim_string_put_to_stream  (GXimString         *string,
                                          GDataOutputStream  *stream,
                                          GCancellable       *cancellable,
                                          GError            **error);
gpointer     g_xim_string_get_from_stream(GDataInputStream   *stream,
                                          GCancellable       *cancellable,
                                          GError            **error);

/**
 * GXimStr:
 *
 **/
#define G_TYPE_XIM_STR			(g_xim_str_get_type())

GType        g_xim_str_get_type       (void) G_GNUC_CONST;
gpointer     g_xim_str_new            (void);
gpointer     g_xim_str_copy           (gpointer            boxed);
void         g_xim_str_free           (gpointer            boxed);
const gchar *g_xim_str_get_string     (const GXimStr      *string);
gsize        g_xim_str_get_length     (const GXimStr      *string);
GXimStr     *g_xim_str_append         (GXimStr            *string,
				       const gchar        *val);
GXimStr     *g_xim_str_append_len     (GXimStr            *string,
				       const gchar        *val,
				       gssize              len);
gsize        g_xim_str_put_to_stream  (GXimStr            *string,
                                       GDataOutputStream  *stream,
                                       GCancellable       *cancellable,
                                       GError            **error);
gpointer     g_xim_str_get_from_stream(GDataInputStream   *stream,
                                       GCancellable       *cancellable,
                                       GError            **error);

/**
 * GXimEncodingInfo:
 *
 **/
#define G_TYPE_XIM_ENCODINGINFO		(g_xim_encodinginfo_get_type())

GType        g_xim_encodinginfo_get_type       (void) G_GNUC_CONST;
gpointer     g_xim_encodinginfo_new            (void);
gpointer     g_xim_encodinginfo_copy           (gpointer                 boxed);
void         g_xim_encodinginfo_free           (gpointer                 boxed);
const gchar *g_xim_encodinginfo_get_string     (const GXimEncodingInfo  *encoding);
gsize        g_xim_encodinginfo_get_length     (GXimEncodingInfo        *encoding);
gsize        g_xim_encodinginfo_put_to_stream  (GXimEncodingInfo        *encoding,
                                                GDataOutputStream       *stream,
                                                GCancellable            *cancellable,
                                                GError                 **error);
gpointer     g_xim_encodinginfo_get_from_stream(GDataInputStream        *stream,
                                                GCancellable            *cancellable,
                                                GError                 **error);

/**
 * GXimNestedList:
 *
 **/
#define G_TYPE_XIM_NESTEDLIST		(g_xim_nested_list_get_type())

typedef struct _GXimNestedListNode	GXimNestedListNode;
typedef gboolean (* GXimNestedFunc)	(GXimNestedListNode *node,
					 gpointer            user_data);

struct _GXimNestedListNode {
	GXimValueType  vtype;
	gchar         *name;
	gpointer       value;
};
struct _GXimNestedList {
	GXimAttr            *attr;
	guint16              n_nodes;
	guint16              allocated_len;
	GXimNestedListNode **nodes;
};

GType    g_xim_nested_list_get_type       (void) G_GNUC_CONST;
gpointer g_xim_nested_list_new            (GXimAttr          *attr,
                                           guint              n_nodes);
gpointer g_xim_nested_list_copy           (gpointer           boxed);
void     g_xim_nested_list_free           (gpointer           boxed);
gboolean g_xim_nested_list_append         (GXimNestedList    *list,
                                           const gchar       *name,
                                           gpointer           value);
gboolean g_xim_nested_list_foreach        (GXimNestedList    *list,
					   GXimNestedFunc     func,
					   gpointer           data);
gsize    g_xim_nested_list_put_to_stream  (GXimNestedList    *list,
                                           GXimProtocol      *proto,
                                           GCancellable      *cancellable,
                                           GError           **error);

/**
 * GXimSepNestedList:
 *
 **/
#define G_TYPE_XIM_SEP_NESTEDLIST	(g_xim_sep_nested_list_get_type())

struct _GXimSepNestedList {
	guint8 dummy;
};

GType    g_xim_sep_nested_list_get_type(void) G_GNUC_CONST;
gpointer g_xim_sep_nested_list_new     (void);
gpointer g_xim_sep_nested_list_copy    (gpointer  boxed);
void     g_xim_sep_nested_list_free    (gpointer  boxed);

/**
 * GXimExt:
 *
 **/
#define G_TYPE_XIM_EXT			(g_xim_ext_get_type())

struct _GXimExt {
	guint8   major_opcode;
	guint8   minor_opcode;
	GString *name;
};

GType    g_xim_ext_get_type       (void) G_GNUC_CONST;
gpointer g_xim_ext_new            (guint8            major_opcode,
                                   guint8            minor_opcode,
                                   const gchar      *name);
gpointer g_xim_ext_copy           (gpointer          boxed);
void     g_xim_ext_free           (gpointer          boxed);
gsize    g_xim_ext_put_to_stream  (GXimExt          *ext,
                                   GXimProtocol     *proto,
                                   GCancellable     *cancellable,
                                   GError           **error);
gpointer g_xim_ext_get_from_stream(GXimProtocol     *proto,
                                   GDataInputStream *stream,
                                   GCancellable     *cancellable,
                                   GError           **error);

/**
 * GXimAttribute:
 *
 **/
#define G_TYPE_XIM_ATTRIBUTE		(g_xim_attribute_get_type())

struct _GXimAttribute {
	guint16       id;
	GXimValueType vtype;
	union {
		guint           i;
		gulong          l;
		gpointer        pointer;
	} v;
};

GType    g_xim_attribute_get_type      (void) G_GNUC_CONST;
gpointer g_xim_attribute_new           (void);
gpointer g_xim_attribute_new_with_value(guint16         id,
					GXimValueType   vtype,
                                        gpointer        value);
void     g_xim_attribute_set           (GXimAttribute  *attr,
					guint16         id,
                                        GXimValueType   vtype,
                                        gpointer        value);
void     g_xim_attribute_clear         (GXimAttribute  *attr);
gpointer g_xim_attribute_copy          (gpointer        boxed);
void     g_xim_attribute_free          (gpointer        boxed);
gsize    g_xim_attribute_put_to_stream (GXimAttribute  *attr,
					GXimProtocol   *proto,
					GCancellable   *cancellable,
					GError        **error);

/**
 * GXimRawAttr:
 *
 **/
#define G_TYPE_XIM_RAW_ATTR		(g_xim_raw_attr_get_type())

struct _GXimRawAttr {
	GXimAttribute  base;
	GString       *attribute_name;
};

GType    g_xim_raw_attr_get_type      (void) G_GNUC_CONST;
gpointer g_xim_raw_attr_new            (void);
gpointer g_xim_raw_attr_new_with_value (guint16            id,
                                        GString           *name,
                                        GXimValueType      vtype);
void     g_xim_raw_attr_set_name       (GXimRawAttr       *attr,
                                        GString           *name);
void     g_xim_raw_attr_clear          (GXimRawAttr       *attr);
gpointer g_xim_raw_attr_copy           (gpointer           boxed);
void     g_xim_raw_attr_free           (gpointer           boxed);
gsize    g_xim_raw_attr_put_to_stream  (GXimRawAttr       *attr,
                                        GXimProtocol      *proto,
                                        GCancellable      *cancellable,
                                        GError           **error);
gpointer g_xim_raw_attr_get_from_stream(GXimProtocol      *proto,
                                        GDataInputStream  *stream,
                                        GCancellable      *cancellable,
                                        GError           **error);

/**
 * GXimText:
 *
 **/
#define G_TYPE_XIM_TEXT			(g_xim_text_get_type())

/* GXimText has to be compatible with XIMText.
 * we however don't want to include Xlib.h here.
 */
struct _GXimText {
	gshort        length;
	GXimFeedback *feedback;
	gboolean      encoding_is_wchar;
	union {
		gchar *multi_byte;
		gunichar2 *wide_char;
	} string;
};

GType        g_xim_text_get_type       (void) G_GNUC_CONST;
gpointer     g_xim_text_new            (void);
gboolean     g_xim_text_set_mbstring   (GXimText          *text,
                                        const gchar       *string,
                                        gssize             length);
gboolean     g_xim_text_set_wcstring   (GXimText          *text,
                                        const gunichar2   *string,
                                        gssize             length);
void         g_xim_text_clear_string   (GXimText          *text);
void         g_xim_text_set_feedback   (GXimText          *text,
                                        GXimFeedback       feedback,
                                        gshort             position);
GXimFeedback g_xim_text_get_feedback   (GXimText          *text,
                                        gshort             position);
gpointer     g_xim_text_copy           (gpointer           boxed);
void         g_xim_text_free           (gpointer           boxed);
gsize        g_xim_text_put_to_stream  (GXimText          *text,
                                        GXimProtocol      *proto,
                                        GCancellable      *cancellable,
                                        GError           **error);
gpointer     g_xim_text_get_from_stream(GXimProtocol      *proto,
                                        GDataInputStream  *stream,
                                        GCancellable      *cancellable,
                                        GError           **error);

/**
 * GXimPreeditCaret:
 *
 **/
#define G_TYPE_XIM_PREEDIT_CARET	(g_xim_preedit_caret_get_type())

/* GXimPreeditCaret has to be compatible with
 * XIMPreeditCaretCallbackStruct. we however don't want to
 * include Xlib.h here.
 */
struct _GXimPreeditCaret {
	gint position;
	GXimCaretDirection direction;
	GXimCaretStyle     style;
};

GType    g_xim_preedit_caret_get_type       (void) G_GNUC_CONST;
gpointer g_xim_preedit_caret_new            (void);
gpointer g_xim_preedit_caret_copy           (gpointer           boxed);
void     g_xim_preedit_caret_free           (gpointer           boxed);
gsize    g_xim_preedit_caret_put_to_stream  (GXimPreeditCaret  *caret,
                                             GXimProtocol      *proto,
                                             GCancellable      *cancellable,
                                             GError           **error);
gpointer g_xim_preedit_caret_get_from_stream(GXimProtocol      *proto,
                                             GDataInputStream  *stream,
                                             GCancellable      *cancellable,
                                             GError           **error);

/**
 * GXimPreeditDraw:
 *
 **/
#define G_TYPE_XIM_PREEDIT_DRAW		(g_xim_preedit_draw_get_type())

/* GXimPreeditDraw has to be compatible with
 * XIMPreeditDrawCallbackStruct. we however don't want to
 * include Xlib.h here.
 */
struct _GXimPreeditDraw {
	gint      caret;
	gint      chg_first;
	gint      chg_length;
	GXimText *text;
};

GType    g_xim_preedit_draw_get_type       (void) G_GNUC_CONST;
gpointer g_xim_preedit_draw_new            (void);
gpointer g_xim_preedit_draw_copy           (gpointer           boxed);
void     g_xim_preedit_draw_free           (gpointer           boxed);
gsize    g_xim_preedit_draw_put_to_stream  (GXimPreeditDraw   *draw,
                                            GXimProtocol      *proto,
                                            GCancellable      *cancellable,
                                            GError           **error);
gpointer g_xim_preedit_draw_get_from_stream(GXimProtocol      *proto,
                                            GDataInputStream  *stream,
                                            GCancellable      *cancellable,
                                            GError           **error);

/**
 * GXimStatusDraw:
 *
 **/
#define G_TYPE_XIM_STATUS_DRAW		(g_xim_status_draw_get_type())

typedef enum {
	G_XIM_XIMTextType,
	G_XIM_XIMBitmapType
} GXimStatusDataType;

struct _GXimStatusDraw {
	GXimStatusDataType  type;
	union {
		GXimText *text;
		GdkPixmap *bitmap;
	} data;
};

GType    g_xim_status_draw_get_type       (void) G_GNUC_CONST;
gpointer g_xim_status_draw_new            (void);
gpointer g_xim_status_draw_copy           (gpointer           boxed);
void     g_xim_status_draw_free           (gpointer           boxed);
gsize    g_xim_status_draw_put_to_stream  (GXimStatusDraw    *draw,
                                           GXimProtocol      *proto,
                                           GCancellable      *cancellable,
                                           GError           **error);
gpointer g_xim_status_draw_get_from_stream(GXimProtocol      *proto,
                                           GDataInputStream  *stream,
                                           GCancellable      *cancellable,
                                           GError           **error);

/**
 * GXimHotkeyTrigger:
 *
 **/
#define G_TYPE_XIM_HOTKEY_TRIGGER	(g_xim_hotkey_trigger_get_type())

struct _GXimHotkeyTrigger {
	guint32 keysym;
	guint32 modifier;
	guint32 modifier_mask;
};

GType    g_xim_hotkey_trigger_get_type       (void) G_GNUC_CONST;
gpointer g_xim_hotkey_trigger_new            (guint32             keysym,
                                              guint32             modifier,
                                              guint32             modifier_mask);
gpointer g_xim_hotkey_trigger_copy           (gpointer            boxed);
void     g_xim_hotkey_trigger_free           (gpointer            boxed);
gsize    g_xim_hotkey_trigger_put_to_stream  (GXimHotkeyTrigger  *hotkey,
                                              GXimProtocol       *proto,
                                              GCancellable       *cancellable,
                                              GError            **error);
gpointer g_xim_hotkey_trigger_get_from_stream(GXimProtocol       *proto,
                                              GDataInputStream   *stream,
                                              GCancellable       *cancellable,
                                              GError            **error);

/**
 * GdkEvent:
 *
 **/
#define G_XIM_GDKEVENT_ERROR		(g_xim_gdkevent_get_error_quark())

typedef enum {
	G_XIM_GDKEVENT_ERROR_BEGIN = 128,
	G_XIM_GDKEVENT_ERROR_UNSUPPORTED_EVENT,
} GXimGdkEventError;

GQuark   g_xim_gdkevent_get_error_quark(void);
gsize    g_xim_gdkevent_put_to_stream  (GdkEvent          *event,
					GXimProtocol      *proto,
					GCancellable      *cancellable,
					GError           **error);
gpointer g_xim_gdkevent_get_from_stream(GXimProtocol      *proto,
					GDataInputStream  *stream,
					GCancellable      *cancellable,
					GError           **error);

G_END_DECLS

#endif /* __G_XIM_MISC_H__ */
