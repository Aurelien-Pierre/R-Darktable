/*
    This file is part of darktable,
    Copyright (C) 2019-2021 darktable developers.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "common/darktable.h"
#include "common/debug.h"
#include "common/image_cache.h"
#include "common/variables.h"
#include "control/conf.h"
#include "control/control.h"
#include "develop/develop.h"
#include "gui/gtk.h"
#include "libs/lib.h"
#include "libs/lib_api.h"

DT_MODULE(1)


typedef struct dt_lib_imageinfo_t
{
  GtkWidget *tview;
} dt_lib_imageinfo_t;

const char *name(dt_lib_module_t *self)
{
  return _("image infos");
}

const char **views(dt_lib_module_t *self)
{
  /* we handle the hidden case here */
  const gboolean is_hidden =
    dt_conf_is_equal("plugins/darkroom/image_infos_position", "hidden");
  if(is_hidden)
  {
    static const char *vv[] = { NULL };
    return vv;
  }

  static const char *v[] = { "darkroom", NULL };
  return v;
}

uint32_t container(dt_lib_module_t *self)
{
  return DT_UI_CONTAINER_PANEL_LEFT_TOP;
}

int expandable(dt_lib_module_t *self)
{
  return 0;
}

int position()
{
  return 1;
}

void _lib_imageinfo_update_message(gpointer instance, dt_lib_module_t *self)
{
  dt_lib_imageinfo_t *d = (dt_lib_imageinfo_t *)self->data;

  // we grab the image
  const int imgid = darktable.develop->image_storage.id;
  if(imgid < 0) return;

  // we compute the info line (we reuse the function used in export to disk)
  char input_dir[512] = { 0 };
  gboolean from_cache = TRUE;
  dt_image_full_path(imgid,  input_dir,  sizeof(input_dir),  &from_cache, __FUNCTION__);

  dt_variables_params_t *vp;
  dt_variables_params_init(&vp);

  vp->filename = input_dir;
  vp->jobcode = "infos";
  vp->imgid = imgid;
  vp->sequence = 0;
  vp->escape_markup = TRUE;

  gchar *pattern = dt_conf_get_string("plugins/darkroom/image_infos_pattern");
  gchar *msg = dt_variables_expand(vp, pattern, TRUE);
  g_free(pattern);
  dt_variables_params_destroy(vp);

  // Support escaped characters (`"\n"` => `'\n'`).
  gchar *compressed = g_strcompress(msg);
  g_free(msg);

  // we change the label
  gtk_label_set_markup(GTK_LABEL(d->tview), compressed);
  g_free(compressed);
}

static void _lib_imageinfo_update_message2(gpointer instance, gpointer imgs, dt_lib_module_t *self)
{
  _lib_imageinfo_update_message(instance, self);
}

void _lib_imageinfo_update_message3(gpointer instance, int query_change, int changed_property, gpointer imgs,
                                    const int next, dt_lib_module_t *self)
{
  _lib_imageinfo_update_message(instance, self);
}

void gui_init(dt_lib_module_t *self)
{
  /* initialize ui widgets */
  dt_lib_imageinfo_t *d = (dt_lib_imageinfo_t *)g_malloc0(sizeof(dt_lib_imageinfo_t));
  self->data = (void *)d;

  self->widget = gtk_event_box_new();
  d->tview = gtk_label_new("");
  gtk_label_set_ellipsize(GTK_LABEL(d->tview), PANGO_ELLIPSIZE_MIDDLE);
  gtk_label_set_justify(GTK_LABEL(d->tview), GTK_JUSTIFY_CENTER);
  gtk_container_add(GTK_CONTAINER(self->widget), d->tview);
  gtk_widget_set_name(GTK_WIDGET(d->tview), "image-info");

  gtk_widget_show_all(self->widget);

  /* lets signup for develop image changed signals */
  DT_DEBUG_CONTROL_SIGNAL_CONNECT(darktable.signals, DT_SIGNAL_DEVELOP_IMAGE_CHANGED,
                            G_CALLBACK(_lib_imageinfo_update_message), self);

  /* signup for develop initialize to update info of current
     image in darkroom when enter */
  DT_DEBUG_CONTROL_SIGNAL_CONNECT(darktable.signals, DT_SIGNAL_DEVELOP_INITIALIZE,
                            G_CALLBACK(_lib_imageinfo_update_message), self);

  DT_DEBUG_CONTROL_SIGNAL_CONNECT(darktable.signals, DT_SIGNAL_IMAGE_INFO_CHANGED,
                                  G_CALLBACK(_lib_imageinfo_update_message2), self);
  DT_DEBUG_CONTROL_SIGNAL_CONNECT(darktable.signals, DT_SIGNAL_COLLECTION_CHANGED,
                                  G_CALLBACK(_lib_imageinfo_update_message3), self);
}

void gui_cleanup(dt_lib_module_t *self)
{
  DT_DEBUG_CONTROL_SIGNAL_DISCONNECT(darktable.signals, G_CALLBACK(_lib_imageinfo_update_message), self);
  DT_DEBUG_CONTROL_SIGNAL_DISCONNECT(darktable.signals, G_CALLBACK(_lib_imageinfo_update_message2), self);
  DT_DEBUG_CONTROL_SIGNAL_DISCONNECT(darktable.signals, G_CALLBACK(_lib_imageinfo_update_message3), self);

  g_free(self->data);
  self->data = NULL;
}



// clang-format off
// modelines: These editor modelines have been set for all relevant files by tools/update_modelines.py
// vim: shiftwidth=2 expandtab tabstop=2 cindent
// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-spaces modified;
// clang-format on
