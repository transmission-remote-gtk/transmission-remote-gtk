#include "config.h"

#if HAVE_RSS

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n.h>

#include "icons.h"
#include "hig.h"
#include "util.h"
#include "trg-rss-cell-renderer.h"

enum {
    PROP_TITLE = 1,
    PROP_FEED,
    PROP_PUBLISHED,
    PROP_UPLOADED
};

#define SMALL_SCALE 0.9
#define COMPACT_ICON_SIZE GTK_ICON_SIZE_MENU
#define FULL_ICON_SIZE GTK_ICON_SIZE_DND

#define FOREGROUND_COLOR_KEY "foreground-rgba"
typedef GdkRGBA GtrColor;
typedef cairo_t GtrDrawable;
typedef GtkRequisition GtrRequisition;

struct TrgRssCellRendererPrivate {
    GtkCellRenderer *text_renderer;
    GtkCellRenderer *icon_renderer;
    GString *gstr1;
    GString *gstr2;
    gchar *title;
    gchar *published;
    gchar *feed;
    gboolean uploaded;
};

static void
trg_rss_cell_renderer_render(GtkCellRenderer * cell,
                             GtrDrawable * window, GtkWidget * widget,
                             const GdkRectangle * background_area,
                             const GdkRectangle * cell_area,
                             GtkCellRendererState flags);

static void
gtr_cell_renderer_render(GtkCellRenderer * renderer,
                         GtrDrawable * drawable,
                         GtkWidget * widget,
                         const GdkRectangle * area,
                         GtkCellRendererState flags)
{
    gtk_cell_renderer_render(renderer, drawable, widget, area, area,
                             flags);
}

static void trg_rss_cell_renderer_set_property(GObject * object,
                                               guint property_id,
                                               const GValue * v,
                                               GParamSpec * pspec)
{
    TrgRssCellRenderer *self = TRG_RSS_CELL_RENDERER(object);
    struct TrgRssCellRendererPrivate *p = self->priv;

    switch (property_id) {
    case PROP_TITLE:
    	g_free(p->title);
    	p->title = g_value_dup_string(v);
        break;
    case PROP_PUBLISHED:
    	g_free(p->published);
    	p->published = g_value_dup_string(v);
    	break;
    case PROP_FEED:
    	g_free(p->feed);
    	p->feed = g_value_dup_string(v);
    	break;
    case PROP_UPLOADED:
    	p->uploaded = g_value_get_boolean(v);
    	break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
trg_rss_cell_renderer_get_property(GObject * object,
                                   guint property_id,
                                   GValue * v, GParamSpec * pspec)
{
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
}

G_DEFINE_TYPE(TrgRssCellRenderer, trg_rss_cell_renderer,
              GTK_TYPE_CELL_RENDERER)

static void trg_rss_cell_renderer_dispose(GObject * o)
{
    TrgRssCellRenderer *r = TRG_RSS_CELL_RENDERER(o);

    if (r && r->priv) {
    	struct TrgRssCellRendererPrivate *priv = r->priv;

        g_string_free(priv->gstr1, TRUE);
        g_free(priv->feed);
        g_free(priv->published);
        g_free(priv->title);
        g_object_unref(G_OBJECT(priv->text_renderer));
        g_object_unref(G_OBJECT(priv->icon_renderer));
        r->priv = NULL;
    }

    G_OBJECT_CLASS(trg_rss_cell_renderer_parent_class)->dispose(o);
}

static GdkPixbuf *get_icon(TrgRssCellRenderer * r, GtkIconSize icon_size,
                           GtkWidget * for_widget)
{
    const char *mime_type = "file";

    return gtr_get_mime_type_icon(mime_type, icon_size, for_widget);
}

static void
trg_rss_cell_renderer_get_size(GtkCellRenderer * cell, GtkWidget * widget,
                               const GdkRectangle * cell_area,
                               gint * x_offset,
                               gint * y_offset,
                               gint * width, gint * height)
{
    TrgRssCellRenderer *self = TRG_RSS_CELL_RENDERER(cell);

    if (self) {
    	struct TrgRssCellRendererPrivate *p = self->priv;
        int xpad, ypad;
        int h = 0, w = 0;
        GtkRequisition icon_size;
        GtkRequisition name_size;
        GtkRequisition stat_size;
        GtkRequisition prog_size;
        GdkPixbuf *icon;

        icon = get_icon(self, FULL_ICON_SIZE, widget);

        gtk_cell_renderer_get_padding(cell, &xpad, &ypad);

        /* get the idealized cell dimensions */
        g_object_set(p->icon_renderer, "pixbuf", icon, NULL);
        gtk_cell_renderer_get_preferred_size(p->icon_renderer, widget, NULL,
                                             &icon_size);
        g_object_set(p->text_renderer, "text", p->title,
                     "weight", PANGO_WEIGHT_BOLD, "scale", 1.0, "ellipsize",
                     PANGO_ELLIPSIZE_NONE, NULL);
        gtk_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                             &name_size);
        g_object_set(p->text_renderer, "text", p->feed, "weight",
                     PANGO_WEIGHT_NORMAL, "scale", SMALL_SCALE, NULL);
        gtk_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                             &prog_size);
        g_object_set(p->text_renderer, "text", p->published, NULL);
        gtk_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                             &stat_size);

        /**
        *** LAYOUT
        **/

        if (width != NULL)
            *width = w =
                xpad * 2 + icon_size.width + GUI_PAD + MAX3(name_size.width,
                                                            prog_size.width,
                                                            stat_size.width);
        if (height != NULL)
            *height = h =
                ypad * 2 + name_size.height + prog_size.height +
                GUI_PAD_SMALL + stat_size.height;

        /* cleanup */
        g_object_unref(icon);

        if (x_offset)
            *x_offset = cell_area ? cell_area->x : 0;

        if (y_offset) {
            int xpad, ypad;
            gtk_cell_renderer_get_padding(cell, &xpad, &ypad);
            *y_offset =
                cell_area ? (int) ((cell_area->height - (ypad * 2 + h)) /
                                   2.0) : 0;
        }
    }
}

