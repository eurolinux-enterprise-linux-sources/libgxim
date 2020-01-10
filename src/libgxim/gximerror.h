/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximerror.h
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA  02110-1301  USA
 */
#ifndef __G_XIM_ERROR_H__
#define __G_XIM_ERROR_H__

#include <glib.h>

G_BEGIN_DECLS

#define G_XIM_ERROR_DECODE_X_ERROR_CODE(_v_)	(((_v_) >> 24) & 0xff)
#define G_XIM_ERROR_DECODE_X_REQUEST_CODE(_v_)	(((_v_) >> 16) & 0xff)
#define G_XIM_ERROR_DECODE_X_MINOR_CODE(_v_)	(((_v_) >> 8) & 0xff)
#define G_XIM_ERROR_ERROR_CODE(_e_,_r_,_m_)	((((_e_) & 0xff) << 24) | (((_r_) & 0xff) << 16) | (((_m_) & 0xff) << 8))

void    g_xim_error_push(void);
guint32 g_xim_error_pop (void);


G_END_DECLS

#endif /* __LIBGXIM_INTERNAL__ERROR_H__ */
