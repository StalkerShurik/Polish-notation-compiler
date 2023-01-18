#include "poliz.h"
#include <stdio.h>
#include <stdlib.h>

struct PolizState {
    int *ptr;
    int size;
    int capacity;
    int error;
};

typedef struct PolizState PolizState;
typedef struct PolizItem PolizItem;

const char *find_command_end(const char *begin) {
    const char *iter = begin;
    while ((*iter) != ' ' && (*iter) != '\0') {
        iter++;
    }
    return iter;
}

int count_commands(const char *begin) {
    const char *iter = begin;
    int count = 1;
    while (*iter != '\0') {
        if (*iter == ' ') {
            count++;
        }
        iter++;
    }
    return count;
}

int push(PolizState *state, int value) {
    if (state->size + 1 > state->capacity) {
        int new_capacity = 2 * (state->capacity + 1);
        int *tmp = realloc(state->ptr, new_capacity * sizeof(*tmp));
        if (!tmp) {
            state->error = PE_OUT_OF_MEMORY;
            return -PE_OUT_OF_MEMORY;
        }
        state->ptr = tmp;
        state->capacity = new_capacity;
    }
    state->ptr[state->size++] = value;
    return PE_OK;
}

int pop(PolizState *state, int *value) {
    if (state->size == 0) {
        state->error = PE_STACK_UNDERFLOW;
        return -PE_STACK_UNDERFLOW;
    } else {
        *value = state->ptr[--state->size];
        return PE_OK;
    }
}

int p_num(PolizState *state, int iextra) {
    if (state->error != PE_OK) {
        return -state->error;
    }
    return push(state, iextra);
}

enum { INT_MIN = -2147483648, INT_MAX = 2147483647 };

int check_sum_overflow(int a, int b) {
    long long c = (long long)a + (long long)b;
    return c > INT_MAX || c < INT_MIN;
}

int p_sum(PolizState *state, int iextra) {
    if (state->error != PE_OK) {
        return -state->error;
    }
    if (state->size >= 2) {
        int a, b;
        pop(state, &a);
        pop(state, &b);
        if (check_sum_overflow(a, b)) {
            state->error = PE_INT_OVERFLOW;
            return -PE_INT_OVERFLOW;
        }
        return push(state, a + b);
    } else {
        state->error = PE_STACK_UNDERFLOW;
        return -PE_STACK_UNDERFLOW;
    }
}

int check_sub_overflow(int b, int a) {
    long long c = (long long)b - (long long)a;
    return c > INT_MAX || c < INT_MIN;
}

int p_sub(PolizState *state, int iextra) {
    if (state->error != PE_OK) {
        return -state->error;
    }
    if (state->size >= 2) {
        int a, b;
        pop(state, &a);
        pop(state, &b);
        if (check_sub_overflow(b, a)) {
            state->error = PE_INT_OVERFLOW;
            return -PE_INT_OVERFLOW;
        }
        return push(state, b - a);
    } else {
        state->error = PE_STACK_UNDERFLOW;
        return -PE_STACK_UNDERFLOW;
    }
}

int check_mul_overflow(int a, int b) {
    if (a == INT_MIN && b == INT_MIN) {
        return 1;
    }
    long long c = (long long)a * (long long)b;
    return c > INT_MAX || c < INT_MIN;
}

int p_mul(PolizState *state, int iextra) {
    if (state->error != PE_OK) {
        return -state->error;
    }
    if (state->size >= 2) {
        int a, b;
        pop(state, &a);
        pop(state, &b);
        if (check_mul_overflow(a, b)) {
            state->error = PE_INT_OVERFLOW;
            return -PE_INT_OVERFLOW;
        }
        return push(state, a * b);
    } else {
        state->error = PE_STACK_UNDERFLOW;
        return -PE_STACK_UNDERFLOW;
    }
}

int check_div_overflow(int b, int a) {
    return b == INT_MIN && a == -1;
}

int custome_div(PolizState *state, int b, int a, int *c) {
    if (a == 0) {
        state->error = PE_DIVISION_BY_ZERO;
        return -PE_DIVISION_BY_ZERO;
    }
    if (check_div_overflow(b, a)) {
        state->error = PE_INT_OVERFLOW;
        return -PE_INT_OVERFLOW;
    }

    int d = b / a;
    int m = b % a;

    if (m >= 0) {
        *c = d;
        return PE_OK;
    }

    if (a > 0) {
        d--;
        m += a;
    } else {
        d++;
        m -= a;
    }
    *c = d;
    return PE_OK;
}

