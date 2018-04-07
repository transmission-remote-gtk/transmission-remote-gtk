/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
 * Copyright (C) 2011-2013  Alan Fitton

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libsecret/secret.h>

#include "trg-secret-schema.h"
#include "trg-prefs.h"

/*
 * Suppress: "warning: missing initializer for field ‘reserved’ of ‘SecretSchema 
 * {aka const struct <anonymous>}’ [-Wmissing-field-initializers]" which is harmless
 */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

const SecretSchema *
trg_secret_get_schema (void)
{
    static const SecretSchema the_schema = {
        "io.github.TransmissionRemoteGtk.password", SECRET_SCHEMA_NONE,
        {
            {  TRG_PREFS_KEY_PROFILE_UUID, SECRET_SCHEMA_ATTRIBUTE_STRING },
            {  NULL, 0 },
        }
    };
    return &the_schema;
}