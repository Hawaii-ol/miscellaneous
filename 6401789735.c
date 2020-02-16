#include <stdio.h>
#define LEN 4096

typedef struct {
    int coef;
    int power;
}Term;

Term diff_term(Term term);
char* dump_terms(Term terms[], int num, char* output);
int simple_FSM(const char* input, Term terms[], int* num);

int main()
{
    char input[LEN], output[LEN];
    Term terms[LEN / 2];
    int num;
    while (gets(input))
    {
        if (simple_FSM(input, terms, &num))
        {
            printf("%s\n", dump_terms(terms, num, output));
        }
        else
        {
            printf("表达式非法\n");
        }
    }
    return 0;
}

Term diff_term(Term term)
{
    Term diff;
    diff.coef = term.coef * term.power;
    diff.power = term.power - 1;
    return diff;
}

char* dump_terms(Term terms[], int num, char* output)
{
    int i;
    char *p = output;
    for (i = 0; i < num; i++)
    {
        Term diff = diff_term(terms[i]);
        if (diff.coef == 0)
            continue;
        if (diff.coef > 0 && i > 0)
            p += sprintf(p, "+");
        if (diff.power == 0)
            p += sprintf(p, "%d", diff.coef);
        else if (diff.power == 1)
            p += sprintf(p, "%dx", diff.coef);
        else
            p += sprintf(p, "%dx^%d", diff.coef, diff.power);
    }
    *p = '\0';
    return output;
}

/* 简单有限状态机 */
int simple_FSM(const char* input, Term* terms, int* num)
{
    int S = 0;
    int sign;
    int coef = 0;
    int power = 0;
    Term term;
    int cnt_term = 0;
    const char* p = input;

    while (1)
    {
        switch (S)
        {
        /* 状态0：系数符号 */
        case 0:
            if (*p == '-')
            {
                sign = -1;
                p++;
            }
            else if (*p == '+')
            {
                sign = 1;
                p++;
            }
            else if (*p >= '0' && *p <= '9' || *p == 'x')
                sign = 1;
            else
                return 0;
            S = 1;
            break;
        /* 状态1：系数 */
        case 1:
            if (*p >= '0' && *p <= '9')
            {
                coef = coef * 10 + *p - '0';
                p++;
            }
            else
            {
                if (coef == 0)
                    coef = 1;
                coef = sign * coef;
                S = 2;
            }
            break;
        /* 状态2：检测x */
        case 2:
            if (*p == 'x')
            {
                p++;
                S = 3;
            }
            else
                S = 4;
            break;
        /* 状态3：有x */
        case 3:
            if (*p == '^')
            {
                p++;
                S = 5;
            }
            else
            {
                power = 1;
                S = 7;
            }
            break;
        /* 状态4：没有x */
        case 4:
            power = 0;
            S = 7;
            break;
        /* 状态5：幂符号 */
        case 5:
            if (*p == '-')
            {
                sign = -1;
                p++;
            }
            else if (*p == '+')
            {
                sign = 1;
                p++;
            }
            else if (*p >= '0' && *p <= '9')
                sign = 1;
            else
                return 0;
            S = 6;
            break;
        /* 状态6：幂 */
        case 6:
            if (*p >= '0' && *p <= '9')
            {
                power = power * 10 + *p - '0';
                p++;
            }
            else
            {
                power = sign * power;
                S = 7;
            }
            break;
        /* 状态7：输出 */
        case 7:
            term.coef = coef;
            term.power = power;
            terms[cnt_term++] = term;
            coef = 0;
            power = 0;
            S = 0;
            break;
        }
        if (*p == '\0' && S == 0)
        {
            *num = cnt_term;
            return 1;
        }
    }
}
