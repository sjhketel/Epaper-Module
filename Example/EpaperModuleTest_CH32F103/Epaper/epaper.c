/*---------------------------------------
- WeAct Studio Official Link
- taobao: weactstudio.taobao.com
- aliexpress: weactstudio.aliexpress.com
- github: github.com/WeActTC
- gitee: gitee.com/WeAct-TC
- blog: www.weact-tc.cn
---------------------------------------*/

#include "epaper.h"
#include "epdfont.h"

#include "systick.h"

// epaper module
// res pin  -> pa8
// busy pin -> pa15
// d/c pin  -> pb14
// cs pin   -> pb12
// sck pin  -> pb13
// mosi pin -> pb15

EPD_PAINT EPD_Paint;

void epd_delay(uint16_t ms)
{
  delay(ms);
}

void epd_res_set()
{
  GPIO_SetBits(GPIOA, GPIO_Pin_8);
}

void epd_res_reset()
{
  GPIO_ResetBits(GPIOA, GPIO_Pin_8);
}

void epd_dc_set()
{
  GPIO_SetBits(GPIOB, GPIO_Pin_14);
}

void epd_dc_reset()
{
  GPIO_ResetBits(GPIOB, GPIO_Pin_14);
}

void epd_cs_set()
{
  GPIO_SetBits(GPIOB, GPIO_Pin_12);
}

void epd_cs_reset()
{
  GPIO_ResetBits(GPIOB, GPIO_Pin_12);
}

uint8_t epd_is_busy()
{
  return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == Bit_RESET ? 0 : 1;
}

void epd_io_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  SPI_InitTypeDef spiConfig = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

  /* configure the epaper module res pin */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* configure the epaper module busy pin */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* configure the epaper module d/c pin */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* configure the epaper module spi2 sck mosi pin */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* configure the epaper module cs pin */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* configure spi2 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

  spiConfig.SPI_DataSize = SPI_DataSize_8b;
  spiConfig.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
  spiConfig.SPI_Direction = SPI_Direction_1Line_Tx;
  spiConfig.SPI_FirstBit = SPI_FirstBit_MSB;
  spiConfig.SPI_Mode = SPI_Mode_Master;
  spiConfig.SPI_CPOL = SPI_CPOL_High;
  spiConfig.SPI_CPHA = SPI_CPHA_2Edge;
  spiConfig.SPI_NSS = SPI_NSS_Soft;
  SPI_Init(SPI2, &spiConfig);
  SPI_Cmd(SPI2, ENABLE);
}

void epd_write_reg(uint8_t reg)
{
  epd_dc_reset();
  epd_cs_reset();

  SPI_I2S_SendData(SPI2, reg);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) != RESET)
    ;

  epd_cs_set();
  epd_dc_set();
}

void epd_write_data(uint8_t data)
{
  epd_cs_reset();

  SPI_I2S_SendData(SPI2, data);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) != RESET)
    ;

  epd_cs_set();
}

void epd_read_data(uint8_t reg, uint8_t *data, uint8_t length)
{
  epd_cs_reset();

  epd_cs_set();
}

void _epd_write_data(uint8_t data)
{

  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
    ;
  SPI_I2S_SendData(SPI2, data);
}

void _epd_write_data_over()
{
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) != RESET)
    ;
}

