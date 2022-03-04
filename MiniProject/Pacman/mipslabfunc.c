/* mipslabfunc.c
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

/* Declare a helper function which is local to this file */
static void num32asc( char * s, int ); 

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)

/* quicksleep:
   A simple function to create a small delay.
   Very inefficient use of computing resources,
   but very handy in some special cases. */
void quicksleep(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
}

/* tick:
   Add 1 to time in memory, at location pointed to by parameter.
   Time is stored as 4 pairs of 2 NBCD-digits.
   1st pair (most significant byte) counts days.
   2nd pair counts hours.
   3rd pair counts minutes.
   4th pair (least significant byte) counts seconds.
   In most labs, only the 3rd and 4th pairs are used. */
void tick( unsigned int * timep )
{
  /* Get current value, store locally */
  register unsigned int t = * timep;
  t += 1; /* Increment local copy */
  
  /* If result was not a valid BCD-coded time, adjust now */

  if( (t & 0x0000000f) >= 0x0000000a ) t += 0x00000006;
  if( (t & 0x000000f0) >= 0x00000060 ) t += 0x000000a0;
  /* Seconds are now OK */

  if( (t & 0x00000f00) >= 0x00000a00 ) t += 0x00000600;
  if( (t & 0x0000f000) >= 0x00006000 ) t += 0x0000a000;
  /* Minutes are now OK */

  if( (t & 0x000f0000) >= 0x000a0000 ) t += 0x00060000;
  if( (t & 0x00ff0000) >= 0x00240000 ) t += 0x00dc0000;
  /* Hours are now OK */

  if( (t & 0x0f000000) >= 0x0a000000 ) t += 0x06000000;
  if( (t & 0xf0000000) >= 0xa0000000 ) t = 0;
  /* Days are now OK */

  * timep = t; /* Store new value */
}

/* display_debug
   A function to help debugging.

   After calling display_debug,
   the two middle lines of the display show
   an address and its current contents.

   There's one parameter: the address to read and display.

   Note: When you use this function, you should comment out any
   repeated calls to display_image; display_image overwrites
   about half of the digits shown by display_debug.
*/   
void display_debug( volatile int * const addr )
{
  display_string( 2, "Addr" );
  display_string( 3, "Data" );
  num32asc( &textbuffer[2][6], (int) addr );
  num32asc( &textbuffer[3][6], *addr );
  display_update();
}

uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 1));
	return SPI2BUF;
}

void display_init(void) {
        DISPLAY_CHANGE_TO_COMMAND_MODE;
	quicksleep(10);
	DISPLAY_ACTIVATE_VDD;
	quicksleep(1000000);
	
	spi_send_recv(0xAE);
	DISPLAY_ACTIVATE_RESET;
	quicksleep(10);
	DISPLAY_DO_NOT_RESET;
	quicksleep(10);
	
	spi_send_recv(0x8D);
	spi_send_recv(0x14);
	
	spi_send_recv(0xD9);
	spi_send_recv(0xF1);
	
	DISPLAY_ACTIVATE_VBAT;
	quicksleep(10000000);
	
	spi_send_recv(0xA1);
	spi_send_recv(0xC8);
	
	spi_send_recv(0xDA);
	spi_send_recv(0x20);
	
	spi_send_recv(0xAF);
}

void display_string(int line, char *s) {
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!s)
		return;
	
	for(i = 0; i < 16; i++)
		if(*s) {
			textbuffer[line][i] = *s;
			s++;
		} else
			textbuffer[line][i] = ' ';
}

void display_image(int x, const uint8_t *data) {
	int i, j;
	
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(x & 0xF);
		spi_send_recv(0x10 | ((x >> 4) & 0xF));
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		
		for(j = 0; j < 32*4; j++) //Change 32 to 32*4 so the whole screen updates
			spi_send_recv(~data[i*32*4 + j]); //Change 32 to 32*4 so the whole screen updates
	}
}

