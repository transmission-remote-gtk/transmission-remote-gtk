/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
 * Copyright (C) 2011  Alan Fitton

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

/* Most of these functions are taken from the Transmission Project. */

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <glib-object.h>
#include <curl/curl.h>
#include <json-glib/json-glib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>

#include "util.h"
#include "dispatch.h"

const gchar *make_error_message(JsonObject * response, int status)
{
    if (status == FAIL_JSON_DECODE) {
        return g_strdup("JSON decoding error.");
    } else if (status == FAIL_RESPONSE_UNSUCCESSFUL) {
        const gchar *resultStr =
            json_object_get_string_member(response, "result");
        if (resultStr == NULL)
            return g_strdup("Server responded, but with no result.");
        else
            return g_strdup(resultStr);
    } else if (status <= -100) {
        return g_strdup_printf("Request failed with HTTP code %d",
                               -(status + 100));
    } else {
        return g_strdup(curl_easy_strerror(status));
    }
}

void response_unref(JsonObject * response)
{
    if (response != NULL)
        json_object_unref(response);
}

char *tr_strlpercent(char *buf, double x, size_t buflen)
{
    return tr_strpercent(buf, x, buflen);
}

char *tr_strpercent(char *buf, double x, size_t buflen)
{
    int precision;
    if (x < 10.0)
        precision = 2;
    else if (x < 100.0)
        precision = 1;
    else
        precision = 0;

    tr_snprintf(buf, buflen, "%.*f%%", precision, tr_truncd(x, precision));
    return buf;
}

double tr_truncd(double x, int decimal_places)
{
    const int i = (int) pow(10, decimal_places);
    double x2 = (int) (x * i);
    return x2 / i;
}

char *tr_strratio(char *buf, size_t buflen, double ratio,
                  const char *infinity)
{
    if ((int) ratio == TR_RATIO_NA)
        tr_strlcpy(buf, "None", buflen);
    else if ((int) ratio == TR_RATIO_INF)
        tr_strlcpy(buf, infinity, buflen);
    else if (ratio < 10.0)
        tr_snprintf(buf, buflen, "%.2f", tr_truncd(ratio, 2));
    else if (ratio < 100.0)
        tr_snprintf(buf, buflen, "%.1f", tr_truncd(ratio, 1));
    else
        tr_snprintf(buf, buflen, "%'.0f", ratio);
    return buf;
}

char *tr_strlratio(char *buf, double ratio, size_t buflen)
{
    return tr_strratio(buf, buflen, ratio, "\xE2\x88\x9E");
}

char *tr_strlsize(char *buf, guint64 size, size_t buflen)
{
    if (!size)
        g_strlcpy(buf, "None", buflen);
#if GLIB_CHECK_VERSION( 2, 16, 0 )
    else {
        char *tmp = g_format_size_for_display(size);
        g_strlcpy(buf, tmp, buflen);
        g_free(tmp);
    }
#else
    else if (size < (guint64) KILOBYTE_FACTOR)
        g_snprintf(buf, buflen,
                   ngettext("%'u byte", "%'u bytes", (guint) size),
                   (guint) size);
    else {
        gdouble displayed_size;
        if (size < (guint64) MEGABYTE_FACTOR) {
            displayed_size = (gdouble) size / KILOBYTE_FACTOR;
            g_snprintf(buf, buflen, _("%'.1f KB"), displayed_size);
        } else if (size < (guint64) GIGABYTE_FACTOR) {
            displayed_size = (gdouble) size / MEGABYTE_FACTOR;
            g_snprintf(buf, buflen, _("%'.1f MB"), displayed_size);
        } else {
            displayed_size = (gdouble) size / GIGABYTE_FACTOR;
            g_snprintf(buf, buflen, _("%'.1f GB"), displayed_size);
        }
    }
#endif
    return buf;
}

