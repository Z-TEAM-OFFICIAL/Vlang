#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <windows.h>

#define MAX_VARS 128
#define MAX_VAR_NAME 32
#define MAX_LINE_LEN 256
#define MAX_COND_STACK 64
#define MAX_INCLUDE_DEPTH 16

// ---------- Data Structures ----------
typedef struct {
    char name[MAX_VAR_NAME];
    int value;
} Variable;

typedef struct {
    char input_file[256];
    char output_file[256];
    int strip_symbols;
    int is_dll;
    int static_link;   // -static flag (ignored, for compatibility)
} CompilerArgs;

// Symbol table
Variable sym_table[MAX_VARS];
int var_count = 0;

// Conditional compilation stack
typedef enum { COND_PROC, COND_SKIP } CondState;
CondState cond_stack[MAX_COND_STACK];
int cond_sp = 0;

// Include stack (for error messages and recursion prevention)
typedef struct {
    char filename[256];
    int line_number;
} IncludeFrame;
IncludeFrame include_stack[MAX_INCLUDE_DEPTH];
int include_sp = 0;
char current_filename[256];
int line_number = 0;

// ---------- Function Prototypes ----------
void error(const char* msg);
int find_variable(const char* name);
int parse_expression(const char* expr);
void push_cond(CondState state);
CondState current_state(void);
void include_file(const char* filename, int* ret_value);
void import_module(const char* module_name, int* ret_value);
void strip_comment(char *line);
int parse_line(const char* line, int* ret_value);
int parse_v_file(const char* path);
void build_pe_binary(CompilerArgs args, int value);

// ---------- Error Reporting ----------
void error(const char* msg) {
    if (include_sp > 0) {
        printf("Error in %s (included from %s line %d): line %d: %s\n",
               current_filename, include_stack[include_sp-1].filename,
               include_stack[include_sp-1].line_number, line_number, msg);
    } else {
        printf("Error in %s line %d: %s\n", current_filename, line_number, msg);
    }
    exit(1);
}

// ---------- Variable Handling ----------
int find_variable(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(sym_table[i].name, name) == 0) return i;
    }
    return -1;
}

// ---------- Expression Parser ----------
typedef struct {
    const char* src;
    int pos;
} Tokenizer;

int peek_char(Tokenizer* t) { return t->src[t->pos]; }
int get_char(Tokenizer* t) { return t->src[t->pos++]; }
void skip_whitespace(Tokenizer* t) { while (isspace(peek_char(t))) get_char(t); }

int parse_ternary(Tokenizer* t);
int parse_comparison(Tokenizer* t);
int parse_addition(Tokenizer* t);
int parse_term(Tokenizer* t);
int parse_primary(Tokenizer* t);

int parse_primary(Tokenizer* t) {
    skip_whitespace(t);
    int c = peek_char(t);
    if (c == '(') {
        get_char(t);
        int val = parse_ternary(t);
        skip_whitespace(t);
        if (get_char(t) != ')') error("Expected ')'");
        return val;
    }
    if (c == '-') {
        get_char(t);
        return -parse_primary(t);
    }
    if (isdigit(c)) {
        int val = 0;
        while (isdigit(peek_char(t))) {
            val = val * 10 + (get_char(t) - '0');
        }
        return val;
    }
    if (isalpha(c) || c == '_') {
        char name[MAX_VAR_NAME];
        int i = 0;
        while (isalnum(peek_char(t)) || peek_char(t) == '_') {
            name[i++] = get_char(t);
            if (i >= MAX_VAR_NAME-1) error("Identifier too long");
        }
        name[i] = 0;

        int idx = find_variable(name);
        if (idx == -1) error("Undefined variable");
        return sym_table[idx].value;
    }
    error("Unexpected character in expression");
    return 0;
}

int parse_term(Tokenizer* t) {
    int left = parse_primary(t);
    while (1) {
        skip_whitespace(t);
        int c = peek_char(t);
        if (c == '*') {
            get_char(t);
            int right = parse_primary(t);
            left = left * right;
        } else if (c == '/') {
            get_char(t);
            int right = parse_primary(t);
            if (right == 0) error("Division by zero");
            left = left / right;
        } else if (c == '%') {
            get_char(t);
            int right = parse_primary(t);
            if (right == 0) error("Modulo by zero");
            left = left % right;
        } else {
            break;
        }
    }
    return left;
}

