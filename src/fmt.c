#include "fmt.h"

static void fmt_trans_unit(FILE *f, Node n);

void formater_go(FILE *f, Node n) {
    fmt_trans_unit(f, n);
}

static inline void fmt_print_indent(FILE *f, int indent) {
    for (int i = 0; i < indent; ++i) {
        fprintf(f, "    ");
    }
}

#define COMMENT_STACK_MAX 64
char *comment_stack[COMMENT_STACK_MAX];
int comment_num = 0;

static inline void comment_push(char *s) {
    comment_stack[comment_num++] = s;
}

static inline char *comment_pop(void) {
    return comment_stack[--comment_num];
}

static void fmt_comment_cons(Node n) {
    comment_push(n->sons[0]->sons[0]->token.str);
    if (n->num > 1) {
        fmt_comment_cons(n->sons[1]);
    }
}

static void check_and_fmt_comment(Node n) {
    if (n->comment != NULL) {
        fmt_comment_cons(n->comment);
    }
}

static inline void newline(FILE *f, int indent) {
    fprintf(f, "\n");
    while (comment_num > 0) {
        fmt_print_indent(f, indent);
        fprintf(f, "%s", comment_pop());
        fprintf(f, "\n");
    }
}

static void fmt_var_cons(FILE *, Node);

// 格式外部变量定义
static void fmt_external_var_decl(FILE *f, Node n) {
    check_and_fmt_comment(n);
    fprintf(f, "%s ", n->sons[0]->token.str);
    fmt_var_cons(f, n->sons[1]);
}

static void fmt_compound(FILE *, Node, int);

// 格式化函数形式参数
static void fmt_arg(FILE *f, Node n) {
    check_and_fmt_comment(n);
    fprintf(f, "%s", n->sons[0]->token.str);
    if (n->sons[0]->token.type != VOID_T) {
        fprintf(f, " %s", n->sons[1]->token.str);
    }
}

// 格式化函数形式参数序列
static void fmt_arg_cons(FILE *f, Node n) {
    check_and_fmt_comment(n);
    fmt_arg(f, n->sons[0]);
    if (n->num > 1) {
        fprintf(f, ", ");
        fmt_arg_cons(f, n->sons[1]);
    }
}

// 格式化函数声明或定义
static void fmt_external_func_decl_or_def(FILE *f, Node n, int indent) {
    check_and_fmt_comment(n);
    fmt_print_indent(f, indent);
    fprintf(f, "%s ", n->sons[0]->token.str); // 返回值类型
    fprintf(f, "%s(", n->sons[1]->token.str); // 函数名
    fmt_arg_cons(f, n->sons[2]);
    fprintf(f, ")");
    if (n->t == ND_FUNC_DECL) { // 如果是函数声明
        fprintf(f, ";");
    } else { // 如果是函数定义
        fprintf(f, " ");
        fmt_compound(f, n->sons[3], indent + 1);
    }
    newline(f, indent);
}

// 格式化预处理语句
static void fmt_preprocess(FILE *f, Node n) {
    check_and_fmt_comment(n);
    fprintf(f, "%s", n->sons[0]->token.str);
}

// 格式化外部定义
// 子节点有四种情况 : 外部变量定义 | 函数定义 | 函数声明 | 预处理语句
static void fmt_external_decl(FILE *f, Node n, int indent) {
    check_and_fmt_comment(n);
    fmt_print_indent(f, indent);
    if (n->num != 0) {
        Node son = n->sons[0];
        switch (son->t) {
            case ND_EXTERNAL_VAR_DECL:
                fmt_external_var_decl(f, son);
                break;
            case ND_FUNC_DECL:
            case ND_FUNC_DEF:
                fmt_external_func_decl_or_def(f, son, indent);
                break;
            case ND_PREPROCESS:
                fmt_preprocess(f, son);
                break;
            default:
                panic("wrong external declaration");
                break;
        }
    }
    newline(f, indent);
}

// 格式化外部定义序列
static void fmt_external_decl_cons(FILE *f, Node n, int indent) {
    check_and_fmt_comment(n);
    fmt_external_decl(f, n->sons[0], indent);
    if (n->num > 1) {
        fmt_external_decl_cons(f, n->sons[1], indent);
    }
}

// 格式化整个程序
static inline void fmt_trans_unit(FILE *f, Node n) {
    fmt_external_decl_cons(f, n->sons[0], 0);
}

static void fmt_exp(FILE *, Node);

// 格式化实参
static void fmt_real_arg_cons(FILE *f, Node n) {
    check_and_fmt_comment(n);
    fmt_exp(f, n->sons[0]);
    if (n->num > 1) {
        fprintf(f, ", ");
        fmt_real_arg_cons(f, n->sons[1]);
    }
}

// 格式化函数调用
static void fmt_func_call(FILE *f, Node n) {
    check_and_fmt_comment(n);
    fprintf(f, "%s(", n->sons[0]->token.str);
    fmt_real_arg_cons(f, n->sons[1]);
    fprintf(f, ")");
}

