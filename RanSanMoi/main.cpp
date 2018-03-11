#include <iostream>
#include <Windows.h>
#include <time.h>
#include <conio.h>
#include <thread>

using namespace std;

#define MAX_SIZE_FOOD 4
#define MAX_SPEED 3
#define MAX_SIZE_SNAKE 7 + MAX_SIZE_FOOD*(MAX_SPEED)

POINT snake[MAX_SIZE_SNAKE];
POINT food[MAX_SIZE_FOOD];
int CHAR_LOCK;
int MOVING;
int SPEED;
int HEIGHT_CONSOLE, WIDTH_CONSOLE;
int FOOD_INDEX;
int SIZE_SNAKE;
int STATE;
char *name = new char[8];

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void FixConsoleWindow() {
	HWND consoleWindow = GetConsoleWindow();
	LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
	style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);
	SetWindowLong(consoleWindow, GWL_STYLE, style);
}

void hidecursor()
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

void GotoXY(int x, int y) {
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void clearRow(int startX, int startY, int len = 100) {
	GotoXY(startX, startY);
	for (int i = 0; i < len; i++)
		cout << " ";
}

bool IsValid(int x, int y) {
	for (int i = 0; i < SIZE_SNAKE; i++)
		if (snake[i].x == x && snake[i].y == y)
			return false;
	return true;
}

void GenerateFood() {
	int x, y;
	srand(time(NULL));
	for (int i = 0; i < MAX_SIZE_FOOD; i++) {
		do {
			x = rand() % (WIDTH_CONSOLE - 1) + 1;
			y = rand() % (HEIGHT_CONSOLE - 1) + 1;
		} while (!IsValid(x, y));
		food[i] = { x , y };
	}
}

bool validGate(int x, int y) {
	for (int i = 0; i < SIZE_SNAKE; i++) {
		if ((snake[i].x == x || snake[i].x == x - 1 || snake[i].x == x - 1) && snake[i].y == y)
			return false;
		if ((snake[i].x == x - 1 || snake[i].x == x + 1) && snake[i].y == y + 1)
			return false;
	}
	if (x < 2 || x > WIDTH_CONSOLE - 2)
		return false;
	if (y < 1 || y > HEIGHT_CONSOLE - 3)
		return false;
	return true;
}

POINT GateCenter;

void GenerateGate() {
	int x, y;
	srand(time(NULL));
	do {
		x = rand() % (WIDTH_CONSOLE + 1);
		y = rand() % (HEIGHT_CONSOLE + 1);
	} while (!validGate(x, y));
	GateCenter = { x,y };
}

void DrawGate() {
	GotoXY(GateCenter.x, GateCenter.y);
	cout << "G";
	GotoXY(GateCenter.x - 1, GateCenter.y);
	cout << "G";
	GotoXY(GateCenter.x + 1, GateCenter.y);
	cout << "G";
	GotoXY(GateCenter.x - 1, GateCenter.y + 1);
	cout << "G";
	GotoXY(GateCenter.x + 1, GateCenter.y + 1);
	cout << "G";
}

bool GateCollision() {
	if ((snake[SIZE_SNAKE - 1].x == GateCenter.x - 1 || snake[SIZE_SNAKE - 1].x == GateCenter.x + 1) && snake[SIZE_SNAKE - 1].y == GateCenter.y)
		return true;
	if ((snake[SIZE_SNAKE - 1].x == GateCenter.x - 1 || snake[SIZE_SNAKE - 1].x == GateCenter.x + 1) && snake[SIZE_SNAKE - 1].y == GateCenter.y + 1)
		return true;
	if (MOVING != 'W' && snake[SIZE_SNAKE - 1].x == GateCenter.x && snake[SIZE_SNAKE - 1].y == GateCenter.y)
		return true;
	return false;
}

void ResetData() {
	CHAR_LOCK = 'A';
	MOVING = 'D';
	SPEED = 1;
	FOOD_INDEX = 0;
	WIDTH_CONSOLE = 70;
	HEIGHT_CONSOLE = 20;
	SIZE_SNAKE = 7;

	for (int i = 0; i < SIZE_SNAKE; i++)
		snake[i] = { 10 + i, 5 };
	GenerateFood();
}

#define HOR_BORDER '-'
#define VER_BORDER '|'
#define CORNER '+'

void DrawBoard(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0) {
	SetConsoleTextAttribute(hConsole, 14);
	GotoXY(x, y);
	cout << CORNER;
	for (int i = 1; i < width; i++)
		cout << HOR_BORDER;
	cout << CORNER;

	GotoXY(x, height + y);
	cout << CORNER;
	for (int i = 1; i < width; i++)
		cout << HOR_BORDER;
	cout << CORNER;

	for (int i = y + 1; i < height + y; i++) {
		GotoXY(x, i);
		cout << VER_BORDER;

		GotoXY(x + width, i);
		cout << VER_BORDER;
	}

	GotoXY(curPosX, curPosY);
	SetConsoleTextAttribute(hConsole, 15);
}

void StartGame() {
	system("cls");
	ResetData();
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE);
	STATE = 1;
}