int parse_addition(Tokenizer* t) {
    int left = parse_term(t);
    while (1) {
        skip_whitespace(t);
        int c = peek_char(t);
        if (c == '+') {
            get_char(t);
            int right = parse_term(t);
            left = left + right;
        } else if (c == '-') {
            get_char(t);
            int right = parse_term(t);
            left = left - right;
        } else {
            break;
        }
    }
    return left;
}

int parse_comparison(Tokenizer* t) {
    int left = parse_addition(t);
    while (1) {
        skip_whitespace(t);
        int c1 = peek_char(t);
        int c2 = t->src[t->pos + 1];
        if (c1 == '<' && c2 == '=') {
            get_char(t); get_char(t);
            int right = parse_addition(t);
            left = (left <= right) ? 1 : 0;
        } else if (c1 == '>' && c2 == '=') {
            get_char(t); get_char(t);
            int right = parse_addition(t);
            left = (left >= right) ? 1 : 0;
        } else if (c1 == '=' && c2 == '=') {
            get_char(t); get_char(t);
            int right = parse_addition(t);
            left = (left == right) ? 1 : 0;
        } else if (c1 == '!' && c2 == '=') {
            get_char(t); get_char(t);
            int right = parse_addition(t);
            left = (left != right) ? 1 : 0;
        } else if (c1 == '<') {
            get_char(t);
            int right = parse_addition(t);
            left = (left < right) ? 1 : 0;
        } else if (c1 == '>') {
            get_char(t);
            int right = parse_addition(t);
            left = (left > right) ? 1 : 0;
        } else {
            break;
        }
    }
    return left;
}

int parse_ternary(Tokenizer* t) {
    int cond = parse_comparison(t);
    skip_whitespace(t);
    if (peek_char(t) == '?') {
        get_char(t);
        int then_expr = parse_ternary(t);
        skip_whitespace(t);
        if (peek_char(t) != ':') error("Expected ':' in ternary");
        get_char(t);
        int else_expr = parse_ternary(t);
        return cond ? then_expr : else_expr;
    }
    return cond;
}

int parse_expression(const char* expr) {
    Tokenizer t = {expr, 0};
    int val = parse_ternary(&t);
    skip_whitespace(&t);
    if (t.src[t.pos] != 0) error("Extra characters after expression");
    return val;
}

// ---------- Conditional Stack Helpers ----------
void push_cond(CondState state) {
    if (cond_sp >= MAX_COND_STACK)
        error("Conditional nesting too deep");
    cond_stack[cond_sp++] = state;
}

CondState current_state() {
    return (cond_sp > 0) ? cond_stack[cond_sp - 1] : COND_PROC;
}

// ---------- Comment Stripping ----------
void strip_comment(char *line) {
    char *p = line;
    int in_string = 0;
    while (*p) {
        if (*p == '"') in_string = !in_string;
        if (*p == '#' && !in_string) {
            *p = '\0';
            break;
        }
        p++;
    }
}

// ---------- File Inclusion ----------
void include_file(const char* filename, int* ret_value) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Cannot open file '%s'", filename);
        error(buf);
    }

    if (include_sp >= MAX_INCLUDE_DEPTH)
        error("Include nesting too deep");

    strcpy(include_stack[include_sp].filename, current_filename);
    include_stack[include_sp].line_number = line_number;
    include_sp++;

    strcpy(current_filename, filename);
    line_number = 0;

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), f)) {
        line_number++;
        line[strcspn(line, "\r\n")] = 0;
        parse_line(line, ret_value);
    }

    fclose(f);

    include_sp--;
    strcpy(current_filename, include_stack[include_sp].filename);
    line_number = include_stack[include_sp].line_number;
}

// ---------- Module Import ----------
void import_module(const char* module_name, int* ret_value) {
    char path[512];
    snprintf(path, sizeof(path), "modules/%s.c", module_name);
    FILE* f = fopen(path, "r");
    if (!f) {
        snprintf(path, sizeof(path), "modules/%s.v", module_name);
        f = fopen(path, "r");
        if (!f) {
            char buf[256];
            snprintf(buf, sizeof(buf), "Module '%s' not found in modules/", module_name);
            error(buf);
        }
    }
    fclose(f);
    include_file(path, ret_value);
}

