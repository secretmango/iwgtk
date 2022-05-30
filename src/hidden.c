/*
 *  Copyright 2020-2022 Jesse Lentz
 *
 *  This file is part of iwgtk.
 *
 *  iwgtk is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  iwgtk is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with iwgtk.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "iwgtk.h"

static const CallbackMessages connect_hidden_messages = {
    "Found hidden network",
    "Failed to connect to hidden network",
    NULL
};

void hidden_ssid_dialog(Station *station) {
    HiddenNetworkDialog *dialog;
    GtkWidget *table, *buttons;

    dialog = g_malloc(sizeof(HiddenNetworkDialog));
    dialog->station = station;

    dialog->window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog->window), "Connect to Hidden Network");

    dialog->ssid = gtk_entry_new();

    table = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(dialog->window), table);

    buttons = dialog_buttons(dialog, (SubmitCallback) hidden_ssid_submit, dialog->window);

    gtk_grid_attach(GTK_GRID(table), gtk_label_new("SSID: "), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), dialog->ssid,            1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), buttons,                 1, 1, 1, 1);

    grid_column_set_alignment(table, 0, GTK_ALIGN_END);
    grid_column_set_alignment(table, 1, GTK_ALIGN_START);

    g_signal_connect_swapped(dialog->window, "destroy", G_CALLBACK(g_free), dialog);
    gtk_widget_show(dialog->window);
}

void hidden_ssid_submit(HiddenNetworkDialog *dialog) {
    const gchar *ssid;

    ssid = gtk_editable_get_text(GTK_EDITABLE(dialog->ssid));
    if (*ssid == '\0') {
	return;
    }

    g_dbus_proxy_call(
	dialog->station->proxy,
	"ConnectHiddenNetwork",
	g_variant_new("(s)", ssid),
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) validation_callback,
	(gpointer) &connect_hidden_messages);

    gtk_window_destroy(GTK_WINDOW(dialog->window));
}

void station_add_hidden_network(Station *station, const gchar *address, const gchar *type, gint16 signal_strength, int index) {
    GtkWidget *status_icon;
    GtkWidget *address_label;
    GtkWidget *security_label;
    GtkWidget *connect_button;

    status_icon = gtk_image_new();
    gtk_widget_set_tooltip_text(status_icon, "Hidden network");

    {
	const gchar *icon_name;

	icon_name = station_icons[get_signal_level(signal_strength)];
	symbolic_icon_set_image(icon_name, &color_gray, status_icon);
    }

    address_label = gtk_label_new(address);
    security_label = gtk_label_new(get_security_type(type));
    connect_button = gtk_button_new_with_label("Connect");

    gtk_widget_set_tooltip_text(address_label, "MAC address");
    gtk_widget_set_tooltip_text(security_label, "Network security");
    gtk_widget_set_tooltip_text(connect_button, "Connect to hidden network");

    g_signal_connect_swapped(connect_button, "clicked", G_CALLBACK(hidden_ssid_dialog), (gpointer) station);

    gtk_grid_attach(GTK_GRID(station->network_table), status_icon,    0, index, 1, 1);
    gtk_grid_attach(GTK_GRID(station->network_table), address_label,  1, index, 1, 1);
    gtk_grid_attach(GTK_GRID(station->network_table), security_label, 2, index, 1, 1);
    gtk_grid_attach(GTK_GRID(station->network_table), connect_button, 3, index, 1, 1);

    gtk_widget_set_halign(status_icon,    GTK_ALIGN_START);
    gtk_widget_set_halign(address_label,  GTK_ALIGN_START);
    gtk_widget_set_halign(security_label, GTK_ALIGN_START);
    gtk_widget_set_halign(connect_button, GTK_ALIGN_FILL);

    gtk_widget_set_hexpand(address_label, TRUE);
}
