%option noyywrap

%%
a*b { printf("Token found\n"); }
%%

int main() { yylex(); }