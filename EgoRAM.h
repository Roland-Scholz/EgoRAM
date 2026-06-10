#define EGO_MAX_SPRITES 64
#define EGO_MAX_SHAPES 64

#define EGO_MODE_ATARI 0
#define EGO_MODE_USB 1

#define EGO_UART_ID uart0
#define EGO_BAUD_RATE 115200
#define EGO_UART_TX_PIN 28
#define EGO_UART_RX_PIN 29

#define EGO_REG_CMD 0
#define EGO_REG_STATUS 1
#define EGO_REG_DATA 2

#define EGO_CMD_RENDER_SPRITES 0
#define EGO_CMD_SET_WRITEABLE 1
#define EGO_CMD_SET_READONLY 2
#define EGO_CMD_ABORT 3
#define EGO_CMD_SHAPE_DATA 4
#define EGO_CMD_DEL_SHAPE 5
#define EGO_CMD_SPRITE_DATA 6
#define EGO_CMD_DEL_SPRITE 7
#define EGO_CMD_ENA_SPRITE 8
#define EGO_CMD_DIS_SPRITE 9
#define EGO_CMD_SET_SPRITE_X 10
#define EGO_CMD_SET_SPRITE_Y 11
#define EGO_CMD_SET_SPRITE_XY 12
#define EGO_CMD_SET_BLIT_WIDTH 13
#define EGO_CMD_SET_BLIT_HEIGHT 14
#define EGO_CMD_MOVEMENT 15

#define EGO_ST_IDLE 0
#define EGO_ST_SHAPE_NO 1
#define EGO_ST_SHAPE_WIDTH 2
#define EGO_ST_SHAPE_HEIGHT 3
#define EGO_ST_SHAPE_DATA 4
#define EGO_ST_SPRITE_NO 5
#define EGO_ST_SPRITE_SHAPE_NO 6
#define EGO_ST_SPRITE_X_LO 7
#define EGO_ST_SPRITE_X_HI 8
#define EGO_ST_SPRITE_Y_LO 9
#define EGO_ST_SPRITE_Y_HI 10
#define EGO_ST_SPRITE_ENA 11
#define EGO_ST_SPRITE_DIS 12
#define EGO_ST_BLITWITH 13
#define EGO_ST_BLITHEIGHT 14

/*
Sprite data:
b00		logical sprite number
b01		width in bytes
b02		height in lines
bxx.yy	width*heigt data
*/

typedef struct
{
	uint8_t count;
	uint8_t width;
	uint8_t height;
	uint8_t *data;
} shape_t;

typedef struct
{
	uint8_t enabled;
	uint16_t xpos;
	uint16_t ypos;
	uint16_t xold;
	uint16_t yold;
	uint8_t shape_no;
} sprite_t;

void set_blitwidth(uint16_t width);
void set_blitheight(uint16_t heigth);

uint16_t get_blitwidth();
uint16_t get_blitheight();

void sprite_draw_all();
void sprite_draw(sprite_t *sp);
void write_d5xx(uint8_t addr, uint8_t data);
uint8_t read_d5xx(uint8_t addr);
void ego_log(const char *format, ...);
