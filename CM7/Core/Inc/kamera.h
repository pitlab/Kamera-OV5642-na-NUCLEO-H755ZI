/*
 * kamera.h
 *
 *  Created on: Apr 7, 2024
 *      Author: PitLab
 */

#ifndef INC_KAMERA_H_
#define INC_KAMERA_H_

#include "stm32h7xx_hal.h"
#include "ov5642_regs.h"
#include "sys_def.h"
#include "konfiguracja.h"

#define OV5642_I2C_ADR	0x78
#define OV5642_ID		0x5642
#define KAMERA_TIMEOUT	1	//czas w milisekundach na wysłanie jednego polecenia do kamery. Nominalnie jest to ok 400us na adres i 3 bajty danych
#define KAMERA_ZEGAR	20000000	//kamera wymaga zegara 24MHz (6-27MHz)
#define KAMERA_DZIELNIK_ZEGARA	(APB1_CLK / KAMERA_ZEGAR)
#define KAMERA_WYPELN_PWM		(KAMERA_DZIELNIK_ZEGARA / 2)	//wypełnienie PWM 50%


uint32_t nBuforKamery[ROZM_BUF32_KAM] __attribute__((section(".BuforyObrazu_AXI")));
uint8_t chBuforCB[ROZM_BUF_CB]  __attribute__((section(".BuforyObrazu_AXI")));
uint8_t chBuforCKraw[ROZM_BUF_CB]  __attribute__((section(".BuforyObrazu_AXI")));
uint16_t sLicznikLiniiKamery;
struct st_KonfKam KonfKam;

extern I2C_HandleTypeDef hi2c4;
extern DMA_HandleTypeDef hdma_dcmi;
extern DCMI_HandleTypeDef hdcmi;
extern uint8_t chNowyObrazKamery;
extern void KameraPWDN(uint32_t SetReset);

HAL_StatusTypeDef KameraInit(void);
HAL_StatusTypeDef RozpocznijPraceDCMI(uint8_t chAParat);
HAL_StatusTypeDef UstawKamere(typKonfKam *konf);
HAL_StatusTypeDef ZrobZdjecie(int16_t sSzerokosc, uint16_t sWysokosc);
HAL_StatusTypeDef ZrobZdjecie2(int16_t sSzerokosc, uint16_t sWysokosc, uint8_t rej);
uint8_t CzytajKamInit(void);
HAL_StatusTypeDef Wyslij_I2C_Kamera(uint16_t rejestr, uint8_t dane);
HAL_StatusTypeDef Czytaj_I2C_Kamera(uint16_t rejestr, uint8_t *dane);
HAL_StatusTypeDef Wyslij_Blok_Kamera(const struct sensor_reg reglist[]);
//void RAW2RGB(uint16_t *nBuforKamery);
//void RAW2RGB(uint8_t *chBuforKamery, uint16_t *sBuforLCD);
void RAW2RGB(uint32_t *nBufKamery, uint16_t *sBufLCD);
void OV5642_OutSize_Set(uint16_t offX, uint16_t offY, uint16_t width, uint16_t height);
uint8_t	SprawdzKamere(void);
uint8_t CzekajNaBit(volatile uint8_t *chBit, uint16_t sTimeout);


extern void Error_Handler(void);

#endif /* INC_KAMERA_H_ */
