/* mipslab.h
   Header file for all labs.
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

/* Declare display-related functions from mipslabfunc.c */
void display_image(int x, const uint8_t *data);
void display_init(void);
void display_string(int line, char *s);
void display_update(void);
uint8_t spi_send_recv(uint8_t data);

/* Declare lab-related functions from mipslabfunc.c */
char * itoaconv( int num );
void labwork(void);
int nextprime( int inval );
void quicksleep(int cyc);
void tick( unsigned int * timep );

/* Declare display_debug - a function to help debugging.

   After calling display_debug,
   the two middle lines of the display show
   an address and its current contents.

   There's one parameter: the address to read and display.

   Note: When you use this function, you should comment out any
   repeated calls to display_image; display_image overwrites
   about half of the digits shown by display_debug.
*/
void display_debug( volatile int * const addr );

/* Declare bitmap array containing font */
extern const uint8_t const font[128*8];
/* Declare text buffer for display output */
extern char textbuffer[4][16];


//////////////////////////////////////////////////////////////////////////////////////

/* Declare functions written by students.
   Note: Since we declare these functions here,
   students must define their functions with the exact types
   specified in the laboratory instructions. */
/* Written as part of asm lab: delay, time2string */
void delay(int);
void time2string( char *, int );
/* Written as part of i/o lab: getbtns, getsw, enable_interrupt */
int getbtns(void);
int getsw(void);
void enable_interrupt(void);


/*--------------------------------> Project functions <-----------------------------*/
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4


struct picture_object{
   int x_coord;
   int y_coord;
   int x_velocity;
   int y_velocity;
};

struct highscore_list{
   int first_place;
   char name1[20];
   int second_place;
   char name2[20];
   int third_place;
   char name3[20];
};

extern int game_score;
extern int pacman_power_up;
extern int selected_map;
extern int current_direction;

extern uint8_t display[];
extern uint8_t occupied[];
extern uint8_t pacman_food[];

extern const uint8_t display_on[];
extern const uint8_t display_off[];
extern const uint8_t maps_1[];
extern const uint8_t maps_2[];
extern const uint8_t maps_3[];
extern const uint8_t pacman_open_mouth_up[];
extern const uint8_t pacman_open_mouth_down[];
extern const uint8_t pacman_open_mouth_right[];
extern const uint8_t pacman_open_mouth_left[];
extern const uint8_t pacman_closed_mouth_up[];
extern const uint8_t pacman_closed_mouth_down[];
extern const uint8_t pacman_closed_mouth_right[];
extern const uint8_t pacman_closed_mouth_left[];
extern const uint8_t pacman_food_model[];
extern const uint8_t ghost[];

int getbtn1();
int block_detection(struct picture_object po, int block_number);
int wall_detection(struct picture_object po);
int collision_pictures_detection(struct picture_object p1, struct picture_object p2);
int check_if_out_of_food(void);
int ghost_out_of_bounds(void);

void draw_pixel(int x, int y, int value);
void draw_picture(int x, int y, const uint8_t *picture, int length_array);
void set_map(const uint8_t *picture);
void create_food(void);
void reset_display();
void eat_food(void);
void pacman_out_of_bounds(void);
void move(void);
void set_pacman_direction(void);
void set_valid_direction_for_ghost(void);
void reset_food();
void create_new_food();

extern struct picture_object pacman;
extern struct picture_object ghost_1;
extern struct highscore_list list_of_highscores;