#include "gximacc.h"
/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */



#ifndef G_XIM_ACC_EVAL_BOOLEAN
#define G_XIM_ACC_EVAL_BOOLEAN(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_CHAR
#define G_XIM_ACC_EVAL_CHAR(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_UCHAR
#define G_XIM_ACC_EVAL_UCHAR(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_INT
#define G_XIM_ACC_EVAL_INT(_v_)		(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_UINT
#define G_XIM_ACC_EVAL_UINT(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_LONG
#define G_XIM_ACC_EVAL_LONG(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_ULONG
#define G_XIM_ACC_EVAL_ULONG(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_ENUM
#define G_XIM_ACC_EVAL_ENUM(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_FLAGS
#define G_XIM_ACC_EVAL_FLAGS(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_FLOAT
#define G_XIM_ACC_EVAL_FLOAT(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_DOUBLE
#define G_XIM_ACC_EVAL_DOUBLE(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_STRING
#define G_XIM_ACC_EVAL_STRING(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_PARAM
#define G_XIM_ACC_EVAL_PARAM(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_BOXED
#define G_XIM_ACC_EVAL_BOXED(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_POINTER
#define G_XIM_ACC_EVAL_POINTER(_v_)	(_v_)
#endif
#ifndef G_XIM_ACC_EVAL_OBJECT
#define G_XIM_ACC_EVAL_OBJECT(_v_)	(_v_)
#endif

/* BOOLEAN:BOOLEAN (./marshal.list:25) */
gboolean
_gxim_acc_signal_accumulator__BOOLEAN(GSignalInvocationHint *hints,
                                      GValue                *return_accu,
                                      const GValue          *handler_return,
                                      gpointer               data)
{
	gboolean ret = g_value_get_boolean(handler_return);

	g_value_set_boolean(return_accu, ret);

	return !G_XIM_ACC_EVAL_BOOLEAN(ret);
}

/* OBJECT:BOXED (./marshal.list:46) */
gboolean
_gxim_acc_signal_accumulator__OBJECT(GSignalInvocationHint *hints,
                                     GValue                *return_accu,
                                     const GValue          *handler_return,
                                     gpointer               data)
{
	GObject * ret = g_value_get_object(handler_return);

	g_value_set_object(return_accu, ret);

	return !G_XIM_ACC_EVAL_OBJECT(ret);
}

/* STRING:VOID (./marshal.list:47) */
gboolean
_gxim_acc_signal_accumulator__STRING(GSignalInvocationHint *hints,
                                     GValue                *return_accu,
                                     const GValue          *handler_return,
                                     gpointer               data)
{
	const gchar * ret = g_value_get_string(handler_return);

	g_value_set_string(return_accu, ret);

	return !G_XIM_ACC_EVAL_STRING(ret);
}

