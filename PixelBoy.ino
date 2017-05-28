//includes
#include <Adafruit_NeoPixel.h>
#include <eeprom.h>

//defines
#pragma region defines
#define STRIP_PIXELS 40
#define STRIP_PIN 6
#define STRIP_MODE NEO_GRBW + NEO_KHZ800
#define BRIGHTNESS 50

#define UNUSED_ANALOG_PIN A0
#define UNUSED_COLOR	static_cast<uint32_t>(0xFFFFFFFF)
#define BLACK			static_cast<uint32_t>(0x00000000)
#define EEPROM_LAST_GAME_ADDRESS 0
#define GAME_CHANGE_TIMEOUT 500

#define MATRIX_X 8
#define MATRIX_Y 5

#define BUTTON_MODE			INPUT_PULLUP
#define BUTTON_PRESSED		LOW
#define BUTTON_RELEASED		HIGH
#define BUTTON_A_PIN		1
#define BUTTON_B_PIN		0
#define BUTTON_UP_PIN		3
#define BUTTON_DOWN_PIN		4
#define BUTTON_LEFT_PIN		5
#define BUTTON_RIGHT_PIN	2

//color definitions in format: WWRRGGBB
#define FLIP_DELAY_NORMAL	200
#define FLIP_DELAY_WIN		50
#define FLIP_COLOR_ON		static_cast<uint32_t>(0x00FF0000)
#define FLIP_COLOR_OFF		static_cast<uint32_t>(0x0000FF00)
#define FLIP_COLOR_WIN		static_cast<uint32_t>(0x000F0F0F)
#define FLIP_COLOR_SELECT	static_cast<uint32_t>(0x000000FF)

#define SNAKE_DELAY_NORMAL			500
#define SNAKE_DELAY_REDUCTION		10
#define SNAKE_WIN_LENGTH			10
#define SNAKE_START_LENGTH			3
#define SNAKE_COLOR_CANDY_RANDOM
#define SNAKE_COLOR_CANDY			static_cast<uint32_t>(0x000000FF)
#define SNAKE_COLOR_WIN				static_cast<uint32_t>(0x0000FF00)

#pragma endregion

//types enums and the like
enum Game : byte
{
	FLIP = 1,
	SNAKE = 2,
};

enum Orientation : byte
{
	NORTH = 1,
	EAST = 2,
	SOUTH = 3,
	WEST = 4
};

struct SnakePart
{
	int x;
	int y;

	SnakePart(int x, int y) : x(x), y(y){}

};

//vars
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_PIXELS, STRIP_PIN, STRIP_MODE);
uint32_t screen[MATRIX_X][MATRIX_Y];
bool gameWon = false;
long lastGameChange;
byte currentGame = SNAKE;

int global_x;
int global_y;
bool global_active;
uint32_t global_underColor;
Orientation global_orientation;

//uses constructor
SnakePart snake_parts[] = {
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
};

//prototypes
void flushScreen();
void clearScreen();
inline void setGame();
void tickGame();
void onGameChange();
inline void incrementByOrientation(int &x, int &y, Orientation ori);

//functions
void setup()
{

	Serial.end();
	noInterrupts();

	//set pin modes for 
	pinMode(BUTTON_A_PIN,		BUTTON_MODE);
	pinMode(BUTTON_B_PIN,		BUTTON_MODE);
	pinMode(BUTTON_UP_PIN,		BUTTON_MODE);
	pinMode(BUTTON_DOWN_PIN,	BUTTON_MODE);
	pinMode(BUTTON_RIGHT_PIN,	BUTTON_MODE);
	pinMode(BUTTON_LEFT_PIN,	BUTTON_MODE);

	strip.begin();
	strip.setBrightness(BRIGHTNESS);
	strip.show();
	
	randomSeed(analogRead(UNUSED_ANALOG_PIN));

	currentGame = EEPROM.read(EEPROM_LAST_GAME_ADDRESS);
	//if not set already, default it to Flip
	if (currentGame == 0 || currentGame > static_cast<byte>(Game::SNAKE))currentGame = Game::FLIP;

	onGameChange();
	

}

void loop()
{

	setGame();

	tickGame();

	flushScreen();

}

void flushScreen() 
{
	int pixel = 0;

	for (int i = 0; i < MATRIX_X; i++)
	{
		for (int j = 0; j < MATRIX_Y; j++)
		{
			strip.setPixelColor(pixel, screen[i][j]);
			pixel++;
		}
	}
	strip.show();
}

void clearScreen() {
	for (int x = 0; x < MATRIX_X; x++) {
		for (int y = 0; y = MATRIX_Y; y++) {
			screen[x][y] = BLACK;
		}
	}
}