void ExitGame(HANDLE t) {
	system("cls");
	TerminateThread(t, 0);
}

void PauseGame(HANDLE t) {
	SuspendThread(t);
}

void DrawSnake(char* str_snake) {
	for (int i = 0; i < SIZE_SNAKE; i++) {
		GotoXY(snake[i].x, snake[i].y);
		SetConsoleTextAttribute(hConsole, i % strlen(str_snake) + 9);
		printf("%c", (i == SIZE_SNAKE - 1) ? 'H' : str_snake[i % strlen(str_snake)]);
	}
	SetConsoleTextAttribute(hConsole, 15);
}

void DrawFood(char* str_food) {
	SetConsoleTextAttribute(hConsole, 12);
	GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
	cout << str_food;
	SetConsoleTextAttribute(hConsole, 15);
}

void ProcessDead() {
	STATE = -1;
}

void Eat() {
	snake[SIZE_SNAKE] = food[FOOD_INDEX];
	if (FOOD_INDEX == MAX_SIZE_FOOD - 1) {
		STATE = 2;
		food[FOOD_INDEX] = { 0, 0 };
		GenerateGate();
		DrawGate();
	}
	else {
		FOOD_INDEX++;
		SIZE_SNAKE++;
	}
}

bool selfDestruction() {
	for (int i = 0; i < SIZE_SNAKE - 1; i++)
		if (snake[SIZE_SNAKE - 1].x == snake[i].x && snake[SIZE_SNAKE - 1].y == snake[i].y)
			return true;
	return false;
}

void MoveRight() {
	if (snake[SIZE_SNAKE - 1].x + 1 == WIDTH_CONSOLE)
		ProcessDead();
	else {
		if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y)
			Eat();
		for (int i = 0; i < SIZE_SNAKE - 1; i++) {
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
		}
		snake[SIZE_SNAKE - 1].x++;
	}
}

void MoveLeft() {
	if (snake[SIZE_SNAKE - 1].x - 1 == 0)
		ProcessDead();
	else {
		if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y)
			Eat();
		for (int i = 0; i < SIZE_SNAKE - 1; i++) {
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
		}
		snake[SIZE_SNAKE - 1].x--;
	}
}

void MoveDown() {
	if (snake[SIZE_SNAKE - 1].y + 1 == HEIGHT_CONSOLE)
		ProcessDead();
	else {
		if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y)
			Eat();
		for (int i = 0; i < SIZE_SNAKE - 1; i++) {
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
		}
		snake[SIZE_SNAKE - 1].y++;
	}
}

