//includes
#include <Adafruit_NeoPixel.h>
#include <eeprom.h>

//defines
#pragma region defines
#define STRIP_PIXELS 40
#define STRIP_PIN 6
#define STRIP_MODE NEO_GRBW + NEO_KHZ800

#define UNUSED_ANALOG_PIN A0
#define UNUSED_COLOR ((uint32_t)0xFFFFFFFF)
#define EEPROM_LAST_GAME_ADDRESS 0

#define MATRIX_X 8
#define MATRIX_Y 5

#define BATTERY_MONITOR		A0
#define BATTERY_LED_PIN		13
#define BATTERY_LOW			256
#define BATTERY_VERY_LOW	128
#define BATTERY_TIMEOUT		64

#define BUTTON_MODE			INPUT_PULLUP
#define BUTTON_PRESSED		LOW
#define BUTTON_RELEASED		HIGH
#define BUTTON_A_PIN		1
#define BUTTON_B_PIN		0
#define BUTTON_UP_PIN		4
#define BUTTON_DOWN_PIN		2
#define BUTTON_LEFT_PIN		5
#define BUTTON_RIGHT_PIN	3

//color definitions in format: WWRRGGBB
#define FLIP_DELAY_NORMAL 20
#define FLIP_DELAY_WIN 5
#define FLIP_COLOR_ON		((uint32_t)0x00FF0000)
#define FLIP_COLOR_OFF		((uint32_t)0x0000FF00)
#define FLIP_COLOR_WIN		((uint32_t)0x000F0F0F)
#define FLIP_COLOR_SELECT	((uint32_t)0x000000FF)

#define setColor(x, y, c) (screen[x][y] = color)
#pragma endregion

//vars
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_PIXELS, STRIP_PIN, STRIP_MODE);
uint32_t screen[MATRIX_X][MATRIX_Y];
bool gameWon = false;
byte currentGame;
void* gameData;

//types enums and the like
enum Game : byte
{
	FLIP = 1,
	SNAKE = 2,
};

//saves data for the flip game
struct FlipData
{
	byte m_x;
	byte m_y;
	bool m_active;
	uint32_t m_underColor;

	FlipData(byte x, byte y, bool active, uint32_t underColor)
		: m_x(x), m_y(y), m_active(active), m_underColor(underColor){}

	~FlipData() {}

};

//prototypes
void flushScreen();
//inline void checkBattery();
inline void setGame();
void tickGame();
void onGameChange();

//functions
void setup()
{
	//pinMode(BATTERY_MONITOR, INPUT);
	//pinMode(BATTERY_LED_PIN, OUTPUT);

	pinMode(BUTTON_A_PIN,		INPUT_PULLUP);
	pinMode(BUTTON_B_PIN,		INPUT_PULLUP);
	pinMode(BUTTON_UP_PIN,		INPUT_PULLUP);
	pinMode(BUTTON_DOWN_PIN,	INPUT_PULLUP);
	pinMode(BUTTON_LEFT_PIN,	INPUT_PULLUP);
	pinMode(BUTTON_RIGHT_PIN,	INPUT_PULLUP);

	strip.begin();
	strip.setBrightness(255);
	strip.show();
	
	randomSeed(analogRead(UNUSED_ANALOG_PIN));
	onGameChange();
	

}

void loop()
{

	//checkBattery();
	setGame();

	tickGame();

	flushScreen();

}

void flushScreen() 
{

	for (int i = 0; i < MATRIX_X; i++)
	{
		for (int j = 0; j < MATRIX_Y; j++)
		{
			strip.setPixelColor(i + (j * MATRIX_X), screen[i][j]);
		}
	}
	strip.show();
}

//power shield malfunctioned
/*
inline void checkBattery()
{
	int monitor = analogRead(BATTERY_MONITOR);
	
	if (monitor < BATTERY_VERY_LOW) 
	{
		if ((millis() / BATTERY_TIMEOUT) < 5) 
		{
			digitalWrite(BATTERY_LED_PIN, HIGH);
		}
		else
		{
			digitalWrite(BATTERY_LED_PIN, LOW);
		}
	}
	if (monitor < BATTERY_LOW) 
	{
		digitalWrite(BATTERY_LED_PIN, HIGH);
	}
	else 
	{
		digitalWrite(BATTERY_LED_PIN, LOW);
	}

}*/

