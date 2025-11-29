// editor.c
// Minimal Line-Based Text Editor Buffer
// FAST University Karachi - assignment implementation
//
// Compile: gcc -std=c99 -Wall -Wextra -O2 editor.c -o editor
//
// Usage: run ./editor and use commands described below.
//
// Explanation: This program stores each line in exactly-sized heap memory (strlen+1).
// The array of line pointers (char**) is dynamically grown/shrunk with realloc.
// This reduces memory waste compared to a big static 2D array (e.g., char lines[10000][1024])
// because we only allocate memory for lines that actually exist and only as long as they are.
//
// NOTE: This program uses fgetc-based readLine() to safely handle arbitrarily long lines.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define INITIAL_CAPACITY 4   // small default capacity to demonstrate growth
#define GROW_FACTOR 2

// ---------- Utility for safe allocation ----------
void *xmalloc(size_t s) {
    void *p = malloc(s);
    if (!p) {
        fprintf(stderr, "Memory allocation failed (malloc %zu bytes): %s\n", s, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return p;
}

void *xrealloc(void *ptr, size_t s) {
    void *p = realloc(ptr, s);
    if (!p) {
        fprintf(stderr, "Memory allocation failed (realloc %zu bytes): %s\n", s, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return p;
}

// ---------- Dynamic buffer structure ----------
typedef struct {
    char **lines;   // dynamic array of pointers to heap-allocated C-strings
    int size;       // number of stored lines
    int capacity;   // allocated capacity of lines[]
} LineBuffer;

// Initialize buffer
void initBuffer(LineBuffer *b) {
    b->size = 0;
    b->capacity = INITIAL_CAPACITY;
    b->lines = (char **) xmalloc(sizeof(char *) * b->capacity);
}

// Free all lines and the array
void freeAll(LineBuffer *b) {
    if (!b) return;
    for (int i = 0; i < b->size; ++i) {
        free(b->lines[i]);
    }
    free(b->lines);
    b->lines = NULL;
    b->size = 0;
    b->capacity = 0;
}

// Ensure capacity >= minCapacity
void ensureCapacity(LineBuffer *b, int minCapacity) {
    if (b->capacity >= minCapacity) return;
    int newCap = b->capacity;
    while (newCap < minCapacity) newCap *= GROW_FACTOR;
    b->lines = (char **) xrealloc(b->lines, sizeof(char *) * newCap);
    b->capacity = newCap;
}

// shrinkToFit: reduce allocation to match size exactly
void shrinkToFit(LineBuffer *b) {
    if (b->capacity == b->size) return; // nothing to do
    if (b->size == 0) {
        // free and reinit a small array
        free(b->lines);
        b->capacity = INITIAL_CAPACITY;
        b->lines = (char **) xmalloc(sizeof(char *) * b->capacity);
        return;
    }
    b->lines = (char **) xrealloc(b->lines, sizeof(char *) * b->size);
    b->capacity = b->size;
}

// Read a line of arbitrary length from FILE * (stdin or file).
// Returns a malloc'd null-terminated string (without the newline character).
// Returns NULL on EOF (no characters read).
char *readLineFILE(FILE *f) {
    size_t bufSize = 128;
    size_t len = 0;
    char *buf = (char *) xmalloc(bufSize);
    int c;

    while ((c = fgetc(f)) != EOF) {
        if (c == '\r') continue; // ignore carriage returns
        if (c == '\n') break;
        if (len + 1 >= bufSize) {
            bufSize *= 2;
            buf = (char *) xrealloc(buf, bufSize);
        }
        buf[len++] = (char) c;
    }

    if (len == 0 && c == EOF) {
        free(buf);
        return NULL; // EOF and no data read
    }

    buf[len] = '\0';
    // shrink to fit exact length + 1
    buf = (char *) xrealloc(buf, len + 1);
    return buf;
}

// Convenience wrapper to read a line from stdin with prompt (returns malloc'd string)
char *readLineStdin(const char *prompt) {
    if (prompt) {
        printf("%s", prompt);
        fflush(stdout);
    }
    return readLineFILE(stdin);
}

// Insert line text at index (0-based). If index == size, append.
// We shift pointers to make room (not the string contents).
void insertLine(LineBuffer *b, int index, const char *text) {
    if (index < 0 || index > b->size) {
        fprintf(stderr, "Insert index out of range (0..%d). Given: %d\n", b->size, index);
        return;
    }

    // allocate exact-sized memory for the string
    size_t needed = strlen(text) + 1;
    char *copy = (char *) xmalloc(needed);
    memcpy(copy, text, needed); // including '\0'

    // grow array if required
    ensureCapacity(b, b->size + 1);

    // shift pointer region to make room
    if (index < b->size) {
        memmove(&b->lines[index + 1], &b->lines[index], sizeof(char *) * (b->size - index));
    }

    b->lines[index] = copy;
    b->size++;
}

// Delete line at index (0-based). Frees the string and compacts pointer array.
void deleteLine(LineBuffer *b, int index) {
    if (index < 0 || index >= b->size) {
        fprintf(stderr, "Delete index out of range (0..%d). Given: %d\n", b->size - 1, index);
        return;
    }

    free(b->lines[index]);

    if (index < b->size - 1) {
        memmove(&b->lines[index], &b->lines[index + 1], sizeof(char *) * (b->size - index - 1));
    }
    b->size--;
    // optional: set last pointer to NULL for safety
    if (b->size == 0 && b->capacity > INITIAL_CAPACITY) {
        // keep capacity but you may want to shrink later with shrinkToFit()
    }
}

// Print all lines with 0-based index
void printAllLines(LineBuffer *b) {
    printf("---- Buffer (%d lines) ----\n", b->size);
    for (int i = 0; i < b->size; ++i) {
        printf("%d: %s\n", i, b->lines[i]);
    }
    printf("---------------------------\n");
}

// Save buffer to a file (one line per file line)
void saveToFile(LineBuffer *b, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Failed to open file '%s' for writing: %s\n", filename, strerror(errno));
        return;
    }
    for (int i = 0; i < b->size; ++i) {
        fprintf(f, "%s\n", b->lines[i]);
    }
    fclose(f);
    printf("Saved %d lines to '%s'\n", b->size, filename);
}

// Load buffer from a file; replaces current buffer contents (freeing previous).
void loadFromFile(LineBuffer *b, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Failed to open file '%s' for reading: %s\n", filename, strerror(errno));
        return;
    }

    // free existing content
    for (int i = 0; i < b->size; ++i) free(b->lines[i]);
    b->size = 0;

    // read lines and append
    char *line;
    while ((line = readLineFILE(f)) != NULL) {
        ensureCapacity(b, b->size + 1);
        b->lines[b->size++] = line;
    }

    fclose(f);
    printf("Loaded %d lines from '%s'\n", b->size, filename);
}

// ---------- Simple interactive command loop ----------
// Commands:
// ADD <text>            : append a new line (text begins after space)
// INSERT <index> <text> : insert text at index (0-based). If index==size, append.
// DELETE <index>        : delete the line at index (0-based).
// PRINT                 : print all lines.
// SAVE <filename>       : save buffer to file.
// LOAD <filename>       : load buffer from file (replaces current contents).
// SHRINK                : shrinkToFit (reduce capacity to match size).
// QUIT                  : exit program.
//
// For long text input with spaces use: ADD (enter) <type text on next line>
// or use INSERT and then you'll be prompted for line text.

void showHelp(void) {
    puts("Commands:");
    puts("  ADD <text>           : append a new line (text after space). If you type just 'ADD' you'll be prompted.");
    puts("  INSERT <index>       : insert at index (0-based); you'll be prompted for text.");
    puts("  DELETE <index>       : delete the line at index (0-based).");
    puts("  PRINT                : print all lines.");
    puts("  SAVE <filename>      : save buffer to file.");
    puts("  LOAD <filename>      : load buffer from file (replaces current).");
    puts("  SHRINK               : shrinkToFit (reduce allocated array to exact size).");
    puts("  HELP                 : show this help.");
    puts("  QUIT                 : exit the editor.");
}

int main(void) {
    LineBuffer buf;
    initBuffer(&buf);

    printf("Minimal Line-Based Text Editor Buffer\n");
    printf("Type HELP for commands.\n");

    char cmdline[256];
    while (1) {
        printf("editor> ");
        if (!fgets(cmdline, sizeof(cmdline), stdin)) {
            // EOF or error => exit
            putchar('\n');
            break;
        }

        // trim trailing newline
        size_t L = strlen(cmdline);
        if (L && cmdline[L-1] == '\n') cmdline[L-1] = '\0';

        // skip empty lines
        if (cmdline[0] == '\0') continue;

        // parse first token
        char *tok = strtok(cmdline, " ");
        if (!tok) continue;

        if (strcasecmp(tok, "ADD") == 0) {
            char *rest = strtok(NULL, ""); // remainder of line
            if (rest && rest[0] != '\0') {
                insertLine(&buf, buf.size, rest);
            } else {
                // prompt for line text
                char *line = readLineStdin("Enter line to ADD: ");
                if (line) {
                    insertLine(&buf, buf.size, line);
                    free(line); // insertLine made its own copy
                }
            }
        }
        else if (strcasecmp(tok, "INSERT") == 0) {
            char *idxs = strtok(NULL, " ");
            if (!idxs) {
                printf("Usage: INSERT <index>\n");
                continue;
            }
            int idx = atoi(idxs);
            char *text = readLineStdin("Enter line to INSERT: ");
            if (text) {
                insertLine(&buf, idx, text);
                free(text);
            }
        }
        else if (strcasecmp(tok, "DELETE") == 0) {
            char *idxs = strtok(NULL, " ");
            if (!idxs) {
                printf("Usage: DELETE <index>\n");
                continue;
            }
            int idx = atoi(idxs);
            deleteLine(&buf, idx);
        }
        else if (strcasecmp(tok, "PRINT") == 0) {
            printAllLines(&buf);
        }
        else if (strcasecmp(tok, "SAVE") == 0) {
            char *fname = strtok(NULL, " ");
            if (!fname) {
                printf("Usage: SAVE <filename>\n");
                continue;
            }
            saveToFile(&buf, fname);
        }
        else if (strcasecmp(tok, "LOAD") == 0) {
            char *fname = strtok(NULL, " ");
            if (!fname) {
                printf("Usage: LOAD <filename>\n");
                continue;
            }
            loadFromFile(&buf, fname);
        }
        else if (strcasecmp(tok, "SHRINK") == 0) {
            shrinkToFit(&buf);
            printf("Shrunk capacity to %d (size %d)\n", buf.capacity, buf.size);
        }
        else if (strcasecmp(tok, "HELP") == 0) {
            showHelp();
        }
        else if (strcasecmp(tok, "QUIT") == 0) {
            break;
        }
        else {
            printf("Unknown command '%s'. Type HELP.\n", tok);
        }
    }

    freeAll(&buf);
    printf("Goodbye.\n");
    return 0;
}
