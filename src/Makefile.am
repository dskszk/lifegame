bin_PROGRAMS = lifegame
lifegame_SOURCES = lifegame.c lifegame.h main.c
lifegame_CFLAGS = -Wall -Wextra -fstack-protector-strong --param=ssp-buffer-size=4 -D_FORTIFY_SOURCE=2 -pipe $(GTK_CFLAGS)
lifegame_LDADD = $(GTK_LIBS)

