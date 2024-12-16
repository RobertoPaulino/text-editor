/*** includes ***/

#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

//error handler
void die(const char *s){
  perror(s);
  exit(1);
}

void disableRawMode() {

  //after the editor is done we reset the attributes.
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcgetattr");


  //get terminal attributes.
  tcgetattr(STDIN_FILENO, &orig_termios);
  
  //reset attributes when exit is proccessed.
  atexit(disableRawMode);

  struct termios raw = orig_termios;

  //Take bitwise-NOT of ECHO and perform bitwise-AND on the local
  //flags to disable ECHO.
  raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag &= ~(CS8);
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

  //set control character values.
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  
  //set the flags
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");

}

/*** init ***/

int main() {
  enableRawMode();

  while (1) {

    char c = '\0';

    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");

    //if the character is a control character we just print the ASCII code
    //else we print the ASCII code and the character.
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q') break;
  }

  return 0;
}
