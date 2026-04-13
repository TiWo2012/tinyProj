#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const char *s;
} Parser;

void skip(Parser *p) {
  while (isspace(*p->s))
    p->s++;
}

int match(Parser *p, char c) {
  skip(p);
  if (*p->s == c) {
    p->s++;
    return 1;
  }
  return 0;
}

double parseExpr(Parser *p);

double parseNumber(Parser *p) {
  skip(p);
  char *end;
  double val = strtod(p->s, &end);
  if (end == p->s) {
    printf("Parse error: expected number\n");
    exit(1);
  }
  p->s = end;
  return val;
}

double parsePrimary(Parser *p) {
  skip(p);

  if (match(p, '(')) {
    double val = parseExpr(p);
    if (!match(p, ')')) {
      printf("Parse error: missing ')'\n");
      exit(1);
    }
    return val;
  }

  return parseNumber(p);
}

double parseUnary(Parser *p) {
  skip(p);

  if (match(p, '+')) {
    return parseUnary(p);
  }

  if (match(p, '-')) {
    return -parseUnary(p);
  }

  return parsePrimary(p);
}

double parsePower(Parser *p) {
  double base = parseUnary(p);

  skip(p);
  if (match(p, '^')) {
    double exp = parsePower(p); // right-associative
    return pow(base, exp);
  }

  return base;
}

double parseTerm(Parser *p) {
  double val = parsePower(p);

  while (1) {
    skip(p);

    if (match(p, '*')) {
      val *= parsePower(p);

    } else if (match(p, '/')) {
      double d = parsePower(p);
      if (d == 0) {
        printf("Math error: division by zero\n");
        exit(1);
      }
      val /= d;

    } else if (match(p, '%')) {
      double rhs = parsePower(p);

      val = fmod(val, rhs);
    } else {
      break;
    }
  }

  return val;
}

double parseExpr(Parser *p) {
  double val = parseTerm(p);

  while (1) {
    skip(p);

    if (match(p, '+')) {
      val += parseTerm(p);
    } else if (match(p, '-')) {
      val -= parseTerm(p);
    } else {
      break;
    }
  }

  return val;
}

double eval(const char *s) {
  Parser p = {s};
  double result = parseExpr(&p);

  skip(&p);
  if (*p.s != '\0') {
    printf("Parse error: unexpected input '%c'\n", *p.s);
    exit(1);
  }

  return result;
}

int main(int argc, char **argv) {
  char buffer[256];

  // CLI mode
  if (argc > 1) {
    printf("%f\n", eval(argv[1]));
    return 0;
  }

  // REPL mode
  printf("C Calculator (type 'exit' to quit)\n");

  while (1) {
    printf("> ");
    if (!fgets(buffer, sizeof(buffer), stdin))
      break;

    buffer[strcspn(buffer, "\n")] = 0;

    if (strcmp(buffer, "exit") == 0)
      break;
    if (strlen(buffer) == 0)
      continue;

    printf("%f\n", eval(buffer));
  }

  return 0;
}
