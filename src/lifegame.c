/*
 * lifegame.c - CONWAY'S GAME OF LIFE
 * Copyright (C) 2013 Daisuke Suzuki
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "lifegame.h"

#define LIFE_GAME_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), LIFE_TYPE_GAME, LifeGamePrivate))

struct _LifeGamePrivate {
    GtkWidget *area;
    gboolean *cell_data;
    gint max;
    gint *count;
    guint gen;
    gint speed;
    gboolean timeout_state;
    GtkWidget *label;
    GtkWidget *grid;
    GtkWidget *reset;
    GtkWidget *random;
    GtkWidget *auto_step;
    GtkWidget *next;
    GtkWidget *scale;
};

typedef struct _LifeGamePrivate LifeGamePrivate;

/* Forwarding declaration */

static void life_game_on_area_draw (GtkWidget *, cairo_t *, gpointer);

static void life_game_next_step (LifeGame *);

static void life_game_cell_initialize (LifeGame *);

static void life_game_cell_randomize (LifeGame *);

static void life_game_on_random_clicked (GtkButton *, gpointer);

static void life_game_on_reset_clicked (GtkButton *, gpointer);

static void life_game_on_auto_step_toggled (GtkToggleButton *, gpointer);

static void life_game_on_next_clicked (GtkButton *, gpointer);

static gboolean life_game_on_area_event (GtkWidget *, GdkEvent *,
        gpointer);

static gboolean life_game_timeout (gpointer);

/* Boilerplate code */

    static void
life_game_class_init (LifeGameClass *klass)
{
    g_type_class_add_private (klass, sizeof (LifeGamePrivate));
}

    static void
life_game_init (LifeGame *self)
{
    LifeGamePrivate *priv;
    self->priv = priv = LIFE_GAME_GET_PRIVATE (self);
    /* Initialize everything */
    priv->area = gtk_drawing_area_new ();
    gtk_widget_set_size_request (priv->area, 510, 510);
    gtk_window_set_default_size (GTK_WINDOW (self), 510, 610);
    gtk_window_set_resizable (GTK_WINDOW (self), FALSE);
    priv->max = 102;
    priv->cell_data = g_new0 (gboolean, priv->max * priv->max);
    priv->count = g_new0 (gint, priv->max * priv->max);
    priv->timeout_state = TRUE;
    priv->reset = gtk_button_new_with_label ("Reset");
    priv->random = gtk_button_new_with_label ("Random");
    priv->auto_step = gtk_toggle_button_new_with_label ("Auto");
    priv->next = gtk_button_new_with_label ("Next");
    priv->label = gtk_label_new ("Generation: 0");
    priv->scale =
        gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 100, 10);
    gtk_range_set_value (GTK_RANGE (priv->scale), 50.0);
    /* Connecting signals */
    g_signal_connect (G_OBJECT (priv->area), "draw",
            G_CALLBACK (life_game_on_area_draw), self);
    g_signal_connect (G_OBJECT (self), "destroy",
            G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (G_OBJECT (priv->random), "clicked",
            G_CALLBACK (life_game_on_random_clicked), self);
    g_signal_connect (G_OBJECT (priv->reset), "clicked",
            G_CALLBACK (life_game_on_reset_clicked), self);
    g_signal_connect (G_OBJECT (priv->auto_step), "toggled",
            G_CALLBACK (life_game_on_auto_step_toggled), self);
    g_signal_connect (G_OBJECT (priv->next), "clicked",
            G_CALLBACK (life_game_on_next_clicked), self);
    gtk_widget_add_events (priv->area, GDK_BUTTON_PRESS_MASK);
    g_signal_connect (G_OBJECT (priv->area), "event",
            G_CALLBACK (life_game_on_area_event), self);
    /* Packing */
    priv->grid = gtk_grid_new ();
    gtk_grid_attach (GTK_GRID (priv->grid), priv->area, 0, 0, 4, 1);
    gtk_grid_attach (GTK_GRID (priv->grid), priv->scale, 0, 1, 4, 1);
    gtk_grid_attach (GTK_GRID (priv->grid), priv->reset, 0, 2, 1, 1);
    gtk_grid_attach (GTK_GRID (priv->grid), priv->auto_step, 1, 2, 1, 1);
    gtk_grid_attach (GTK_GRID (priv->grid), priv->next, 2, 2, 1, 1);
    gtk_grid_attach (GTK_GRID (priv->grid), priv->random, 3, 2, 1, 1);
    gtk_grid_attach (GTK_GRID (priv->grid), priv->label, 0, 3, 4, 1);
    gtk_container_add (GTK_CONTAINER (self), priv->grid);
    life_game_cell_initialize (self);
    gtk_widget_show_all (GTK_WIDGET (self));
}

G_DEFINE_TYPE (LifeGame, life_game, GTK_TYPE_WINDOW)

    LifeGame
*life_game_new ()
{
    return LIFE_GAME (g_object_new (LIFE_TYPE_GAME, NULL));
}

/* Callback functions */

    static void
life_game_on_random_clicked (GtkButton *button, gpointer data)
{
    life_game_cell_randomize (data);
}

    static void
life_game_on_reset_clicked (GtkButton *button, gpointer data)
{
    GtkWidget *rand = (LIFE_GAME (data))->priv->random;
    gtk_widget_set_sensitive (rand, TRUE);
    life_game_cell_initialize (data);
}

    static void
