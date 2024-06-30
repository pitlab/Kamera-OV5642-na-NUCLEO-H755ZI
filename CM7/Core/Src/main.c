/* USER CODE BEGIN Header */
//////////////////////////////////////////////////////////////////////////////
//
// AutoPitLot v3.0
// Moduł pętli głównej obługi multimediów na rdzeniu Cortex-M7
//
// (c) PitLab 2024
// http://www.pitlab.pl
//////////////////////////////////////////////////////////////////////////////
/*Pamięć:
 * 0x30020000..0x30040000 - 128k (0x20000)stos lwIP
 * 0x30040000..0x30040200 - 512  (0x200) deskryptory DMA ETH
 * 0x30040200..0x38000000 - 130k (0x7FBEFE00) wolne
 *
 *
 *
 *
 *Zrobć:
 * Dodać funkcję wysyłania danych
 * Dodać znacznik czasu
 * */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "string.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "display.h"
#include "..\..\..\..\include\PoleceniaKomunikacyjne.h"
#include "analiza_obrazu.h"
#include "..\..\..\Common\Inc\errcode.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#endif

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma location=0x30000000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x30000200
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x30000000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x30000200))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __GNUC__ ) /* GNU Compiler */

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));   /* Ethernet Tx DMA Descriptors */
#endif

ETH_TxPacketConfig TxConfig;
COM_InitTypeDef BspCOMInit;
__IO uint32_t BspButtonState = BUTTON_RELEASED;
CRC_HandleTypeDef hcrc;
DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;
ETH_HandleTypeDef heth;
I2C_HandleTypeDef hi2c4;
TIM_HandleTypeDef htim14;
UART_HandleTypeDef huart7;
UART_HandleTypeDef huart2;

osThreadId defaultTaskHandle;
osThreadId wftKomUartHandle;
/* USER CODE BEGIN PV */
uint8_t chTrybPracy = TP_MENU;
uint8_t chPozycjaMenu = 0;
uint16_t sMenuTimer = 3200;
HAL_StatusTypeDef chErr = 0;

//extern uint8_t nowy_pomiar_mag;
//extern float fVectMag[3];	//wynik odczytany z magnetometru
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM14_Init(void);
static void MX_DCMI_Init(void);
static void MX_I2C4_Init(void);
static void MX_ETH_Init(void);
static void MX_CRC_Init(void);
static void MX_UART7_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void const * argument);
void StartKomUart(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
/* USER CODE BEGIN Boot_Mode_Sequence_0 */
  int32_t timeout;
/* USER CODE END Boot_Mode_Sequence_0 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

/* USER CODE BEGIN Boot_Mode_Sequence_1 */
  /* Wait until CPU2 boots and enters in stop mode or timeout*/
  timeout = 0xFFFF;
  while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) != RESET) && (timeout-- > 0));
  if ( timeout < 0 )
  {
  Error_Handler();
  }
/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
/* USER CODE BEGIN Boot_Mode_Sequence_2 */
/* When system initialization is finished, Cortex-M7 will release Cortex-M4 by means of
HSEM notification */
/*HW semaphore Clock enable*/
__HAL_RCC_HSEM_CLK_ENABLE();
/*Take HSEM */
HAL_HSEM_FastTake(HSEM_ID_0);
/*Release HSEM in order to notify the CPU2(CM4)*/
HAL_HSEM_Release(HSEM_ID_0,0);
/* wait until CPU2 wakes up from stop mode */
timeout = 0xFFFF;
while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && (timeout-- > 0));
if ( timeout < 0 )
{
Error_Handler();
}
/* USER CODE END Boot_Mode_Sequence_2 */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM14_Init();
  //MX_DCMI_Init();		//jest inicjowany w funkcji KameraInit()
  MX_I2C4_Init();
  MX_ETH_Init();
  MX_CRC_Init();
  MX_UART7_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  chErr  = KameraInit();
  chErr += InitEth(&heth);
  InitDisplay();
  InitProtokol();
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of wftKomUart */
  osThreadDef(wftKomUart, StartKomUart, osPriorityBelowNormal, 0, 128);
  wftKomUartHandle = osThreadCreate(osThread(wftKomUart), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Initialize leds */
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_YELLOW);
  BSP_LED_Init(LED_RED);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN BSP */
  /* -- Sample board code to send message over COM1 port ---- */
  printf("\n\rAutoPitLot melduje gotowosc do pracy\n\r");
  printf("SysCLK = %lu MHz\n\r", (uint32_t)HAL_RCC_GetSysClockFreq()/1000000);



  /* -- Sample board code to switch on leds ---- */
  BSP_LED_On(LED_GREEN);
  BSP_LED_On(LED_YELLOW);
  //BSP_LED_On(LED_RED);
  /* USER CODE END BSP */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_CSI|RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.CSIState = RCC_CSI_ON;
  RCC_OscInitStruct.CSICalibrationValue = RCC_CSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 4;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief DCMI Initialization Function
  * @param None
  * @retval None
  */