void display_update(void) {
	int i, j, k;
	int c;
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(0x0);
		spi_send_recv(0x10);
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		
		for(j = 0; j < 16; j++) {
			c = textbuffer[i][j];
			if(c & 0x80)
				continue;
			
			for(k = 0; k < 8; k++)
				spi_send_recv(font[c*8 + k]);
		}
	}
}

/* Helper function, local to this file.
   Converts a number to hexadecimal ASCII digits. */
static void num32asc( char * s, int n ) 
{
  int i;
  for( i = 28; i >= 0; i -= 4 )
    *s++ = "0123456789ABCDEF"[ (n >> i) & 15 ];
}

/*
 * nextprime
 * 
 * Return the first prime number larger than the integer
 * given as a parameter. The integer must be positive.
 */
#define PRIME_FALSE   0     /* Constant to help readability. */
#define PRIME_TRUE    1     /* Constant to help readability. */
int nextprime( int inval )
{
   register int perhapsprime = 0; /* Holds a tentative prime while we check it. */
   register int testfactor; /* Holds various factors for which we test perhapsprime. */
   register int found;      /* Flag, false until we find a prime. */

   if (inval < 3 )          /* Initial sanity check of parameter. */
   {
     if(inval <= 0) return(1);  /* Return 1 for zero or negative input. */
     if(inval == 1) return(2);  /* Easy special case. */
     if(inval == 2) return(3);  /* Easy special case. */
   }
   else
   {
     /* Testing an even number for primeness is pointless, since
      * all even numbers are divisible by 2. Therefore, we make sure
      * that perhapsprime is larger than the parameter, and odd. */
     perhapsprime = ( inval + 1 ) | 1 ;
   }
   /* While prime not found, loop. */
   for( found = PRIME_FALSE; found != PRIME_TRUE; perhapsprime += 2 )
   {
     /* Check factors from 3 up to perhapsprime/2. */
     for( testfactor = 3; testfactor <= (perhapsprime >> 1) + 1; testfactor += 1 )
     {
       found = PRIME_TRUE;      /* Assume we will find a prime. */
       if( (perhapsprime % testfactor) == 0 ) /* If testfactor divides perhapsprime... */
       {
         found = PRIME_FALSE;   /* ...then, perhapsprime was non-prime. */
         goto check_next_prime; /* Break the inner loop, go test a new perhapsprime. */
       }
     }
     check_next_prime:;         /* This label is used to break the inner loop. */
     if( found == PRIME_TRUE )  /* If the loop ended normally, we found a prime. */
     {
       return( perhapsprime );  /* Return the prime we found. */
     } 
   }
   return( perhapsprime );      /* When the loop ends, perhapsprime is a real prime. */
} 

/*
 * itoa
 * 
 * Simple conversion routine
 * Converts binary to decimal numbers
 * Returns pointer to (static) char array
 * 
 * The integer argument is converted to a string
 * of digits representing the integer in decimal format.
 * The integer is considered signed, and a minus-sign
 * precedes the string of digits if the number is
 * negative.
 * 
 * This routine will return a varying number of digits, from
 * one digit (for integers in the range 0 through 9) and up to
 * 10 digits and a leading minus-sign (for the largest negative
 * 32-bit integers).
 * 
 * If the integer has the special value
 * 100000...0 (that's 31 zeros), the number cannot be
 * negated. We check for this, and treat this as a special case.
 * If the integer has any other value, the sign is saved separately.
 * 
 * If the integer is negative, it is then converted to
 * its positive counterpart. We then use the positive
 * absolute value for conversion.
 * 
 * Conversion produces the least-significant digits first,
 * which is the reverse of the order in which we wish to
 * print the digits. We therefore store all digits in a buffer,
 * in ASCII form.
 * 
 * To avoid a separate step for reversing the contents of the buffer,
 * the buffer is initialized with an end-of-string marker at the
 * very end of the buffer. The digits produced by conversion are then
 * stored right-to-left in the buffer: starting with the position
 * immediately before the end-of-string marker and proceeding towards
 * the beginning of the buffer.
 * 
 * For this to work, the buffer size must of course be big enough
 * to hold the decimal representation of the largest possible integer,
 * and the minus sign, and the trailing end-of-string marker.
 * The value 24 for ITOA_BUFSIZ was selected to allow conversion of
 * 64-bit quantities; however, the size of an int on your current compiler
 * may not allow this straight away.
 */
