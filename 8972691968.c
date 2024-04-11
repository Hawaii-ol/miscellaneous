#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <conio.h>
#define KB_UP 0x48
#define KB_DOWN 0x50
#define KB_RIGHT 0x4D
#define KB_LEFT 0x4B
#define KB_ENTER '\r'
#else
#include <termios.h>
#include <unistd.h>
#define KB_UP 0x41
#define KB_DOWN 0x42
#define KB_RIGHT 0x43
#define KB_LEFT 0x44
#define KB_ENTER '\n'

int getch(void)
{
	struct termios oldattr, newattr;
	int ch;
	tcgetattr(STDIN_FILENO, &oldattr);
	newattr = oldattr;
	newattr.c_lflag &= ~(ICANON | ICRNL | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
	return ch;
}
#endif

char board[3][3] = { 0 };
int marks[3][3] = { 0 };
struct {
	int x, y;
}cursor;

void paint_board(int player)
{
	for (int i = 0; i < 3; i++) {
		if (i != 0) {
			for (int j = 0; j < 5; j++) {
				if (j & 1)
					printf("┼");
				else
					printf("─");
			}
			putchar('\n');
		}
		
		for (int j = 0; j < 3; j++) {
			if (j != 0)
				printf("│");
			if (player && i == cursor.x && j == cursor.y) {
				if (board[i][j] == ' ')
					putchar(player);
				else
					printf("\033[31m%c\033[0m", board[i][j]);
			}
			else if (marks[i][j])
				printf("\033[32m%c\033[0m", board[i][j]);
			else
				putchar(board[i][j]);
		}
		putchar('\n');
	}
}

int main()
{
	int count = 0, player = 'O', winner = 0;
	memset(board, ' ', sizeof(board));
	printf("Tic-Tac-Toe\n");
	// hide cursor
	printf("\033[?25l");
	
	while (count++ < 9) {
		int place = 0;
		while (!place) {
			printf("It's %c's turn\n", player);
			printf("Press Arrow Keys to move, Enter to place.\n");
			paint_board(player);
			switch (getch()) {
			case KB_UP:
				if (cursor.x > 0)
					cursor.x--;
				break;
			case KB_DOWN:
				if (cursor.x < 2)
					cursor.x++;
				break;
			case KB_LEFT:
				if (cursor.y > 0)
					cursor.y--;
				break;
			case KB_RIGHT:
				if (cursor.y < 2)
					cursor.y++;
				break;
			case KB_ENTER:
				if (board[cursor.x][cursor.y] == ' ') {
					board[cursor.x][cursor.y] = tolower(player);
					place = 1;
				}
			}
			printf("\033[7A");
		}
		
		// 检查行
		for (int i = 0; i < 3; i++) {
			if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
				winner = board[i][0];
				marks[i][0] = marks[i][1] = marks[i][2] = 1;
				goto RESULT;
			}
		}
		// 检查列
		for (int i = 0; i < 3; i++) {
			if (board[0][i] != ' ' && board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
				winner = board[0][i];
				marks[0][i] = marks[1][i] = marks[2][i] = 1;
				goto RESULT;
			}
		}
		// 检查对角线
		if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
			winner = board[1][1];
			marks[0][0] = marks[1][1] = marks[2][2] = 1;
			goto RESULT;
		}
		if (board[2][0] != ' ' && board[2][0] == board[1][1] && board[1][1] == board[0][2]) {
			winner = board[1][1];
			marks[2][0] = marks[1][1] = marks[0][2] = 1;
			goto RESULT;
		}
		
		player = player == 'O' ? 'X' : 'O';
	}
	
RESULT:
	// erase prompt messages
	printf("\033[7M");
	// show cursor
	printf("\033[?25h");
	paint_board(0);
	if (winner == 0)
		printf("Draw!\n");
	else
		printf("Winner is %c!\n", winner);
	return 0;
}
