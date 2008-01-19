/*
 *  Copyright (C) 2007 Mark Ellis
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gnome.h>
#include <string.h>
#include <synce_log.h>
#include <rapi.h>

#include "registry-list.h"
#include "misc.h"

static gint log_level = 2;

static GOptionEntry options[] =
  {
    { "debug", 'd', 0, G_OPTION_ARG_INT, &log_level, N_("Set debug level 1-5 (default 2)"), "level" },
    { NULL }
  };


static void
on_quit_button_clicked(GtkButton *button, gpointer user_data)
{
  gtk_widget_destroy(GTK_WIDGET(user_data));
}

static void
on_mainwindow_destroy(GtkWidget *widget, gpointer user_data)
{
  gtk_main_quit();
}

static void
on_refresh_button_clicked(GtkButton *button, gpointer user_data)
{
  GtkTreeView *registry_key_treeview = GTK_TREE_VIEW(user_data);
  setup_registry_key_tree_store(registry_key_treeview);
}
    
static gboolean
idle_populate_key_treeview(gpointer user_data)
{
  GtkTreeView *registry_key_treeview = GTK_TREE_VIEW(user_data);
  setup_registry_key_tree_store(registry_key_treeview);

  return FALSE;
}

int
main (int argc, char **argv)
{
  GtkWidget *mainwindow,
    *tool_button_quit, *tool_button_refresh,
    *registry_key_treeview, *registry_value_listview;

  GladeXML *gladefile = NULL;

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  GOptionContext *option_context = g_option_context_new (" - gnome registry tool for synCE");
  g_option_context_add_main_entries (option_context, options, GETTEXT_PACKAGE);
  gnome_program_init (PACKAGE, VERSION,
		      LIBGNOMEUI_MODULE,
		      argc, argv,
		      GNOME_PARAM_GOPTION_CONTEXT, option_context,
		      GNOME_PARAM_HUMAN_READABLE_NAME,_("Synce Registry Tool"),
		      GNOME_PROGRAM_STANDARD_PROPERTIES,
		      NULL);

  synce_log_set_level(log_level);

  gladefile = glade_xml_new(SYNCE_DATA "synce-registry-tool.glade",
			    "mainwindow", NULL); 

  mainwindow = glade_xml_get_widget(gladefile, "mainwindow");
  tool_button_quit = glade_xml_get_widget(gladefile, "tool_button_quit");
  tool_button_refresh = glade_xml_get_widget(gladefile, "tool_button_refresh");

  registry_key_treeview = glade_xml_get_widget(gladefile, "registry_key_treeview");
  registry_value_listview = glade_xml_get_widget(gladefile, "registry_value_listview");

  g_signal_connect(G_OBJECT(tool_button_quit), "clicked",
		   G_CALLBACK(on_quit_button_clicked),
		   mainwindow);

  g_signal_connect(G_OBJECT(tool_button_refresh), "clicked", 
		   G_CALLBACK(on_refresh_button_clicked),
		   registry_key_treeview);

  g_signal_connect(G_OBJECT(mainwindow),"destroy",
		   G_CALLBACK(on_mainwindow_destroy),
		   NULL);

  HRESULT hr;
  hr = CeRapiInit();

  if (FAILED(hr)) {
    g_error("%s: Unable to initialize RAPI: %s",
	    G_STRFUNC, synce_strerror(hr));
  }

  setup_registry_value_list_view(GTK_TREE_VIEW(registry_value_listview));
  setup_registry_key_tree_view(GTK_TREE_VIEW(registry_key_treeview), GTK_TREE_VIEW(registry_value_listview));

  gtk_widget_show_all(mainwindow);
  gtk_idle_add(idle_populate_key_treeview, registry_key_treeview);


  gtk_main ();

  CeRapiUninit();
  return(0);
}