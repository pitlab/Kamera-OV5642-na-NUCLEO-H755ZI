/*
 * komunikacja.h
 *
 *  Created on: May 26, 2024
 *      Author: PitLab
 */

#ifndef INC_PROTOKOLKOM_H_
#define INC_PROTOKOLKOM_H_

#include "sys_def.h"
#include "stm32h7xx_nucleo.h"
#include "..\..\..\..\include\PoleceniaKomunikacyjne.h"
#include "konfiguracja.h"

#define ROZM_BUF_KOL	32	//rozmiar musi być potęgą 2 aby zapętlić bufor operacją AND

union _un8_16		//unia do konwersji między danymi 16 i 8 bit
{
	uint16_t dane16;
	uint8_t dane8[2];
} un8_16;

union _un8_32		//unia do konwersji między danymi 32 i 8 bit
{
	uint32_t dane32;
	uint8_t dane8[4];
} un8_32;

//definicje zmiennych
uint8_t chStanProtokolu[ILOSC_INTERF_KOM];
uint8_t chAdresZdalny[ILOSC_INTERF_KOM];	//adres sieciowy strony zdalnej
uint8_t chAdresLokalny;						//własny adres sieciowy
uint8_t chLicznikDanych[ILOSC_INTERF_KOM];
uint8_t chZnakCzasu[ILOSC_INTERF_KOM];
uint16_t sCrc16We;
uint8_t chRamkaWyj[ROZMIAR_RAMKI_UART];
uint8_t chBufKom[ROZM_BUF_KOL];	//bufor kołowy
volatile uint8_t chWskNap, chWskOpr;		//wskaźniki napełniania i opróżniania bufora kołowego
int16_t sSzerZdjecia, sWysZdjecia;
uint8_t chStatusZdjecia;		//status gotowości wykonania zdjęcia


//definicje funkcji lokalnych
uint8_t InitProtokol(void);
uint8_t AnalizujDaneKom(uint8_t chWe, uint8_t chInterfejs);
uint8_t DekodujRamke(uint8_t chWe, uint8_t *chAdrZdalny, uint8_t *chZnakCzasu, uint8_t *chPolecenie, uint8_t *chRozmDanych, uint8_t *chDane, uint8_t chInterfejs);
void InicjujCRC16(uint16_t sInit, uint16_t sWielomian);
uint16_t LiczCRC16(uint8_t chDane);
uint8_t PrzygotujRamke(uint8_t chAdrZdalny, uint8_t chAdrLokalny,  uint8_t chZnakCzasu, uint8_t chPolecenie, uint8_t chRozmDanych, uint8_t *chDane, uint8_t *chRamka);
uint8_t WyslijRamke(uint8_t chAdrZdalny, uint8_t chPolecenie, uint8_t chRozmDanych, uint8_t *chDane, uint8_t chInterfejs);
uint8_t Wyslij_OK(uint8_t chInterfejs);
uint8_t Wyslij_ERR(uint8_t chKodBledu, uint8_t chParametr, uint8_t chInterfejs);
void StartKomUart(void const * argument);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

//deklaracje zmiennych zdalnych
extern volatile uint32_t nCzasSystemowy;
extern uint32_t nBuforKamery[ROZM_BUF32_KAM];
extern uint8_t chTrybPracy;
//extern struct st_KonfKam KonfKam;

//deklaracje funkcji zdalnych


#endif /* INC_PROTOKOLKOM_H_ */