void setGame()
{

	if (lastGameChange + GAME_CHANGE_TIMEOUT > millis())return;

	if (digitalRead(BUTTON_B_PIN) == BUTTON_PRESSED) 
	{	

		clearScreen();

		switch (currentGame) 
		{
			//Flip to Snake
			case Game::FLIP:
				currentGame = Game::SNAKE;
				onGameChange();
				break;
			//Snake to Flip
			case Game::SNAKE:
				currentGame = Game::FLIP;
				onGameChange();
			//for wired errors
			default:
				currentGame = Game::FLIP;
				onGameChange();
				break;
		}
		//update to avoid unneccecary write cycles on EEPROM
		EEPROM.update(EEPROM_LAST_GAME_ADDRESS, currentGame);
		lastGameChange = millis();
	}
}

void onGameChange() 
{
	gameWon = false;

	switch (currentGame)
	{
	//change TO flip
#pragma region toFLIP
	case FLIP:

		//generate "map"
		for (int i = 0; i < MATRIX_X; i++) {
			for (int j = 0; j < MATRIX_Y; j++) {
				screen[i][j] = random(2) ? FLIP_COLOR_ON : FLIP_COLOR_OFF;
			}
		}
		
		global_x = random(MATRIX_X);
		global_y = random(MATRIX_Y);
		global_active = true;
		global_underColor = screen[global_x][global_y];

		flushScreen();

		break;
#pragma endregion

#pragma region toSNAKE
	case SNAKE:

		global_x = random(MATRIX_X);
		global_y = random(MATRIX_Y);

		global_orientation = NORTH;

		//init x coord
		for (int i = 0; i > SNAKE_WIN_LENGTH; i++) {
			snake_parts[i].x = i < SNAKE_START_LENGTH ? i : (-1);

		}
		//init y coord
		for (int i = 0; i > SNAKE_WIN_LENGTH; i++) {
			snake_parts[i].x = i < SNAKE_START_LENGTH ? 2 : (-1);
		}

		for (int i = 0; i < SNAKE_START_LENGTH; i++)
		{

			int x = snake_parts[i].x;
			int y = snake_parts[i].y;

#ifdef SNAKE_COLOR_CANDY_RANDOM
			screen[x][y] = Adafruit_NeoPixel::Color(random(255), random(255), random(255));
#else
			screen[x][y] = SNAKE_COLOR_CANDY;
#endif

		}

		flushScreen();
		delay(SNAKE_DELAY_NORMAL - (SNAKE_DELAY_REDUCTION * SNAKE_START_LENGTH));

		break;
#pragma endregion
	}

}