static void MX_DCMI_Init(void)
{

  /* USER CODE BEGIN DCMI_Init 0 */

  /* USER CODE END DCMI_Init 0 */

  /* USER CODE BEGIN DCMI_Init 1 */

  /* USER CODE END DCMI_Init 1 */
  hdcmi.Instance = DCMI;
  hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
  hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_FALLING;
  hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
  hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_HIGH;
  hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
  hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
  hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
  hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
  hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
  hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;
  if (HAL_DCMI_Init(&hdcmi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DCMI_Init 2 */

  /* USER CODE END DCMI_Init 2 */

}

/**
  * @brief ETH Initialization Function
  * @param None
  * @retval None
  */
static void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

   static uint8_t MACAddr[6];

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH;
  MACAddr[0] = 0x00;
  MACAddr[1] = 0x80;
  MACAddr[2] = 0xE1;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x00;
  MACAddr[5] = 0x00;
  heth.Init.MACAddr = &MACAddr[0];
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = 1524;

  /* USER CODE BEGIN MACADDRESS */

  /* USER CODE END MACADDRESS */

  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    Error_Handler();
  }

  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  /* USER CODE BEGIN ETH_Init 2 */

  /* USER CODE END ETH_Init 2 */

}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C4_Init(void)
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x00000E14;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 0;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 5;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 2;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim14, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */
  HAL_TIM_MspPostInit(&htim14);

}

/**
  * @brief UART7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART7_Init(void)
{

  /* USER CODE BEGIN UART7_Init 0 */

  /* USER CODE END UART7_Init 0 */

  /* USER CODE BEGIN UART7_Init 1 */

  /* USER CODE END UART7_Init 1 */
  huart7.Instance = UART7;
  huart7.Init.BaudRate = 115200;
  huart7.Init.WordLength = UART_WORDLENGTH_8B;
  huart7.Init.StopBits = UART_STOPBITS_1;
  huart7.Init.Parity = UART_PARITY_NONE;
  huart7.Init.Mode = UART_MODE_TX_RX;
  huart7.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart7.Init.OverSampling = UART_OVERSAMPLING_16;
  huart7.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart7.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart7.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart7) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart7, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart7, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart7) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART7_Init 2 */

  /* USER CODE END UART7_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_WR_Pin|LCD_RST_Pin|LCD_RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_RD_Pin|LCD_D6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CAM_PWDN_GPIO_Port, CAM_PWDN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, LCD_D5_Pin|LCD_D3_Pin|LCD_D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_D1_GPIO_Port, LCD_D1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, LCD_D0_Pin|LCD_D7_Pin|LCD_D2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LCD_WR_Pin LCD_RST_Pin LCD_RS_Pin */
  GPIO_InitStruct.Pin = LCD_WR_Pin|LCD_RST_Pin|LCD_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RD_Pin LCD_D6_Pin */
  GPIO_InitStruct.Pin = LCD_RD_Pin|LCD_D6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_CS_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CAM_PWDN_Pin */
  GPIO_InitStruct.Pin = CAM_PWDN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CAM_PWDN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_D5_Pin LCD_D3_Pin LCD_D4_Pin */
  GPIO_InitStruct.Pin = LCD_D5_Pin|LCD_D3_Pin|LCD_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_D1_Pin */
  GPIO_InitStruct.Pin = LCD_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_D1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_D0_Pin LCD_D7_Pin LCD_D2_Pin */
  GPIO_InitStruct.Pin = LCD_D0_Pin|LCD_D7_Pin|LCD_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

////////////////////////////////////////////////////////////////////////////////
// Sterowanie pinem Powwer Down kamery
// Parametry: akcja do wykonania na pinie: GPIO_PIN_SET lub GPIO_PIN_RESET
// Zwraca: kod błędu HAL
////////////////////////////////////////////////////////////////////////////////
void KameraPWDN(uint32_t SetReset)
{
	HAL_GPIO_WritePin(CAM_PWDN_GPIO_Port, CAM_PWDN_Pin, SetReset);		//włącz PWDN
}



