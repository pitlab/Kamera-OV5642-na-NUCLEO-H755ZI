/*
 * lcd.h
 *
 *  Created on: 1 sty 2020
 *      Author: PitLab
 */

#ifndef LCD_H_
#define LCD_H_
#include "display.h"
#include "stm32h7xx_hal.h"

#define SRODKOWA_POZYCJA_MENU	9
#define OSTATNIA_POZYCJA_MENU	18

const char *build_date = __DATE__;
const char *build_time = __TIME__;

//definicje zmiennych
char chNapis[40];	//bufor znak�w na szeroko�c ekranu 320/8 = 40 znak�w
int x, x2, y, y2, y3, y4;	//pomocnicze wsp�rz�dne ekranowe
unsigned char n, m;			//zap�tlacze
unsigned char chOdswiez;	//licznik od�wie�ania ekranu
unsigned char chRysujRaz = 1;	//flaga informująca o konieczności jednorazowego narysowania statycznych elementów ekranu
//unsigned char chSelChan;	//wybrany kana�
//unsigned char chMode;		//tryb pracy
unsigned short sFractalBuf[DISP_X_SIZE*DISP_Y_SIZE];
float fReal, fImag;
unsigned char chHistogram[4][DISP_X_SIZE];	//histogram na ca�� szeroko�� ekranu
float fZoom, fX, fY;
unsigned char chMnozPalety;
unsigned char chDemoMode;
unsigned char chLiczIter;		//licznik iteracji fraktala
unsigned short sKolorTab[8] = {RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW, WHITE, BLACK};


//deklaracje zmiennych
extern unsigned char MidFont[];
extern unsigned char BigFont[];

//definicje funkcji
void InitDisplay(void);
unsigned char Menu(unsigned char chPozycja);
void RysujMenuTimer(unsigned short sCzas);
void LCD_Test(void);
void FraktalTest(unsigned char chTyp);
void FraktalDemo(void);
void GenerateJulia(unsigned short size_x, unsigned short size_y, unsigned short offset_x, unsigned short offset_y, unsigned short zoom, unsigned short * buffer);
void GenerateMandelbrot(float centre_X, float centre_Y, float Zoom, unsigned short IterationMax, unsigned short * buffer);
void InitFraktal(unsigned char chTyp);
unsigned int MinalCzas(unsigned int  nStart);
void WyswietlPomoc(void);
void WyswietlDane(char *str, uint8_t dane, uint8_t pozY);
void WyswietlDaneFloat(char *str, float dane, uint8_t pozY);
void WyswietlKodBledu(uint8_t blad, uint8_t pozX, uint8_t pozY);
void WyswietlAnalizeEth(uint8_t* chAdrEthNad, uint8_t* chAdrIPNad);

//deklaracje funkcji
extern void LCD_init(void);
extern void LCD_rect(unsigned short col, unsigned short row, unsigned short width, unsigned short height, unsigned short color) ;
extern void LCD_clear(void);
extern void LCD_Orient(unsigned char orient);
extern void Display_char(char znak);
extern void drawVLine(int x, int y, int len);
extern void drawHLine(int x, int y, int len);
extern void drawLine(int x1, int y1, int x2, int y2);
extern void setXY(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2);
extern void clrXY(void);
extern void setColorRGB(unsigned char r, unsigned char g, unsigned char b);
extern void setColor(unsigned short color);
extern unsigned short getColor(void);
extern void setBackColorRGB(unsigned char r, unsigned char g, unsigned char b);
extern void setBackColor(unsigned short color);
extern unsigned short getBackColor(void);
extern void setPixel(unsigned short color);
extern void drawPixel(int x, int y);
extern void drawRect(int x1, int y1, int x2, int y2);
extern void drawRoundRect(int x1, int y1, int x2, int y2);
extern void fillRect(int x1, int y1, int x2, int y2);
extern void fillRoundRect(int x1, int y1, int x2, int y2);
extern void printChar(unsigned char c, int x, int y);
extern unsigned char printChar2(unsigned char c, int x, int y);
extern void rotateChar(unsigned char c, int x, int y, int pos, int deg);
extern void setFont(unsigned char* font);
extern unsigned char GetFontX(void);
extern unsigned char GetFontY(void);
extern void print(char *st, int x, int y, int deg);
extern void print2(char *st, int x, int y);
extern void fillCircle(int x, int y, int radius);
extern void drawCircle(int x, int y, int radius);
extern void drawBitmap(int x, int y, int sx, int sy, const unsigned short* data);

#endif /* LCD_H_ */
