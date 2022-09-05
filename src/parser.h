#include "lexer.h"
#include "def.h"
#include "err.h"

#define NodeType_enum \
    X(ND_UNDEFINED, "未定义") X(ND_EMPTY, "空节点") X(ND_PREPROCESS, "预处理") X(ND_TRANS_UNIT, "程序") \
    X(ND_EXTERNAL_DECL, "外部定义") X(ND_EXTERNAL_DECL_CONS, "外部定义序列") X(ND_EXTERNAL_VAR_DECL, "变量的外部定义") \
    X(ND_FUNC_DECL, "函数定义") X(ND_TYPE_SPEC, "类型说明符") X(ND_VAR, "变量") X(ND_VAR_CONS, "变量序列") X(ND_FUNC_NAME, "函数名") \
    X(ND_ARG, "函数形参") X(ND_ARG_CONS, "函数形参序列") X(ND_EXP, "表达式") X(ND_STATEMENT_CONS, "语句序列") X(ND_STATEMENT, "语句") \
    X(ND_COMPOUND, "复合语句") X(ND_IDENT, "标识符") X(ND_LOCAL_VAR_DECL, "局部变量定义") X(ND_LOCAL_VAR_DECL_CONS, "局部变量定义序列") \
    X(ND_RETURN, "return") X(ND_IF, "if") X(ND_REAL_ARG_CONS, "函数实参序列") X(ND_FUNC_CALL, "函数调用") X(ND_ARRAY, "数组")\
    X(ND_WHILE, "while") X(ND_FOR, "for") X(ND_BREAK, "break") X(ND_CONTINUE, "continue") \
    X(ND_CONST_NUM, "常量数字") X(ND_OPT, "双目运算符") X(ND_ASSIGNMENT, "=")

typedef enum {
#define X(a, b) a,
    NodeType_enum
#undef X
} NodeType;

extern const char *node_type_strings[];

typedef struct {
    int pos;
    int size;
    Lex_Token *tokens;
} TokenStream;

extern TokenStream TS_new(Lex_TokenState st);

#define SONSMAX 5
typedef struct node {
    bool is_leaf;
    union {
        Lex_Token token;
        struct {
            NodeType t;
            int num;
            struct node *sons[SONSMAX];
        };
    };
} *Node;

extern Node parser_go(Lex_TokenState st);