char *tr_strlspeed(char *buf, double kb_sec, size_t buflen)
{
    const double speed = kb_sec;

    if (speed < 1000.0)         /* 0.0 KB to 999.9 KB */
        g_snprintf(buf, buflen, "%.1f KB/s", speed);
    else if (speed < 102400.0)  /* 0.98 MB to 99.99 MB */
        g_snprintf(buf, buflen, "%.2f MB/s", (speed / KILOBYTE_FACTOR));
    else if (speed < 1024000.0) /* 100.0 MB to 999.9 MB */
        g_snprintf(buf, buflen, "%.1f MB/s", (speed / MEGABYTE_FACTOR));
    else                        /* insane speeds */
        g_snprintf(buf, buflen, "%.2f GB/s", (speed / GIGABYTE_FACTOR));

    return buf;
}

char *tr_strltime_short(char *buf, gint64 seconds, size_t buflen)
{
    int hours, minutes;

    if (seconds < 0)
        seconds = 0;

    hours = seconds / 3600;
    minutes = (seconds % 3600) / 60;
    seconds = (seconds % 3600) % 60;

    g_snprintf(buf, buflen, "%02d:%02d:%02ld", hours, minutes, seconds);

    return buf;
}

char *tr_strltime_long(char *buf, gint64 seconds, size_t buflen)
{
    int days, hours, minutes;
    char d[128], h[128], m[128], s[128];

    if (seconds < 0)
        seconds = 0;

    days = seconds / 86400;
    hours = (seconds % 86400) / 3600;
    minutes = (seconds % 3600) / 60;
    seconds = (seconds % 3600) % 60;

    g_snprintf(d, sizeof(d), days > 1 ? "%d days" : "%d day", days);
    g_snprintf(h, sizeof(h), hours > 1 ? "%d hours" : "%d hour", hours);
    g_snprintf(m, sizeof(m), minutes > 1 ? "%d minutes" : "%d minute",
               minutes);
    g_snprintf(s, sizeof(s),
               seconds > 1 ? "%ld seconds" : "%ld second", seconds);

    if (days) {
        if (days >= 4 || !hours) {
            g_strlcpy(buf, d, buflen);
        } else {
            g_snprintf(buf, buflen, "%s, %s", d, h);
        }
    } else if (hours) {
        if (hours >= 4 || !minutes) {
            g_strlcpy(buf, h, buflen);
        } else {
            g_snprintf(buf, buflen, "%s, %s", h, m);
        }
    } else if (minutes) {
        if (minutes >= 4 || !seconds) {
            g_strlcpy(buf, m, buflen);
        } else {
            g_snprintf(buf, buflen, "%s, %s", m, s);
        }
    } else {
        g_strlcpy(buf, s, buflen);
    }

    return buf;
}

char *gtr_localtime(time_t time)
{
    const struct tm tm = *localtime(&time);
    char buf[256], *eoln;

    g_strlcpy(buf, asctime(&tm), sizeof(buf));
    if ((eoln = strchr(buf, '\n')))
        *eoln = '\0';

    return g_locale_to_utf8(buf, -1, NULL, NULL, NULL);
}

char *gtr_localtime2(char *buf, time_t time, size_t buflen)
{
    char *tmp = gtr_localtime(time);
    g_strlcpy(buf, tmp, buflen);
    g_free(tmp);
    return buf;
}

int tr_snprintf(char *buf, size_t buflen, const char *fmt, ...)
{
    int len;
    va_list args;

    va_start(args, fmt);
    len = evutil_vsnprintf(buf, buflen, fmt, args);
    va_end(args);
    return len;
}

size_t tr_strlcpy(char *dst, const void *src, size_t siz)
{
#ifdef HAVE_STRLCPY
    return strlcpy(dst, src, siz);
#else
    char *d = dst;
    const char *s = src;
    size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0')
                break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (siz != 0)
            *d = '\0';          /* NUL-terminate dst */
        while (*s++);
    }

    return s - (char *) src - 1;        /* count does not include NUL */
#endif
}

int
evutil_vsnprintf(char *buf, size_t buflen, const char *format, va_list ap)
{
#ifdef _MSC_VER
    int r = _vsnprintf(buf, buflen, format, ap);
    buf[buflen - 1] = '\0';
    if (r >= 0)
        return r;
    else
        return _vscprintf(format, ap);
#else
    int r = vsnprintf(buf, buflen, format, ap);
    buf[buflen - 1] = '\0';
    return r;
#endif
}