// ---------- Line Parser ----------
int parse_line(const char* line, int* ret_value) {
    char buffer[MAX_LINE_LEN];
    strcpy(buffer, line);
    char *p = buffer;
    while (isspace(*p)) p++;
    if (*p == '#' || *p == 0) return 0;

    // Conditional keywords
    if (strncmp(p, "if", 2) == 0 && (isspace(p[2]) || p[2] == '\0')) {
        p += 2;
        while (isspace(*p)) p++;
        CondState curr = current_state();
        if (curr == COND_PROC) {
            strip_comment(p);
            int cond = parse_expression(p);
            push_cond(cond ? COND_PROC : COND_SKIP);
        } else {
            push_cond(COND_SKIP);
        }
        return 0;
    }
    else if (strncmp(p, "elif", 4) == 0 && (isspace(p[4]) || p[4] == '\0')) {
        if (cond_sp == 0) error("elif without if");
        p += 4;
        while (isspace(*p)) p++;
        CondState curr = cond_stack[cond_sp - 1];
        if (curr == COND_PROC) {
            cond_stack[cond_sp - 1] = COND_SKIP;
        } else {
            strip_comment(p);
            int cond = parse_expression(p);
            if (cond) cond_stack[cond_sp - 1] = COND_PROC;
        }
        return 0;
    }
    else if (strncmp(p, "else", 4) == 0 && (isspace(p[4]) || p[4] == '\0')) {
        if (cond_sp == 0) error("else without if");
        CondState curr = cond_stack[cond_sp - 1];
        if (curr == COND_PROC) {
            cond_stack[cond_sp - 1] = COND_SKIP;
        } else {
            cond_stack[cond_sp - 1] = COND_PROC;
        }
        return 0;
    }
    else if (strncmp(p, "end", 3) == 0 && (isspace(p[3]) || p[3] == '\0')) {
        if (cond_sp == 0) error("end without if");
        cond_sp--;
        return 0;
    }

    if (current_state() == COND_SKIP) return 0;

    // Module import
    if (strncmp(p, "imp ", 4) == 0) {
        p += 4;
        while (isspace(*p)) p++;
        char module[64];
        int i = 0;
        while (isalnum(*p) || *p == '_') {
            module[i++] = *p++;
            if (i >= 63) error("Module name too long");
        }
        module[i] = 0;
        import_module(module, ret_value);
        return 0;
    }

    // File include
    if (strncmp(p, "inc ", 4) == 0) {
        p += 4;
        while (isspace(*p)) p++;
        if (*p != '"') error("Expected quoted filename after inc");
        p++;
        char filename[256];
        int i = 0;
        while (*p && *p != '"') {
            filename[i++] = *p++;
            if (i >= 255) error("Filename too long");
        }
        filename[i] = 0;
        if (*p != '"') error("Missing closing quote in filename");
        p++;
        include_file(filename, ret_value);
        return 0;
    }

    // Ignore fn main and class lines
    if (strncmp(p, "fn main", 7) == 0) return 0;
    if (strncmp(p, "class ", 6) == 0) return 0;

    char name[MAX_VAR_NAME];
    // val assignment
    if (strncmp(p, "val ", 4) == 0) {
        p += 4;
        int i = 0;
        while (isspace(*p)) p++;
        while (isalnum(*p) || *p == '_') {
            name[i++] = *p++;
            if (i >= MAX_VAR_NAME-1) error("Variable name too long");
        }
        name[i] = 0;
        while (isspace(*p)) p++;
        if (*p != '=') error("Expected '=' after variable name");
        p++;
        while (isspace(*p)) p++;
        strip_comment(p);
        int value = parse_expression(p);
        int idx = find_variable(name);
        if (idx == -1) {
            if (var_count >= MAX_VARS) error("Too many variables");
            strcpy(sym_table[var_count].name, name);
            sym_table[var_count].value = value;
            var_count++;
        } else {
            sym_table[idx].value = value;
        }
        return 0;
    }

    // set statement
    if (strncmp(p, "set ", 4) == 0) {
        p += 4;
        int i = 0;
        while (isspace(*p)) p++;
        while (isalnum(*p) || *p == '_') {
            name[i++] = *p++;
            if (i >= MAX_VAR_NAME-1) error("Variable name too long");
        }
        name[i] = 0;
        int idx = find_variable(name);
        if (idx == -1) error("Undefined variable in set");
        while (isspace(*p)) p++;
        if (*p != '=') error("Expected '=' after variable name");
        p++;
        while (isspace(*p)) p++;
        strip_comment(p);
        int value = parse_expression(p);
        sym_table[idx].value = value;
        return 0;
    }

    // return statement
    if (strncmp(p, "ret ", 4) == 0) {
        p += 4;
        while (isspace(*p)) p++;
        strip_comment(p);
        *ret_value = parse_expression(p);
        return 1;
    }

    // Unrecognized line: ignore
    return 0;
}

