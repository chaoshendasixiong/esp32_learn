#include "lcd_init.h" 

//-----------------LCD端口定义----------------

#define LCD_SCLK_Clr() gpio_set_level(MY_CLK_PIN, 0); //LCD_SCL=0//SCL=SCLK  
#define LCD_SCLK_Set() gpio_set_level(MY_CLK_PIN, 1); //LCD_SCL=1

#define LCD_MOSI_Clr() gpio_set_level(MY_MOSI_PIN, 0); //LCD_SDA=0//SDA=MOSI
#define LCD_MOSI_Set() gpio_set_level(MY_MOSI_PIN, 1); //LCD_SDA=1

#define LCD_RES_Clr() gpio_set_level(MY_RES_PIN, 0); //LCD_RES=0//RES
#define LCD_RES_Set() gpio_set_level(MY_RES_PIN, 1); //LCD_RES=1

#define LCD_DC_Clr() gpio_set_level(MY_DC_PIN, 0); //LCD_DC=0//DC
#define LCD_DC_Set() gpio_set_level(MY_DC_PIN, 1); //LCD_DC=1

#define LCD_BLK_Clr()  gpio_set_level(MY_BLK_PIN, 0); //LCD_BLK=0//BLK
#define LCD_BLK_Set()  gpio_set_level(MY_BLK_PIN, 1); //LCD_BLK=1

#define delay_ms(ms)  vTaskDelay(ms/portTICK_PERIOD_MS)
#define delay_us    os_delay_us
#define TFT_REST(v) gpio_set_level( MY_RES_PIN, v); 
#define TFT_DC(v)   gpio_set_level( MY_DC_PIN, v); 
#define TFT_BLK(v)  gpio_set_level( MY_BLK_PIN, v); 
static void IRAM_ATTR spi_event_callback(int event, void *arg) {}

static inline int spi_send(uint8_t *pData, uint32_t len) {
    if(len <=0) return 0;
    #define DATA_BUF_SIZE 64
    uint32_t i;
    uint32_t remain;
    uint8_t data_buf[DATA_BUF_SIZE];
    spi_trans_t trans = {0}; 

    for(i = 0; i < len/DATA_BUF_SIZE; i++) {
        memcpy(data_buf, pData+i*DATA_BUF_SIZE, DATA_BUF_SIZE);
        trans.mosi =  (uint32_t*)data_buf;
        trans.bits.mosi = DATA_BUF_SIZE*8;// bit单位
        spi_trans(HSPI_HOST,&trans);
    }
    remain = len%DATA_BUF_SIZE;
    if(remain == 0) return 0;
    memcpy(data_buf, pData+i*DATA_BUF_SIZE, remain);
    trans.mosi =  (uint32_t*)data_buf;
    trans.bits.mosi = remain*8;// bit单位
    spi_trans(HSPI_HOST,&trans);
    return 0;
}