uint8_t epd_wait_busy()
{
  uint32_t timeout = 0;
  while (epd_is_busy())
  {
    timeout++;
    if (timeout > 20000)
    {
      return 1;
    }
    epd_delay(1);
  }
  return 0;
}
uint8_t epd_init(void)
{
  epd_res_reset();
  epd_delay(50);
  epd_res_set();
  epd_delay(50);

  if (epd_wait_busy())
    return 1;

  epd_write_reg(0x12); // SWRESET

  if (epd_wait_busy())
    return 1;

  epd_write_reg(0x01); // Driver output control
  epd_write_data(0x27);
  epd_write_data(0x01);
  epd_write_data(0x01);

  epd_write_reg(0x11); // data entry mode
  epd_write_data(0x01);

  epd_write_reg(0x44); // set Ram-X address start/end position
  epd_write_data(0x00);
  epd_write_data(0x0F); // 0x0F-->(15+1)*8=128

  epd_write_reg(0x45);  // set Ram-Y address start/end position
  epd_write_data(0x27); // 0xF9-->(249+1)=250
  epd_write_data(0x01);
  epd_write_data(0x00);
  epd_write_data(0x00);

  epd_write_reg(0x3C); // BorderWavefrom
  epd_write_data(0x05);

  epd_write_reg(0x21); //  Display update control
  epd_write_data(0x00);
  epd_write_data(0x80);

  epd_write_reg(0x18); // Read built-in temperature sensor
  epd_write_data(0x80);

  epd_write_reg(0x4E); // set RAM x address count to 0;
  epd_write_data(0x00);
  epd_write_reg(0x4F); // set RAM y address count to 0x127;
  epd_write_data(0x27);
  epd_write_data(0x01);

  if (epd_wait_busy())
    return 1;

  return 0;
}

void epd_enter_deepsleepmode(uint8_t mode)
{
  epd_write_reg(0x10);
  epd_write_data(mode);
}

void epd_init_internalTempSensor(void)
{
  epd_write_reg(0x18);
  epd_write_data(0x80);

  epd_write_reg(0x1A);
  epd_write_data(0x7F);
  epd_write_data(0xF0);
}
uint8_t data[3];
int32_t epd_get_internalTempSensor(void)
{

  uint32_t temp;

  epd_read_data(0x1B, data, 3);

  temp = data[0];
  temp = temp << 4 | data[1] >> 4;

  if (temp & (1 << 11))
  {
    temp = (((~temp) & 0xfff) + 1) * 10 / 16;
    temp = -temp;
  }
  else
  {
    temp = temp * 10 / 16;
  }
  return temp;
}

void epd_update(void)
{
  epd_write_reg(0x22); // Display Update Control
  epd_write_data(0xF7);
  // epd_write_data(0xFF);
  epd_write_reg(0x20); // Activate Display Update Sequence

  epd_wait_busy();
}

void epd_setpos(uint16_t x, uint16_t y)
{
  uint8_t _x;
  uint16_t _y;

  _x = x / 8;
  if (y > 249)
    _y = 46;
  else
    _y = 295 - y;

  epd_write_reg(0x4E); // set RAM x address count to 0;
  epd_write_data(_x);
  epd_write_reg(0x4F); // set RAM y address count to 0x127;
  epd_write_data(_y & 0xff);
  epd_write_data(_y >> 8 & 0x01);
}

void epd_display(uint8_t *Image1, uint8_t *Image2)
{
  uint32_t Width, Height, i, j;
  uint32_t k = 0;
  Width = 250;
  Height = 16;
  epd_setpos(0, 0);
  epd_write_reg(0x24);
  epd_cs_reset();
  for (j = 0; j < Height; j++)
  {
    for (i = 0; i < Width; i++)
    {
      _epd_write_data(Image1[k]);
      k++;
    }
  }
  _epd_write_data_over();
  epd_cs_set();
  epd_setpos(0, 0);
  epd_write_reg(0x26);
  k = 0;
  epd_cs_reset();
  for (j = 0; j < Height; j++)
  {
    for (i = 0; i < Width; i++)
    {
      _epd_write_data(~Image2[k]);
      k++;
    }
  }
  _epd_write_data_over();
  epd_cs_set();
  epd_update();
}

void epd_displayBW(uint8_t *Image)
{
  uint32_t Width, Height, i, j;
  uint32_t k = 0;
  Width = 250;
  Height = 16;
  epd_setpos(0, 0);
  epd_write_reg(0x24);
  epd_cs_reset();
  for (j = 0; j < Height; j++)
  {
    for (i = 0; i < Width; i++)
    {
      _epd_write_data(Image[k]);
      k++;
    }
  }
  _epd_write_data_over();
  epd_cs_set();
  epd_update();
}

