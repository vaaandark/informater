#include "irregex/irregex.h"

typedef enum {
    UNKNOWN_T,
    AUTO_T,
    BREAK_T,
    CASE_T,
    CHAR_T,
    CONST_T,
    CONTINUE_T,
    DEFAULT_T,
    DO_T,
    DOUBLE_T,
    ELSE_T,
    ENUM_T,
    EXTERN_T,
    FLOAT_T,
    FOR_T,
    GOTO_T,
    IF_T,
    INLINE_T,
    INT_T,
    LONG_T,
    REGISTER_T,
    RESTRICT_T,
    RETURN_T,
    SHORT_T,
    SIGNED_T,
    SIZEOF_T,
    STATIC_T,
    STRUCT_T,
    SWITCH_T,
    TYPEDEF_T,
    UNION_T,
    UNSIGNED_T,
    VOID_T,
    VOLATILE_T,
    WHILE_T,
    BOOL_T,
    COMPLEX_T,
    IMAGINARY_T,
    IDENTIFIER_T,
    COMMENT_T,
    HEX_FLOAT_CONST_T,
    DEC_FLOAT_T,
    DEC_CONST_T,
    OCT_CONST_T,
    HEX_CONST_T,
    STR_LITERAL_T,
    CHARACTER_T,
    LEFT_SQUARE_BRACKET_T,
    RIGHT_SQUARE_BRACKET_T,
    LEFT_PARETHESIS_T,
    RIGHT_PARETHESIS_T,
    LEFT_BRACE_T,
    RIGHT_BRACE_T,
    LOGICAL_OR_T,
    ASSIGNMENT_T,
    LOGICAL_AND_T,
    LOGICAL_NOT_T,
    LOGICAL_NE_T,
    BITWISE_NOT_T,
    LOGICAL_LT_T,
    LOGICAL_LE_T,
    LOGICAL_EQ_T,
    LOGICAL_GE_T,
    LOGICAL_GT_T,
    BITWISE_RIGHT_SHIFT_T,
    BITWISE_RIGHT_LEFT_T,
    INCREASEMENT_T,
    DECEASEMENT_T,
    ADD_ASSIGNMENT_T,
    MINUS_ASSIGNMENT_T,
    MULTIPLY_ASSIGNMENT_T,
    DIVIDE_ASSIGNMENT_T,
    MODULO_ASSIGNMENT_T,
    MINUS_T,
    ADD_T,
    DIVIDE_T,
    MODULO_T,
    QUESTION_MARK_T,
    COLON_T,
    SEMICOLON_T,
    COMMA_T,
    BITWISE_XOR_T,
    BITWISE_OR_T,
    STAR_T,
    AND_T,
    DOT_T,
    ARROW_T,
    PREPROCESSOR_COMMAND_T,
    NEWLINE_T,
    SPACE_T
} Lex_TokenType;

typedef struct {
    const char *regex;
    const char *token_name;
    const Lex_TokenType type;
} Lex_RegexTable;

typedef struct {
    char *str;
    Lex_TokenType type;
    const char *type_name;
    int line;
    int column;
} Lex_Token;

typedef struct {
    int num;
    int size;
    Lex_Token *tokens;
} Lex_TokenState;

extern const Lex_RegexTable regex_and_action[];

extern Lex_TokenState Lex_lexer_go(const char *filename);

extern void LexS_drop(Lex_TokenState *st);

extern void LexS_display(FILE *fp, Lex_TokenState *st);