inline void setGame()
{
	
	if (digitalRead(BUTTON_A_PIN) == BUTTON_PRESSED && 
		digitalRead(BUTTON_B_PIN) == BUTTON_PRESSED) 
	{	
		switch (currentGame) 
		{
			case Game::FLIP:
				currentGame = Game::SNAKE;
				onGameChange();
				break;
			case Game::SNAKE:
				currentGame = Game::FLIP;
				onGameChange();
				break;
			default:
				currentGame = Game::FLIP;
				onGameChange();
				break;
		}
		EEPROM.update(EEPROM_LAST_GAME_ADDRESS, currentGame);
	}
}

void onGameChange() 
{
	gameWon = false;

	switch (currentGame)
	{
	case FLIP:
		
		

		for (int i = 0; i < MATRIX_X; i++) {
			for (int j = 0; j < MATRIX_Y; j++) {
				screen[i][j] = random(2) ? FLIP_COLOR_ON : FLIP_COLOR_OFF;
			}
		}

		byte x = random(MATRIX_X);
		byte y = random(MATRIX_Y);
		gameData = new FlipData(x, y, true, screen[x][y]);

		flushScreen();
		delay(FLIP_DELAY_NORMAL);

		break;

	}

}

void tickGame()
{

	switch (currentGame)
	{
	case Game::FLIP:

		
		if (!gameWon) {

			uint32_t color = UNUSED_COLOR;
			bool win = true;
			for (int x = 0; x < MATRIX_X; x++) {
				for (int y = 0; y < MATRIX_Y; y++) {
					if (color != UNUSED_COLOR && screen[x][y] != color)win = false;
				}
			}
			gameWon = win;

			if (win) {
				((FlipData*)gameData)->m_x = 0;
				break;
			}

			FlipData* data = (FlipData*)gameData;

			//change coursor position
			if (digitalRead(BUTTON_UP_PIN) == BUTTON_PRESSED) {

				if (data->m_active) {
					screen[data->m_x][data->m_y] = data->m_underColor;
				}
				data->m_x++;

				if (data->m_x >= MATRIX_X)data->m_x = 0;

				data->m_underColor = screen[data->m_x][data->m_y];

			}
			else if (digitalRead(BUTTON_DOWN_PIN) == BUTTON_PRESSED) {

				if (data->m_active) {
					screen[data->m_x][data->m_y] = data->m_underColor;
				}


				if ((int)data->m_x - 1 < 0) {
					data->m_x = MATRIX_Y;
				}
				else {
					data->m_x--;
				}

				data->m_underColor = screen[data->m_x][data->m_y];


			}
			else if (digitalRead(BUTTON_LEFT_PIN) == BUTTON_PRESSED) {
				if (data->m_active) {
					screen[data->m_x][data->m_y] = data->m_underColor;
				}
				data->m_y++;

				if (data->m_y >= MATRIX_Y)data->m_y = 0;

				data->m_underColor = screen[data->m_x][data->m_y];

			}
			else if (digitalRead(BUTTON_RIGHT_PIN) == BUTTON_PRESSED) {

				if (data->m_active) {
					screen[data->m_x][data->m_y] = data->m_underColor;
				}

				if ((int)data->m_y - 1 < 0) {
					data->m_y = MATRIX_X;
				}
				else {
					data->m_y--;
				}

				data->m_underColor = screen[data->m_x][data->m_y];

			}

			//cycle coursor color
			if (data->m_active) {

				screen[data->m_x][data->m_y] = FLIP_COLOR_SELECT;
				data->m_active = false;

			}
			else {

				screen[data->m_x][data->m_y] = data->m_underColor;
				data->m_active = true;

			}

			//process button press
			if (digitalRead(BUTTON_A_PIN) == BUTTON_PRESSED) {

				int x = data->m_x;
				int y = data->m_y;

				data->m_underColor = data->m_underColor == FLIP_COLOR_ON ? FLIP_COLOR_ON : FLIP_COLOR_OFF;
				data->m_active = true;

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
		else {

			FlipData* data = (FlipData*)gameData;

			if (data->m_active) {
				
				for (int x = 0; x < MATRIX_X; x++) {
					for (int y = 0; y < MATRIX_Y; y++) {
						screen[x][y] = FLIP_COLOR_WIN;
					}
				}
				data->m_active = false;
			}
			else {
				for (int x = 0; x < MATRIX_X; x++) {
					for (int y = 0; y < MATRIX_Y; y++) {
						screen[x][y] = FLIP_COLOR_WIN;
					}
				}
				data->m_active = false;
			}
			data->m_x++;

			if (data->m_x == 255) {
				gameWon = false;
				delete gameData;
				onGameChange();
			}

			delay(FLIP_DELAY_WIN);

		}

		break;
	case Game::SNAKE:

		break;
	}


}