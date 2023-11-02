#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#define ishex(ch) (((ch) >= '0' && (ch) <= '9') || (tolower(ch) >= 'a' && tolower(ch) <= 'f'))
#define tohex(ch) ((ch) >= '0' && (ch) <= '9' ? (ch) - '0' : tolower(ch) - 'a' + 10)
/* 测试n*16是否会溢出int */
#define will_overflow(n) ((n) >= 0x8000000)
/* 定义状态：
S0-初始状态
S1-位于第一个操作数中
S2-第一个操作数溢出
S3-运算符
S4-位于第二个操作数中
S5-第二个操作数溢出
S6-终止状态
*/

int main()
{
	int ch, state = 0;
	unsigned op1 = 0, op2 = 0, result;
	char operator;
	while (state != 6) {
		ch = getchar();
		if (ishex(ch) || ch == '+' || ch == '-' || ch == '\n' || ch == EOF) {
			switch (state) {
			case 0: /* 初始状态 */
				if (ch == '+' || ch == '-') {
					printf("0(0)");
					op1 = 0;
					operator = ch;
					state = 3;
				} else if (ch == '\n' || ch == EOF) {
					;
				} else {
					op1 = tohex(ch);
					state = 1;
				}
				break;
			case 1: /* 位于第一个操作数中 */
				if (ch == '+' || ch == '-') {
					printf("%#x(%d)", op1, (int)op1);
					operator = ch;
					state = 3;
				} else if (ch == '\n' || ch == EOF) {
					printf("%#x(%d)\n", op1, (int)op1);
					state = 0;
				} else {
					if (will_overflow(op1)) {
						printf("%#x[", op1);
						putchar(tolower(ch));
						state = 2;
					} else
						op1 = op1 * 16 + tohex(ch);
				}
				break;
			case 2: /* 第一个操作数溢出 */
				if (ch == '+' || ch == '-') {
					printf("](%d)", (int)op1);
					operator = ch;
					state = 3;
				} else if (ch == '\n' || ch == EOF) {
					printf("](%d)\n", (int)op1);
					state = 0;
				} else
					putchar(tolower(ch));
				break;
			case 3: /* 运算符 */
				putchar(operator);
				if (ch == '\n' || ch == EOF) {
					printf("0(0)=%#x(%d)\n", op1, (int)op1);
					state = 0;
				} else {
					op2 = tohex(ch);
					state = 4;
				}
				break;
			case 4: /* 位于第二个操作数中 */
				if (ch == '\n' || ch == EOF) {
					result = (operator == '+') ? op1 + op2 : op1 - op2;
					printf("%#x(%d)=%#x(%d)\n", op2, (int)op2, result, (int)result);
					state = 0;
				} else {
					if (will_overflow(op2)) {
						printf("%#x[", op2);
						putchar(tolower(ch));
						state = 5;
					} else
						op2 = op2 * 16 + tohex(ch);
				}
				break;
			case 5: /* 第二个操作数溢出 */
				if (ch == '\n' || ch == EOF) {
					result = (operator == '+') ? op1 + op2 : op1 - op2;
					printf("](%d)=%#x(%d)\n", (int)op2, result, (int)result);
					state = 0;
				} else
					putchar(tolower(ch));
			default:
				break;
			}

			if (ch == EOF)
				state = 6;
		}
	}
	return 0;
}