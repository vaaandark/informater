#include "parser.h"

#define X(a, b) b,
const char *node_type_strings[] = { NodeType_enum };
#undef X

static Node parse_trans_unit(TokenStream *);

Node parser_go(Lex_TokenState st) {
    TokenStream s = TS_new(st);
    return parse_trans_unit(&s);
}

// 创建一个 Token 流
TokenStream TS_new(Lex_TokenState st) {
    return (TokenStream) {
        .tokens = st.tokens,
        .pos = 0,
        .size = st.num
    };
}

// 从 Token 流中读出一个 Token
static inline Lex_Token TS_get_token(TokenStream *s) {
    return s->tokens[s->pos++];
}

// 判断是否是 Token 流结尾
static inline bool TS_end_of_token(TokenStream *s) {
    return s->pos >= s->size;
}

// 判断向后第 n 个字符是否是 Token 流结尾
static inline bool TS_nend_of_token(TokenStream *s, int n) {
    return s->pos + n >= s->size;
}

// 向前看第 n 个 Token 且不消耗 Token
static inline Lex_Token TS_peek(TokenStream *s, int n) {
    if (s->pos + n > s->size) {
        panic("out of range, meet eof too early");
    }
    return s->tokens[s->pos + n];
}

// 判断 Token 是否属于提供种类
static inline bool in_range(Lex_TokenType t, const Lex_TokenType *range,
        int size) {
    for (int i = 0; i < size; ++i) {
        if (t == range[i]) {
            return true;
        }
    }
    return false;
}

const Lex_TokenType type_spec_range[] = {
    INT_T, FLOAT_T, CHAR_T, CHAR_T, DOUBLE_T,
    FLOAT_T, INT_T, LONG_T, SHORT_T, VOID_T
};

const Lex_TokenType exp_end_range[] = {
    RIGHT_SQUARE_BRACKET_T, RIGHT_BRACE_T, RIGHT_PARETHESIS_T, SEMICOLON_T, COMMA_T
};

#define in_range(t, range) \
    in_range(t, range, (int)(sizeof(range) / sizeof(range[0])))

// 创建一个 AST 节点
static Node Node_new(void) {
    Node n = (Node)malloc(sizeof(struct node));
    n->num = 0;
    n->t = ND_UNDEFINED;
    n->comment = NULL;
    return n;
}

void Node_drop(Node n) {
    if (!n->is_leaf) {
        for (int i = 0; i < n->num; ++i) {
            Node_drop(n->sons[i]);
        }
        if (n->comment != NULL) {
            Node_drop(n->comment);
        }
    }
    free(n);
}

// 创建一个 AST 非叶子节点
static Node Node_new_normal(NodeType t) {
    Node n = Node_new();
    n->is_leaf = false;
    n->t = t;
    return n;
}

// 创建一个 AST 叶子节点
static Node Node_new_leaf(Lex_Token t) {
    Node n = (Node)malloc(sizeof(struct node));
    n->is_leaf = true;
    n->token = t;
    return n;
}

// 给节点 n 添加儿子节点 s
static void Node_add_son(Node n, Node s) {
    if (n->num == SONSMAX) {
        panic("too many sons for a node");
    }
    n->sons[n->num++] = s;
}


static Node parse_external_decl_cons(TokenStream *);

// 解析程序
// 程序 ::= 外部定义序列
// 有一个儿子节点
static Node parse_trans_unit(TokenStream *s) {
//    puts("call parse_trans_unit");
    Node res = Node_new_normal(ND_TRANS_UNIT);
    if (TS_end_of_token(s)) {
        fmt_panic(0, 0, "EOF", "empty file");
    }
    Node_add_son(res, parse_external_decl_cons(s));
    return res;
}

static Node parse_func_decl_or_def(TokenStream *);
static Node parse_external_var_decl(TokenStream *);

// 解析预处理语句
static Node parse_preprocess(TokenStream *s) {
//    puts("call parse_preprocess");
    Node res = Node_new_normal(ND_PREPROCESS);
    Node_add_son(res, Node_new_leaf(TS_get_token(s)));
    return res;
}

