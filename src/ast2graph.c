#include "parser.h"

void Node_display_simply(Node n, int indent) {
    for (int i = 0; i < indent; ++i) {
        printf("  ");
    }
    if (n->is_leaf) {
        printf("%s\n", n->token.str);
    } else {
        printf("%s\n", node_type_strings[n->t]);
        for (int i = 0; i < n->num; ++i) {
            Node_display_simply(n->sons[i], indent + 1);
        }
    }
}

void print_string(FILE *f, char *s) {
    while (*s != '\0') {
        if (*s == '\"') {
            fputs("\\\"", f);
        } else {
            fputc(*s, f);
        }
        s++;
    }
}

void Node_display(FILE *f, Node n) {
    if (n->is_leaf) {
        fprintf(f, "    nd%p [label=\"", n);
        print_string(f, n->token.str);
        fprintf(f, "\" style=filled];\n");
    } else {
        fprintf(f, "    nd%p [label=\"%s\"];\n",
                n, node_type_strings[n->t]);
        for (int i = 0; i < n->num; ++i) {
            fprintf(f, "    nd%p->nd%p;\n", n, n->sons[i]);
            Node_display(f, n->sons[i]);
        }
    }
}

void AST2graph(FILE *f, Node n) {
    fprintf(f, "digraph AST {\n");
    fprintf(f, "    label=\"抽象语法树\"");
    fprintf(f, "    labelloc=top;\n");
    fprintf(f, "    labeljust=left;\n");
    Node_display(f, n);
    fprintf(f, "}\n");
}

int main(int argc, char *argv[]) {
    assert(argc == 2);
    Lex_TokenState st = Lex_lexer_go(argv[1]);
//    LexS_display(stdout, &st);
    Node n = parser_go(st);
    FILE *f = fopen("./AST-graph.dot", "w");
    AST2graph(f, n);
    LexS_drop(&st);
    return 0;
}
