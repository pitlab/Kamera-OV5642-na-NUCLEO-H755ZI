/*
 * sys_def.h
 *
 *  Created on: 30 gru 2019
 *      Author: PitLab
 */

#ifndef SYS_DEF_H_
#define SYS_DEF_H_
#include <math.h>


#define WER_GLOWNA	0
#define WER_PODRZ	1
#define WER_REPO	6	//numer wersji w repozytorium

#define	APB1_CLK	120000000


#define DEG2RAD     (float)(M_PI/180)   //przeliczanie stopni na radiany
#define DEG2RAD2    (float)(M_PI/360)   //przeliczanie stopni na radiany - użyj całego kąta
#define RAD2DEG     (float)(180/M_PI)   //przeliczanie radianów na stopnie
#define PIERW2		(float)(1.414213562)
#define CLEAR		16	//przesunięcie do czyszczenia bitu poleceniem BSSR


#define COL_R	0
#define COL_G	1
#define COL_B	2

//rozmiary wyświetlacza
#define LCD_SZER	320
#define LCD_WYS		240

//rozmiary buforów wyrażone w słowach 16-bit
#define ROZM_BUF_CB		LCD_SZER * LCD_WYS
#define ROZM_BUF16_LCD	LCD_SZER * LCD_WYS
#define ROZM_BUF32_KAM	LCD_SZER * LCD_WYS / 2

//tryby pracy
#define TP_KAMERA_RGB	0
#define TP_KAM_SET2		1
#define TP_HIST_BIT		2
#define TP_KAM_CB		3
#define TP_DET_KRAW_ROB	4
#define TP_DET_KRAW_SOB	5
#define TP_ODSZUMIANIE	6
#define TP_DYLATACJA	7
#define TP_DOMYKANIE	8

#define TP_HIST_RGB		10

#define TP_FRAKTAL		15
#define TP_ANALIZA_ETH	16
#define TP_POMOC		17

//miejsce na rozbudowe menu
#define TP_ZDJECIE		20	//wykonaj zdjęcie
#define TP_MENU			21
#define TP_CAN_MAGN		22




#define INTERF_UART		1
#define INTERF_ETH		2
#define INTERF_USB		3

#define ILOSC_INTERF_KOM	3


#endif /* SYS_DEF_H_ */