static void tft_reset() {
    #define ili9341_cmd LCD_WR_REG
    #define ili9341_dat LCD_WR_DATA8
    delay_ms(100);
    TFT_DC(1);
    TFT_REST(0);
    delay_us(10);
    TFT_REST(1);
    delay_ms(100);
    TFT_BLK(1);

    ili9341_cmd(0x11);    // Sleep Out
    delay_ms(100);

    ili9341_cmd(0xCF);
	ili9341_dat(0x00);
	ili9341_dat(0xC1);
	ili9341_dat(0X30);

    ili9341_cmd(0xED);
	ili9341_dat(0x64);
	ili9341_dat(0x03);
	ili9341_dat(0X12);
	ili9341_dat(0X81);
	ili9341_cmd(0xE8);
	ili9341_dat(0x85);
	ili9341_dat(0x00);
	ili9341_dat(0x79);
	ili9341_cmd(0xCB);
	ili9341_dat(0x39);
	ili9341_dat(0x2C);
	ili9341_dat(0x00);
	ili9341_dat(0x34);
	ili9341_dat(0x02);
	ili9341_cmd(0xF7);
	ili9341_dat(0x20);
	ili9341_cmd(0xEA);
	ili9341_dat(0x00);
	ili9341_dat(0x00);
	ili9341_cmd(0xC0); //Power control
	ili9341_dat(0x1D); //VRH[5:0]
	ili9341_cmd(0xC1); //Power control
	ili9341_dat(0x12); //SAP[2:0];BT[3:0]
	ili9341_cmd(0xC5); //VCM control
	ili9341_dat(0x33);
	ili9341_dat(0x3F);
	ili9341_cmd(0xC7); //VCM control
	ili9341_dat(0x92);
    ili9341_cmd(0x3A);    // Pixel Format
    ili9341_dat(0x55);   // 16bit Color
    ili9341_cmd(0x36); // Memory Access Control
	if(USE_HORIZONTAL==0)ili9341_dat(0x08);
	else if(USE_HORIZONTAL==1)ili9341_dat(0xC8);
	else if(USE_HORIZONTAL==2)ili9341_dat(0x78);
	else ili9341_dat(0xA8);

    ili9341_cmd(0xB1);// Frame Control
	ili9341_dat(0x00);
	ili9341_dat(0x12);
	ili9341_cmd(0xB6); // Display Function Control
	ili9341_dat(0x0A);
	ili9341_dat(0xA2);

	ili9341_cmd(0x44);
	ili9341_dat(0x02);

	ili9341_cmd(0xF2); // 3Gamma Function Disable
	ili9341_dat(0x00);
	ili9341_cmd(0x26); //Gamma curve selected
	ili9341_dat(0x01);
	ili9341_cmd(0xE0); //Set Gamma
	ili9341_dat(0x0F);
	ili9341_dat(0x22);
	ili9341_dat(0x1C);
	ili9341_dat(0x1B);
	ili9341_dat(0x08);
	ili9341_dat(0x0F);
	ili9341_dat(0x48);
	ili9341_dat(0xB8);
	ili9341_dat(0x34);
	ili9341_dat(0x05);
	ili9341_dat(0x0C);
	ili9341_dat(0x09);
	ili9341_dat(0x0F);
	ili9341_dat(0x07);
	ili9341_dat(0x00);
	ili9341_cmd(0XE1); //Set Gamma
	ili9341_dat(0x00);
	ili9341_dat(0x23);
	ili9341_dat(0x24);
	ili9341_dat(0x07);
	ili9341_dat(0x10);
	ili9341_dat(0x07);
	ili9341_dat(0x38);
	ili9341_dat(0x47);
	ili9341_dat(0x4B);
	ili9341_dat(0x0A);
	ili9341_dat(0x13);
	ili9341_dat(0x06);
	ili9341_dat(0x30);
	ili9341_dat(0x38);
	ili9341_dat(0x0F);

    ili9341_cmd(0x29);    // Display On
}

static void LCD_GPIO_Init(void) {
    // 初始化gpio
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask =  (1ULL<<MY_RES_PIN)| (1ULL<<MY_DC_PIN)| (1ULL<<MY_BLK_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

static void LCD_SPI_Init(void) {
    // 初始化sp
    spi_config_t spi_config;
    // Load default interface parameters
    /* CS_EN:1, MISO_EN:1, MOSI_EN:1, BYTE_TX_ORDER:0, BYTE_TX_ORDER:0, BIT_RX_ORDER:0, BIT_TX_ORDER:0, CPHA:0, CPOL:0 */
    spi_config.interface.val = SPI_DEFAULT_INTERFACE;
    // Load default interrupt enable
    // TRANS_DONE: true, WRITE_STATUS: false, READ_STATUS: false, WRITE_BUFFER: false, READ_BUFFER: false
    spi_config.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE;
    // Cancel hardware cs
    spi_config.interface.cs_en = 0;
    // MISO pin is used for D
    spi_config.interface.miso_en = 0;
    // CPOL: 1, CPHA: 1
    spi_config.interface.cpol = 1;
    spi_config.interface.cpha = 1; 
    // Set SPI to master mode
    // 8266 Only support half-duplex 
    spi_config.mode = SPI_MASTER_MODE; 
    // Set the SPI clock frequency division factor
    spi_config.clk_div = SPI_8MHz_DIV;
    spi_config.event_cb = spi_event_callback;
    spi_init(HSPI_HOST, &spi_config);
}

void LCD_Init(void)  {
    LCD_GPIO_Init();
    LCD_SPI_Init();
    tft_reset();
}

void LCD_WR_DATA8(u8 dat) {
    TFT_DC(1);
    spi_send(&dat, 1);
}
void LCD_WR_DATA(u16 dat) {
    TFT_DC(1); 
    spi_send((uint8_t[]){(uint8_t)(dat>>8), (uint8_t)dat, 0, 0}, 2);
}
void LCD_WR_REG(u8 dat) {
    // DC低电平
    TFT_DC(0);
    spi_send(&dat, 1); 
}

void LCD_WR_ARRAY(uint8_t* pData, uint32_t len) {
    TFT_DC(1); 
    spi_send(pData, len);
}

void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2) {
    LCD_WR_REG(0x2a);//列地址设置
    LCD_WR_DATA(x1);
    LCD_WR_DATA(x2);
    LCD_WR_REG(0x2b);//行地址设置
    LCD_WR_DATA(y1);
    LCD_WR_DATA(y2);
    LCD_WR_REG(0x2c);//储存器写 
}
