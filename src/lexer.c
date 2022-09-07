#include "lexer.h"

// the rules for lexer
const Lex_RegexTable regex_and_action[] = {
    { "(auto)", "auto", AUTO_T },
    { "(break)", "break", BREAK_T },
    { "(case)", "case", CASE_T },
    { "(char)", "char", CHAR_T },
    { "(const)", "const", CONST_T },
    { "(continue)", "continue", CONTINUE_T },
    { "(default)", "default", DEFAULT_T },
    { "(do)", "do", DO_T },
    { "(double)", "double", DOUBLE_T },
    { "(else)", "else", ELSE_T },
    { "(enum)", "enum", ENUM_T },
    { "(extern)", "extern", EXTERN_T },
    { "(float)", "float", FLOAT_T },
    { "(for)", "for", FOR_T },
    { "(goto)", "goto", GOTO_T },
    { "(if)", "if", IF_T },
    { "(inline)", "inline", INLINE_T },
    { "(int)", "int", INT_T },
    { "(long)", "long", LONG_T },
    { "(register)", "register", REGISTER_T },
    { "(restrict)", "restrict", RESTRICT_T },
    { "(return)", "return", RETURN_T },
    { "(short)", "short", SHORT_T },
    { "(signed)", "signed", SIGNED_T },
    { "(sizeof)", "sizeof", SIZEOF_T },
    { "(static)", "static", STATIC_T },
    { "(struct)", "struct", STRUCT_T },
    { "(switch)", "switch", SWITCH_T },
    { "(typedef)", "typedef", TYPEDEF_T },
    { "(union)", "union", UNION_T },
    { "(unsigned)", "unsigned", UNSIGNED_T },
    { "(void)", "void", VOID_T },
    { "(volatile)", "volatile", VOLATILE_T },
    { "(while)", "while", WHILE_T },
    { "(_Bool)", "_Bool", BOOL_T },
    { "(_Complex)", "_Complex", COMPLEX_T },
    { "(_Imaginary)", "_Imaginary", IMAGINARY_T },
    { "(([A-Za-z_])(([A-Za-z_])|([0-9]))*)", "identifier", IDENTIFIER_T },
    { "(([/][*]([^*]|\\*[^/])*[*][/])|([/][/][^\n]*))", "comment", COMMENT_T },
    { "((0x|0X)((([0-9a-fA-F])+)?\\.(([0-9a-fA-F])+)|(([0-9a-fA-F])+)\\.)([pP]([+-])?(([0-9])+))([flFL])?|(0x|0X)(([0-9a-fA-F])+)([pP]([+-])?(([0-9])+))([flFL])?)", "hexadecimal floating constant", HEX_FLOAT_CONST_T },
    { "(((([0-9])+)?\\.(([0-9])+)|(([0-9])+)\\.)([Ee]([+-])?(([0-9])+))?([flFL])?|(([0-9])+)([Ee]([+-])?(([0-9])+))([flFL])?)", "decimal floating number", DEC_FLOAT_T },
    { "(([1-9])([0-9])*((u|U)(l|L)?|(u|U)(ll|LL)|(l|L)(u|U)?|(ll|LL)(u|U)?)?)", "decimal constant", DEC_CONST_T },
    { "(0([0-7])*((u|U)(l|L)?|(u|U)(ll|LL)|(l|L)(u|U)?|(ll|LL)(u|U)?)?)", "octal constant", OCT_CONST_T },
    { "((0x|0X)([0-9a-fA-F])+((u|U)(l|L)?|(u|U)(ll|LL)|(l|L)(u|U)?|(ll|LL)(u|U)?)?)", "hexadecimal constant", HEX_CONST_T },
    { "(L?\"([^\"\\\n]|((\\\\.)|(\\\\[0-7]{1,3})|(\\\\x([0-9a-fA-F])+)))*\")", "string literal", STR_LITERAL_T },
    { "(L?'([^'\\\n]|((\\\\.)|(\\\\[0-7]{1,3})|(\\\\x([0-9a-fA-F])+)))+')", "character", CHARACTER_T },
    { "(\\[)", "left square brackets", LEFT_SQUARE_BRACKET_T },
    { "(\\])", "right square brackets", RIGHT_SQUARE_BRACKET_T },
    { "(\\()", "left parentheses", LEFT_PARETHESIS_T },
    { "(\\))", "right parentheses", RIGHT_PARETHESIS_T },
    { "(\\{)", "left braces", LEFT_BRACE_T },
    { "(\\})", "right braces", RIGHT_BRACE_T },
    { "(\\|\\|)", "logical or", LOGICAL_OR_T },
    { "(=)", "assignment", ASSIGNMENT_T },
    { "(&&)", "logical and", LOGICAL_AND_T },
    { "(!)", "logical not", LOGICAL_NOT_T },
    { "(!=)", "logical ne", LOGICAL_NE_T },
    { "(~)", "bitwise not", BITWISE_NOT_T },
    { "(<)", "logical lt", LOGICAL_LT_T },
    { "(<=)", "logical le", LOGICAL_LE_T },
    { "(==)", "logical eq", LOGICAL_EQ_T },
    { "(>=)", "logical ge", LOGICAL_GE_T },
    { "(>)", "logical gt", LOGICAL_GT_T },
    { "(>>)", "bitwise rshift", BITWISE_RIGHT_SHIFT_T },
    { "(\\+\\+)", "increasement", INCREASEMENT_T },
    { "(--)", "decreasement", DECEASEMENT_T },
    { "(\\+=)", "add assignment", ADD_ASSIGNMENT_T },
    { "(-=)", "minus assignment", MINUS_ASSIGNMENT_T },
    { "(\\*=)", "multiply assignment", MULTIPLY_ASSIGNMENT_T },
    { "(/=)", "divide assignment", DIVIDE_ASSIGNMENT_T },
    { "(%=)", "modulo assignment", MODULO_ASSIGNMENT_T },
    { "(-)", "minus", MINUS_T },
    { "(\\+)", "add", ADD_T },
    { "(/)", "divide", DIVIDE_T },
    { "(%)", "modulo", MODULO_T },
    { "(\\?)", "question mark", QUESTION_MARK_T },
    { "(:)", "colon", COLON_T },
    { "(;)", "semicolon", SEMICOLON_T },
    { "(,)", "comma", COMMA_T },
    { "(\\^)", "bitwise xor", BITWISE_XOR_T },
    { "(\\|)", "bitwise or", BITWISE_OR_T },
    { "(\\*)", "star", STAR_T },
    { "(&)", "and", AND_T },
    { "(\\.)", "dot", DOT_T },
    { "(->)", "arrow", ARROW_T },
    { "(#(\\\\\n|[^\n])*)", "preprocessor command", PREPROCESSOR_COMMAND_T },
    { "([[:space:]])", "", SPACE_T }
};