life_game_on_auto_step_toggled (GtkToggleButton *tbutton, gpointer data)
{
    LifeGame *self = LIFE_GAME (data);
    gdouble speed;
    if (gtk_toggle_button_get_active (tbutton)) {
        gtk_widget_set_sensitive (self->priv->next, FALSE);
        gtk_widget_set_sensitive (self->priv->scale, FALSE);
        gtk_widget_set_sensitive (self->priv->random, FALSE);
        gtk_widget_set_sensitive (self->priv->reset, FALSE);
        self->priv->timeout_state = TRUE;
        speed = gtk_range_get_value (GTK_RANGE (self->priv->scale));
        g_timeout_add (505 - 5 * (gint) speed,
                (GSourceFunc) life_game_timeout, self);
    } else {
        self->priv->timeout_state = FALSE;
        gtk_widget_set_sensitive (self->priv->scale, TRUE);
        gtk_widget_set_sensitive (self->priv->next, TRUE);
        gtk_widget_set_sensitive (self->priv->reset, TRUE);
    }
}

    static void
life_game_on_next_clicked (GtkButton *button, gpointer data)
{
    LifeGame *self = LIFE_GAME (data);
    if (self->priv->gen == 0) {
        gtk_widget_set_sensitive(self->priv->random, FALSE);
    }
    life_game_next_step (data);
}

    static void
life_game_on_area_draw (GtkWidget *area, cairo_t *cr, gpointer data)
{
    gint i, j;
    gchar str[16];
    LifeGame *self = LIFE_GAME (data);
    gint max = self->priv->max;
    gboolean *cell_data = self->priv->cell_data;
    for (i = 1; i < max - 1; i++) {
        for (j = 1; j < max - 1; j++) {
            if (cell_data[i * max + j]) {
                cairo_rectangle (cr, 5 * (j - 1), 5 * (i - 1), 5, 5);
                cairo_fill (cr);
            }
        }
    }
    g_sprintf (str, "Generation: %u", self->priv->gen);
    gtk_label_set_text (GTK_LABEL (self->priv->label), str);
}

    static gboolean
life_game_on_area_event (GtkWidget *area, GdkEvent *event, gpointer data)
{
    LifeGame *self = LIFE_GAME (data);
    gint *cell_data = self->priv->cell_data;
    gint max = self->priv->max;
    gint i, j;
    if (event->type == GDK_BUTTON_PRESS) {
        if (self->priv->gen == 0) {
            i = event->button.y / 5 + 1;
            j = event->button.x / 5 + 1;
            cell_data[max * i + j] = !cell_data[max * i + j];
            gtk_widget_queue_draw_area (self->priv->area, 0, 0, 500, 500);
        }
    }
    return TRUE;
}

/* Utils (private) */

    static void
life_game_cell_initialize (LifeGame *self)
{
    gint i;
    gint max = self->priv->max;
    for (i = 0; i < max * max; i++) {
        self->priv->cell_data[i] = FALSE;
    }
    self->priv->gen = 0;
    gtk_widget_queue_draw_area (self->priv->area, 0, 0, 500, 500);
}

    static void
life_game_cell_randomize (LifeGame *self)
{
    gint i, ri;
    GRand *ran = g_rand_new ();
    gint max = self->priv->max;
    gboolean *cell_data = self->priv->cell_data;
    gint rmax = g_rand_int_range (ran, 0, max * max / 8);
    for (i = 0; i < max * max; i++) {
        cell_data[i] = FALSE;
    }
    for (i = 0; i < rmax; i++) {
        ri = g_rand_int_range (ran, 0, max * max / 4);
        if (ri >= max / 2) {
            ri = ri % (max / 2) + (ri / max * 2) * max;
        }
        cell_data[ri] = !cell_data[ri];
    }
    life_game_next_step (self);
    self->priv->gen = 0;
    gtk_widget_queue_draw_area (self->priv->area, 0, 0, 500, 500);
}

    static gboolean
life_game_timeout (gpointer data)
{
    LifeGame *self = LIFE_GAME (data);
    gboolean timeout_state = self->priv->timeout_state;
    if (timeout_state) {
        life_game_next_step (data);
    }
    return timeout_state;
}

    static void
life_game_next_step (LifeGame *self)
{
    gint i, j;
    gboolean *cell_data = self->priv->cell_data;
    gint *count = self->priv->count;
    gint max = self->priv->max;
    for (i = 0; i < max; i++) {
        for (j = 0; j < max; j++) {
            count[i * max + j] = 0;
        }
    }
    for (i = 1; i < max - 1; i++) {
        for (j = 1; j < max - 1; j++) {
            if (cell_data[i * max + j]) {
                count[(i - 1) * max + j - 1]++;
                count[(i - 1) * max + j]++;
                count[(i - 1) * max + j + 1]++;
                count[i * max + j - 1]++;
                count[i * max + j + 1]++;
                count[(i + 1) * max + j - 1]++;
                count[(i + 1) * max + j]++;
                count[(i + 1) * max + j + 1]++;
            }
        }
    }
    for (i = 1; i < max - 1; i++) {
        count[i * max + 1] += count[(i + 1) * max - 1];
        count[(i + 1) * max - 2] += count[i * max];
        count[max + i] += count[(max - 1) * max + i];
        count[(max - 2) * max + i] += count[i];
    }
    count[max + 1] += count[max * max - 1];
    count[max * (max - 1) - 2] += count[0];
    count[2 * max - 2] += count[max * (max - 1)];
    count[max * (max - 1) + 1] += count[max - 1];
    for (i = 1; i < max - 1; i++) {
        for (j = 1; j < max - 1; j++) {
            if (cell_data[i * max + j]) {
                if (count[i * max + j] != 2 && count[i * max + j] != 3)
                    cell_data[i * max + j] = FALSE;
            } else if (count[i * max + j] == 3)
                cell_data[i * max + j] = TRUE;
        }
    }
    self->priv->gen++;
    gtk_widget_queue_draw_area (self->priv->area, 0, 0, 500, 500);
}
