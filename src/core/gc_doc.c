/* Reading and writing G-code text: line endings, ';' end-of-block marks, and splitting a block into
   words. Mirrors splitBlocks() and tokenize() in the mockup. */
#include "glosscode.h"
#include "gc_util.h"

#include <stdlib.h>
#include <string.h>

static int is_word_char(int c)
{
    return gc_is_alpha(c) || gc_is_digit(c) || c == '_';
}

static const char *const KEYWORDS[] = { "CALL", "RTS", "GOTO" };

char **gc_tokenize(const char *s, size_t len, int *ntok)
{
    char **out = NULL;
    int n = 0, cap = 0;
    size_t i = 0;
    /* A "/" at the start of a block is block delete (/1 to /9 on Fanuc-style controls): its own first word. */
    while (i < len && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r' || s[i] == '\f' || s[i] == '\v')) i++;
    if (i < len && s[i] == '/') {
        size_t end = i + 1;
        if (end < len && gc_is_digit((unsigned char)s[end])) end++;
        out = gc_xrealloc(out, 8 * sizeof *out);
        cap = 8;
        out[n++] = gc_strndup(s + i, end - i);
        i = end;
    } else
        i = 0;
    while (i < len) {
        size_t start = i, end = 0;
        int comment = 0;
        if (s[i] == '(') {
            end = i + 1;
            while (end < len && s[end] != ')') end++;
            if (end < len) end++; /* the closing ')' */
            comment = 1;
        } else if (gc_is_alpha((unsigned char)s[i])) {
            if (i == 0 || !is_word_char((unsigned char)s[i - 1])) {
                for (size_t k = 0; k < sizeof KEYWORDS / sizeof *KEYWORDS && !end; k++) {
                    size_t kl = strlen(KEYWORDS[k]), j;
                    if (i + kl > len) continue;
                    for (j = 0; j < kl; j++)
                        if (gc_to_upper((unsigned char)s[i + j]) != KEYWORDS[k][j]) break;
                    if (j == kl && (i + kl == len || !is_word_char((unsigned char)s[i + kl]))) end = i + kl;
                }
            }
            if (!end) {
                end = i + 1;
                if (end < len && (s[end] == '-' || s[end] == '+')) end++;
                while (end < len && (gc_is_digit((unsigned char)s[end]) || s[end] == '.')) end++;
            }
        } else {
            i++;
            continue;
        }
        char *t = gc_strndup(s + start, end - start);
        if (!comment)
            for (char *p = t; *p; p++) *p = (char)gc_to_upper((unsigned char)*p);
        if (n == cap) {
            cap = cap ? cap * 2 : 8;
            out = gc_xrealloc(out, (size_t)cap * sizeof *out);
        }
        out[n++] = t;
        i = end;
    }
    *ntok = n;
    return out;
}

void gc_tokens_free(char **tok, int ntok)
{
    for (int i = 0; i < ntok; i++) free(tok[i]);
    free(tok);
}

gc_doc *gc_doc_new(void)
{
    gc_doc *d = gc_xcalloc(1, sizeof *d);
    strcpy(d->eol, "\r\n");
    d->final_eol = 1;
    return d;
}

static void add_line_owned(gc_doc *doc, char **tok, int ntok, char *raw)
{
    if ((doc->nlines & (doc->nlines - 1)) == 0) {
        size_t cap = doc->nlines ? (size_t)doc->nlines * 2 : 16;
        doc->lines = gc_xrealloc(doc->lines, cap * sizeof *doc->lines);
    }
    gc_line *l = &doc->lines[doc->nlines++];
    l->tok = tok;
    l->ntok = ntok;
    l->raw = raw;
}

void gc_doc_add_line(gc_doc *doc, const char *const *tok, int ntok, const char *raw)
{
    char **copy = ntok ? gc_xmalloc((size_t)ntok * sizeof *copy) : NULL;
    gc_buf b;
    gc_buf_init(&b);
    for (int i = 0; i < ntok; i++) {
        copy[i] = gc_strdup(tok[i]);
        if (i) gc_buf_putc(&b, ' ');
        gc_buf_puts(&b, tok[i]);
    }
    char *r = raw ? gc_strdup(raw) : gc_buf_take(&b);
    gc_buf_free(&b);
    add_line_owned(doc, copy, ntok, r);
}

static int is_space(int c)
{
    return c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r' || c == '\n';
}

static int blank(const char *s, size_t n)
{
    for (size_t i = 0; i < n; i++)
        if (!is_space((unsigned char)s[i])) return 0;
    return 1;
}

/* One source line: split on ';' outside comments. Returns 1 if the line had a ';'. */
static int add_source_line(gc_doc *doc, const char *s, size_t len)
{
    size_t start = 0;
    int depth = 0, hit = 0, blocks = 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (c == '(') depth++;
        else if (c == ')') depth = depth > 0 ? depth - 1 : 0;
        if (c == ';' && !depth) {
            int nt;
            char **t = gc_tokenize(s + start, i - start, &nt);
            add_line_owned(doc, t, nt, gc_strndup(s + start, i - start));
            blocks++;
            start = i + 1;
            hit = 1;
        }
    }
    if (!blank(s + start, len - start) || !blocks) {
        int nt;
        char **t = gc_tokenize(s + start, len - start, &nt);
        add_line_owned(doc, t, nt, gc_strndup(s + start, len - start));
    }
    return hit;
}

