#include "parser.h"
#include <getopt.h>

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
        if (n->comment != NULL) {
            Node_display_simply(n->comment, indent + 1);
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
        if (n->comment != NULL) {
            fprintf(f, "    nd%p->nd%p;\n", n, n->comment);
            Node_display(f, n->comment);
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

void help(void) {
    unimplemented("thers is no manual now");
}

int main(int argc, char *argv[]) {
    FILE *fp = stdout;
    FILE *fp_dot = stdout;
    bool redirect = false;
    bool generate_ast = false;
    int opt;
    while ((opt = getopt(argc, argv, "ho:t:")) != -1) {
        switch (opt) {
        case 'h':
            help();
            exit(0);
        case 'o':
            redirect = true;
            fp = fopen(optarg, "w");
            if (fp == NULL) {
                fprintf(stderr, "informater: cannot open %s\n", optarg);
                exit(1);
            }
            break;
        case 't':
            generate_ast = true;
            fp_dot = fopen(optarg, "w");
            if (fp_dot == NULL) {
                fprintf(stderr, "informater: cannot open %s\n", optarg);
                exit(1);
            }
            break;
        default:
            break;
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "informater: expected argument\n");
    }

    Lex_TokenState st = Lex_lexer_go(argv[optind]);

    Node n = parser_go(st);
    if (generate_ast) {
        AST2graph(fp_dot, n);
        fclose(fp_dot);
    }
    Node_drop(n);

    LexS_drop(&st);

    if (redirect) {
        fclose(fp);
    }

    return 0;
}