#define ITOA_BUFSIZ ( 24 )
char * itoaconv( int num )
{
  register int i, sign;
  static char itoa_buffer[ ITOA_BUFSIZ ];
  static const char maxneg[] = "-2147483648";
  
  itoa_buffer[ ITOA_BUFSIZ - 1 ] = 0;   /* Insert the end-of-string marker. */
  sign = num;                           /* Save sign. */
  if( num < 0 && num - 1 > 0 )          /* Check for most negative integer */
  {
    for( i = 0; i < sizeof( maxneg ); i += 1 )
    itoa_buffer[ i + 1 ] = maxneg[ i ];
    i = 0;
  }
  else
  {
    if( num < 0 ) num = -num;           /* Make number positive. */
    i = ITOA_BUFSIZ - 2;                /* Location for first ASCII digit. */
    do {
      itoa_buffer[ i ] = num % 10 + '0';/* Insert next digit. */
      num = num / 10;                   /* Remove digit from number. */
      i -= 1;                           /* Move index to next empty position. */
    } while( num > 0 );
    if( sign < 0 )
    {
      itoa_buffer[ i ] = '-';
      i -= 1;
    }
  }
  /* Since the loop always sets the index i to the next empty position,
   * we must add 1 in order to return a pointer to the first occupied position. */
  return( &itoa_buffer[ i + 1 ] );
}


/*--------------------------------> Project data <-----------------------------*/

void draw_pixel(int x, int y, int value){
  int index = x + (y/8)*128;
  uint8_t byte = display[index];

  if(value == 0){
    switch(y%8){
    case 0:
      byte = byte&0xFE;
      break;
    case 1:
      byte = byte&0xFD;
      break;
    case 2:
      byte = byte&0xFB;
      break;
    case 3:
      byte = byte&0xF7;
      break;
    case 4:
      byte = byte&0xEF;
      break;
    case 5:
      byte = byte&0xDF;
      break;
    case 6:
      byte = byte&0xBF;
      break;
    case 7:
      byte = byte&0x7F;
      break;
    }
  }
  else{
    switch(y%8){
    case 0:
      byte = byte|0x1;
      break;
    case 1:
      byte = byte|0x2;
      break;
    case 2:
      byte = byte|0x4;
      break;
    case 3:
      byte = byte|0x8;
      break;
    case 4:
      byte = byte|0x10;
      break;
    case 5:
      byte = byte|0x20;
      break;
    case 6:
      byte = byte|0x40;
      break;
    case 7:
      byte = byte|0x80;
      break;
    }
  }

  display[index]=byte;
  return;
}

void reset_display(const uint8_t *picture){
  int i = 0;
  for(i = 0; i<512; i++){
    display[i]=0;
  }
  return;
}

void draw_picture(int x, int y, const uint8_t *picture, int length_array){
  int i;
  for(i = 0; i<length_array; i++){
    int byte = picture[i];
    int value = byte&0x01;
    draw_pixel(x+i,y,value);
    value = byte&0x02;
    draw_pixel(x+i,y+1,value);
    value = byte&0x04;
    draw_pixel(x+i,y+2,value);
    value = byte&0x08;
    draw_pixel(x+i,y+3,value);
    value = byte&0x10;
    draw_pixel(x+i,y+4,value);
    value = byte&0x20;
    draw_pixel(x+i,y+5,value);
    value = byte&0x40;
    draw_pixel(x+i,y+6,value);
    value = byte&0x80;
    draw_pixel(x+i,y+7,value);
  }

  return;
}

