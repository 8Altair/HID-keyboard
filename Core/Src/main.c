/* USER CODE BEGIN Header */
/*
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_custom_hid_if.h"
#include <stdbool.h>
#include <string.h>	// For memset

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s2;
I2S_HandleTypeDef hi2s3;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;

static uint8_t keyboard_report[8] = {0};   // Modifier, reserved, key1..key6
static bool button_press_state = 0;	// 0 = unpressed, 1 = pressed
static bool button_last_state = 0;	// 0 = unpressed, 1 = pressed

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S2_Init(void);
static void MX_I2S3_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

void hid_send_key(uint8_t modifier, uint8_t keycode);
void hid_send_char(char c);
void hid_send_string(const char *s);
void hid_send_ascii_range(char from, char to);
uint8_t ascii_to_hid(char c, uint8_t *modifier, uint8_t *keycode);

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

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2S2_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  uint8_t button_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);


	  if (button_state && !button_last_state)	// Detect rising edge (button just pressed)
	  {
		  HAL_Delay(30);	// Simple debounce
		  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))  // Still pressed
		  {
			  button_press_state = !button_press_state;
			  if (button_press_state)  // Even press
			  {
				  hid_send_string("29402");	// Type "29402"
			  }
			  else	// Odd press
			  {
				  hid_send_ascii_range(36, 124);	// Type ASCII characters from 36 ('$') to 124 ('|')
			  }
		  }
	  }

	  button_last_state = button_state;
	  HAL_Delay(5);  // Small poll delay
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 200;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 5;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2S2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S2_Init(void)
{

  /* USER CODE BEGIN I2S2_Init 0 */

  /* USER CODE END I2S2_Init 0 */

  /* USER CODE BEGIN I2S2_Init 1 */

  /* USER CODE END I2S2_Init 1 */
  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_96K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_ENABLE;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S2_Init 2 */

  /* USER CODE END I2S2_Init 2 */

}

/**
  * @brief I2S3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S3_Init(void)
{

  /* USER CODE BEGIN I2S3_Init 0 */

  /* USER CODE END I2S3_Init 0 */

  /* USER CODE BEGIN I2S3_Init 1 */

  /* USER CODE END I2S3_Init 1 */
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_96K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S3_Init 2 */

  /* USER CODE END I2S3_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : DATA_Ready_Pin */
  GPIO_InitStruct.Pin = DATA_Ready_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DATA_Ready_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CS_I2C_SPI_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_I2C_SPI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : INT1_Pin INT2_Pin MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = INT1_Pin|INT2_Pin|MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LD4_Pin LD3_Pin LD5_Pin LD6_Pin
                           Audio_RST_Pin */
  GPIO_InitStruct.Pin = LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/*
  Send one HID keyboard key (press + release).
  modifier: bitfield (bit 1 = LeftShift, bit 2 = LeftCtrl, etc.)
  keycode : HID usage ID for the key.
*/
void hid_send_key(uint8_t modifier, uint8_t keycode)
{
	// Press
	memset(keyboard_report, 0, sizeof(keyboard_report));	// Clear report bytes
	keyboard_report[0] = modifier;	// Set first byte for modifier
	keyboard_report[2] = keycode;	// First key slot (only one used)

	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, keyboard_report, sizeof(keyboard_report));	// Key press
	HAL_Delay(20);   // Key hold time

	// Release
	memset(keyboard_report, 0, sizeof(keyboard_report));	// Reset memory
	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, keyboard_report, sizeof(keyboard_report));	// Nothing is being pressed
	HAL_Delay(245);	// Delay between sending of characters
}
/*
    Convert ASCII character to HID modifier + keycode.
    Returns 1 if conversion exists, 0 otherwise.

    NOTE: Mapping is for a standard HID Keyboard with Bosnian
    QWERTZ layout.
 */