#define rule_num ((int)(sizeof(regex_and_action) / sizeof(*regex_and_action)))

NFAGraph compiled_regex[rule_num];

static char *slurp(const char *filename) {
    char *buf = (char *)malloc(16);
    int bufsize = 16;
    int cnt = 0;
    int ch, peek;
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "informater: cannot open %s\n", filename);
        exit(1);
    }
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\\') {
            peek = fgetc(fp);
            if (peek == '\n') {
                continue;
            }
            ungetc(peek, fp);
        }
        if (cnt == bufsize - 1) {
            bufsize = bufsize * 2 + 4;
            buf = realloc(buf, bufsize);
            assert(buf != NULL);
        }
        buf[cnt++] = ch;
    }
    buf[cnt++] = '\0';

    return buf;
}

#define TOKENS_INITIAL_SIZE 1024
Lex_TokenState LexS_new(void) {
    return (Lex_TokenState) {
        .size = TOKENS_INITIAL_SIZE,
        .num = 0,
        .tokens = (Lex_Token *)malloc(sizeof(Lex_Token) * TOKENS_INITIAL_SIZE)
    };
}

void LexS_resize(Lex_TokenState *st) {
    st->size = 2 * st->size + 4;
    st->tokens = (Lex_Token *)realloc(st->tokens, sizeof(Lex_Token) * st->size);
}

void LexS_add_token(Lex_TokenState *st, Lex_Token t) {
    if (st->num == st->size) {
        LexS_resize(st);
    }
    st->tokens[st->num++] = t;
}

Lex_Token Lex_Token_new(int line, int column, const char *str, Lex_TokenType t,
        const char *type_name) {
    char *s = (char *)malloc(strlen(str) + 1);
    strcpy(s, str);
    return (Lex_Token) {
        .line = line,
        .column = column,
        .str = s,
        .type = t,
        .type_name = type_name
    };
}

static inline void Lex_Token_drop(Lex_Token *t) {
    free(t->str);
}

Lex_TokenState Lex_lexer_go(const char *filename) {
    char *pos;
    char *buf;
    Lex_TokenState st = LexS_new();
    int line = 1;

    // compile the regex
    for (int i = 0; i < rule_num; ++i) {
        compiled_regex[i] = regex_compile(regex_and_action[i].regex);
    }

    buf = slurp(filename);
    pos = buf;
    char *line_begin_pos = pos - 1;

    while (*pos != '\0') {
        int matched_item = -1;
        int matched_len = 0;
        int tmp;

        for (int i = 0; i < rule_num; ++i) {
            int res = regex_execute(&compiled_regex[i], pos);
            if (res != -1) {
                int len = res;
                if (matched_item == -1 || len > matched_len) {
                    matched_item = i;
                    matched_len = len;
                }
            }
        }

        if (matched_item == -1) {
            panic("cannot recognize");
        }
        tmp = pos[matched_len];
        pos[matched_len] = '\0';
        if (*regex_and_action[matched_item].token_name != '\0') {
            LexS_add_token(&st, Lex_Token_new(line, pos - line_begin_pos,
                        pos, regex_and_action[matched_item].type,
                        regex_and_action[matched_item].token_name));
        }
        if (tmp == '\n') {
            line++;
            line_begin_pos = pos + matched_len;
        }
        pos[matched_len] = tmp;
        pos += matched_len;
    }

    for (int i = 0; i < rule_num; ++i)
        regex_free(&compiled_regex[i]);
    free(buf);

    return st;
}

void LexS_drop(Lex_TokenState *st) {
    for (int i = 0; i < st->num; ++i) {
        Lex_Token_drop(&st->tokens[i]);
    }
    free(st->tokens);
}

void LexS_display(FILE *fp, Lex_TokenState *st) {
    for (int i = 0; i < st->num; ++i) {
        Lex_Token *t = &st->tokens[i];
        fprintf(fp, "[\033[1;032m%s\033[0m]\n@%2d, %2d: %s\n",
                t->str, t->line, t->column, t->type_name);
    }
}