// 解析注释
static Node parse_comment(TokenStream *s) {
    Node res = Node_new_normal(ND_COMMENT);
    Node_add_son(res, Node_new_leaf(TS_get_token(s)));
    return res;
}

// 解析注释序列
static Node parse_comment_cons(TokenStream *s) {
    Node res = Node_new_normal(ND_COMMENT_CONS);
    Node_add_son(res, parse_comment(s));
    if (TS_peek(s, 0).type == COMMENT_T) {
        Node_add_son(res, parse_comment_cons(s));
    }
    return res;
}

// 向 AST 插入注释
static void check_and_add_comment(Node n, TokenStream *s) {
    if (TS_peek(s, 0).type == COMMENT_T) {
        n->comment = parse_comment_cons(s);
    }
}

// 解析外部定义
// 外部定义 ::= 外部变量定义 |
//              函数定义 |
//              函数声明 |
//              预处理语句
// 有一个儿子节点
static Node parse_external_decl(TokenStream *s) {
    Node res = Node_new_normal(ND_EXTERNAL_DECL);
    check_and_add_comment(res, s);
    Lex_Token t = TS_peek(s, 0);
    switch (t.type) {
        case PREPROCESSOR_COMMAND_T: // 预处理
            Node_add_son(res, parse_preprocess(s));
            break;
        case IDENTIFIER_T: // 自定义类型或别名
            unimplemented("user defined type or typedef");
            break;
        case CHAR_T:
        case DOUBLE_T:
        case FLOAT_T:
        case INT_T:
        case LONG_T:
        case SHORT_T:
        case VOID_T: // 函数定义（声明）或外部变量声明
            if (!TS_nend_of_token(s, 3) && TS_peek(s, 2).type == LEFT_PARETHESIS_T) {
                Node_add_son(res, parse_func_decl_or_def(s));
            } else {
                Node_add_son(res, parse_external_var_decl(s));
            }
            break;
        default:
            fmt_panic(t.line, t.column, t.str, "wrong external declaration");
            break;
    }
    return res;
}

// 解析外部定义序列
// 外部定义序列 ::= 外部定义 外部定义序列 | 外部定义
// 以一个广义表的二叉树形式存储
// 有一个或者两个儿子节点
static Node parse_external_decl_cons(TokenStream *s) {
    Node res = Node_new_normal(ND_EXTERNAL_DECL_CONS);
    check_and_add_comment(res, s);
    Node_add_son(res, parse_external_decl(s));
    if (!TS_end_of_token(s)) {
        Node_add_son(res, parse_external_decl_cons(s));
    }
    return res;
}

static Node parse_var_cons(TokenStream *);

// 解析外部变量定义
// 外部变量定义 ::= 类型说明符 变量序列;
// 有两个儿子节点
static Node parse_external_var_decl(TokenStream *s) {
    Node res = Node_new_normal(ND_EXTERNAL_VAR_DECL);
    check_and_add_comment(res, s);
    if (!in_range(TS_peek(s, 0).type, type_spec_range)) {
        res->t = ND_EMPTY;
    } else {
        Node_add_son(res, Node_new_leaf(TS_get_token(s)));
        Node_add_son(res, parse_var_cons(s));
        Lex_Token t;
        if ((t = TS_get_token(s)).type != SEMICOLON_T) {
            fmt_panic_with_expect(t.line, t.column, t.str, ";",
                    "wrong external variable declaration");
        }
    }
    return res;
}

static Node parse_array(TokenStream *s);

