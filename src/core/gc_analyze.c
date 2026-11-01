/* gc_analyze(): translation and checks for one document on one machine. */
#include "gc_internal.h"

#include <stdlib.h>
#include <string.h>

gc_analysis *gc_analyze(const gc_data *data, const gc_json *machine, const gc_doc *doc, int style)
{
    gc_ctx c;
    gc_ctx_init(&c, data, machine, doc, style);
    gc_analysis *a = gc_xcalloc(1, sizeof *a);
    int n = doc->nlines;
    a->nlines = n;
    a->cells = gc_xcalloc((size_t)(n ? n : 1), sizeof *a->cells);
    a->ncells = gc_xcalloc((size_t)(n ? n : 1), sizeof *a->ncells);
    a->modes = gc_xcalloc((size_t)(n ? n : 1), sizeof *a->modes);
    a->kind = gc_xcalloc((size_t)(n ? n : 1), sizeof *a->kind);
    gc_translate(&c, a);
    gc_check(&c, a);
    a->dialect_label = gc_strdup(gc_json_get_str(c.dialect, "label") ? gc_json_get_str(c.dialect, "label") : "");
    a->dialect_verified = gc_json_truthy(gc_json_get(c.dialect, "verified"));
    gc_ctx_free(&c);
    return a;
}

void gc_analysis_free(gc_analysis *a)
{
    if (!a) return;
    for (int i = 0; i < a->nlines; i++) {
        for (int k = 0; k < a->ncells[i]; k++) {
            free(a->cells[i][k].g);
            free(a->cells[i][k].e);
            free(a->cells[i][k].u);
            free(a->cells[i][k].d);
        }
        free(a->cells[i]);
        free(a->kind[i]);
    }
    for (int i = 0; i < a->nproblems; i++) {
        free(a->problems[i].id);
        free(a->problems[i].sev);
        free(a->problems[i].title);
        free(a->problems[i].msg);
        free(a->problems[i].why);
    }
    free(a->cells);
    free(a->ncells);
    free(a->modes);
    free(a->kind);
    free(a->problems);
    free(a->dialect_label);
    free(a);
}