// ---------- Main Compilation ----------
int parse_v_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        printf("Error: Cannot open input file %s\n", path);
        return 0;
    }

    strcpy(current_filename, path);
    line_number = 0;
    cond_sp = 0;

    char line[MAX_LINE_LEN];
    int final_ret = 0;

    while (fgets(line, sizeof(line), f)) {
        line_number++;
        line[strcspn(line, "\r\n")] = 0;
        parse_line(line, &final_ret);
    }

    fclose(f);
    return final_ret;
}

// ---------- PE Binary Generation ----------
void build_pe_binary(CompilerArgs args, int value) {
    FILE* f = fopen(args.output_file, "wb");
    if (!f) {
        printf("Error: Cannot create output file %s\n", args.output_file);
        return;
    }

    IMAGE_DOS_HEADER dos = {0x5A4D};
    dos.e_lfanew = 0x40;
    fwrite(&dos, 1, sizeof(dos), f);

    IMAGE_NT_HEADERS64 nt = {0};
    nt.Signature = 0x00004550;
    nt.FileHeader.Machine = 0x8664;
    nt.FileHeader.NumberOfSections = 1;
    nt.FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER64);
    nt.FileHeader.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE;
    if (args.is_dll) nt.FileHeader.Characteristics |= IMAGE_FILE_DLL;
    if (args.strip_symbols) nt.FileHeader.Characteristics |= IMAGE_FILE_LOCAL_SYMS_STRIPPED;

    nt.OptionalHeader.Magic = 0x020B;
    nt.OptionalHeader.AddressOfEntryPoint = 0x1000;
    nt.OptionalHeader.ImageBase = 0x400000;
    nt.OptionalHeader.SectionAlignment = 0x1000;
    nt.OptionalHeader.FileAlignment = 0x200;
    nt.OptionalHeader.MajorSubsystemVersion = 6;
    nt.OptionalHeader.SizeOfImage = 0x2000;
    nt.OptionalHeader.SizeOfHeaders = 0x200;
    nt.OptionalHeader.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
    fwrite(&nt, 1, sizeof(nt), f);

    IMAGE_SECTION_HEADER sect = {0};
    memcpy(sect.Name, ".text", 5);
    sect.Misc.VirtualSize = 0x1000;
    sect.VirtualAddress = 0x1000;
    sect.SizeOfRawData = 0x200;
    sect.PointerToRawData = 0x200;
    sect.Characteristics = 0x60000020;
    fwrite(&sect, 1, sizeof(sect), f);

    fseek(f, 0x200, SEEK_SET);

    unsigned char code[512] = { 0xB8, 0x00, 0x00, 0x00, 0x00, 0xC3 };
    memcpy(&code[1], &value, 4);
    fwrite(code, 1, 512, f);

    fclose(f);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("V Compiler (with modules, includes, and -static)\n");
        printf("Usage: vlang <file.v> -o <out.exe> [-s] [-static]\n");
        return 1;
    }

    CompilerArgs args = {0};
    strcpy(args.input_file, argv[1]);
    strcpy(args.output_file, "output.exe");

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            strcpy(args.output_file, argv[++i]);
            if (strstr(args.output_file, ".dll")) args.is_dll = 1;
        } else if (strcmp(argv[i], "-s") == 0) {
            args.strip_symbols = 1;
        } else if (strcmp(argv[i], "-static") == 0) {
            args.static_link = 1;   // accepted, does nothing (all compilation is static)
        }
    }

    int result_value = parse_v_file(args.input_file);
    build_pe_binary(args, result_value);

    printf("VLang: Successfully compiled %s -> %s (Value: %d)\n",
           args.input_file, args.output_file, result_value);
    return 0;
}