// 打印数组
static void fmt_array(FILE *f, Node n) {
    check_and_fmt_comment(n);
    fprintf(f, "%s[", n->sons[0]->token.str);
    fmt_exp(f, n->sons[1]);
    fprintf(f, "]");
}

// 格式化表达式
static void fmt_exp(FILE *f, Node n) {
    check_and_fmt_comment(n);
    if (n->t == ND_EMPTY) {
        return;
    }

    switch (n->num) {
        case 1: // 标识符 | 常量数字 | 标识符(实参序列) | 标识符[表达式]
            if (n->sons[0]->t == ND_FUNC_CALL) { // 函数调用
                fmt_func_call(f, n->sons[0]);
            } else if (n->sons[0]->t == ND_ARRAY) { // 数组
                fmt_array(f, n->sons[0]);
            } else {
                fprintf(f, "%s", n->sons[0]->token.str);
            }
            break;
        case 3: // 表达式 双目运算符号 表达式 | 标识符 = 表达式
            fmt_exp(f, n->sons[0]);
            fprintf(f, " %s ", n->sons[1]->token.str);
            fmt_exp(f, n->sons[2]);
            break;
        default:
            panic("wrong expression");
            break;
    }
}

// 格式化变量序列
static void fmt_var_cons(FILE *f, Node n) {
    check_and_fmt_comment(n);
    if (n->sons[0]->t == ND_ARRAY) {
        fmt_array(f, n->sons[0]);
    } else {
        fprintf(f, "%s", n->sons[0]->token.str);
    }

    if (n->num > 1) {
        fprintf(f, ", ");
        fmt_var_cons(f, n->sons[1]);
    } else {
        fprintf(f, ";");
    }
}

// 格式化局部变量定义
static void fmt_local_var_decl(FILE *f, Node n, int indent) {
    check_and_fmt_comment(n);
    fmt_print_indent(f, indent);
    fprintf(f, "%s ", n->sons[0]->token.str); // 类型标识符
    fmt_var_cons(f, n->sons[1]);
    newline(f, indent);
}

static void fmt_if(FILE *f, Node n, int indent) {
    check_and_fmt_comment(n);
    fprintf(f, "if (");
    fmt_exp(f, n->sons[0]);
    fprintf(f, ") ");
    fmt_compound(f, n->sons[1], indent + 1);
}

static void fmt_while(FILE *f, Node n, int indent) {
    check_and_fmt_comment(n);
    fprintf(f, "while (");
    fmt_exp(f, n->sons[0]);
    fprintf(f, ") ");
    fmt_compound(f, n->sons[1], indent + 1);
}

static void fmt_for(FILE *f, Node n, int indent) {
    check_and_fmt_comment(n);
    fprintf(f, "for (");
    fmt_exp(f, n->sons[0]);
    fprintf(f, "; ");
    fmt_exp(f, n->sons[1]);
    fprintf(f, "; ");
    fmt_exp(f, n->sons[2]);
    fprintf(f, ") ");
    fmt_compound(f, n->sons[3], indent + 1);
}

// 格式化语句
static void fmt_statement(FILE *f, Node n, int indent) {
    check_and_fmt_comment(n);
    fmt_print_indent(f, indent);
    Node son = n->sons[0];
    switch (son->t) {
        case ND_EXP:
            fmt_exp(f, son);
            fprintf(f, ";");
            break;
        case ND_RETURN:
            fprintf(f, "return ");
            fmt_exp(f, son->sons[0]);
            fprintf(f, ";");
            break;
        case ND_BREAK:
            fprintf(f, "break;");
            break;
        case ND_CONTINUE:
            fprintf(f, "continue;");
            break;
        case ND_IF:
            fmt_if(f, son, indent);
            break;
        case ND_WHILE:
            fmt_while(f, son, indent);
            break;
        case ND_FOR:
            fmt_for(f, son, indent);
            break;
        default:
            panic("wrong statement");
            break;
    }
    newline(f, indent);
}

// 格式化局部变量定义序列
static void fmt_local_var_decl_cons(FILE *f, Node n, int indent) {
    check_and_fmt_comment(n);
    if (n->t != ND_EMPTY) {
        fmt_local_var_decl(f, n->sons[0], indent);
        if (n->num > 1) {
            fmt_local_var_decl_cons(f, n->sons[1], indent);
        }
    }
}

// 格式化语句序列
static void fmt_statement_cons(FILE *f, Node n, int indent) {
    check_and_fmt_comment(n);
    fmt_statement(f, n->sons[0], indent);
    if (n->num > 1) {
        fmt_statement_cons(f, n->sons[1], indent);
    }
}

// 格式化复合语句
static void fmt_compound(FILE *f, Node n, int indent) {
    check_and_fmt_comment(n);
    fprintf(f, "{");
    newline(f, indent);
    fmt_local_var_decl_cons(f, n->sons[0], indent); // 局部变量定义序列
    if (n->num > 1) {
        fmt_statement_cons(f, n->sons[1], indent); // 语句序列
    }
    fmt_print_indent(f, indent - 1);
    fprintf(f, "}");
}

