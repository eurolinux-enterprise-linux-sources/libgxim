/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * main.h
 * Copyright (C) 2007,2008 Akira TAGOH
 * Copyright (C) 2008 Red Hat, Inc. All rights reserved.
 * 
 * Authors:
 *   Akira TAGOH  <tagoh@redhat.com>
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
#ifndef __G_XIM_TEST_MAIN_H__
#define __G_XIM_TEST_MAIN_H__

#include <check.h>
#include <glib.h>

G_BEGIN_DECLS

#define G_XIM_TEST_ERROR	g_xim_test_get_error_quark()
#define TDEF(fn)		START_TEST (test_ ## fn)
#define TEND			END_TEST
#define T(fn)			tcase_add_test(tc, test_ ## fn)
#define TNUL(obj)		fail_unless((obj) != NULL, "Failed to create an object")


void    setup                     (void);
void    teardown                  (void);
Suite  *g_xim_suite               (void);
GQuark  g_xim_test_get_error_quark(void);
gchar  *g_xim_test_pop_error      (void) G_GNUC_MALLOC;


G_END_DECLS

#endif /* __IMSETTINGS_TEST_MAIN_H__ */
