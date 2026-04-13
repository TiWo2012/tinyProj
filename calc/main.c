#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const char *s;
  int error;
} Parser;

int decimal_precision = 3;

double last_result = 0;
double registers[9] = {0};
int has_last_result = 0;

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
void printResult(double result);

double parseNumber(Parser *p) {
  skip(p);
  char *end;
  double val = strtod(p->s, &end);
  if (end == p->s) {
    printf("Parse error: expected number\n");
    p->error = 1;
    return 0;
  }
  p->s = end;
  return val;
}

int isIdentStart(char c) { return isalpha(c) || c == '_'; }

int isIdentChar(char c) { return isalnum(c) || c == '_'; }

const char *parseIdent(Parser *p) {
  skip(p);
  const char *start = p->s;
  if (!isIdentStart(*p->s)) {
    return NULL;
  }
  while (isIdentChar(*p->s)) {
    p->s++;
  }
  return start;
}

double parseFunction(Parser *p, const char *name) {
  if (strcmp(name, "CA") == 0) {
    if (!match(p, '(')) {
      printf("Parse error: expected '(' after function '%s'\n", name);
      p->error = 1;
      return 0;
    }
    if (!match(p, ')')) {
      printf("Parse error: missing ')' after function '%s'\n", name);
      p->error = 1;
      return 0;
    }
    // Clear all registers and last result
    for (int i = 0; i < 9; i++) {
      registers[i] = 0;
    }
    last_result = 0;
    has_last_result = 0;
    printf("All registers and last result cleared\n");
    return 0;
  }

  if (!match(p, '(')) {
    printf("Parse error: expected '(' after function '%s'\n", name);
    p->error = 1;
    return 0;
  }
  double arg = parseExpr(p);
  if (p->error)
    return 0;
  if (!match(p, ')')) {
    printf("Parse error: missing ')' after function '%s'\n", name);
    p->error = 1;
    return 0;
  }

  if (strcmp(name, "sqrt") == 0)
    return sqrt(arg);
  if (strcmp(name, "sin") == 0)
    return sin(arg);
  if (strcmp(name, "cos") == 0)
    return cos(arg);
  if (strcmp(name, "tan") == 0)
    return tan(arg);
  if (strcmp(name, "asin") == 0)
    return asin(arg);
  if (strcmp(name, "acos") == 0)
    return acos(arg);
  if (strcmp(name, "atan") == 0)
    return atan(arg);
  if (strcmp(name, "sinh") == 0)
    return sinh(arg);
  if (strcmp(name, "cosh") == 0)
    return cosh(arg);
  if (strcmp(name, "tanh") == 0)
    return tanh(arg);
  if (strcmp(name, "log") == 0)
    return log(arg);
  if (strcmp(name, "log10") == 0)
    return log10(arg);
  if (strcmp(name, "exp") == 0)
    return exp(arg);
  if (strcmp(name, "abs") == 0)
    return fabs(arg);
  if (strcmp(name, "floor") == 0)
    return floor(arg);
  if (strcmp(name, "ceil") == 0)
    return ceil(arg);
  if (strcmp(name, "round") == 0)
    return round(arg);
  if (strcmp(name, "deg") == 0)
    return arg * 180.0 / M_PI;
  if (strcmp(name, "rad") == 0)
    return arg * M_PI / 180.0;
  if (strcmp(name, "dp") == 0) {
    decimal_precision = (int)arg;
    return 0;
  }

  printf("Parse error: unknown function '%s'\n", name);
  p->error = 1;
  return 0;
}

double parsePrimary(Parser *p) {
  skip(p);

  if (match(p, '(')) {
    double val = parseExpr(p);
    if (p->error)
      return 0;
    if (!match(p, ')')) {
      printf("Parse error: missing ')'\n");
      p->error = 1;
      return 0;
    }
    return val;
  }

  const char *ident = parseIdent(p);
  if (ident) {
    size_t len = p->s - ident;
    skip(p);
    if (*p->s == '(') {
      char name[32];
      if (len >= sizeof(name)) {
        printf("Parse error: function name too long\n");
        exit(1);
      }
      memcpy(name, ident, len);
      name[len] = '\0';
      return parseFunction(p, name);
    } else {
      if (strncmp(ident, "pi", len) == 0)
        return M_PI;
      if (strncmp(ident, "e", len) == 0)
        return M_E;
      if (strncmp(ident, "_", len) == 0) {
        if (!has_last_result) {
          printf("Parse error: no previous result\n");
          p->error = 1;
          return 0;
        }
        return last_result;
      }
      if (len >= 2 && strncmp(ident, "_", 1) == 0 && isdigit(ident[1])) {
        int reg_num = ident[1] - '1';
        if (reg_num >= 0 && reg_num < 9 && len == 2) {
          return registers[reg_num];
        }
      }
      printf("Parse error: unknown identifier\n");
      p->error = 1;
      return 0;
    }
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
      if (p->error)
        return 0;
      if (d == 0) {
        printf("Math error: division by zero\n");
        p->error = 1;
        return 0;
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

int parseAssignment(Parser *p) {
  const char *start = p->s;

  // Check for _<number>= pattern
  if (*p->s == '_') {
    p->s++;
    if (isdigit(*p->s)) {
      int reg_num = *p->s - '1'; // Convert '1'-'9' to 0-8
      if (reg_num >= 0 && reg_num < 9) {
        p->s++;
        skip(p);
        if (*p->s == '=') {
          p->s++;
          double value = parseExpr(p);
          if (!p->error) {
            registers[reg_num] = value;
            printf("_%.0f = ", (double)(reg_num + 1));
            printResult(value);
            return 1; // Assignment successful
          }
        }
      }
    }
  }

  p->s = start;
  return 0; // Not an assignment
}

double eval(const char *s) {
  Parser p = {s, 0};

  // Check if this is an assignment first
  if (parseAssignment(&p)) {
    return 0; // Assignment handled, return dummy value
  }

  p.s = s;
  p.error = 0;
  double result = parseExpr(&p);

  if (p.error)
    return 0;
  skip(&p);
  if (*p.s != '\0') {
    printf("Parse error: unexpected input '%c'\n", *p.s);
    return 0;
  }

  // Store as last result if not an assignment
  last_result = result;
  has_last_result = 1;
  return result;
}

void printResult(double result) {
  if (fabs(result) >= 1e6 || (fabs(result) < 1e-6 && result != 0.0)) {
    printf("%.*e\n", decimal_precision, result);
  } else {
    printf("%.*f\n", decimal_precision, result);
  }
}

int main(int argc, char **argv) {
  char buffer[256];

  // CLI mode
  if (argc > 1) {
    printResult(eval(argv[1]));
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
    if (strcmp(buffer, "clear") == 0) {
      // Clear terminal screen
      printf("\033[2J\033[H");
      continue;
    }
    if (strlen(buffer) == 0)
      continue;

    // Check for CA() function
    if (strcmp(buffer, "CA()") == 0) {
      eval(buffer);
      continue;
    }

    Parser test_p = {buffer, 0};
    int is_assignment = parseAssignment(&test_p);
    if (!is_assignment) {
      double result = eval(buffer);
      Parser p = {buffer, 0};
      parseExpr(&p);
      if (!p.error) {
        printResult(result);
      }
    }
  }

  return 0;
}
