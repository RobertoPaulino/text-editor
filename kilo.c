#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

struct termios orig_termios;

void disableRawMode() {
  //after the editor is done we reset the attributes.
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {

  //get terminal attributes.
  tcgetattr(STDIN_FILENO, &orig_termios);
  
  //reset attributes when exit is proccessed.
  atexit(disableRawMode);

  struct termios raw = orig_termios;

  //Take bitwise-NOT of ECHO and perform bitwise-AND on the local
  //flags to disable ECHO.
  raw.c_lflag &= ~(ECHO | ICANON);
  
  //set the flags
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

}

int main() {
  enableRawMode();

  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {

    //if the character is a control character we just print the ASCII code
    //else we print the ASCII code and the character.
    if (iscntrl(c)) {
      printf("%d\n", c);
    } else {
      printf("%d ('%c')\n", c, c);
    }
  }

  return 0;
}