// 解析变量序列
// 变量序列 ::= 变量, 变量序列 | 变量
// 以一个广义表的二叉树形式存储
//      变量 ::= 标识符
// 有一个或者两个儿子节点
static Node parse_var_cons(TokenStream *s) {
    Node res = Node_new_normal(ND_VAR_CONS);
    check_and_add_comment(res, s);
    if (TS_peek(s, 1).type == LEFT_SQUARE_BRACKET_T) {
        Node_add_son(res, parse_array(s));
    } else {
        Node_add_son(res, Node_new_leaf(TS_get_token(s)));
    }
    Lex_Token t = TS_peek(s, 0);
    if (t.type == COMMA_T) {
        TS_get_token(s); // 吞掉','
        if ((t = TS_peek(s, 0)).type != IDENTIFIER_T) {
            fmt_panic_with_expect(t.line, t.column, t.str, "indentifier",
                    "wrong variable declaration");
        }
        Node_add_son(res, parse_var_cons(s));
    }
    return res;
}

static Node parse_arg_cons(TokenStream *s);
static Node parse_statement_cons(TokenStream *s);
static Node parse_compound(TokenStream *s);

// 解析函数定义或函数声明
// 函数定义 ::= 类型说明符 函数名 (形式参数列表) 复合语句
// 对应地，函数定义节点有以上四个子节点
//      类型说明符 ::= int | float | char
//      函数名 ::= 标识符
static Node parse_func_decl_or_def(TokenStream *s) {
    Node res = Node_new_normal(ND_FUNC_DEF);
    check_and_add_comment(res, s);

    Node_add_son(res, Node_new_leaf(TS_get_token(s)));
    Node_add_son(res, Node_new_leaf(TS_get_token(s)));

    Lex_Token t = TS_get_token(s);
    if (t.type != LEFT_PARETHESIS_T) {
        fmt_panic_with_expect(t.line, t.column, t.str,
                "(", "wrong function declaration");
    }
    Node_add_son(res, parse_arg_cons(s));

    t = TS_get_token(s);
    if (t.type != RIGHT_PARETHESIS_T) { // 吞掉 ')'
        fmt_panic_with_expect(t.line, t.column, t.str,
                ")", "wrong function declaration");
    }

    t = TS_peek(s, 0);
    if (t.type == SEMICOLON_T) {
        TS_get_token(s);
        res->t = ND_FUNC_DECL;
    } else if (t.type == LEFT_BRACE_T) {
        Node_add_son(res, parse_compound(s));
    } else {
        fmt_panic_with_expect(t.line, t.column, t.str,
                "'{' or ';'", "wrong function declaration or define");
    }
    return res;
}

// 解析形式参数
// 形式参数 ::= 类型说明符 标识符
// 有两个儿子节点，都是叶子节点
static Node parse_arg(TokenStream *s) {
    Node res = Node_new_normal(ND_ARG);
    check_and_add_comment(res, s);
    Lex_Token t = TS_peek(s, 0);
    Node_add_son(res, Node_new_leaf(TS_get_token(s)));
    if (t.type != VOID_T) {
        if (TS_peek(s, 1).type == LEFT_SQUARE_BRACKET_T) { // 形参中有数组
            Node_add_son(res, parse_array(s));
        } else {
            Node_add_son(res, Node_new_leaf(TS_get_token(s)));
        }
    }
    return res;
}

// 解析形式参数序列
// 形式参数序列 ::= 形式参数, 形式参数序列 | 空
// 有一个或者两个儿子节点
static Node parse_arg_cons(TokenStream *s) {
    Node res = Node_new_normal(ND_ARG_CONS);
    check_and_add_comment(res, s);
    Lex_Token t = TS_peek(s, 0);
    if (in_range(t.type, type_spec_range)) {
        Node_add_son(res, parse_arg(s));
    } else {
        fmt_panic(t.line, t.column, t.str, "wrong arguments");
    }
    t = TS_peek(s, 0);
    if (t.type == COMMA_T) {
        TS_get_token(s);
        Node_add_son(res, parse_arg_cons(s));
    }
    return res;
}

static Node parse_local_var_decl(TokenStream *);
static Node parse_local_var_decl_cons(TokenStream *s);

