/*
 * lcd.h
 *
 *  Created on: Jul 24, 2024
 *      Author: Secon
 */

//#pragma once oznacza ze kod zrodlowy nie bedzie zawarty przez preprocesor wiele razy
//(w wielu plikach) w ciagu jednej kompilacji
#pragma once

#include <stdint.h>
#include <stdbool.h>

//Stale definiujace rozdzielczosc wyswietlacza:
#define LCD_WIDTH 	160
#define LCD_HEIGHT 	128

//Definicje stalych reprezentujacych kolory z odwrocona kolejnoscia bajtow
//(poniewaz najpierw przesylamy starszy bajt, a pozniej mlodszy):
#define BLACK     0x0000
#define RED       0x00f8
#define GREEN     0xe007
#define BLUE      0x1f00
#define YELLOW    0xe0ff
#define MAGENTA   0x1ff8
#define CYAN      0xff07
#define WHITE     0xffff

//Funkcja realizujaca wstepna inicjalizacje wyswietlacza:
void lcd_init(void);

//Funkcja rysujaca prostokaty:
//void lcd_fill_box(int x, int y, int width, int height, uint16_t color);

//Funkcja rysujaca pojedynczy pixel:
void lcd_put_pixel(int x, int y, uint16_t color);

//Funkcja rysujaca obrazy:
//void lcd_draw_image(int x, int y, int width, int height, const uint8_t* data);

//Funkcja przesylajaca zawartosc bufora do wyswietlacza:
void lcd_copy(void);

//Funkcja wywolywana po zakonczeniu transmisji po SPI:
void lcd_transfer_done(void);

//Funkcja sprawdzajacy czy transmisja SPI dalej trwa (zwraca true gdy trwa, false gdy nie):
bool lcd_is_busy(void);