uint8_t ascii_to_hid(char c, uint8_t *modifier, uint8_t *keycode)
{
	*modifier = 0;
	*keycode = 0;

	switch (c)
	{
		// From 36 to 47 ASCII
		case '$': *modifier = 0x02; *keycode = 0x21; break;	// Shift + 4
		case '%': *modifier = 0x02; *keycode = 0x22; break;	// Shift + 5
		case '&': *modifier = 0x02; *keycode = 0x23; break;	// Shift + 6
		case '\'': *keycode = 0x2D; break;	// Next to 0
		case '(': *modifier = 0x02; *keycode = 0x25; break;	// Shift + 8
		case ')': *modifier = 0x02; *keycode = 0x26; break;	// Shift + 9
		case '*': *modifier = 0x02; *keycode = 0x2E; break;	// Shift + '+' (last key row)
		case '+': *keycode = 0x2E; break;	// '+'
		case ',': *keycode = 0x36; break;
		case '-': *keycode = 0x38; break;	// '-' (near right Shift)
		case '.': *keycode = 0x37; break;
		case '/': *modifier = 0x02; *keycode = 0x24; break;	// Shift + 7

		// From 48 to 57 ASCII
		case '0': *keycode = 0x27; break;	// 0
		case '1': *keycode = 0x1E; break;	// 1
		case '2': *keycode = 0x1F; break;	// 2
		case '3': *keycode = 0x20; break;	// 3
		case '4': *keycode = 0x21; break;	// 4
		case '5': *keycode = 0x22; break;	// 5
		case '6': *keycode = 0x23; break;	// 6
		case '7': *keycode = 0x24; break;	// 7
		case '8': *keycode = 0x25; break;	// 8
		case '9': *keycode = 0x26; break;	// 9

		// From 58 to 64 ASCII
		case ':': *modifier = 0x02; *keycode = 0x37; break;	// Shift + '.'
		case ';': *modifier = 0x02; *keycode = 0x36; break;	// Shift + ','
		case '<': *modifier = 0x40; *keycode = 0x36; break;	// AltGr + ','
		case '=': *modifier = 0x02; *keycode = 0x27; break;	// Shift + 0
		case '>': *modifier = 0x40; *keycode = 0x37; break;	// AltGr + ','
		case '?': *modifier = 0x02; *keycode = 0x2D; break;	// Shift + ''
		case '@': *modifier = 0x40; *keycode = 0x19; break;	// AltGr + 'v'

		// From 65 to 90 ASCII
		case 'A': *modifier = 0x02; *keycode = 0x04; break;
		case 'B': *modifier = 0x02; *keycode = 0x05; break;
		case 'C': *modifier = 0x02; *keycode = 0x06; break;
		case 'D': *modifier = 0x02; *keycode = 0x07; break;
		case 'E': *modifier = 0x02; *keycode = 0x08; break;
		case 'F': *modifier = 0x02; *keycode = 0x09; break;
		case 'G': *modifier = 0x02; *keycode = 0x0A; break;
		case 'H': *modifier = 0x02; *keycode = 0x0B; break;
		case 'I': *modifier = 0x02; *keycode = 0x0C; break;
		case 'J': *modifier = 0x02; *keycode = 0x0D; break;
		case 'K': *modifier = 0x02; *keycode = 0x0E; break;
		case 'L': *modifier = 0x02; *keycode = 0x0F; break;
		case 'M': *modifier = 0x02; *keycode = 0x10; break;
		case 'N': *modifier = 0x02; *keycode = 0x11; break;
		case 'O': *modifier = 0x02; *keycode = 0x12; break;
		case 'P': *modifier = 0x02; *keycode = 0x13; break;
		case 'Q': *modifier = 0x02; *keycode = 0x14; break;
		case 'R': *modifier = 0x02; *keycode = 0x15; break;
		case 'S': *modifier = 0x02; *keycode = 0x16; break;
		case 'T': *modifier = 0x02; *keycode = 0x17; break;
		case 'U': *modifier = 0x02; *keycode = 0x18; break;
		case 'V': *modifier = 0x02; *keycode = 0x19; break;
		case 'W': *modifier = 0x02; *keycode = 0x1A; break;
		case 'X': *modifier = 0x02; *keycode = 0x1B; break;
		case 'Y': *modifier = 0x02; *keycode = 0x1D; break;	// QWERTZ: physical 'Y' key is usage 0x1D
		case 'Z': *modifier = 0x02; *keycode = 0x1C; break;	// QWERTZ: physical 'Z' key is usage 0x1C

		// From 91 to 96 ASCII
		case '[': *modifier = 0x40; *keycode = 0x09; break;	// AltGr + 'f'
		case '\\':*modifier = 0x40; *keycode = 0x14; break;	// AltGr + 'q'
		case ']': *modifier = 0x40; *keycode = 0x0A; break;	// AltGr + 'g'
		case '^': *modifier = 0x40; *keycode = 0x20; break;	// AltGr + 3
		case '_': *modifier = 0x02; *keycode = 0x38; break;	// Shift + '-'
		case '`': *modifier = 0x40; *keycode = 0x24; break;	// Shift + 7

		// From 97 to 122 ASCII
		case 'a': *keycode = 0x04; break;
		case 'b': *keycode = 0x05; break;
		case 'c': *keycode = 0x06; break;
		case 'd': *keycode = 0x07; break;
		case 'e': *keycode = 0x08; break;
		case 'f': *keycode = 0x09; break;
		case 'g': *keycode = 0x0A; break;
		case 'h': *keycode = 0x0B; break;
		case 'i': *keycode = 0x0C; break;
		case 'j': *keycode = 0x0D; break;
		case 'k': *keycode = 0x0E; break;
		case 'l': *keycode = 0x0F; break;
		case 'm': *keycode = 0x10; break;
		case 'n': *keycode = 0x11; break;
		case 'o': *keycode = 0x12; break;
		case 'p': *keycode = 0x13; break;
		case 'q': *keycode = 0x14; break;
		case 'r': *keycode = 0x15; break;
		case 's': *keycode = 0x16; break;
		case 't': *keycode = 0x17; break;
		case 'u': *keycode = 0x18; break;
		case 'v': *keycode = 0x19; break;
		case 'w': *keycode = 0x1A; break;
		case 'x': *keycode = 0x1B; break;
		case 'y': *keycode = 0x1D; break;	// Swapped y/z
		case 'z': *keycode = 0x1C; break;

		// From 123 to 124 ASCII
		case '{': *modifier = 0x40; *keycode = 0x05; break;	// AltGr + 'b'
		case '|': *modifier = 0x40; *keycode = 0x1A; break;	// AltGr + 'w'

		default:
		  return 0;   // Not implemented (ignored)
	  }

	  return 1;
}

void hid_send_char(char c)
{
	uint8_t mod, code;	// Modifier and keycode initialization
	if (!ascii_to_hid(c, &mod, &code)) return;   // Skip unknown character
	hid_send_key(mod, code);	// Send a key using the obtained modifier and code
}

void hid_send_string(const char *s)
{
	while (*s)
	{
		hid_send_char(*s++);
	}
}

void hid_send_ascii_range(char from, char to)
{
	for (char c = from; c <= to; c++)
	{
		hid_send_char(c);
	}
}


/* USER CODE END 4 */

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
#ifdef USE_FULL_ASSERT
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