// 解析复合语句
// 复合语句 ::= { 局部变量定义序列 语句序列 }
// 有两个儿子节点
static Node parse_compound(TokenStream *s) {
    Node res = Node_new_normal(ND_COMPOUND);
    check_and_add_comment(res, s);
    Lex_Token t = TS_get_token(s);
    if (t.type != LEFT_BRACE_T) {
        fmt_panic_with_expect(t.line, t.column, t.str,
                "{", "wrong compoud statement");
    }
    Node_add_son(res, parse_local_var_decl_cons(s));
    if (TS_peek(s, 0).type != RIGHT_BRACE_T) {
        Node_add_son(res, parse_statement_cons(s));
    }
    if ((t = TS_get_token(s)).type != RIGHT_BRACE_T) { // 吞掉 '}'
        fmt_panic_with_expect(t.line, t.column, t.str,
                "}", "wrong compoud statement");
    }
    return res;
}

// 解析局部变量定义序列
// 局部变量定义序列 ::= 局部变量定义 局部变量定义序列 | 空
// 有一个或者两个儿子节点
static Node parse_local_var_decl_cons(TokenStream *s) {
    Node res = Node_new_normal(ND_LOCAL_VAR_DECL_CONS);
    check_and_add_comment(res, s);
    if (!in_range(TS_peek(s, 0).type, type_spec_range)) {
        res->t = ND_EMPTY;
    } else {
        Lex_Token t = TS_peek(s, 0);
        if (in_range(t.type, type_spec_range)) {
            Node_add_son(res, parse_local_var_decl(s));
        } else {
            fmt_panic(t.line, t.column, t.str, "wrong local variable declaration");
        }
        t = TS_peek(s, 0);
        if (in_range(t.type, type_spec_range)) {
            Node_add_son(res, parse_local_var_decl_cons(s));
        }
    }
    return res;
}

// 解析局部变量定义
// 局部变量定义 ::= 类型说明符 变量序列 ;
// 有两个儿子节点
static Node parse_local_var_decl(TokenStream *s) {
    Node res = Node_new_normal(ND_LOCAL_VAR_DECL);
    check_and_add_comment(res, s);
    Node_add_son(res, Node_new_leaf(TS_get_token(s)));
    Node_add_son(res, parse_var_cons(s));
    TS_get_token(s); // 吞掉 ';'
    return res;
}

static Node parse_statement(TokenStream *);

// 解析语句序列
// 语句序列 ::= 语句 语句序列 | 空
// 有一个或者两个儿子节点
static Node parse_statement_cons(TokenStream *s) {
    Node res = Node_new_normal(ND_STATEMENT_CONS);
    check_and_add_comment(res, s);
    Node_add_son(res, parse_statement(s));
    Lex_Token t = TS_peek(s, 0);
    if (!TS_end_of_token(s) && t.type != RIGHT_BRACE_T) {
        Node_add_son(res, parse_statement_cons(s));
    }
    return res;
}

static Node parse_exp(TokenStream *);

// 解析返回语句
static Node parse_return(TokenStream *s) {
    Node res = Node_new_normal(ND_RETURN);
    check_and_add_comment(res, s);
    TS_get_token(s);
    Lex_Token t = TS_peek(s, 0);
    if (t.type != SEMICOLON_T) {
        Node_add_son(res, parse_exp(s));
    }
    return res;
}

// 解析 if 语句
static Node parse_if(TokenStream *s) {
    Node res = Node_new_normal(ND_IF);
    check_and_add_comment(res, s);

    TS_get_token(s);
    Lex_Token t = TS_get_token(s);
    if (t.type != LEFT_PARETHESIS_T) {
        fmt_panic_with_expect(t.line, t.column, t.str,
                "(", "wrong function declaration");
    }

    Node_add_son(res, parse_exp(s));

    t = TS_get_token(s);
    if (t.type != RIGHT_PARETHESIS_T) {
        fmt_panic_with_expect(t.line, t.column, t.str,
                ")", "wrong function declaration");
    }

    Node_add_son(res, parse_compound(s));

    t = TS_peek(s, 0);
    if (t.type == ELSE_T) {
        TS_get_token(s);
        Node_add_son(res, parse_compound(s));
    }

    return res;
}

