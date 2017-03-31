//includes
#include <Vector.h>
#include <Adafruit_NeoPixel.h>
#include <eeprom.h>

//defines
#pragma region defines
#define STRIP_PIXELS 40
#define STRIP_PIN 6
#define STRIP_MODE NEO_GRBW + NEO_KHZ800

#define BAUD 9600
#define EEPROM_LAST_GAME_ADDRESS 0
#define EEPROM_RANDOM_ADDRESS 1

#define MATRIX_X 8
#define MATRIX_Y 5

#define BATTERY_MONITOR A0
#define BATTERY_LED_PIN 13
#define BATTERY_LOW 256
#define BATTERY_VERY_LOW 128
#define BATTERY_TIMEOUT 64

#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 4
#define BUTTON_UP_PIN 7
#define BUTTON_DOWN_PIN 8
#define BUTTON_LEFT_PIN 9
#define BUTTON_RIGHT_PIN 10

#define setColor(x, y, c) (screen[x][y] = color)
#pragma endregion
//vars
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_PIXELS, STRIP_PIN, STRIP_MODE);
uint32_t screen[MATRIX_X][MATRIX_Y];
byte currentGame;
//careful with this, arduinos have almost no ram!
Vector<byte> gameData; 

//types enums and the like
enum Game : byte
{
	SNAKE = 1,
	FLIP = 2,
};

//prototypes
void flushScreen();
void checkBattery();
void setGame();
void tickGame();

//functions
void setup()
{
	Serial.begin(BAUD);
	while (!Serial) {}

	pinMode(BATTERY_MONITOR, INPUT);
	pinMode(BATTERY_LED_PIN, OUTPUT);

	pinMode(BUTTON_A_PIN, INPUT);
	pinMode(BUTTON_B_PIN, INPUT);
	pinMode(BUTTON_UP_PIN, INPUT);
	pinMode(BUTTON_DOWN_PIN, INPUT);
	pinMode(BUTTON_LEFT_PIN, INPUT);
	pinMode(BUTTON_RIGHT_PIN, INPUT);

	strip.begin();
	strip.setBrightness(255);
	strip.show();

	currentGame = EEPROM.read(EEPROM_LAST_GAME_ADDRESS);
	long seed;
	EEPROM.get(EEPROM_RANDOM_ADDRESS, seed);
	randomSeed(seed);

}

void loop()
{

	checkBattery();
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

void checkBattery()
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

}

void setGame()
{
	if (digitalRead(BUTTON_A_PIN) && digitalRead(BUTTON_B_PIN)) {

		switch (currentGame) 
		{
			case Game::SNAKE:
				currentGame = Game::FLIP;
				break;
			case Game::FLIP:
				currentGame = Game::SNAKE;
				break;
			default:
				currentGame = Game::SNAKE;
				break;
		}
		EEPROM.update(EEPROM_LAST_GAME_ADDRESS, currentGame);
	}
}

void tickGame()
{




}

