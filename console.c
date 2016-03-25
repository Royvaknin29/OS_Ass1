// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

#define MAX_HISTORY 16
#define MAX_BUFFER 128
 char historyArray[MAX_HISTORY][MAX_BUFFER * sizeof(char)];
static int historyPos = 0;
static int historyReadPos = 0;

static void replaceCurrentLine(char*);
static void printBufferToLine();

static void initHistoryArray(){
	int i;
	for(i = 0; i < MAX_HISTORY; i++){
		//TODO: change to normal initialization.
		 historyArray[0][i] = 0;
		}
	}

static void strcpy(char* dest, const char* source){
	int i;
	for(i = 0; i < MAX_BUFFER; i++){
		dest[i] = source[i];
	}
}

// static void strcpyn(char* dest, const char* source, int n){
// 	int i;
// 	for(i = 0; i < n; i++){
// 		dest[i] = source[i];
// 	}
// }

int sys_history(void)
{
	char *buffer;
	int item;
  if(argstr(0, &buffer) < 0 || argint(1, &item) < 0){
  	cprintf("%s", "Error in buffer:");
    return -1;
  }
	if(item < 0 || item > 15){ //Illegal History ID
		return -2;
	}
	strcpy(buffer, historyArray[item]);
  return 0;
}

static void consputc(int);

static int panicked = 0;
static int inputCaretPos = 0;
static struct {
  struct spinlock lock;
  int locking;
} cons;

static void


printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}
//PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&cons.lock);
}

void print(char* string){
      cli();
      cons.locking = 0;
      cprintf(string);
      cprintf("\n");
    }

void
panic(char *s)
{
  int i;
  uint pcs[10];
  
  cli();
  cons.locking = 0;
  cprintf("cpu%d: panic: ", cpu->id);
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for(i=0; i<10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;)
    ;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

static void
cgaputc(int c)
{
  int pos;
  
  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
    pos += 80 - pos%80;
  else if(c == BACKSPACE){
    if(pos > 0) --pos;
  } else
    crt[pos++] = (c&0xff) | 0x0700;  // black on white

  if(pos < 0 || pos > 25*80)
    panic("pos under/overflow");
  
  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
  }
  
  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  crt[pos] = ' ' | 0x0700;
}

static void moveCursorLeft()
{
  int pos;
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);
  pos--;
  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
}

static void moveCursorRight()
{
  int pos;
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);
  pos++;
  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
}

void
consputc(int c)
{
  if(panicked){
    cli();
    for(;;)
      ;
  }

  if(c == BACKSPACE){
    uartputc('\b'); uartputc(' '); uartputc('\b');
  } else
    uartputc(c);
  cgaputc(c);
}

#define INPUT_BUF 128
struct {
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} input;

#define C(x)  ((x)-'@')  // Control-x

void shiftBufferRight(){
    uint charToMove = input.e - 1;
    while(charToMove != inputCaretPos - 1){
      input.buf[(charToMove + 1) % INPUT_BUF] = input.buf[charToMove % INPUT_BUF];
      charToMove--;
    }
}

void shiftBufferLeft(){
    uint charToMove = inputCaretPos ;
    while(charToMove < input.e + 1){
      input.buf[charToMove % INPUT_BUF] = input.buf[charToMove + 1 % INPUT_BUF];
      charToMove++;
    }
}

static void clearCurrentLine(){
	int pos;
outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);
  pos = pos - pos%80+2;
    int i;
    for(i = pos; i < pos - pos%80 + 78; i++){
  	crt[i] = ' ' | 0x0700; 
  }
}


//***************************************************************
//	Replace the current line:
//***************************************************************
static void replaceCurrentLine(char* buff)
{
  int pos, prevPos;

	clearCurrentLine();
    //Move cursor to beginning of the line:
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);
  prevPos = pos;
  pos = pos - pos%80+2;

  //Write the buffer to the line:
  int index = 0;
  char c = buff[index];
  while (c!=0){
  	crt[pos++] = c | 0x0700;
  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  index++;
  c = buff[index];
  }

  //Put cursor back in previous location:
  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, prevPos);
}

static void getBuffer(char* buffer){
	int length = input.e - input.w;
	int i;
	for(i = 0; i<length; i++){
		buffer[i] = input.buf[(input.w + i)%INPUT_BUF];
	}
	buffer[length] = 0;
}

static void printBufferToLine(){
	char line[MAX_BUFFER];
	getBuffer(line);
	replaceCurrentLine(line);
}

void putInBuffer(char* string){
	input.e = input.w;
	int index = 0;
	char c = string[index];
	while(c != 0){
		input.buf[input.e % INPUT_BUF] = c;
		input.e++;
		index++;
		c = string[index];
	}
	input.buf[input.e] = 0;
}