void set_map(const uint8_t *picture){
  int i;
  for(i=0;i<512;i++){
    display[i]=picture[i];
  }

  for(i=0;i<64;i++){
    occupied[i]=0;
  }

  int j = 0;
  for(i=0;i<512;i+=8){
    if(picture[i]==0xFF){
      occupied[j]=1;
    }
    j++;
  }
  return;
}

void create_food(void){
  static int condition = 1;
  int i;
  int j;
  if(condition){
    for(i=0;i<64;i++){
    if(occupied[i])
      continue;
    for(j=0;j<8;j++){
      display[i*8+j]=pacman_food_model[j];
    }
    pacman_food[i]=1;
    }
    condition = 0;
  }
  else{
    for(i=0;i<64;i++){
      if(!pacman_food[i])
        continue;
      for(j=0;j<8;j++){
      display[i*8+j]=pacman_food_model[j];
      }
    }
  }

  return;
}

void pacman_out_of_bounds(void){
  if(pacman.x_coord<0){
    pacman.x_coord=0;
  }
  if(pacman.x_coord>120){
    pacman.x_coord=120;
  }
  if(pacman.y_coord<0){
    pacman.y_coord=0;
  }
  if(pacman.y_coord>24){
    pacman.y_coord=24;
  }
}

//There are 4*16 blocks. 0-63 blocks
int block_detection(struct picture_object po, int block_number){
  if(occupied[block_number]==0)
    return 0;

  int xb = (block_number%16)*8;
  int yb = (block_number/16)*8;
  int xp = po.x_coord;
  int yp = po.y_coord;
  
  if(xp<=(xb+7)&&(xp+7)>=xb){
    if(yp<=(yb+7)&&(yp+7)>=yb){
      return 1;
    }
  }

  return 0;
}

//Detects if we have run into a wall on the map
int wall_detection(struct picture_object po){
  int i;
  for(i=0;i<64;i++){
    if(block_detection(po, i))
      return 1;
  }
  return 0;
}

//Used for detecting if the ghosts and pacman have collided
int collision_pictures_detection(struct picture_object p1, struct picture_object p2){
  if(p1.x_coord+7>=p2.x_coord&&p1.x_coord<=p2.x_coord+7){
    if(p1.y_coord+7>=p2.y_coord&&p1.y_coord<=p2.y_coord+7){
      return 1;
    }
  }
  return 0;
}

//Lets pacman eat food
void eat_food(void){
  int x = pacman.x_coord;
  int y = pacman.y_coord;
  int block = x/8 + (y/8)*16;
  int x2 = (block%16)*8;
  int y2 = (block/16)*8;

  if((x+7)>=(x2+4)&&x<=(x2+5)){
    if((y+7)>=(y2+4)&&y<=(y2+5)){
      if(pacman_food[block]){
        game_score+=1;
        pacman_food[block]=0;
      }
    }
  }
}

int check_if_out_of_food(){
  if(game_score<=10)
    return 0;

  int i;
  for(i=0;i<64;i++){
    if(pacman_food[i])
      return 0;
  }

  return 1;
}

//Makes pacman move around
void move(void){
  pacman.x_coord += pacman.x_velocity;
  pacman.y_coord += pacman.y_velocity;
  pacman_out_of_bounds();
  if(wall_detection(pacman)){
    pacman.x_coord -= pacman.x_velocity;
    pacman.y_coord -= pacman.y_velocity;
  }
}

//Sets the movement direction for pacman
void set_pacman_direction(void){
  int buttons = getbtns();

  //Change direction to down
  if(buttons == 1){
    pacman.x_velocity = 0;
    pacman.y_velocity = 1;
    current_direction = DOWN;
  }

  //Change direction to up
  if(buttons == 2){
    pacman.x_velocity = 0;
    pacman.y_velocity = -1;
    current_direction = UP;
  }

  //Change direction to left
  if(buttons == 4){
    pacman.x_velocity = -1;
    pacman.y_velocity = 0;
    current_direction = LEFT;
  }

  buttons = getbtn1();
  //Change direction to right
  if(buttons == 1){
    pacman.x_velocity = 1;
    pacman.y_velocity = 0;
    current_direction = RIGHT;
  }

  return;
}

