  //Jake Watkins
// The purpose of this program
// is to help reinforce use
// of the correct SHIFT key on the keyboard
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>
#include <curses.h>
#include <locale.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
int score = 0;
int lives = 3;
char random_letter(){
  return 'A' + (rand() % 26);
}
void getuserletter(char *user_letter){
  // system ("/bin/stty raw");
  *user_letter = getchar();
  // system ("/bin/stty cooked");
}
void simple_version(){
  char rand_letter;
  char user_letter;
  bool playing = true;
  while (playing) {
    rand_letter = random_letter();
    fprintf(stdout, "Score: %4d Lives %d Letter: %c ..:",score,lives , rand_letter);
    getuserletter(&user_letter);
    fprintf(stdout,"\n");
if ((int)user_letter==(int)rand_letter){
      score++;
    }
    else{
      lives--;
      if(lives==0){
        playing = false;
      }
    }
  }
}
void update_win(int i,  WINDOW *w[5], WINDOW *sw[5]){
  touchwin(w[i]);
  wrefresh(sw[i]);
}
void createwindows(int WPOS[5][4],  WINDOW *w[5], WINDOW *sw[5]){
  int i;
  setlocale(LC_ALL,"");
  initscr();
  cbreak();
  noecho();
  nonl();
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);
  clear();
  for (i=0;i<5;i++){
      w[i]=newwin(WPOS[i][0],WPOS[i][1],WPOS[i][2],WPOS[i][3]);
      sw[i]=subwin(w[i],WPOS[i][0]-2,WPOS[i][1]-2,WPOS[i][2]+1,WPOS[i][3]+1);
      //scrollok(sw[i],TRUE);
      wborder(w[i],0,0,0,0,0,0,0,0);
      touchwin(w[i]);
      wrefresh(w[i]);
      wrefresh(sw[i]);
  }
  waddstr(sw[0],"score ");
  waddstr(sw[1],"upcoming ");
  waddstr(sw[2],"lives ");
  waddstr(sw[3],"letters ");
  waddstr(sw[4],"user entry ");
  for (i=0;i<5;i++){update_win(i, w, sw);}
}
void shift_left(char **field, int lines, int columns){
  int x,y;
  for(y = 0;y<lines;y++){
    for(x = 0;x<columns-1;x++){
      field[y][x] = field[y][x+1];
    }
  }
  for(y = 0;y<lines;y++){
    field[y][columns-1] = ' ';
  }
}
void add_rand_chars(char **field, int lines, int columns){
  int y;
  for(y = 0;y<lines;y++){
    if(rand()%(lines) > (lines-5)){// adjust how many letters are added each column
      field[y][columns-1] = random_letter();
    }
    else{
      field[y][columns-1] = ' ';
    }
  }
}
void fill_field(char **field,int lines, int columns){
  int x,y;
  for(y = 0;y<lines;y++){
    for(x = 0;x<columns;x++){
      field[y][x] = ' ';
    }
  }
}
void update_field(WINDOW *w[5], WINDOW *sw[5],char **playfield, int lines){
  int k;
  for(k = 0;k<lines;k++){
      mvwprintw(sw[3],k, 0, &playfield[k][0]);
      update_win(3, w, sw);
  }
}
int findletter(char **playfield, int lines, int columns, char user_letter){
  int i,j;
  for (j = 0;j<columns;j++){
    for (i = 0;i<lines;i++){
      if(user_letter==playfield[i][j]){
        playfield[i][j] = ' ';
        return 1;
      }
    }
  }
  return 0;
}
void update_score(int score, int lives,WINDOW *w[5], WINDOW *sw[5]){
  char buf[10];
  sprintf(buf,"score %3d", score);
  mvwprintw(sw[0],0,0,buf);
  sprintf(buf,"lives %3d", lives);
  mvwprintw(sw[2],0,0,buf);
}
void interactive_version(){
  int lines, columns, i;
  struct winsize win;
  ioctl(0, TIOCGWINSZ, &win);
  lines =  win.ws_row;
  columns = win.ws_col;
  columns -= (columns % 3);
  WINDOW *w[5];
  WINDOW *sw[5];
  int WPos[5][4] = {{3,columns/3,0,0},{3,columns/3,0,columns/3},{3,columns/3,0,(columns/3) * 2}
                    ,{lines - 6,columns,3,0}
                    ,{3,columns,lines-3,0}};
  lines-=8;
  columns-=2;
  createwindows(WPos, w, sw);

  char **playfield = (char **) calloc(lines,sizeof(char*));
  for(i = 0;i<lines;i++){
    playfield[i] = (char*)calloc(columns, sizeof(char));
  }

  fill_field(playfield, lines, columns);
  time_t seconds;
  char c;
  fd_set readfds;
  int    num_readable;
  struct timeval tv;
  struct timeval time_of_day;
  int    fd_stdin;
  fd_stdin = fileno(stdin);
  seconds = time(NULL);
  while(lives > 0){//main game loop
    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);
    gettimeofday(&time_of_day,NULL);
    tv.tv_sec = 0;
    tv.tv_usec = 1000000-time_of_day.tv_usec;//adjust how fast lines move
    update_field(w,sw,playfield, lines);
    for (i=0;i<5;i++){update_win(i, w, sw);}
    num_readable = select(fd_stdin + 1, &readfds, NULL, NULL, &tv);
    if (num_readable == 0) {
      shift_left(playfield, lines, columns);
      add_rand_chars(playfield, lines, columns);
    }
    else{
      getuserletter(&c);
      if(findletter(playfield, lines, columns, c)){
        score++;
      }
      else{
        lives--;
      }
    }
    update_score(score, lives, w, sw);
    update_field(w,sw,playfield, lines);
    // if no match -1 life

  }
  endwin();
  setlocale(LC_ALL,"C");
  exit(0);
}

int main(int argc, char const *argv[]) {
  srand( (unsigned)time( NULL ) );
  int mode = 1;
  switch (mode){
    case 0: simple_version();
            break;
    case 1: interactive_version();
            break;
    default:break;
  }

  return 0;
}