void tickGame()
{

	switch (currentGame)
	{
#pragma region tickFLIP
		case Game::FLIP:
	
			if (!gameWon) {

				//check if won
				bool win = true;
				for (int x = 0; x < MATRIX_X; x++) {
					for (int y = 0; y < MATRIX_Y; y++) {
						if (screen[x][y] != FLIP_COLOR_ON)win = false;
					}
				}
				gameWon = win;

				//if won, prepare for animation
				if (win) {
					global_x = 0;
					break;
				}
				//change coursor position

				//up -> x+1
				if (digitalRead(BUTTON_UP_PIN) == BUTTON_PRESSED) {

					if (global_active) {
						screen[global_x][global_y] = global_underColor;
					}
					global_x++;

					if (global_x >= MATRIX_X)global_x = 0;

					global_underColor = screen[global_x][global_y];

				}
				//down -> x-1
				if (digitalRead(BUTTON_DOWN_PIN) == BUTTON_PRESSED) {

					if (global_active) {
						screen[global_x][global_y] = global_underColor;
					}


					if ((int)global_x - 1 < 0) {
						global_x = MATRIX_Y;
					}
					else {
						global_x--;
					}

					global_underColor = screen[global_x][global_y];

				}
				//left -> y-1
				if (digitalRead(BUTTON_LEFT_PIN) == BUTTON_PRESSED) {
					if (global_active) {
						screen[global_x][global_y] = global_underColor;
					}
					global_y++;

					if (global_y >= MATRIX_Y)global_y = 0;

					global_underColor = screen[global_x][global_y];

				}
				//right -> y+1
				if (digitalRead(BUTTON_RIGHT_PIN) == BUTTON_PRESSED) {

					if (global_active) {
						screen[global_x][global_y] = global_underColor;
					}

					if ((int)global_y - 1 < 0) {
						global_y = MATRIX_X;
					}
					else {
						global_y--;
					}

					global_underColor = screen[global_x][global_y];

				}

				//cycle coursor color
				if (global_active) {
					screen[global_x][global_y] = FLIP_COLOR_SELECT;
					global_active = false;
				}
				else {
					screen[global_x][global_y] = global_underColor;
					global_active = true;
				}

				//process button press
				if (digitalRead(BUTTON_A_PIN) == BUTTON_PRESSED) {

					int x = global_x;
					int y = global_y;

					global_underColor = global_underColor == FLIP_COLOR_ON ? FLIP_COLOR_ON : FLIP_COLOR_OFF;
					global_active = false;

					if (x - 1 >= 0) {
						screen[x - 1][y] = screen[x - 1][y] == FLIP_COLOR_ON ? FLIP_COLOR_ON : FLIP_COLOR_OFF;
					}

					if (x + 1 < MATRIX_X) {
						screen[x + 1][y] = screen[x + 1][y] == FLIP_COLOR_ON ? FLIP_COLOR_ON : FLIP_COLOR_OFF;
					}

					if (y - 1 >= 0) {
						screen[x][y - 1] = screen[x][y - 1] == FLIP_COLOR_ON ? FLIP_COLOR_ON : FLIP_COLOR_OFF;
					}

					if (y + 1 < MATRIX_Y) {
						screen[x][y + 1] = screen[x][y + 1] == FLIP_COLOR_ON ? FLIP_COLOR_ON : FLIP_COLOR_OFF;
					}

				}

				delay(FLIP_DELAY_NORMAL);

			}
			//won
			else {

				if (global_active) {
					
					for (int x = 0; x < MATRIX_X; x++) {
						for (int y = 0; y < MATRIX_Y; y++) {
							screen[x][y] = FLIP_COLOR_WIN;
						}
					}
					global_active = false;
				}
				else {
					for (int x = 0; x < MATRIX_X; x++) {
						for (int y = 0; y < MATRIX_Y; y++) {
							screen[x][y] = FLIP_COLOR_WIN;
						}
					}
					global_active = false;
				}
				global_x++;

				if (global_x == 10) {
					onGameChange();
				}

				delay(FLIP_DELAY_WIN);

			}

			break;
#pragma endregion
#pragma region tickSNAKE
	case SNAKE:


		if (!gameWon) {
			
			int cX = global_x;
			int cY = global_y;

			int x = snake_parts[0].x;
			int y = snake_parts[0].y;
			bool candyAssimilated = false;

			//warnig, x and y get modified!
			incrementByOrientation(x, y, global_orientation);

			//generate new candy
			if (cX == x && cY == y) {

				candyAssimilated = true;
				bool posValid = false;

				pos: while (!posValid) {

					cX = random(MATRIX_X);
					cY = random(MATRIX_Y);

					for (int i = 0; i < SNAKE_WIN_LENGTH; i++) {
						if (cX == snake_parts[i].x && cY == snake_parts[i].y) {
							//no java labels :(
							goto pos;
						}
					}
					posValid = true;
				}

#ifdef SNAKE_COLOR_CANDY_RANDOM
				screen[cX][cY] = Adafruit_NeoPixel::Color(random(255), random(255), random(255));
#else
				screen[cX][cY] = SNAKE_COLOR_CANDY;
#endif
			}

			if (candyAssimilated) {
				for (int i = 0; i < SNAKE_WIN_LENGTH; i++) {

					if (snake_parts[i - 1].x == -1)break;
					
					cX = snake_parts[i].x;
					cY = snake_parts[i].y;
					
					snake_parts[i].x = x;
					snake_parts[i].y = y;

					x = cX;
					y = cY;

				}
			}
			else {

				for (int i = 0; i < SNAKE_WIN_LENGTH; i++) {

					screen[x][y] = screen[snake_parts[i].x][snake_parts[i].y];

					if (snake_parts[i].x == -1)break;

					if (i != 0) {
						cX = snake_parts[i].x;
						cY = snake_parts[i].y;
					}

					snake_parts[i].x = x;
					snake_parts[i].y = y;

					x = cX;
					y = cY;

				}

			}
			flushScreen();

			int cntParts = 0;
			for (int i = 0; i < SNAKE_WIN_LENGTH; i++) {
				if (snake_parts[i].x == -1) {
					cntParts = i;
					break;
				}
			}

			if (cntParts >= SNAKE_WIN_LENGTH) {
				gameWon = true;
			}

			delay(SNAKE_DELAY_NORMAL - (cntParts * SNAKE_DELAY_REDUCTION));

		}
		else {

			if (global_y) {
				for (int x = 0; x < MATRIX_X; x++) {
					for (int y = 0; y < MATRIX_Y; y++) {
						screen[x][y] = SNAKE_COLOR_WIN;
					}
				}
			}
			else {
				for (int x = 0; x < MATRIX_X; x++) {
					for (int y = 0; y < MATRIX_Y; y++) {
						screen[x][y] = SNAKE_COLOR_CANDY;
					}
				}
			}


			if (global_x >= 10) {
				onGameChange();
			}

		}

		break;
#pragma endregion
	}

}

inline void incrementByOrientation(int &x, int &y, Orientation ori) {

	switch (ori) {
	case NORTH:
		x++;
		return;
	case SOUTH:
		x--;
		return;
	case WEST:
		y++;
		return;
	case EAST:
		y--;
		return;
	}

}