static void
trg_rss_cell_renderer_class_init(TrgRssCellRendererClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS(klass);

    g_type_class_add_private(klass,
                             sizeof(struct TrgRssCellRendererPrivate));

    cell_class->render = trg_rss_cell_renderer_render;
    cell_class->get_size = trg_rss_cell_renderer_get_size;
    gobject_class->set_property = trg_rss_cell_renderer_set_property;
    gobject_class->get_property = trg_rss_cell_renderer_get_property;
    gobject_class->dispose = trg_rss_cell_renderer_dispose;

    g_object_class_install_property(gobject_class,
                                    PROP_TITLE,
                                    g_param_spec_string("title",
                                                        "title",
                                                        "Title",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));

    g_object_class_install_property(gobject_class,
                                    PROP_PUBLISHED,
                                    g_param_spec_string("published",
                                                        "published",
                                                        "Published",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));

    g_object_class_install_property(gobject_class,
                                    PROP_FEED,
                                    g_param_spec_string("feed",
                                                        "feed",
                                                        "Feed",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));

    g_object_class_install_property(gobject_class,
                                    PROP_UPLOADED,
                                    g_param_spec_boolean("uploaded",
                                                        "uploaded",
                                                        "Uploaded",
                                                        FALSE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));

    /*g_object_class_install_property(gobject_class, P_BAR_HEIGHT,
                                    g_param_spec_int("bar-height", NULL,
                                                     "Bar Height",
                                                     1, INT_MAX,
                                                     DEFAULT_BAR_HEIGHT,
                                                     G_PARAM_READWRITE));*/

}

static void trg_rss_cell_renderer_init(TrgRssCellRenderer * self)
{
    struct TrgRssCellRendererPrivate *p;

    p = self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,
                                                 TRG_RSS_CELL_RENDERER_TYPE,
                                                 struct
                                                 TrgRssCellRendererPrivate);

    p->gstr1 = g_string_new(NULL);
    p->gstr2 = g_string_new(NULL);
    p->text_renderer = gtk_cell_renderer_text_new();
    g_object_set(p->text_renderer, "xpad", 0, "ypad", 0, NULL);
    p->icon_renderer = gtk_cell_renderer_pixbuf_new();
    g_object_ref_sink(p->text_renderer);
    g_object_ref_sink(p->icon_renderer);
}


GtkCellRenderer *trg_rss_cell_renderer_new(void)
{
    return (GtkCellRenderer *) g_object_new(TRG_RSS_CELL_RENDERER_TYPE,
                                            NULL);
}

static void
get_text_color(TrgRssCellRenderer * r, GtkWidget * widget,
               GtrColor * setme)
{
    struct TrgRssCellRendererPrivate *p = r->priv;

    if (p->uploaded)
        gtk_style_context_get_color(gtk_widget_get_style_context(widget),
                                    GTK_STATE_FLAG_INSENSITIVE, setme);
    else
        gtk_style_context_get_color(gtk_widget_get_style_context(widget),
                                    GTK_STATE_FLAG_NORMAL, setme);
}

