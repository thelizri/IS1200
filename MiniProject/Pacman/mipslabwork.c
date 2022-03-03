/* mipslabwork.c

   This file written 2015 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   This file should be changed by YOU! So you must
   add comment(s) here with your name(s) and date(s):

   This file modified 2017-04-31 by Ture Teknolog 

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

void paint(int map);
void move(void);
void game_over(void);
void display_menu();
void clear_menu();
void paint_pacman();
void select_a_map();
void display_score();
void work_delay(int x);
void enter_highscore();
void enter_name(int xxx);

int timeoutcount = 0;
int select_restart = 0;
int game_running = 0;
int pause = 0;
int game_score;
int pacman_power_up;
int selected_map;
int current_direction;

char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZZZZZZZZZZZZZZ";

/* Interrupt Service Routine */
void user_isr( void )
{
  if(game_running){

    //Timer2 interrupt
    int x = IFS(0); //Interrupt register. Flag is stored in bit 8.
    x = x>>8;
    x = x&0x01;
    if(x){
      timeoutcount++;
      if(timeoutcount>=4){
        timeoutcount = 0;
        if(!pause){
          set_valid_direction_for_ghost();
          move();
          eat_food();
          PORTE = game_score;
          if(check_if_out_of_food()||collision_pictures_detection(pacman, ghost_1)){
            game_over();
          }
          else{
            paint(selected_map);
          }
        }
      }
      IFSCLR(0) = 0x100;
    }

    //Switch 1 interrupt
    x = IFS(0);
    x = x>>7;
    x = x&0x01;
    if(x){
      if(pause)
        pause = 0;
      else
        pause = 1;
      IFSCLR(0) = 0x80;
    }
  }
  else{
    //Must still clear all the flags
    IFSCLR(0) = 0x180;
  }
  

  return;
}

/* Lab-specific initialization goes here */
void labinit( void )
{
  //Leds
  TRISECLR =0xFF; //Setting to output
  PORTE = 0; //Setting starting value to 0

  //Buttons and switches
  TRISDSET = 0x0FE0;
  TRISFSET = 0x02;

  game_score = 1000;
  enter_highscore();

  display_menu();
  select_a_map();

  clear_menu();
  game_running = 1;

  //Timers
  T2CON = 0x0; //Control register. Configures the prescaling.
  TMR2 = 0x0; //TMR2 holds the current 16-bit timer value
  PR2 = 3125; //Period register. Timer counts upwards until it reaches this value.
  T2CONSET = 0x8070; //Tells timer to start with prescaling 1:256
  IECSET(0) = 0x0180; //Enables timer 2 interrupt and switch 1 interrupt
  IPCSET(2) = 0x01F; ///Priority for timer 2
  IPCSET(1) = 0x1F000000; //Priority control for switch 1
  enable_interrupt();
  

  //Pacman position. Block 0 at row 3
  pacman.x_coord = 0;
  pacman.y_coord = 16;
  current_direction = 0;

  //Ghost position. Block 7 at row 2.
  ghost_1.x_coord=56;
  ghost_1.y_coord=8;

  //Game mechanics
  game_score = 0;
  pacman_power_up = 0;

  return;
}

/* This function is called repetitively from the main program */
void labwork( void )
{
  if(game_running){
    set_pacman_direction();

    while(pause){
      //Do nothing
    }
  }
}

void paint(int map){
  static int toggle = 0;
  if(toggle>=10)
    toggle = 0;

  switch(map){
    case 1:
      set_map(maps_1);
      break;
    case 2:
      set_map(maps_2);
      break;
    case 3:
      set_map(maps_3);
      break;
  }
  create_food();

  paint_pacman();

  draw_picture(ghost_1.x_coord, ghost_1.y_coord, ghost, 8);

  display_update();
  display_image(0, display);
}

void game_over(void){
  game_running = 0;

  display_update();
  display_image(0, display_off);

  //Game over screen
  display_string(0, "Game over");
  display_string(1, "Game score:");
  display_string(2, itoaconv(game_score));
  display_update();

  //Enter highscore
  work_delay(5000);
  enter_highscore();
  work_delay(5000);
  display_score();

  while(1){
    //Do nothing. It is game over
  }
}