void processLine(){
	int bufferLength  = input.e - input.w;
	char line[MAX_BUFFER];
	getBuffer(line);
	if(!(line[0] == 'h' && 
		line[1] == 'i' &&
		line[2] == 's' &&
		line[3] == 't' &&
		line[4] == 'o' &&
		line[5] == 'r' &&
		line[6] == 'y' &&
		line[7] == '\n')){
		if(bufferLength > 1){// Save to history:
			getBuffer(historyArray[historyPos]);
			historyArray[historyPos][bufferLength - 1] = 0;
			if(historyPos == MAX_HISTORY -1){//Need to push back buffer:
				int i;
				for(i = 0; i < MAX_HISTORY; i++){
					int j;
					for(j = 0; j < MAX_BUFFER; j++){
						historyArray[i][j] = historyArray[i + 1][j];
					}
				}
			}
			historyArray[historyPos][bufferLength - 1] = 0;
			
			if(historyPos < MAX_HISTORY - 1)
				historyPos++;
		}
		historyReadPos = historyPos;
		}
		input.w = input.e;

	wakeup(&input.r);
}

void
consoleintr(int (*getc)(void))
{
  
  int c, doprocdump = 0;
  acquire(&cons.lock);
  while((c = getc()) >= 0){
    switch(c){
    case C('P'):  // Process listing.
      doprocdump = 1;   // procdump() locks cons.lock indirectly; invoke later
      break;
    case C('U'):  // Kill line.
      while(input.e != input.w &&
            input.buf[(input.e-1) % INPUT_BUF] != '\n'){
        input.e--;
        consputc(BACKSPACE);
      inputCaretPos = input.e;
      }
      break;
    case C('H'): case '\x7f':  // Backspace
      if(input.e != input.w){
      	      input.e--;
      	      inputCaretPos--;

      	 if(inputCaretPos < input.e){
	        shiftBufferLeft();
	        moveCursorLeft();
	        printBufferToLine();
    	}
    	else{
        consputc(BACKSPACE);
    	}
      }
      break;
    case 228: //Left Arrow
      	if (inputCaretPos > input.w){
      	inputCaretPos--;
      	moveCursorLeft();
      	}
      break;
    case 229: //Right Arrow
    	 if(inputCaretPos < input.e){
    	 	inputCaretPos++;
    	 	moveCursorRight();
    	 }
    	 break;
    case 226: //up:
      if(historyReadPos > 0)

      	replaceCurrentLine(historyArray[historyReadPos - 1]);
      	putInBuffer(historyArray[historyReadPos - 1]);
		historyReadPos--;


      break;
    case 227: //down:
    if(historyReadPos < historyPos)
    	historyReadPos++;
      if(historyReadPos == historyPos){
      	replaceCurrentLine("");
        putInBuffer("");
      }
      else{
		replaceCurrentLine(historyArray[historyReadPos]);
		putInBuffer(historyArray[historyReadPos]);

      }
      break;
    default:
      //print("Defalut");
      if (input.e - inputCaretPos > 0 && c != 0 && c!= '\n'){ //Marker is not at the end of the line:
      	        c = (c == '\r') ? '\n' : c;

      	   if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
      	   	  input.buf[input.e++ % INPUT_BUF] = c;
      	   	  inputCaretPos = input.e;
	          processLine();
	          break;
        	}
        	//not enter:
        	input.e++;
          shiftBufferRight();
          input.buf[inputCaretPos++ % INPUT_BUF] = c;
          printBufferToLine();
          moveCursorRight();
      }

      else if(c != 0 && input.e-input.r < INPUT_BUF){
        c = (c == '\r') ? '\n' : c;
        inputCaretPos++;
        input.buf[input.e++ % INPUT_BUF] = c;
        consputc(c);
        if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
          inputCaretPos = input.e;
          processLine();
        }
      }
      break;
    }
  }
  release(&cons.lock);
  if(doprocdump) {
    procdump();  // now call procdump() wo. cons.lock held
  }
}

int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&cons.lock);
  while(n > 0){
    while(input.r == input.w){
      if(proc->killed){
        release(&cons.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &cons.lock);
    }
    c = input.buf[input.r++ % INPUT_BUF];
    if(c == C('D')){  // EOF
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n')
      break;
  }
  release(&cons.lock);
  ilock(ip);

  return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void
consoleinit(void)
{
  initlock(&cons.lock, "console");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;

  picenable(IRQ_KBD);
  ioapicenable(IRQ_KBD, 0);
  //cprintf("%s", "WELCOME... Caret Navitation\n");
  initHistoryArray();
}