// 解析 while
static Node parse_while(TokenStream *s) {
    Node res = Node_new_normal(ND_WHILE);
    check_and_add_comment(res, s);
    TS_get_token(s);
    
    Lex_Token t = TS_get_token(s);
    if (t.type != LEFT_PARETHESIS_T) {
        fmt_panic_with_expect(t.line, t.column, t.str,
                "(", "wrong function declaration");
    }

    Node_add_son(res, parse_exp(s));

    t = TS_get_token(s);
    if (t.type != RIGHT_PARETHESIS_T) {
        fmt_panic_with_expect(t.line, t.column, t.str,
                ")", "wrong function declaration");
    }

    Node_add_son(res, parse_compound(s));

    return res;
}

// 解析 for
static Node parse_for(TokenStream *s) {
    Node res = Node_new_normal(ND_FOR);
    check_and_add_comment(res, s);
    TS_get_token(s);
    
    Lex_Token t = TS_get_token(s);
    if (t.type != LEFT_PARETHESIS_T) {
        fmt_panic_with_expect(t.line, t.column, t.str,
                "(", "wrong 'for' statement");
    }

    for (int i = 0; i < 3; ++i) {
        if ((i < 2 && TS_peek(s, 0).type == SEMICOLON_T) ||
                (i == 2 && TS_peek(s, 0).type == RIGHT_PARETHESIS_T)) {
            Node_add_son(res, Node_new_normal(ND_EMPTY));
        } else {
            Node_add_son(res, parse_exp(s));
        }

        if (i < 2) {
            t = TS_get_token(s);
            if (t.type != SEMICOLON_T) {
                fmt_panic_with_expect(t.line, t.column, t.str,
                        ";", "wrong 'for' statement");
            }
        }
    }

    t = TS_get_token(s);
    if (t.type != RIGHT_PARETHESIS_T) {
        fmt_panic_with_expect(t.line, t.column, t.str,
                ")", "wrong function declaration");
    }

    Node_add_son(res, parse_compound(s));

    return res;
}

// 解析语句
// 语句 ::= 表达式; |
//          return 表达式 ; |
//          if (表达式) 复合语句 |
//          if (表达式) 复合语句 else 复合语句 |
//          while (表达式) 复合语句 |
//          for (表达式; 表达式; 表达式) 复合语句 |
//          break; | continue;
static Node parse_statement(TokenStream *s) {
    Node res = Node_new_normal(ND_STATEMENT);
    check_and_add_comment(res, s);
    Lex_Token t;
    switch (TS_peek(s, 0).type) {
        case RETURN_T:
            Node_add_son(res, parse_return(s));
            goto swallow_semi;
            break;
        case IF_T:
            Node_add_son(res, parse_if(s));
            break;
        case WHILE_T:
            Node_add_son(res, parse_while(s));
            break;
        case FOR_T:
            Node_add_son(res, parse_for(s));
            break;
        case BREAK_T:
            TS_get_token(s);
            Node_add_son(res, Node_new_normal(ND_BREAK));
            goto swallow_semi;
            break;
        case CONTINUE_T:
            TS_get_token(s);
            Node_add_son(res, Node_new_normal(ND_CONTINUE));
            goto swallow_semi;
            break;
        default:
            Node_add_son(res, parse_exp(s));
swallow_semi:
            t = TS_get_token(s);
            if (t.type != SEMICOLON_T) {
                fmt_panic_with_expect(t.line, t.column, t.str,
                        ";", "wrong statement");
            }
            break;
    }
    return res;
}

static Node parse_real_arg_cons(TokenStream *);

// 解析函数调用
static Node parse_func_call(TokenStream *s) {
    Node res = Node_new_normal(ND_FUNC_CALL);
    check_and_add_comment(res, s);
    Node_add_son(res, Node_new_leaf(TS_get_token(s)));

    Lex_Token t = TS_get_token(s);
    if (t.type != LEFT_PARETHESIS_T) {
        fmt_panic_with_expect(t.line, t.column, t.str,
                "(", "wrong function call");
    }

    Node_add_son(res, parse_real_arg_cons(s));

    t = TS_get_token(s);
    if (t.type != RIGHT_PARETHESIS_T) {
        fmt_panic_with_expect(t.line, t.column, t.str,
                ")", "wrong function call");
    }

    return res;
}

