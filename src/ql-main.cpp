#include <cstring>
#include <format.h>
#include <qisslex.h>
#include <o.h>
#include <objectio.h>
#include <primio.h>
#include <token.h>
#include <unistd.h>

namespace {
}

int main() {
    yyscan_t scanner;
    yylex_init(&scanner);
    Token token = NEWLINE;
    while (token == NEWLINE) {
        if (isatty(0)) H(1) << "  " << flush;
        while ((token = Token(yylex(scanner))) &&
               token != NEWLINE && token != QUIT) {
            const char* const text = yyget_text(scanner);
            L<C> s;
            s << token;
            while (s.size() < 10) s.push_back(C(' '));
            if (yylval) H(1) << s << "| " << O(yylval) << '\n' << flush;
            else        H(1) << s << "| " << text      << '\n' << flush;
        }
    }
    yylex_destroy(scanner);
}
