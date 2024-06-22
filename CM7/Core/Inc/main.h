/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sys_def.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
#define BUTTON_RELEASED                    0U
#define BUTTON_PRESSED                     1U
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_WR_Pin GPIO_PIN_0
#define LCD_WR_GPIO_Port GPIOC
#define LCD_RST_Pin GPIO_PIN_2
#define LCD_RST_GPIO_Port GPIOC
#define LCD_RS_Pin GPIO_PIN_3
#define LCD_RS_GPIO_Port GPIOC
#define LCD_RD_Pin GPIO_PIN_3
#define LCD_RD_GPIO_Port GPIOA
#define LCD_CS_Pin GPIO_PIN_1
#define LCD_CS_GPIO_Port GPIOB
#define CAM_PWDN_Pin GPIO_PIN_11
#define CAM_PWDN_GPIO_Port GPIOF
#define LCD_D5_Pin GPIO_PIN_11
#define LCD_D5_GPIO_Port GPIOE
#define LCD_D3_Pin GPIO_PIN_13
#define LCD_D3_GPIO_Port GPIOE
#define LCD_D4_Pin GPIO_PIN_14
#define LCD_D4_GPIO_Port GPIOE
#define LCD_D1_Pin GPIO_PIN_15
#define LCD_D1_GPIO_Port GPIOD
#define LCD_D6_Pin GPIO_PIN_8
#define LCD_D6_GPIO_Port GPIOA
#define LCD_D0_Pin GPIO_PIN_9
#define LCD_D0_GPIO_Port GPIOG
#define LCD_D7_Pin GPIO_PIN_12
#define LCD_D7_GPIO_Port GPIOG
#define LCD_D2_Pin GPIO_PIN_14
#define LCD_D2_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */

void KameraPWDN(uint32_t SetReset);
extern uint16_t sBuforLCD[];
extern uint32_t nBuforKamery[];
extern char chNapis[];
extern unsigned char chRysujRaz;	//flaga informująca o konieczności jednorazowego narysowania statycznych elementów ekranu
extern void InitDisplay(void);
extern void LCD_clear(void);
extern void drawBitmap(int x, int y, int sx, int sy, const unsigned short* data);
extern void drawBitmapYUV(int x, int y, int sx, int sy, const unsigned short* data);
extern void fillRect(int x1, int y1, int x2, int y2);
extern void setColor(unsigned short color);
extern void WyswietlDane8(char *str, uint8_t dane, uint8_t pozY);
extern void WyswietlDane32(char *str, uint32_t dane, uint8_t pozY);
extern void WyswietlDaneFloat(char *str, float dane, uint8_t pozY);
extern void WyswietlKodBledu(uint8_t blad, uint8_t pozX, uint8_t pozY);
extern unsigned char Menu(unsigned char chPozycja);
extern void FraktalDemo(void);
extern void RysujMenuTimer(unsigned short sCzas);
extern void WyswietlPomoc(void);
extern HAL_StatusTypeDef KameraInit(void);
extern HAL_StatusTypeDef InitKamera1(void);
extern HAL_StatusTypeDef InitKamera2(void);
extern HAL_StatusTypeDef InitKamera3(void);
extern HAL_StatusTypeDef InitKamera4(void);
extern HAL_StatusTypeDef InitKamera5(void);
extern  uint8_t chNowyObrazKamery;
extern uint8_t CzytajKamInit(void);
extern void RAW2RGB(uint32_t *nBufKamery, uint16_t *sBufLCD);
extern HAL_StatusTypeDef ZrobZdjecie(int16_t sSzerokosc, uint16_t sWysokosc);
extern HAL_StatusTypeDef ZrobZdjecie2(int16_t sSzerokosc, uint16_t sWysokosc, uint8_t rej);
extern unsigned int MinalCzas(unsigned int nStart);

//Ethernet
extern uint8_t  InitEth(ETH_HandleTypeDef* pEth);
extern uint8_t  AnalizujEth(ETH_HandleTypeDef* pEth);

//Protokół komunikacyjny
extern uint8_t InitProtokol(void);
extern uint8_t OdbierzDaneKom(uint8_t chWe, uint8_t chInterfejs);
extern uint8_t Wyslij_OK(uint8_t chInterfejs);
extern int16_t sSzerZdjecia, sWysZdjecia;
extern uint8_t chStatusZdjecia;		//status gotowości wykonania zdjęcia
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