/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
	extern volatile uint8_t chObrazGotowy;
  /* Infinite loop */
	Menu(chPozycjaMenu);
	for(;;)
	{
	  	//obsługa przycisku
	  	if (BspButtonState == BUTTON_PRESSED)
	  	{
	  		BspButtonState = BUTTON_RELEASED;
	  		if (chTrybPracy != TP_MENU)
	  		  chRysujRaz = 1;
	  		chPozycjaMenu = Menu(chPozycjaMenu);
	  		chTrybPracy = TP_MENU;
	  		sMenuTimer = 3000;	//czas w [ms]
	  	}

	  	//obsługa trybów pracy
	      switch(chTrybPracy)
	      {
	      	case TP_MENU:
	      		if (sMenuTimer)
	  				RysujMenuTimer(sMenuTimer/10);
	  			else
	  			{
	  				chTrybPracy = chPozycjaMenu;
	  				LCD_clear();
	  				chRysujRaz = 1;
	  			}
	  			break;

	      	case TP_KAMERA_RGB:
	      		if (chNowyObrazKamery)
	  			{
	  				drawBitmap(0, 0, 320, 240, (const unsigned short*)nBuforKamery);	//214ms
	  				chNowyObrazKamery = 0;
	  				WyswietlKodBledu(chErr, 10, 220);
	  			}
	  			break;

	      	case TP_CAN_MAGN:
				WyswietlDaneFloat("mag X", 0, 100);
				WyswietlDaneFloat("mag Y", 1, 140);
				WyswietlDaneFloat("mag Z", 2, 180);
				HAL_Delay(100);
	  			break;

	      	case TP_ANALIZA_ETH:
      			chErr = AnalizujEth(&heth);
	      		HAL_Delay(100);
	  			break;

	      	case TP_KAM_SET2:
	      		if (chRysujRaz)
	  			{
	      			chErr = KameraInit();
	  				chRysujRaz = 0;
	  				chTrybPracy = TP_KAMERA_RGB;
	  			}
	  			break;

	      	case TP_HIST_RGB:		//histogram obrazu RGB565
				uint8_t histR[32], histG[64], histB[32];
				chObrazGotowy = 0;
				uint32_t nCzasOper, nCzasCalk;		//czas całkowity i czas operacji
				chErr = ZrobZdjecie(320, 240);
				if (!chErr)
				{
					do; while (!chObrazGotowy);	//czekaj na zakończenie transferu DMA
					drawBitmap(0, 0, 320, 240, (unsigned short*)nBuforKamery);	//214ms
					nCzasOper = HAL_GetTick();
					HistogramRGB565((uint8_t*)nBuforKamery, histR, histG, histB, 320*240);
					nCzasOper = MinalCzas(nCzasOper);

					//rysuj histogram na  ekranie
					setColor(RED);
					for (uint8_t x=0; x<32; x++)
						fillRect(x*2, 240-histR[x], x*2+1, 240);
					setColor(GREEN);
					for (uint8_t x=0; x<64; x++)
						fillRect(x*2+64, 240-histG[x], x*2+65, 240);
					setColor(BLUE);
					for (uint8_t x=0; x<32; x++)
						fillRect(x*2+192, 240-histB[x], x*2+193, 240);
					setColor(GREEN);
					sprintf(chNapis, "To:  %ld ms", nCzasOper);	//czas operacji i całkowity
					print(chNapis, 1, 225, 0);
	  			}
	  			break;

	      	case TP_HIST_BIT:		//histogram bitów obrazu kamery
				chErr = CzekajNaBit(&chObrazGotowy, 1000);	//czekaj z timeoutem na zakończenie transferu DMA
				if (!chErr)
				{
					uint16_t m, pix;
					uint32_t histogram[16];

					do; while (!chObrazGotowy);	//czekaj na zakończenie transferu DMA
					drawBitmap(0, 0, 320, 240, (unsigned short*)nBuforKamery);	//214ms
					for (uint32_t n=0; n<320*240; n++)
					{
						pix = *((uint16_t*)nBuforKamery + n);
						for (m=0; m<16; m++)
						{

							if (pix & 0x01)
								histogram[m]++;
							pix >>= 1;
						}
					}

					//normalizacja histogramu
					for (uint8_t x=0; x<16; x++)
					{
						histogram[x] >>= 9;
						if (histogram[x] > 0xFF)
							histogram[x] = 0xFF;
					}

					//rysuj histogram na  ekranie
					setColor(BLUE);
					for (uint8_t x=0; x<16; x++)
						fillRect(x*10, 240-histogram[x], x*10+6, 240);
				}
				chObrazGotowy = 0;
				chErr = ZrobZdjecie(320, 240);
	  			break;

	      	case TP_KAM_CB:
	      		chErr = CzekajNaBit(&chObrazGotowy, 1000);	//czekaj z timeoutem na zakończenie transferu DMA
				if (!chErr)
				{
					nCzasCalk = nCzasOper = HAL_GetTick();
	      			KonwersjaRGB565doCB7((uint16_t*)nBuforKamery, chBuforCB, ROZM_BUF_CB);
	      			nCzasOper = MinalCzas(nCzasOper);
	      			KonwersjaCB7doRGB565(chBuforCB, (uint16_t*)nBuforKamery, ROZM_BUF_CB);
	      			nCzasCalk = MinalCzas(nCzasCalk);
	      			drawBitmap(0, 0, 320, 240, (unsigned short*)nBuforKamery);
	      			setColor(GREEN);
	      			sprintf(chNapis, "To/Tca:  %ld / %ld ms", nCzasOper, nCzasCalk);	//czas operacji i całkowity
					print(chNapis, 10, 220, 0);
	  			}
				chObrazGotowy = 0;
				chErr = ZrobZdjecie(320, 240);
	  			break;

	      	case TP_DET_KRAW_ROB:
	      		chErr = CzekajNaBit(&chObrazGotowy, 1000);	//czekaj z timeoutem na zakończenie transferu DMA
				if (!chErr)
				{
					nCzasCalk = HAL_GetTick();
					KonwersjaRGB565doCB7((uint16_t*)nBuforKamery, chBuforCB, ROZM_BUF_CB);
					nCzasOper = HAL_GetTick();
					DetekcjaKrawedziRoberts(chBuforCB, chBuforCKraw,  320,  240, 16);
					nCzasOper = MinalCzas(nCzasOper);
					KonwersjaCB7doRGB565(chBuforCKraw, (uint16_t*)nBuforKamery, ROZM_BUF_CB);
					nCzasCalk = MinalCzas(nCzasCalk);
					drawBitmap(0, 0, 320, 240, (unsigned short*)nBuforKamery);
					setColor(GREEN);
					sprintf(chNapis, "To/Tca:  %ld / %ld ms", nCzasOper, nCzasCalk);	//czas operacji i całkowity
					print(chNapis, 10, 220, 0);
				}
				chObrazGotowy = 0;
				chErr = ZrobZdjecie(320, 240);
	      		break;

	      	case TP_DET_KRAW_SOB:
				chErr = CzekajNaBit(&chObrazGotowy, 1000);	//czekaj z timeoutem na zakończenie transferu DMA
				if (!chErr)
				{
					nCzasCalk = HAL_GetTick();
					KonwersjaRGB565doCB7((uint16_t*)nBuforKamery, chBuforCB, ROZM_BUF_CB);
					nCzasOper = HAL_GetTick();
					DetekcjaKrawedziSobel(chBuforCB, chBuforCKraw,  320,  240, 16);
					nCzasOper = MinalCzas(nCzasOper);
					KonwersjaCB7doRGB565(chBuforCKraw, (uint16_t*)nBuforKamery, ROZM_BUF_CB);
					nCzasCalk = MinalCzas(nCzasCalk);
					drawBitmap(0, 0, 320, 240, (unsigned short*)nBuforKamery);
					setColor(GREEN);
					sprintf(chNapis, "To/Tca:  %ld / %ld ms", nCzasOper, nCzasCalk);	//czas operacji i całkowity
					print(chNapis, 1, 220, 0);
				}
				chObrazGotowy = 0;
				chErr = ZrobZdjecie(320, 240);
				break;

	      	case TP_ODSZUMIANIE:
				chErr = CzekajNaBit(&chObrazGotowy, 1000);	//czekaj z timeoutem na zakończenie transferu DMA
				if (!chErr)
				{
					nCzasCalk = HAL_GetTick();
					KonwersjaRGB565doCB7((uint16_t*)nBuforKamery, chBuforCB, ROZM_BUF_CB);
					DetekcjaKrawedziRoberts(chBuforCB, chBuforCKraw,  320,  240, 16);
					nCzasOper = HAL_GetTick();
					Odszumianie(chBuforCKraw, chBuforCB, 320, 240, 16);
					nCzasOper = MinalCzas(nCzasOper);
					KonwersjaCB7doRGB565(chBuforCB, (uint16_t*)nBuforKamery, ROZM_BUF_CB);
					nCzasCalk = MinalCzas(nCzasCalk);
					drawBitmap(0, 0, 320, 240, (unsigned short*)nBuforKamery);
					setColor(GREEN);
					sprintf(chNapis, "To/Tca:  %ld / %ld ms", nCzasOper, nCzasCalk);	//czas operacji i całkowity
					print(chNapis, 1, 225, 0);
					chObrazGotowy = 0;
					chErr = ZrobZdjecie(320, 240);
				}
				break;

	      	case TP_DYLATACJA:
	      		chErr = CzekajNaBit(&chObrazGotowy, 1000);	//czekaj z timeoutem na zakończenie transferu DMA
				if (!chErr)
				{
					nCzasCalk = HAL_GetTick();
					KonwersjaRGB565doCB7((uint16_t*)nBuforKamery, chBuforCB, ROZM_BUF_CB);
					DetekcjaKrawedziSobel(chBuforCB, chBuforCKraw,  320,  240, 16);
					//DetekcjaKrawedziRoberts(chBuforCB, chBuforCKraw,  320,  240, 16);
					Odszumianie(chBuforCKraw, chBuforCB, 320, 240, 16);
					nCzasOper = HAL_GetTick();
					Dylatacja(chBuforCB, chBuforCKraw, 320, 240, 16);
					nCzasOper = MinalCzas(nCzasOper);
					KonwersjaCB7doRGB565(chBuforCKraw, (uint16_t*)nBuforKamery, ROZM_BUF_CB);
					nCzasCalk = MinalCzas(nCzasCalk);
					drawBitmap(0, 0, 320, 240, (unsigned short*)nBuforKamery);
					setColor(GREEN);
					sprintf(chNapis, "To/Tca:  %ld / %ld ms", nCzasOper, nCzasCalk);	//czas operacji i całkowity
					print(chNapis, 1, 225, 0);
					chObrazGotowy = 0;
					chErr = ZrobZdjecie(320, 240);
				}
				break;

	      	case TP_DOMYKANIE:
	      		chErr = CzekajNaBit(&chObrazGotowy, 1000);	//czekaj z timeoutem na zakończenie transferu DMA
				if (!chErr)
				{
					nCzasCalk = HAL_GetTick();
					KonwersjaRGB565doCB7((uint16_t*)nBuforKamery, chBuforCB, ROZM_BUF_CB);
					DetekcjaKrawedziSobel(chBuforCB, chBuforCKraw,  320,  240, 16);
					//DetekcjaKrawedziRoberts(chBuforCB, chBuforCKraw,  320,  240, 16);
					Odszumianie(chBuforCKraw, chBuforCB, 320, 240, 16);
					nCzasOper = HAL_GetTick();
					Dylatacja(chBuforCB, chBuforCKraw, 320, 240, 16);
					Erozja(chBuforCKraw, chBuforCB, 320, 240, 16);
					nCzasOper = MinalCzas(nCzasOper);
					KonwersjaCB7doRGB565(chBuforCB, (uint16_t*)nBuforKamery, ROZM_BUF_CB);
					nCzasCalk = MinalCzas(nCzasCalk);
					drawBitmap(0, 0, 320, 240, (unsigned short*)nBuforKamery);
					setColor(GREEN);
					sprintf(chNapis, "To/Tca:  %ld / %ld ms", nCzasOper, nCzasCalk);	//czas operacji i całkowity
					print(chNapis, 1, 220, 0);
					chObrazGotowy = 0;
					chErr = ZrobZdjecie(320, 240);
				}
				break;


	      	case TP_FRAKTAL:	FraktalDemo();		break;
	      	//case TP_POMOC:		WyswietlPomoc();	break;

	      	case TP_ZDJECIE:		//wykonaj zdjęcie o podanych rozmiarach
	      		chErr = ZrobZdjecie(sSzerZdjecia, sWysZdjecia);
	      		if (chErr)
	      			chStatusZdjecia = SGZ_BLAD;		//wystapił błąd wykonania zdjecia
	      		else
	      			chStatusZdjecia = SGZ_GOTOWE;	//Zdjecie gotowe, można je pobrać
	      		WyswietlDane8("Wykonano zdjecie: ", chErr, 220);
	      		chTrybPracy = TP_KAMERA_RGB;		//wróć do wyświetlania obrazu
	      		chNowyObrazKamery = 1;
	      		break;


	      	default:			break;
	      }

	      /* USER CODE BEGIN 3 */
	      //BSP_LED_Toggle(LED_YELLOW);
	      BSP_LED_On(LED_YELLOW);

	      osDelay(1);
	}

  /* USER CODE END 5 */
}