int p_div(PolizState *state, int iextra) {
    if (state->error != PE_OK) {
        return -state->error;
    }
    if (state->size >= 2) {
        int a, b, c;
        pop(state, &a);
        pop(state, &b);

        int error_code = custome_div(state, b, a, &c);
        if (error_code != PE_OK) {
            return error_code;
        }
        return push(state, c);
    } else {
        state->error = PE_STACK_UNDERFLOW;
        return -PE_STACK_UNDERFLOW;
    }
}

int custome_mod(PolizState *state, int b, int a, int *c) {
    if (a == 0) {
        state->error = PE_DIVISION_BY_ZERO;
        return -PE_DIVISION_BY_ZERO;
    }

    int d = b / a;
    int m = b % a;

    if (m >= 0) {
        *c = m;
        return PE_OK;
    }

    if (a > 0) {
        d--;
        m += a;
    } else {
        d++;
        m -= a;
    }
    *c = m;
    return PE_OK;
}

int p_mod(PolizState *state, int iextra) {
    if (state->error != PE_OK) {
        return -state->error;
    }
    if (state->size >= 2) {
        int a, b, c;
        pop(state, &a);
        pop(state, &b);
        int error_code = custome_mod(state, b, a, &c);
        if (error_code != PE_OK) {
            return error_code;
        }
        return push(state, c);
    } else {
        state->error = PE_STACK_UNDERFLOW;
        return -PE_STACK_UNDERFLOW;
    }
}

int check_neg_oweflow(int a) {
    return a == INT_MIN;
}

int p_neg(PolizState *state, int iextra) {
    if (state->error != PE_OK) {
        return -state->error;
    }
    if (state->size >= 1) {
        if (check_neg_oweflow(state->ptr[state->size - 1])) {
            state->error = PE_INT_OVERFLOW;
            return -PE_INT_OVERFLOW;
        }
        state->ptr[state->size - 1] = -state->ptr[state->size - 1];
        return PE_OK;
    } else {
        state->error = PE_STACK_UNDERFLOW;
        return -PE_STACK_UNDERFLOW;
    }
}

int p_read(PolizState *state, int iextra) {
    if (state->error != PE_OK) {
        return -state->error;
    }
    int value;
    if (scanf("%d", &value) != 1) {
        state->error = PE_READ_FAILED;
        return -PE_READ_FAILED;
    }
    return push(state, value);
}

int p_write(PolizState *state, int iextra) {
    if (state->error != PE_OK) {
        return -state->error;
    }
    if (state->size > 0) {
        printf("%d", state->ptr[state->size - 1]);
        int tmp;
        return pop(state, &tmp);
    } else {
        state->error = PE_STACK_UNDERFLOW;
        return -PE_STACK_UNDERFLOW;
    }
}

int p_new_line(PolizState *state, int iextra) {
    if (state->error != PE_OK) {
        return -state->error;
    }
    printf("\n");
    return PE_OK;
}

int p_pop(PolizState *state, int iextra) {
    if (state->error != PE_OK) {
        return -state->error;
    }
    int a;
    return pop(state, &a);
}

int p_dnum(PolizState *state, int iextra) {
    if (state->error != PE_OK) {
        return -state->error;
    }
    if (iextra >= state->size || iextra < 0) {
        state->error = PE_INVALID_INDEX;
        return -PE_INVALID_INDEX;
    }
    int indx = state->size - 1;
    indx -= iextra;
    return push(state, state->ptr[indx]);
}

void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int p_snum(PolizState *state, int iextra) {
    if (state->error != PE_OK) {
        return -state->error;
    }
    if (iextra == 0) {
        return PE_OK;
    }
    if (iextra >= state->size || iextra < 0) {
        state->error = PE_INVALID_INDEX;
        return -PE_INVALID_INDEX;
    }
    int indx = state->size - 1;
    indx -= iextra;
    swap(&state->ptr[state->size - 1], &state->ptr[indx]);
    return PE_OK;
}

struct Pair {
    int a;
    int b;
};

