/*
 * display.c
 */
#include "board.h"
#include "compiler.h"
#include "dip204.h" //in the header, too, please!
#include "gpio.h"
#include "spi.h"
#include <avr32/io.h>
#include "delay.h"
//#include "pwm.h"





void init_display(unsigned long fcpu_hz)
{
  // gpio pin mapping and function select
  static const gpio_map_t dip204_spi_gpio_map =
  {
    {DIP204_SPI_SCK_PIN,  DIP204_SPI_SCK_FUNCTION }, // SPI clock
    {DIP204_SPI_MISO_PIN, DIP204_SPI_MISO_FUNCTION}, // MISO
    {DIP204_SPI_MOSI_PIN, DIP204_SPI_MOSI_FUNCTION}, // MOSI
    {DIP204_SPI_NPCS_PIN, DIP204_SPI_NPCS_FUNCTION}  // Chip Select
  };
  // spi bus options
  spi_options_t spi_options =
  {
    .reg          = DIP204_SPI_NPCS,
    .baudrate     = 1000000,
    .bits         = 8,
    .spck_delay   = 0,
    .trans_delay  = 0,
    .stay_act     = 1,
    .spi_mode     = 0,
    .modfdis      = 1
  };
  // enable specified gpio mapping
  gpio_enable_module(dip204_spi_gpio_map,
                     sizeof(dip204_spi_gpio_map) /
                       sizeof(dip204_spi_gpio_map[0]));
  // Init as master
  spi_initMaster(DIP204_SPI, &spi_options);
  // Set selection mode: variable_ps, pcs_decode, delay
  spi_selectionMode(DIP204_SPI, 0, 0, 0);
  // Enable SPI
  spi_enable(DIP204_SPI);
  // setup chip registers
  spi_setupChipReg(DIP204_SPI, &spi_options, fcpu_hz);
  // initialize LCD
  delay_init(fcpu_hz);

  dip204_init(backlight_PWM, TRUE);
}