//// 解析赋值语句
//Node parse_assignment(TokenStream *s) {
//    Node res = Node_new_normal(ND_ASSIGNMENT);
//    Node_add_son(res, Node_new_leaf(TS_get_token(s)));
//    TS_get_token(s);
//    Node_add_son(res, parse_exp(s));
//    return res;
//}

// 解析数组
static Node parse_array(TokenStream *s) {
    Node res = Node_new_normal(ND_ARRAY);
    check_and_add_comment(res, s);
    Node_add_son(res, Node_new_leaf(TS_get_token(s)));
    TS_get_token(s);
    if (TS_peek(s, 0).type == RIGHT_SQUARE_BRACKET_T) {
        Node_add_son(res, Node_new_normal(ND_EMPTY));
    } else {
        Node_add_son(res, parse_exp(s));
    }
    Lex_Token t = TS_get_token(s);
    if (t.type != RIGHT_SQUARE_BRACKET_T) {
        fmt_panic_with_expect(t.line, t.column, t.str,
                "]", "wrong array");
    }

    return res;
}

// 解析简单表达式
// 简单表达式 ::= 常量数字 | 标识符 |
//                标识符(实参序列) | 标识符[表达式] |
static Node parse_simple_exp(TokenStream *s) {
    Node res = Node_new_normal(ND_EXP);
    check_and_add_comment(res, s);
    Lex_Token t;
    switch (TS_peek(s, 0).type) {
        case HEX_FLOAT_CONST_T:
        case DEC_FLOAT_T:
        case DEC_CONST_T:
        case OCT_CONST_T:
        case HEX_CONST_T:
        case STR_LITERAL_T:
        case CHARACTER_T: // 常量数字
            Node_add_son(res, Node_new_leaf(TS_get_token(s)));
            break;
        case IDENTIFIER_T:
            // TODO
            if (TS_peek(s, 1).type == LEFT_PARETHESIS_T) { // 函数调用
                Node_add_son(res, parse_func_call(s));
            } else if (TS_peek(s, 1).type == LEFT_SQUARE_BRACKET_T) { // 数组
                Node_add_son(res, parse_array(s));
            } else { // 只有一个标识符
                Node_add_son(res, Node_new_leaf(TS_get_token(s)));
            }
            break;
        default:
            t = TS_get_token(s);
            fmt_panic(t.line, t.column, t.str, "wrong expression");
            break;
    }
    return res;
}

// 解析表达式
// 表达式 ::= 表达式 双目运算符号 表达式 |
//            常量数字 | 标识符 |
//            标识符(实参序列) |
//            标识符[表达式] |
//            标识符 = 表达式
//      双目运算符号 ::= + | - | * | / | % |
//                       == | != | > | < |
//                       >= | <=
static Node parse_exp(TokenStream *s) {
    Node res = NULL;
    Node exp = parse_simple_exp(s);
    check_and_add_comment(exp, s);
    Lex_Token t = TS_peek(s, 0);
    if (!in_range(t.type, exp_end_range)) {
        res = Node_new_normal(ND_EXP);
        Node_add_son(res, exp);
        Node_add_son(res, Node_new_leaf(TS_get_token(s)));
        Node_add_son(res, parse_exp(s));
    } else {
        res = exp;
    }
    return res;
}

// 解析实参序列
// 实参序列 ::= 表达式, 实参序列 | 空
// 有一个或者两个儿子节点
static Node parse_real_arg_cons(TokenStream *s) {
    Node res = Node_new_normal(ND_REAL_ARG_CONS);
    check_and_add_comment(res, s);
    // TODO
    Node_add_son(res, parse_exp(s));
    if (TS_peek(s, 0).type == COMMA_T) {
        TS_get_token(s);
        Node_add_son(res, parse_real_arg_cons(s));
    }
    return res;
}