void epd_displayRED(uint8_t *Image)
{
  uint32_t Width, Height, i, j;
  uint32_t k = 0;
  Width = 250;
  Height = 16;
  epd_setpos(0, 0);
  epd_write_reg(0x26);
  epd_cs_reset();
  for (j = 0; j < Height; j++)
  {
    for (i = 0; i < Width; i++)
    {
      _epd_write_data(Image[k]);
      k++;
    }
  }
  _epd_write_data_over();
  epd_cs_set();
  epd_update();
}

void epd_paint_newimage(uint8_t *image, uint16_t Width, uint16_t Height, uint16_t Rotate, uint16_t Color)
{
  EPD_Paint.Image = 0x00;
  EPD_Paint.Image = image;

  EPD_Paint.WidthMemory = Width;
  EPD_Paint.HeightMemory = Height;
  EPD_Paint.Color = Color;
  EPD_Paint.WidthByte = (Width % 8 == 0) ? (Width / 8) : (Width / 8 + 1);
  EPD_Paint.HeightByte = Height;
  EPD_Paint.Rotate = Rotate;
  if (Rotate == EPD_ROTATE_0 || Rotate == EPD_ROTATE_180)
  {

    EPD_Paint.Width = Height;
    EPD_Paint.Height = Width;
  }
  else
  {
    EPD_Paint.Width = Width;
    EPD_Paint.Height = Height;
  }
}

void epd_paint_setpixel(uint16_t Xpoint, uint16_t Ypoint, uint16_t Color)
{
  uint16_t X, Y;
  uint32_t Addr;
  uint8_t Rdata;
  switch (EPD_Paint.Rotate)
  {
  case 0:

    X = EPD_Paint.WidthMemory - Ypoint - 1;
    Y = Xpoint;
    break;
  case 90:
    X = EPD_Paint.WidthMemory - Xpoint - 1;
    Y = EPD_Paint.HeightMemory - Ypoint - 1;
    break;
  case 180:
    X = Ypoint;
    Y = EPD_Paint.HeightMemory - Xpoint - 1;
    break;
  case 270:
    X = Xpoint;
    Y = Ypoint;
    break;
  default:
    return;
  }
  Addr = X / 8 + Y * EPD_Paint.WidthByte;
  Rdata = EPD_Paint.Image[Addr];
  if (Color == EPD_COLOR_BLACK)
  {
    EPD_Paint.Image[Addr] = Rdata & ~(0x80 >> (X % 8));
  }
  else
    EPD_Paint.Image[Addr] = Rdata | (0x80 >> (X % 8));
}

void epd_paint_clear(uint16_t color)
{
  uint16_t X, Y;
  uint32_t Addr;

  for (Y = 0; Y < EPD_Paint.HeightByte; Y++)
  {
    for (X = 0; X < EPD_Paint.WidthByte; X++)
    { // 8 pixel =  1 byte
      Addr = X + Y * EPD_Paint.WidthByte;
      EPD_Paint.Image[Addr] = color;
    }
  }
}

void epd_paint_selectimage(uint8_t *image)
{
  EPD_Paint.Image = image;
}

void epd_paint_drawPoint(uint16_t Xpoint, uint16_t Ypoint, uint16_t Color)
{
  epd_paint_setpixel(Xpoint - 1, Ypoint - 1, Color);
}

void epd_paint_drawLine(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t Color)
{
  uint16_t Xpoint, Ypoint;
  int32_t dx, dy;
  int32_t XAddway, YAddway;
  int32_t Esp;
  char Dotted_Len;
  Xpoint = Xstart;
  Ypoint = Ystart;
  dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
  dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;

  XAddway = Xstart < Xend ? 1 : -1;
  YAddway = Ystart < Yend ? 1 : -1;

  Esp = dx + dy;
  Dotted_Len = 0;

  for (;;)
  {
    Dotted_Len++;
    epd_paint_drawPoint(Xpoint, Ypoint, Color);
    if (2 * Esp >= dy)
    {
      if (Xpoint == Xend)
        break;
      Esp += dy;
      Xpoint += XAddway;
    }
    if (2 * Esp <= dx)
    {
      if (Ypoint == Yend)
        break;
      Esp += dx;
      Ypoint += YAddway;
    }
  }
}

