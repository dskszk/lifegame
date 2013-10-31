/*
 * lifegame.h - CONWAY'S GAME OF LIFE
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

#ifndef LIFE_GAME_H
#define LIFE_GAME_H

#include <gtk/gtk.h>

#define LIFE_TYPE_GAME	(life_game_get_type ())
#define LIFE_IS_GAME(obj)	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), LIFE_TYPE_GAME))
#define LIFE_GAME(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), LIFE_TYPE_GAME, LifeGame))
#define LIFE_IS_GAME_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), LIFE_TYPE_GAME))
#define LIFE_GAME_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), LIFE_TYPE_GAME, LifeGameClass))

typedef struct _LifeGame LifeGame;
typedef struct _LifeGameClass LifeGameClass;
typedef struct _LifeGamePrivate LifeGamePrivate;

struct _LifeGame {
    GtkWindow parent;
    LifeGamePrivate *priv;
};

struct _LifeGameClass {
    GtkWindowClass parent;
};

LifeGame *life_game_new ();
GType life_game_get_type ();

#endif /* LIFE_GAME_H */
