/* Linux entry point. */
#include <gtk/gtk.h>

int gc_main(int argc, char **argv);

int main(int argc, char **argv)
{
    /* The window's app id / WM_CLASS and icon match the installed glosscode.desktop and the
       hicolor "glosscode" icon, so the desktop shows the GlossCode icon and groups the window. */
    g_set_prgname("glosscode");
    gtk_window_set_default_icon_name("glosscode");
    return gc_main(argc, argv);
}
