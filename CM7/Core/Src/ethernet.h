/*
 * ethernet.h
 *
 *  Created on: May 24, 2024
 *      Author: PitLab
 */

#ifndef SRC_ETHERNET_H_
#define SRC_ETHERNET_H_

#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
//#include "stm32h755xx.h"
//#include "stm32h7xx_hal_eth.h"

//static uint8_t chRxBufEth[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE]    __attribute__ ((aligned (4)));

uint8_t  InitEth(ETH_HandleTypeDef* pEth);
uint8_t  AnalizujEth(ETH_HandleTypeDef* pEth);

extern void WyswietlAnalizeEth(uint8_t* chAdrEthNad, uint8_t* chAdrIPNad);

#endif /* SRC_ETHERNET_H_ */
