#include "parser.h"
#include <getopt.h>

void help(void) {
    unimplemented("thers is no manual now");
}

int main(int argc, char *argv[]) {
    FILE *fp = stdout;
    bool oflag = false;
    int opt;
    while ((opt = getopt(argc, argv, "ho:t")) != -1) {
        switch (opt) {
        case 'h':
            help();
            exit(0);
        case 'o':
            oflag = true;
            fp = fopen(optarg, "w");
            if (fp == NULL) {
                fprintf(stderr, "informater: cannot open %s\n", optarg);
                exit(1);
            }
            break;
        case 't':
            unimplemented("AST is not supported");
            break;
        default:
            break;
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "informater: expected argument\n");
    }

    Lex_TokenState st = Lex_lexer_go(argv[optind]);
    LexS_display(fp, &st);

    LexS_drop(&st);

    if (oflag) {
        fclose(fp);
    }
    return 0;
}