void epd_paint_drawRectangle(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t Color, uint8_t mode)
{
  uint16_t i;
  if (mode)
  {
    for (i = Ystart; i < Yend; i++)
    {
      epd_paint_drawLine(Xstart, i, Xend, i, Color);
    }
  }
  else
  {
    epd_paint_drawLine(Xstart, Ystart, Xend, Ystart, Color);
    epd_paint_drawLine(Xstart, Ystart, Xstart, Yend, Color);
    epd_paint_drawLine(Xend, Yend, Xend, Ystart, Color);
    epd_paint_drawLine(Xend, Yend, Xstart, Yend, Color);
  }
}

void epd_paint_drawCircle(uint16_t X_Center, uint16_t Y_Center, uint16_t Radius, uint16_t Color, uint8_t mode)
{
  uint16_t Esp, sCountY;
  uint16_t XCurrent, YCurrent;
  XCurrent = 0;
  YCurrent = Radius;
  Esp = 3 - (Radius << 1);
  if (mode)
  {
    while (XCurrent <= YCurrent)
    { // Realistic circles
      for (sCountY = XCurrent; sCountY <= YCurrent; sCountY++)
      {
        epd_paint_drawPoint(X_Center + XCurrent, Y_Center + sCountY, Color); // 1
        epd_paint_drawPoint(X_Center - XCurrent, Y_Center + sCountY, Color); // 2
        epd_paint_drawPoint(X_Center - sCountY, Y_Center + XCurrent, Color); // 3
        epd_paint_drawPoint(X_Center - sCountY, Y_Center - XCurrent, Color); // 4
        epd_paint_drawPoint(X_Center - XCurrent, Y_Center - sCountY, Color); // 5
        epd_paint_drawPoint(X_Center + XCurrent, Y_Center - sCountY, Color); // 6
        epd_paint_drawPoint(X_Center + sCountY, Y_Center - XCurrent, Color); // 7
        epd_paint_drawPoint(X_Center + sCountY, Y_Center + XCurrent, Color);
      }
      if ((int)Esp < 0)
        Esp += 4 * XCurrent + 6;
      else
      {
        Esp += 10 + 4 * (XCurrent - YCurrent);
        YCurrent--;
      }
      XCurrent++;
    }
  }
  else
  { // Draw a hollow circle
    while (XCurrent <= YCurrent)
    {
      epd_paint_drawPoint(X_Center + XCurrent, Y_Center + YCurrent, Color); // 1
      epd_paint_drawPoint(X_Center - XCurrent, Y_Center + YCurrent, Color); // 2
      epd_paint_drawPoint(X_Center - YCurrent, Y_Center + XCurrent, Color); // 3
      epd_paint_drawPoint(X_Center - YCurrent, Y_Center - XCurrent, Color); // 4
      epd_paint_drawPoint(X_Center - XCurrent, Y_Center - YCurrent, Color); // 5
      epd_paint_drawPoint(X_Center + XCurrent, Y_Center - YCurrent, Color); // 6
      epd_paint_drawPoint(X_Center + YCurrent, Y_Center - XCurrent, Color); // 7
      epd_paint_drawPoint(X_Center + YCurrent, Y_Center + XCurrent, Color); // 0
      if ((int)Esp < 0)
        Esp += 4 * XCurrent + 6;
      else
      {
        Esp += 10 + 4 * (XCurrent - YCurrent);
        YCurrent--;
      }
      XCurrent++;
    }
  }
}