static void
trg_rss_cell_renderer_render(GtkCellRenderer * cell,
                             GtrDrawable * window, GtkWidget * widget,
                             const GdkRectangle * background_area,
                             const GdkRectangle * cell_area,
                             GtkCellRendererState flags)
{
    TrgRssCellRenderer *self = TRG_RSS_CELL_RENDERER(cell);
    int xpad, ypad;
    GtkRequisition size;
    GdkRectangle fill_area;
    GdkRectangle icon_area;
    GdkRectangle name_area;
    GdkRectangle stat_area;
    GdkRectangle prog_area;
    GdkRectangle prct_area;
    GdkPixbuf *icon;
    GtrColor text_color;
    struct TrgRssCellRendererPrivate *p;

    if (!self)
    	return;

    p = self->priv;

    icon = get_icon(self, FULL_ICON_SIZE, widget);
    gtk_cell_renderer_get_padding(GTK_CELL_RENDERER(cell), &xpad, &ypad);
    get_text_color(self, widget, &text_color);

    /* get the idealized cell dimensions */
    g_object_set(p->icon_renderer, "pixbuf", icon, NULL);
    gtk_cell_renderer_get_preferred_size(p->icon_renderer, widget, NULL,
                                         &size);
    icon_area.width = size.width;
    icon_area.height = size.height;
    g_object_set(p->text_renderer, "text", p->title,
                 "weight", PANGO_WEIGHT_BOLD, "ellipsize",
                 PANGO_ELLIPSIZE_NONE, "scale", 1.0, NULL);
    gtk_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                         &size);
    name_area.width = size.width;
    name_area.height = size.height;
    g_object_set(p->text_renderer, "text", p->feed, "weight",
                 PANGO_WEIGHT_NORMAL, "scale", SMALL_SCALE, NULL);
    gtk_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                         &size);
    prog_area.width = size.width;
    prog_area.height = size.height;
    g_object_set(p->text_renderer, "text", p->published, NULL);
    gtk_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                         &size);
    stat_area.width = size.width;
    stat_area.height = size.height;

    /**
    *** LAYOUT
    **/

    fill_area = *background_area;
    fill_area.x += xpad;
    fill_area.y += ypad;
    fill_area.width -= xpad * 2;
    fill_area.height -= ypad * 2;

    /* icon */
    icon_area.x = fill_area.x;
    icon_area.y = fill_area.y + (fill_area.height - icon_area.height) / 2;

    /* name */
    name_area.x = icon_area.x + icon_area.width + GUI_PAD;
    name_area.y = fill_area.y;
    name_area.width =
        fill_area.width - GUI_PAD - icon_area.width - GUI_PAD_SMALL;

    /* prog */
    prog_area.x = name_area.x;
    prog_area.y = name_area.y + name_area.height;
    prog_area.width = name_area.width;

    /* progressbar */
    prct_area.x = prog_area.x;
    prct_area.y = prog_area.y + prog_area.height + GUI_PAD_SMALL;
    prct_area.width = prog_area.width;
    prct_area.height = 0;

    /* status */
    stat_area.x = prct_area.x;
    stat_area.y = prct_area.y + prct_area.height + GUI_PAD_SMALL;
    stat_area.width = prct_area.width;

    /**
    *** RENDER
    **/

    g_object_set(p->icon_renderer, "pixbuf", icon, "sensitive", TRUE,
                 NULL);
    gtr_cell_renderer_render(p->icon_renderer, window, widget, &icon_area,
                             flags);
    g_object_set(p->text_renderer, "text", p->title,
                 "scale", 1.0, FOREGROUND_COLOR_KEY, &text_color,
                 "ellipsize", PANGO_ELLIPSIZE_END, "weight",
                 PANGO_WEIGHT_BOLD, NULL);
    gtr_cell_renderer_render(p->text_renderer, window, widget, &name_area,
                             flags);
    g_object_set(p->text_renderer, "text", p->feed, "scale",
                 SMALL_SCALE, "weight", PANGO_WEIGHT_NORMAL, NULL);
    gtr_cell_renderer_render(p->text_renderer, window, widget, &prog_area,
                             flags);
    g_object_set(p->text_renderer, "text", p->published,
                 FOREGROUND_COLOR_KEY, &text_color, NULL);
    gtr_cell_renderer_render(p->text_renderer, window, widget, &stat_area,
                             flags);

    /* cleanup */
    g_object_unref(icon);
}

#endif
