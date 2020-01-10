/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gximtransport.c
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "libgxim/gximtransport.h"
#include "main.h"


GXimTransport *o;

/************************************************************/
/* common functions                                         */
/************************************************************/
void
setup(void)
{
}

void
teardown(void)
{
}

/************************************************************/
/* Test cases                                               */
/************************************************************/
TDEF (g_xim_transport_set_version)
{
} TEND

TDEF (g_xim_transport_get_version)
{
} TEND

TDEF (g_xim_transport_set_transport_size)
{
} TEND

TDEF (g_xim_transport_get_transport_size)
{
} TEND

TDEF (g_xim_transport_set_transport_max)
{
} TEND

TDEF (g_xim_transport_get_transport_max)
{
} TEND

TDEF (g_xim_transport_set_xdisplay)
{
} TEND

TDEF (g_xim_transport_get_xdisplay)
{
} TEND

TDEF (g_xim_transport_get_atom)
{
} TEND

TDEF (g_xim_transport_set_client_window)
{
} TEND

TDEF (g_xim_transport_get_client_window)
{
} TEND

TDEF (g_xim_transport_get_channel)
{
} TEND

TDEF (g_xim_transport_send_via_property)
{
} TEND

TDEF (g_xim_transport_send_via_cm)
{
} TEND

TDEF (g_xim_transport_send_via_property_notify)
{
} TEND

TDEF (g_xim_transport_dump)
{
} TEND

/************************************************************/
Suite *
g_xim_suite(void)
{
	Suite *s = suite_create("GXimTransport");
	TCase *tc = tcase_create("Generic Functionalities");

	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_set_timeout(tc, 10);

	T (g_xim_transport_set_version);
	T (g_xim_transport_get_version);
	T (g_xim_transport_set_transport_size);
	T (g_xim_transport_get_transport_size);
	T (g_xim_transport_set_transport_max);
	T (g_xim_transport_get_transport_max);
	T (g_xim_transport_set_xdisplay);
	T (g_xim_transport_get_xdisplay);
	T (g_xim_transport_get_atom);
	T (g_xim_transport_set_client_window);
	T (g_xim_transport_get_client_window);
	T (g_xim_transport_get_channel);
	T (g_xim_transport_send_via_property);
	T (g_xim_transport_send_via_cm);
	T (g_xim_transport_send_via_property_notify);
	T (g_xim_transport_dump);

	suite_add_tcase(s, tc);

	return s;
}