void display_menu(){
  display_string(0, "Choose");
  display_string(1, "Map 1");
  display_string(2, "Map 2");
  display_string(3, "Map 3");
  display_update();
}

void clear_menu(){
  display_string(0, "");
  display_string(1, "");
  display_string(2, "");
  display_string(3, "");
}

void paint_pacman(){
  static int toggle = 0;
  if(toggle>=20)
    toggle = 0;

  if(toggle>=10){
    switch(current_direction){
      case UP:
        draw_picture(pacman.x_coord, pacman.y_coord, pacman_open_mouth_up, 8);
        break;
      case DOWN:
        draw_picture(pacman.x_coord, pacman.y_coord, pacman_open_mouth_down, 8);
        break;
      case LEFT:
        draw_picture(pacman.x_coord, pacman.y_coord, pacman_open_mouth_left, 8);
        break;
      case RIGHT:
        draw_picture(pacman.x_coord, pacman.y_coord, pacman_open_mouth_right, 8);
        break;
      default:
        draw_picture(pacman.x_coord, pacman.y_coord, pacman_open_mouth_right, 8);
        break;
    }
    toggle++;
  }
  else{
    switch(current_direction){
      case UP:
        draw_picture(pacman.x_coord, pacman.y_coord, pacman_closed_mouth_up, 8);
        break;
      case DOWN:
        draw_picture(pacman.x_coord, pacman.y_coord, pacman_closed_mouth_down, 8);
        break;
      case LEFT:
        draw_picture(pacman.x_coord, pacman.y_coord, pacman_closed_mouth_left, 8);
        break;
      case RIGHT:
        draw_picture(pacman.x_coord, pacman.y_coord, pacman_closed_mouth_right, 8);
        break;
      default:
        draw_picture(pacman.x_coord, pacman.y_coord, pacman_closed_mouth_right, 8);
        break;
    }
    toggle++;
  }
}

void select_a_map(){
  while(1){
    int buttons = getbtns();

    //Pick map 2
    if(buttons == 1){
      selected_map = 2;
      break;
    }

    //Pick map 3
    if(buttons == 2){
      selected_map = 3;
      break;
    }

    //Pick map 3
    if(buttons == 4){
      display_score();
      display_menu();
      work_delay(1000);
    }

    buttons = getbtn1();
    //Pick map 1
    if(buttons == 1){
      selected_map = 1;
      break;
    }
  }
}

void display_score(){
  display_string(0, list_of_highscores.name1);
  display_string(1, itoaconv(list_of_highscores.first_place));
  display_string(2, list_of_highscores.name2);
  display_string(3, itoaconv(list_of_highscores.second_place));
  display_update();

  work_delay(1000);

  while(1){
    int buttons = getbtns();
    if(buttons == 4)
      break;
  }
}

void enter_highscore(){
  if(game_score>list_of_highscores.first_place){
    list_of_highscores.first_place = game_score;
    enter_name(1);
    return;
  }
  if(game_score>list_of_highscores.second_place){
    list_of_highscores.second_place = game_score;
    enter_name(2);
  }
  return;
}

void enter_name(int xxx){

  clear_menu();

  char name[4] = "???";
  char to_display[2];
  to_display[1] = 0;
  volatile int i=0;
  volatile int jjj=0;
  
  work_delay(1000);


  while(1){
    volatile int buttons = getbtns();

    PORTE = i+1;
    to_display[0] = alphabet[i];
    display_string(0, to_display);
    display_string(1, name);
    display_update();

    if(jjj>2){
      break;
    }

    //Down
    if(buttons == 1){
      i--;
      if(i<0)
        i=0;
      work_delay(300);
    }

    //Up
    if(buttons == 2){
      i++;
      i = i%26;
      work_delay(300);
    }

    //Pick 
    if(buttons == 4){
      name[jjj]=alphabet[i];
      jjj+=1;
      i=0;
      work_delay(300);
    }
  }

  if(xxx==1){
    for(i=0;i<4;i++){
      list_of_highscores.name1[i] =(char) name[i];
    }
  }
  else{
    for(i=0;i<4;i++){
      list_of_highscores.name2[i] =(char) name[i];
    }
  }
  return;
}

void work_delay(int x){
  int i;
  int j;
  for(i=0;i<x;i++){
    for(j=0;j<4500;j++){
      //Do nothing
    }
  }
  return;
}