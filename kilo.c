/*** includes ***/

#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <stdio.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct editorConfig{
  int screenrows;
  int screencols;
  struct termios orig_termios;
};
struct editorConfig E;

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
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

//Changes terminal attributes to disable cooked mode.
void enableRawMode() {

  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");

  //reset attributes when exit is proccessed.
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;

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

int getCursorPosition(int *rows, int *cols){
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

  printf("\r\n");
  char c;

  while (i < sizeof(buf) -1 ) {
    if (iscntrl(c)) {
      if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
      if (buf[i] == 'R') break;
      i++;
    }
  }
  buf[i] = '\0';
  
  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
  
  return 0;
}

int getWindowsSize(int *rows, int *cols) {
  struct winsize ws;

  //TODO remove testing 1.
  if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
    //Complex scape sequences here, just move cursor 999 places right and down
    //C and B command should stop at the edge of screen.
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    editorReadKey();
    return getCursorPosition(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

/*** output ***/

//Draw rows of tilde for aesthetic purposes.
void editorDrawRows() {
  int y;
  for (y = 0; y < E.screenrows; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

// refreshes the screen and sets cursor to col 1 row 1
void editorRefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  editorDrawRows();
  
  //Resets cursor again after drawing tildes.
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

void initEditor() {
  
  //get the window size.
  if (getWindowsSize(&E.screenrows, &E.screencols) == -1) die ("getWindowSize");
}

int main() {

  enableRawMode();
  initEditor();

  while (1){
    editorRefreshScreen();
    editorProcessKeypress();
  } 

  return 0;
}