void set_valid_direction_for_ghost(){
  static int count = 0;
  static int previous_direction = 0;
  if(count>20){
    count = 0;

    //Set to up
    ghost_1.x_velocity =0;
    ghost_1.y_velocity = (-1);

    ghost_1.x_coord += ghost_1.x_velocity;
    ghost_1.y_coord += ghost_1.y_velocity;
    if(wall_detection(ghost_1)||ghost_out_of_bounds()||previous_direction==DOWN){
        ghost_1.x_coord -= ghost_1.x_velocity;
        ghost_1.y_coord -= ghost_1.y_velocity;
        ghost_1.x_velocity = 0;
        ghost_1.y_velocity = 0;
    }
    else{
      previous_direction = UP;
      return;
    }

    //Set to left
    ghost_1.x_velocity = (-1);
    ghost_1.y_velocity = 0;

    ghost_1.x_coord += ghost_1.x_velocity;
    ghost_1.y_coord += ghost_1.y_velocity;
    if(wall_detection(ghost_1)||ghost_out_of_bounds()||previous_direction==RIGHT){
        ghost_1.x_coord -= ghost_1.x_velocity;
        ghost_1.y_coord -= ghost_1.y_velocity;
        ghost_1.x_velocity = 0;
        ghost_1.y_velocity = 0;
    }
    else{
      previous_direction = LEFT;
      return;
    }

    //Set to down
    ghost_1.x_velocity = 0;
    ghost_1.y_velocity = 1;

    ghost_1.x_coord += ghost_1.x_velocity;
    ghost_1.y_coord += ghost_1.y_velocity;
    if(wall_detection(ghost_1)||ghost_out_of_bounds()||previous_direction==UP){
        ghost_1.x_coord -= ghost_1.x_velocity;
        ghost_1.y_coord -= ghost_1.y_velocity;
        ghost_1.x_velocity = 0;
        ghost_1.y_velocity = 0;

    }
    else{
      previous_direction = DOWN;
      return;
    }

    //Set to right
    ghost_1.x_velocity = 1;
    ghost_1.y_velocity = 0;

    ghost_1.x_coord += ghost_1.x_velocity;
    ghost_1.y_coord += ghost_1.y_velocity;
    if(wall_detection(ghost_1)||ghost_out_of_bounds()||previous_direction==LEFT){
        ghost_1.x_coord -= ghost_1.x_velocity;
        ghost_1.y_coord -= ghost_1.y_velocity;
        ghost_1.x_velocity = 0;
        ghost_1.y_velocity = 0;

    }
    else{
      previous_direction = RIGHT;
      return;
    }
  }
  else{
    count++;
    ghost_1.x_coord += ghost_1.x_velocity;
    ghost_1.y_coord += ghost_1.y_velocity;
    if(wall_detection(ghost_1)||ghost_out_of_bounds()){
        ghost_1.x_coord -= ghost_1.x_velocity;
        ghost_1.y_coord -= ghost_1.y_velocity;
        ghost_1.x_velocity = 0;
        ghost_1.y_velocity = 0;
    }
  }
}

int ghost_out_of_bounds(){
  if(ghost_1.x_coord<0){
    return 1;
  }
  if(ghost_1.x_coord>120){
    return 1;
  }
  if(ghost_1.y_coord<0){
    return 1;
  }
  if(ghost_1.y_coord>24){
    return 1;
  }

  return 0;
}

void reset_food(){
  int i;
  for(i=0;i<64;i++){
    pacman_food[i]=0;
  }
  return;
}

void create_new_food(){
  int i;
  int j;
  for(i=0;i<64;i++){
    if(occupied[i])
      continue;
    for(j=0;j<8;j++){
      display[i*8+j]=pacman_food_model[j];
    }
    pacman_food[i]=1;
  }
}