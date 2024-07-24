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

//Definicje stalych reprezentujacych kolory:
#define BLACK     0x0000
#define RED       0xf800
#define GREEN     0x07e0
#define BLUE      0x001f
#define YELLOW    0xffe0
#define MAGENTA   0xf81f
#define CYAN      0x07ff
#define WHITE     0xffff

//Funkcja realizujaca wstepna inicjalizacje wyswietlacza:
void lcd_init(void);

//Funkcja rysujaca prostokaty:
void lcd_fill_box(int x, int y, int width, int height, uint16_t color);