void epd_paint_showChar(uint16_t x, uint16_t y, uint16_t chr, uint16_t size1, uint16_t color)
{
  uint16_t i, m, temp, size2, chr1;
  uint16_t x0, y0;
  x += 1, y += 1, x0 = x, y0 = y;
  if (x - size1 > EPD_H)
    return;
  if (size1 == 8)
    size2 = 6;
  else
    size2 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * (size1 / 2);
  chr1 = chr - ' ';
  for (i = 0; i < size2; i++)
  {
    if (size1 == 8)
    {
      temp = asc2_0806[chr1][i];
    } // 0806
    else if (size1 == 12)
    {
      temp = asc2_1206[chr1][i];
    } // 1206
    else if (size1 == 16)
    {
      temp = asc2_1608[chr1][i];
    } // 1608
    else if (size1 == 24)
    {
      temp = asc2_2412[chr1][i];
    } // 2412
    else
      return;
    for (m = 0; m < 8; m++)
    {
      if (temp & 0x01)
        epd_paint_drawPoint(x, y, color);
      else
        epd_paint_drawPoint(x, y, !color);
      temp >>= 1;
      y++;
    }
    x++;
    if ((size1 != 8) && ((x - x0) == size1 / 2))
    {
      x = x0;
      y0 = y0 + 8;
    }
    y = y0;
  }
}

void epd_paint_showString(uint16_t x, uint16_t y, uint8_t *chr, uint16_t size1, uint16_t color)
{
  while (*chr != '\0')
  {
    epd_paint_showChar(x, y, *chr, size1, color);
    chr++;
    x += size1 / 2;
  }
}

// m^n
static uint32_t _Pow(uint16_t m, uint16_t n)
{
  uint32_t result = 1;
  while (n--)
  {
    result *= m;
  }
  return result;
}

void epd_paint_showNum(uint16_t x, uint16_t y, uint32_t num, uint16_t len, uint16_t size1, uint16_t color)
{
  uint8_t t, temp, m = 0;
  if (size1 == 8)
    m = 2;
  for (t = 0; t < len; t++)
  {
    temp = (num / _Pow(10, len - t - 1)) % 10;
    if (temp == 0)
    {
      epd_paint_showChar(x + (size1 / 2 + m) * t, y, '0', size1, color);
    }
    else
    {
      epd_paint_showChar(x + (size1 / 2 + m) * t, y, temp + '0', size1, color);
    }
  }
}

void epd_paint_showChinese(uint16_t x, uint16_t y, uint16_t num, uint16_t size1, uint16_t color)
{
  uint16_t m, temp;
  uint16_t x0, y0;
  uint16_t i, size3 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * size1;
  x += 1, y += 1, x0 = x, y0 = y;
  for (i = 0; i < size3; i++)
  {
    if (size1 == 16)
    {
      temp = Hzk1[num][i];
    } // 16*16
    else if (size1 == 24)
    {
      temp = Hzk2[num][i];
    } // 24*24
    else if (size1 == 32)
    {
      temp = Hzk3[num][i];
    } // 32*32
    else if (size1 == 64)
    {
      temp = Hzk4[num][i];
    } // 64*64
    else
      return;
    for (m = 0; m < 8; m++)
    {
      if (temp & 0x01)
        epd_paint_drawPoint(x, y, color);
      else
        epd_paint_drawPoint(x, y, !color);
      temp >>= 1;
      y++;
    }
    x++;
    if ((x - x0) == size1)
    {
      x = x0;
      y0 = y0 + 8;
    }
    y = y0;
  }
}

void epd_paint_showPicture(uint16_t x, uint16_t y, uint16_t sizex, uint16_t sizey, const uint8_t BMP[], uint16_t Color)
{
  uint16_t j = 0;
  uint16_t i, n = 0, temp = 0, m = 0;
  uint16_t x0 = 0, y0 = 0;
  x += 1, y += 1, x0 = x, y0 = y;
  sizey = sizey / 8 + ((sizey % 8) ? 1 : 0);
  for (n = 0; n < sizey; n++)
  {
    for (i = 0; i < sizex; i++)
    {
      temp = BMP[j];
      j++;
      for (m = 0; m < 8; m++)
      {
        if (temp & 0x01)
          epd_paint_drawPoint(x, y, !Color);
        else
          epd_paint_drawPoint(x, y, Color);
        temp >>= 1;
        y++;
      }
      x++;
      if ((x - x0) == sizex)
      {
        x = x0;
        y0 = y0 + 8;
      }
      y = y0;
    }
  }
}