gc_doc *gc_doc_parse(const char *text, size_t len)
{
    gc_doc *doc = gc_doc_new();
    int found_eol = 0;
    size_t start = 0, i = 0;
    doc->final_eol = 0;
    if (len >= 3 && (unsigned char)text[0] == 0xEF && (unsigned char)text[1] == 0xBB && (unsigned char)text[2] == 0xBF)
        start = i = 3;
    while (i < len) {
        if (text[i] == '\r' || text[i] == '\n') {
            size_t eol = (text[i] == '\r' && i + 1 < len && text[i + 1] == '\n') ? 2 : 1;
            if (!found_eol) {
                memcpy(doc->eol, text + i, eol);
                doc->eol[eol] = '\0';
                found_eol = 1;
            }
            if (add_source_line(doc, text + start, i - start)) doc->eob = 1;
            i += eol;
            start = i;
            if (i == len) doc->final_eol = 1;
            continue;
        }
        i++;
    }
    if (start < len || doc->nlines == 0)
        if (add_source_line(doc, text + start, len - start)) doc->eob = 1;
    if (!found_eol) strcpy(doc->eol, "\r\n");
    return doc;
}

void gc_doc_free(gc_doc *doc)
{
    if (!doc) return;
    for (int i = 0; i < doc->nlines; i++) {
        gc_tokens_free(doc->lines[i].tok, doc->lines[i].ntok);
        free(doc->lines[i].raw);
    }
    free(doc->lines);
    free(doc);
}

static int raw_matches(const gc_line *l)
{
    if (!l->raw) return 0;
    int n;
    char **t = gc_tokenize(l->raw, strlen(l->raw), &n);
    int same = n == l->ntok;
    for (int i = 0; same && i < n; i++) same = strcmp(t[i], l->tok[i]) == 0;
    gc_tokens_free(t, n);
    return same;
}

char *gc_doc_serialize(const gc_doc *doc, int eob, size_t *len)
{
    gc_buf b;
    gc_buf_init(&b);
    for (int i = 0; i < doc->nlines; i++) {
        const gc_line *l = &doc->lines[i];
        if (raw_matches(l)) {
            /* Unchanged: keep the text as written (spacing, '%' and macro text the word list
               doesn't hold), less trailing spaces before a ';'. */
            size_t n = strlen(l->raw);
            if (eob)
                while (n && (l->raw[n - 1] == ' ' || l->raw[n - 1] == '\t')) n--;
            gc_buf_putn(&b, l->raw, n);
        } else {
            for (int j = 0; j < l->ntok; j++) {
                if (j && l->tok[j - 1][0] != '/') gc_buf_putc(&b, ' '); /* "/N30", not "/ N30" */
                gc_buf_puts(&b, l->tok[j]);
            }
        }
        /* ';' ends every block that holds a word; blank lines and '%' stay as they are. */
        if (eob && l->ntok) gc_buf_putc(&b, ';');
        if (i < doc->nlines - 1 || doc->final_eol) gc_buf_puts(&b, doc->eol);
    }
    if (len) *len = b.len;
    return gc_buf_take(&b);
}

/* ---------- Text encoding ---------- */

int gc_is_utf8(const char *s, size_t len)
{
    const unsigned char *p = (const unsigned char *)s, *end = p + len;
    while (p < end) {
        unsigned char c = *p;
        size_t n = c < 0x80 ? 1 : (c >> 5) == 6 ? 2 : (c >> 4) == 14 ? 3 : (c >> 3) == 30 ? 4 : 0;
        if (!n || (size_t)(end - p) < n) return 0;
        for (size_t i = 1; i < n; i++)
            if ((p[i] & 0xC0) != 0x80) return 0;
        p += n;
    }
    return 1;
}

char *gc_latin1_to_utf8(const char *s, size_t len, size_t *outlen)
{
    gc_buf b;
    gc_buf_init(&b);
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c < 0x80) {
            gc_buf_putc(&b, (char)c);
        } else {
            char t[2] = { (char)(0xC0 | (c >> 6)), (char)(0x80 | (c & 0x3F)) };
            gc_buf_putn(&b, t, 2);
        }
    }
    if (outlen) *outlen = b.len;
    return gc_buf_take(&b);
}

char *gc_utf8_to_latin1(const char *s, size_t len, size_t *outlen)
{
    gc_buf b;
    gc_buf_init(&b);
    const unsigned char *p = (const unsigned char *)s, *end = p + len;
    while (p < end) {
        if (*p < 0x80) {
            gc_buf_putc(&b, (char)*p++);
        } else if ((*p >> 5) == 6 && p + 1 < end) {
            unsigned cp = ((unsigned)(p[0] & 0x1F) << 6) | (p[1] & 0x3F);
            gc_buf_putc(&b, cp < 0x100 ? (char)cp : '?');
            p += 2;
        } else {
            /* Characters Latin-1 can't hold. */
            size_t n = (*p >> 4) == 14 ? 3 : (*p >> 3) == 30 ? 4 : 1;
            gc_buf_putc(&b, '?');
            p += n;
        }
    }
    if (outlen) *outlen = b.len;
    return gc_buf_take(&b);
}