typedef struct Pair Pair;

enum {
    X0 = 0,
    X1 = 1,
    X2 = 2,
    X3 = 3,
    X4 = 4,
    X5 = 5,
    X6 = 6,
    X7 = 7,
    X8 = 8,
    X9 = 9,
    X10 = 10,
    X11 = 11,
    X12 = 12
};

Pair parse_function(const char *begin, const char *end) {
    Pair to_return = {-1, 0};
    if (end - begin == 1) {
        switch (*begin) {
        case '+':
            to_return.a = X1;
            break;
        case '-':
            to_return.a = X2;
            break;
        case '*':
            to_return.a = X3;
            break;
        case '/':
            to_return.a = X4;
            break;
        case '%':
            to_return.a = X5;
            break;
        case '#':
            to_return.a = X6;
            break;
        case 'r':
            to_return.a = X7;
            break;
        case 'w':
            to_return.a = X8;
            break;
        case 'n':
            to_return.a = X9;
            break;
        case ';':
            to_return.a = X10;
            break;
        case 'd':
            to_return.a = X11;
            to_return.b = 0;
            break;
        case 's':
            to_return.a = X12;
            to_return.b = 1;
            break;
        default:
            to_return.a = X0;
            to_return.b = (int)*begin - (int)'0';
        }
    } else {
        if (*begin == 'd') {
            char *pre_end;
            to_return.a = X11;
            to_return.b = (int)strtol(begin + 1, &pre_end, 10);
        } else if (*begin == 's') {
            char *pre_end;
            to_return.a = X12;
            to_return.b = (int)strtol(begin + 1, &pre_end, 10);
        } else {
            char *pre_end;
            to_return.a = X0;
            to_return.b = (int)strtol(begin, &pre_end, 10);
        }
    }
    return to_return;
}

enum { COMMANDS_NUMBER = 13 };

poliz_func_t adresses[COMMANDS_NUMBER] = {
    &p_num,  &p_sum,   &p_sub,      &p_mul, &p_div,  &p_mod, &p_neg,
    &p_read, &p_write, &p_new_line, &p_pop, &p_dnum, &p_snum};

struct PolizItem *poliz_compile(const char *str) {
    int count = count_commands(str);
    PolizItem *start_instruction = NULL;

    PolizItem *tmp =
        realloc(start_instruction, (count + 1) * sizeof(*start_instruction));
    if (tmp == NULL) {
        return NULL;
    } else {
        start_instruction = tmp;
    }

    const char *begin = str, *end = str;

    for (int i = 0; i < count; ++i) {
        end = find_command_end(begin);
        Pair parsed_command = parse_function(begin, end);
        if (parsed_command.a != -1) {
            start_instruction[i].handler = adresses[parsed_command.a];
            start_instruction[i].iextra = parsed_command.b;
        } else {
            // handle error
        }
        begin = end + 1;
    }
    start_instruction[count].handler = NULL;
    return start_instruction;
}

enum { START_CAP = 100 };

PolizState *poliz_new_state(void) {
    PolizState *new_state = NULL;
    PolizState *tmp = realloc(new_state, 1 * sizeof(*new_state));
    if (tmp == NULL) {
        return NULL;
    } else {
        new_state = tmp;
    }
    new_state->ptr = NULL;
    int *i_tmp = realloc(new_state->ptr, START_CAP * sizeof(*i_tmp));
    if (i_tmp == NULL) {
        // handle error
    } else {
        new_state->ptr = i_tmp;
    }
    new_state->size = 0;
    new_state->capacity = START_CAP;
    new_state->error = PE_OK;
    return new_state;
}

void poliz_free_state(PolizState *state) {
    state->size = 0;
    state->capacity = 0;
    free(state->ptr);
    free(state);
}

int poliz_last_error(struct PolizState *state) {
    return state->error;
}


int main() {
    char *command = "r r r r r w w w w w";
    PolizItem *items = poliz_compile(command);
    PolizState *state = poliz_new_state();
    for (int i = 0; items[i].handler != NULL; ++i) {
        int err = items[i].handler(state, items[i].iextra);
        if (err < 0) {
            fprintf(stderr, "error: %d\n", -err);
        } else if (err > 0) {
            abort();
        }
    }
    poliz_free_state(state);
    free(items);
}