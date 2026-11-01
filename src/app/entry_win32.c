/* Windows entry point: a GUI program (no console window) whose command line arrives as UTF-16.
   Converts the arguments to UTF-8 and runs gc_main(). */
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>

int gc_main(int argc, char **argv);

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, LPWSTR cmd, int show)
{
    (void)inst;
    (void)prev;
    (void)cmd;
    (void)show;
    /* Messages for --help and --version go to the console that started us, if there is one. */
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        FILE *f;
        (void)freopen_s(&f, "CONOUT$", "w", stdout);
        (void)freopen_s(&f, "CONOUT$", "w", stderr);
    }
    int argc = 0;
    wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    char **argv = calloc((size_t)argc + 1, sizeof *argv);
    for (int i = 0; i < argc; i++) {
        int n = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL);
        argv[i] = malloc((size_t)(n > 0 ? n : 1));
        if (n > 0) WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], n, NULL, NULL);
        else argv[i][0] = 0;
    }
    LocalFree(wargv);
    int r = gc_main(argc, argv);
    for (int i = 0; i < argc; i++) free(argv[i]);
    free(argv);
    return r;
}
