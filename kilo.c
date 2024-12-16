#include <unistd.h>
#include <termios.h>

void enableRawMode() {
  struct termios raw;
  
  //get terminal attributes
  tcgetattr(STDIN_FILENO, &raw);
  
  //Take bitwise-NOT of ECHO and perform bitwise-AND on the local
  //flags to disable ECHO.
  raw.c_lflag &= ~(ECHO);
  
  //set the flags
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

}
int main() {
  enableRawMode();

  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
  return 0;
}