////////////////////////////////////////////////////////////////////////////////
// Konfiguruje Memory Protection Unit. Czytaj Hardware Manual (PM0253)
//
// Parametry: nic
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct = {0};

	/* Disables the MPU */
	HAL_MPU_Disable();

	//Wyłącz dostęp do nieuzywanych zakresów pamięci
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.BaseAddress = 0x0;
	MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
	MPU_InitStruct.SubRegionDisable = 0x87;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;				//Strongly-ordered, shareable
	MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;			//współdzielone między procesory
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;		//nie ma dostępu do cache
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

    //DTCM D1: zmienne krytyczne, rozmiar=0x20000
	MPU_InitStruct.Number = MPU_REGION_NUMBER1;
	MPU_InitStruct.BaseAddress = 0x20000000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
	MPU_InitStruct.SubRegionDisable = 0x87;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;			//tylko procesor M7
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	//SRAM_AXI_D1: dane i bufory, stos, sterta, rozmiar=0x80000
	MPU_InitStruct.Number = MPU_REGION_NUMBER2;
	MPU_InitStruct.BaseAddress = 0x24000000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
	MPU_InitStruct.SubRegionDisable = 0x87;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;					//Strongly-ordered, shareable
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.SubRegionDisable = 0x87;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;					//Device, shareable
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;				//tylko ten obszar ma dostęp do cache
	MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	//SRAM_AHB1_D2: bufory DMA
	MPU_InitStruct.Number = MPU_REGION_NUMBER3;
	MPU_InitStruct.BaseAddress = 0x30000000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_128B;
	MPU_InitStruct.SubRegionDisable = 0x87;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;			//tylko procesor M7
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	//SRAM_AHB2_D2: bufory DMA, bufory ethernetu,  stos lwIP
	MPU_InitStruct.Number = MPU_REGION_NUMBER4;
	MPU_InitStruct.BaseAddress = 0x30020000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
	MPU_InitStruct.SubRegionDisable = 0x0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;					//Device, shareable
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;			//tylko procesor M7
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	//SRAM_AHB3_D2: deskryptory ethernet 2 x 256B
	MPU_InitStruct.Number = MPU_REGION_NUMBER5;
	MPU_InitStruct.BaseAddress = 0x30040000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_512B;
	MPU_InitStruct.SubRegionDisable = 0x0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;					//Strongly-ordered, shareable
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;				//współdzielone między procesory
	MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	//SRAM_AHB3_D2: bufory USB, współdzielenie danych, rozmiar=0x8000 (32k)
	MPU_InitStruct.Number = MPU_REGION_NUMBER6;
	MPU_InitStruct.BaseAddress = 0x30040200;
	MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
	MPU_InitStruct.SubRegionDisable = 0x0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;					//Strongly-ordered, shareable
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;				//współdzielone między procesory
	MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	//SRAM_AHB4_D3: backup w czasie stabdby, współdzielenie danych między rdzeniami, rozmiar=0x10000 (64k)
	MPU_InitStruct.Number = MPU_REGION_NUMBER6;
	MPU_InitStruct.BaseAddress = 0x38000000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
	MPU_InitStruct.SubRegionDisable = 0x0;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;				//współdzielone między procesory
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	//BACKUP: backup bateryjny, rozmiar=0x1000
	MPU_InitStruct.Number = MPU_REGION_NUMBER8;
	MPU_InitStruct.BaseAddress = 0x38800000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_4KB;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;					//Strongly-ordered, shareable
	MPU_InitStruct.SubRegionDisable = 0x0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;				//współdzielone między procesory
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Enables the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  BSP Push Button callback
  * @param  Button Specifies the pressed button
  * @retval None
  */
void BSP_PB_Callback(Button_TypeDef Button)
{
  if (Button == BUTTON_USER)
  {
    BspButtonState = BUTTON_PRESSED;
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