void GateEnter() {
	GotoXY(0, HEIGHT_CONSOLE + 3);

	char *str_snake = name;
	for (int k = 0; k < SIZE_SNAKE; k++) {
		for (int i = SIZE_SNAKE - 1; i > -1; i--) {
			GotoXY(snake[i].x, snake[i].y);
			if (i - k > 0) {
				SetConsoleTextAttribute(hConsole, (i - k) % strlen(str_snake) + 9);
				printf("%c", str_snake[(i - k) % strlen(str_snake)]);
			}
			else {
				cout << " ";
			}
		}
		Sleep(50);
	}
	SetConsoleTextAttribute(hConsole, 15);
	
	int speed = SPEED;
	int size = SIZE_SNAKE;

	if (speed == MAX_SPEED) {
		GotoXY(0, HEIGHT_CONSOLE + 3);
		cout << "Congratulation! Restarting...";
		Sleep(1000);
		StartGame();
	}
	else {
		GotoXY(0, HEIGHT_CONSOLE + 3);
		cout << "Level Up! Waiting...";
		Sleep(1000);

		StartGame();
		SPEED = speed + 1;
		SIZE_SNAKE = size;
		for (int i = 0; i < SIZE_SNAKE; i++)
			snake[i] = { 10 + i, 5 };
	}
}

void MoveUp() {
	if (snake[SIZE_SNAKE - 1].y - 1 == 0)
		ProcessDead();
	else if (snake[SIZE_SNAKE - 1].x == GateCenter.x && snake[SIZE_SNAKE - 1].y - 1 == GateCenter.y)
		GateEnter();
	else {
		if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y)
			Eat();
		for (int i = 0; i < SIZE_SNAKE - 1; i++) {
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
		}
		snake[SIZE_SNAKE - 1].y--;
	}
}

void KillSnake() {
	char die[] = "DEAD";
	SetConsoleTextAttribute(hConsole, 15);
	for (int i = SIZE_SNAKE - 1; i > -1; i--) {
		GotoXY(snake[i].x, snake[i].y);
		cout << (char)die[(SIZE_SNAKE - i - 1) % strlen(die)];
		Sleep(150);
	}
	SetConsoleTextAttribute(hConsole, 12);
	for (int i = 0; i < SIZE_SNAKE; i++) {
		GotoXY(snake[i].x, snake[i].y);
		cout << (char)die[(SIZE_SNAKE - i - 1) % strlen(die)];
		Sleep(100);
	}
	SetConsoleTextAttribute(hConsole, 4);
	for (int i = SIZE_SNAKE - 1; i > -1; i--) {
		GotoXY(snake[i].x, snake[i].y);
		cout << (char)die[(SIZE_SNAKE - i - 1) % strlen(die)];
		Sleep(70);
	}
	SetConsoleTextAttribute(hConsole, 15);
	for (int i = 0; i < SIZE_SNAKE; i++) {
		GotoXY(snake[i].x, snake[i].y);
		cout << " ";
		Sleep(50);
	}
	GotoXY(0, HEIGHT_CONSOLE + 2);
	cout << "Dead, type Y to Continue or any key to Exit.";
	STATE = 0;
}

void ThreadFunc() {
	while (true) {
		if (STATE == 1 || STATE == 2) {
			if (STATE == 1)
				DrawFood(" ");
			DrawSnake(" ");
			switch (MOVING) {
			case 'A':
				MoveLeft();
				break;
			case 'D':
				MoveRight();
				break;
			case 'W':
				MoveUp();
				break;
			case 'S':
				MoveDown();
				break;
			}
			if (selfDestruction() || (STATE == 2 && GateCollision()))
				ProcessDead();
			if (STATE == -1) {
				DrawSnake(name);
				KillSnake();
			}
			else {
				if (STATE == 1)
					DrawFood("F");
				DrawSnake(name);
				Sleep(1000 / (10 * SPEED));
			}
		}
	}
}

