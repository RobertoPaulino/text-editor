/*** includes ***/

#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

//Error handler.
void die(const char *s){
  //Clears screen and resets cursor on error exit.
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}

//Disables raw mode on exit to reset all flags modified
void disableRawMode() {

  //after the editor is done we reset the attributes.
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

//Changes terminal attributes to disable cooked mode.
void enableRawMode() {

  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");

  //reset attributes when exit is proccessed.
  atexit(disableRawMode);

  struct termios raw = orig_termios;

  //Take bitwise-NOT of ECHO and perform bitwise-AND on the local
  //flags to disable ECHO.
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  //set control character values.
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  
  //set the flags
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");

}

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die ("read");
  }
 
  return c;
}
/*** output ***/

// refreshes the screen and sets cursor to col 1 row 1
void editorRefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

void editorProcessKeypress() {
  char c = editorReadKey();

  switch (c) {
    case CTRL_KEY('q'):
      
      //Clears screen and reset cursor on exit sequence.
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);

      exit(0);
      break;
  }
}

/*** init ***/

int main() {
  enableRawMode();

  while (1){
    editorRefreshScreen();
    editorProcessKeypress();
  } 

  return 0;
}
