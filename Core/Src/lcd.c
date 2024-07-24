/*
 * lcd.c
 *
 *  Created on: Jul 24, 2024
 *      Author: Secon
 */

#include "lcd.h"
#include "spi.h"

//Lista komend do obsługi wyswietlacza:
#define ST7735S_SLPOUT			0x11
#define ST7735S_DISPOFF			0x28
#define ST7735S_DISPON			0x29
#define ST7735S_CASET			0x2a
#define ST7735S_RASET			0x2b
#define ST7735S_RAMWR			0x2c
#define ST7735S_MADCTL			0x36
#define ST7735S_COLMOD			0x3a
#define ST7735S_FRMCTR1			0xb1
#define ST7735S_FRMCTR2			0xb2
#define ST7735S_FRMCTR3			0xb3
#define ST7735S_INVCTR			0xb4
#define ST7735S_PWCTR1			0xc0
#define ST7735S_PWCTR2			0xc1
#define ST7735S_PWCTR3			0xc2
#define ST7735S_PWCTR4			0xc3
#define ST7735S_PWCTR5			0xc4
#define ST7735S_VMCTR1			0xc5
#define ST7735S_GAMCTRP1		0xe0
#define ST7735S_GAMCTRN1		0xe1

//Makro ustawiające 9 bit 16-bitowej zmiennej na 1 w celu wyslania danych jako komenda:
#define CMD(x)					((x) | 0x100)

//Definicja stalych zawierajacych offset, poniewaz wyswyeitlacz ma rozdzielczosc 160 x 128, a
//sterownik ST7735S obsluguje rozdzielczoscmdo 162 x 132:
#define LCD_OFFSET_X 1
#define LCD_OFFSET_Y 2

//Tablica zawierajaca polecenia realizujace poczatkowa inicjalizacje wyswietlacza.
//(pochodzi ona z programow dolaczonych do modulu przez producenta):
static const uint16_t init_table[] = {
  CMD(ST7735S_FRMCTR1), 0x01, 0x2c, 0x2d,
  CMD(ST7735S_FRMCTR2), 0x01, 0x2c, 0x2d,
  CMD(ST7735S_FRMCTR3), 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,
  CMD(ST7735S_INVCTR), 0x07,
  CMD(ST7735S_PWCTR1), 0xa2, 0x02, 0x84,
  CMD(ST7735S_PWCTR2), 0xc5,
  CMD(ST7735S_PWCTR3), 0x0a, 0x00,
  CMD(ST7735S_PWCTR4), 0x8a, 0x2a,
  CMD(ST7735S_PWCTR5), 0x8a, 0xee,
  CMD(ST7735S_VMCTR1), 0x0e,
  CMD(ST7735S_GAMCTRP1), 0x0f, 0x1a, 0x0f, 0x18, 0x2f, 0x28, 0x20, 0x22,
                         0x1f, 0x1b, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10,
  CMD(ST7735S_GAMCTRN1), 0x0f, 0x1b, 0x0f, 0x17, 0x33, 0x2c, 0x29, 0x2e,
                         0x30, 0x30, 0x39, 0x3f, 0x00, 0x07, 0x03, 0x10,
  CMD(0xf0), 0x01,
  CMD(0xf6), 0x00,
  CMD(ST7735S_COLMOD), 0x05,
  CMD(ST7735S_MADCTL), 0xa0,
};

//Funkcja wysylajaca komende do wyswietlacza (static oznacza ze funkcja nie jest widoczna
//w innych plikach, a ta funkcja bedzie uzywana tylko w tym pliku):
static void lcd_cmd(uint8_t cmd) {

	//Ustawienie stanu niskiego na linii DC oznacza ze wysylamy komende:
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
	//Ustawienie stanu niskiego na linii CS oznacza ze rozpoczynamy komunikacje SPI:
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	//Wyslanie komendy przekazanej do funkcji:
	HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);
	//Ustawienie stanu wysokiego na linii CS oznacza ze konczymy komunikacje SPI:
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);

}

//Funkcja wysylajaca 8-bitow dane do wyswietlacza:
static void lcd_data(uint8_t data) {

	//Ustawienie stanu wysokiego na linii DC oznacza ze wysylamy dane:
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	//Ustawienie stanu niskiego na linii CS oznacza ze rozpoczynamy komunikacje SPI:
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	//Wyslanie danych przekazanych do funkcji:
	HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
	//Ustawienie stanu wysokiego na linii CS oznacza ze konczymy komunikacje SPI:
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);

}

//Funkcja wysylajaca 16-bitowe dane do wyswietlacza (najpierw wysylany jest bardziej
//znaczacy bajt, z pozniej mniej znaczacy):
static void lcd_data16(uint16_t value) {

	//Wyslanie bardziej znaczacego bajtu:
	lcd_data(value >> 8);
	//Wyslanie mniej znaczacego bajtu:
	lcd_data(value);

}

//Funkcja wysylajaca przekazywana wartosc jako komenda lub jako dane. Funkcja jako argument
//przyjmuje wartosc 16-bitowa. Na 9 bicie znajduje sie bit który oznacza czy wartosc bedzie
//wysylana jako komenda czy jako dane (1 - komenda, 0 - dane):
static void lcd_send(uint16_t value) {

	//Sprawdzenie czy wartosc ma byc wyslana jako komenda
	if(value & 0x100) {
		lcd_cmd(value);
	}
	//W przeciwnym razie wartosc jest wysylana jako dane:
	else {
		lcd_data(value);
	}

}

//Funkcja realizujaca inicjalizacje wyswietlacza (nie jest oznaczona jako statyczna,
//poniewaz bedzie wywolywana w pliku main.c):
void lcd_init(void) {

	int i;

	//Reset sterownika wyswietlacza:
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(100);

	//Wysylanie danych z tablicy do realizacji poczatkowej inicjalizacji wyswietlacza:
	for(i = 0; i < sizeof(init_table) / sizeof(uint16_t); i++) {
		lcd_send(init_table[i]);
	}

	HAL_Delay(200);

	//Obudzenie wyswietlacza z trybu uspienia:
	lcd_cmd(ST7735S_SLPOUT);

	HAL_Delay(120);

	//Wlaczenie wyswietlacza:
	lcd_cmd(ST7735S_DISPON);
}

//Definicja okna które bedzie rysowane na wyswietlaczu:
static void lcd_set_window(int x, int y, int width, int height)
{
  lcd_cmd(ST7735S_CASET);
  lcd_data16(LCD_OFFSET_X + x);
  lcd_data16(LCD_OFFSET_X + x + width - 1);

  lcd_cmd(ST7735S_RASET);
  lcd_data16(LCD_OFFSET_Y + y);
  lcd_data16(LCD_OFFSET_Y + y + height- 1);
}

//Funkcja rysujaca kolorowy prostokat:
void lcd_fill_box(int x, int y, int width, int height, uint16_t color) {

	//Ustawienie obszaru rysowania:
	lcd_set_window(x, y, width, height);

	//Przeslanie danych do kolorowania pojedynczych pikseli:
	lcd_cmd(ST7735S_RAMWR);
	for(int i = 0; i < width * height; i++) {
		lcd_data16(color);
	}

}

//Funkcja rysujaca pojedynczy pixel:
void lcd_put_pixel(int x, int y, uint16_t color) {

	//Rysowanie pojedynczego pixela sprowadza sie do narysowania prostokata o
	//wymiarach  1 x 1:
	lcd_fill_box(x, y, 1, 1, color);
}