void SaveGame() {
	GotoXY(0, HEIGHT_CONSOLE + 2);
	char *filename = new char[100];
	GotoXY(0, HEIGHT_CONSOLE + 2);
	cout << "[SAVE] Input file name: ";
	cin >> filename;

	FILE* f = fopen(strcat(filename,".txt"), "w");

	fprintf(f, "%s\n", name);
	fprintf(f, "%d\n", SIZE_SNAKE);
	for (int i = 0; i < SIZE_SNAKE; i++)
		fprintf(f, "%d %d\n", snake[i].x, snake[i].y);
	fprintf(f, "%d\n", FOOD_INDEX);
	fprintf(f, "%d %d\n", food[FOOD_INDEX].x, food[FOOD_INDEX].y);
	fprintf(f, "%c\n", CHAR_LOCK);
	fprintf(f, "%c\n", MOVING);
	fprintf(f, "%d\n", SPEED);
	fprintf(f, "%d\n", STATE);
	if (STATE == 2)
		fprintf(f, "%d %d\n", GateCenter.x, GateCenter.y);

	printf("Successfully saved! Press any key to continue.");
	Sleep(1000);
	clearRow(0, HEIGHT_CONSOLE + 2);
	clearRow(0, HEIGHT_CONSOLE + 3);
	fclose(f);
}

bool LoadGame() {
	GotoXY(0, HEIGHT_CONSOLE + 2);
	char *filename = new char[100];
	cout << "[LOAD] Input file name: ";
	cin >> filename;

	FILE* f = fopen(strcat(filename, ".txt"), "r");
	if (f == NULL) {
		cout << "File not found." << endl;
		return false;
	}

	StartGame();

	fgets(name, 8, f);
	name[strlen(name) - 1] = '\0';
	fscanf(f, "%d\n", &SIZE_SNAKE);
	for (int i = 0; i < SIZE_SNAKE; i++)
		fscanf(f, "%d %d\n", &snake[i].x, &snake[i].y);
	fscanf(f, "%d\n", &FOOD_INDEX);
	fscanf(f, "%d %d\n", &food[FOOD_INDEX].x, &food[FOOD_INDEX].y);
	fscanf(f, "%c\n", &CHAR_LOCK);
	fscanf(f, "%c\n", &MOVING);
	fscanf(f, "%d\n", &SPEED);
	fscanf(f, "%d\n", &STATE);

	if (STATE == 1)
		DrawFood("F");
	else if (STATE == 2) {
		fscanf(f, "%d %d", &GateCenter.x, &GateCenter.y);
		DrawGate();
	}

	DrawSnake(name);

	GotoXY(0, HEIGHT_CONSOLE + 3);
	printf("Successfully loaded! Press any key to continue.");
	Sleep(1000);
	clearRow(0, HEIGHT_CONSOLE + 3);

	fclose(f);
	return true;
}

void main() {
	int temp;

	FixConsoleWindow();
	hidecursor();

	cout << "---- SNAKE ----!" << endl;
	cout << "* Press any key to start or" << endl;
	cout << "* Press T to Load Game";
	temp = toupper(_getch());
	if (temp == 'T') {
		if (!LoadGame()) {
			Sleep(1000);
			StartGame();
		}
	}
	else {
		clearRow(0, 2);
		GotoXY(0, 2);
		cout << "Input identification: ";
		scanf("%s", name);
		StartGame();
	}

	thread t1(ThreadFunc);
	HANDLE handle_t1 = t1.native_handle();
	while (true) {
		temp = toupper(_getch());
		if (STATE == 1 || STATE == 2) {
			if (temp == 'P')
				PauseGame(handle_t1);
			else if (temp == 27) {
				ExitGame(handle_t1);
				return;
			}
			else if (temp == 'Z') {
				PauseGame(handle_t1);
				SaveGame();
			}
			else if (temp == 'X') {
				PauseGame(handle_t1);
				LoadGame();
			}
			else {
				ResumeThread(handle_t1);
				if ((temp != CHAR_LOCK) && (temp == 'D' || temp == 'A' || temp == 'W' || temp == 'S')) {
					if (temp == 'D')
						CHAR_LOCK = 'A';
					else if (temp == 'W')
						CHAR_LOCK = 'S';
					else if (temp == 'S')
						CHAR_LOCK = 'W';
					else 
						CHAR_LOCK = 'D';
					MOVING = temp;
				}
			}
		}
		else if (STATE == 0) {
			if (temp == 'Y')
				StartGame();
			else {
				ExitGame(handle_t1);
				return;
			}
		}
	}
}