#include <Arduino.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

//Includes for joystick
#include <Wire.h>
#include "SparkFun_Qwiic_Joystick_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_joystick

//Init joystick
uint8_t Address = 0x20; //Start address (Default 0x20)
JOYSTICK joystick; //Create instance of this object

// stuff for running a delay
const long DELAY = 1000.; // time step for falling pieces
unsigned long previousMillis = 0.;
unsigned long currentMillis;


// Display/matrix size
const int XSIZE = 16;
const int YSIZE = 8;

// exit codes for moving a tetris piece
const int FORBIDDEN = 0;
const int ALLOWED = 1;
const int FINAL = 2;


U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

class Matrix;
class Rectangle;

class Rectangle
{
  public:
    int coords[4][2] = {
      {0, 3},
      {0, 4},
      {1, 3},
      {1, 4},
    };
    int moveLeft(Matrix);
    int moveRight(Matrix);
    int moveDown(Matrix);
    int moveUp();
    void draw(U8X8_SH1106_128X64_NONAME_HW_I2C);
};

class Matrix  // stores all fixed blocks
{
  public:
    Matrix();
    void draw(U8X8_SH1106_128X64_NONAME_HW_I2C);
    void addRectangle(Rectangle);
    bool checkLine(int);
    bool checkAllLines();
    bool checkGameOver();
    void clearLine(int);
    int data[XSIZE][YSIZE];
};

bool Matrix::checkGameOver() {
  // TODO: improve game over ruling
  for (int i = 0; i < YSIZE; i++) {
    if (data[0][i] == 1) {return true;}
  }
  return false;
}

bool Matrix::checkAllLines() {
  for (int i = XSIZE - 1; i > 0; i--) {
    while (checkLine(i)) {
      clearLine(i);
    }
  }
}

bool Matrix::checkLine(int X) {
  for (int i = 0; i < YSIZE; i++) {
    if (data[X][i] == 0) {
      return false;
    }
  }
  return true;
}

void Matrix::clearLine(int X) {
  for (int i = X; i >= 0; i--) {
    for (int j = 0; j < YSIZE; j++) {
      data[i][j] = data[i - 1][j];
    }
  }
  for (int j = 0; j < YSIZE; j++) {
    data[0][j] = 0;
  }
}

void Matrix::addRectangle(Rectangle rect) {
  for (int i = 0; i < 4; i++) {
    data[rect.coords[i][0]][rect.coords[i][1]] = 1;
  }
}

void Matrix::draw(U8X8_SH1106_128X64_NONAME_HW_I2C u8) {
  for (int i = 0; i < XSIZE; i++) {
    for (int j = 0; j < YSIZE; j++) {
      if (data[i][j] == 1) {
        u8.drawString(i, j, "o");
      }
    }
  }
}

int Rectangle::moveLeft(Matrix mat)
{
  for (int i = 0; i < 4; i++) {
    if (coords[i][1] - 1 < 0) {  
      return FORBIDDEN;
    }
  }

  if (mat.data[coords[0][0]][coords[0][1]-1] == 1 || mat.data[coords[2][0]][coords[2][1]-1] == 1) {
    return FORBIDDEN;
  }

  for (int i = 0; i < 4; i++)
  {
    coords[i][1]--;
  }
  return ALLOWED;
}

int Rectangle::moveRight(Matrix mat)
{
  for (int i = 0; i < 4; i++) {
    if (coords[i][1] + 1 > YSIZE - 1) {
      return FORBIDDEN;
    }
  }

  if (mat.data[coords[1][0]][coords[1][1]+1] == 1 || mat.data[coords[3][0]][coords[3][1]+1] == 1) {
    return FORBIDDEN;
  }

  for (int i = 0; i < 4; i++) {
    coords[i][1]++;
  }
  return ALLOWED;
}

int Rectangle::moveDown(Matrix mat) {
  for (int i = 0; i < 4; i++) {
    if (coords[i][0] + 1 > XSIZE - 1)
    {
      return FINAL;
    }
  }

  if (mat.data[coords[2][0]+1][coords[2][1]] == 1 || mat.data[coords[3][0]+1][coords[3][1]] == 1) {
    return FINAL;
  }

  for (int i = 0; i < 4; i++) {
    coords[i][0]++;
  }
  return ALLOWED;
}

int Rectangle::moveUp() {
  for (int i = 0; i < 4; i++) {
    if (coords[i][0] - 1 < 0)
    {
      return FORBIDDEN;
    }
  }
  for (int i = 0; i < 4; i++) {
    coords[i][0]--;
  }
  return ALLOWED;
}



Matrix::Matrix() {
  // fill all entries with zero
  for (int i = 0; i < XSIZE; i++) {
    for (int j = 0; j < YSIZE; j++) {
      data[i][j] = 0;
    }
  }
};


void Rectangle::draw(U8X8_SH1106_128X64_NONAME_HW_I2C u8) {
  for (int i = 0; i < 4; i++) {
    u8.drawString(coords[i][0], coords[i][1], "o");
  }
}



int i = 0;
int j = 0;
int prev_j = 0;
int prev_i = 0;

Rectangle rect;
Matrix mat;


void setup() {
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  //mat.data[14][4] = 1;
  Serial.begin(9600);
  Serial.println("Qwiic Joystick Example");

  if(joystick.begin(Wire, Address) == false)
  {
    Serial.println("Joystick does not appear to be connected. Please check wiring. Freezing...");
    while(1);
  }

  delay(2000);
  u8x8.clearDisplay();
  rect.draw(u8x8);
  u8x8.refreshDisplay();
  for (int i = 0; i++; i < 4)
  {
    Serial.print(rect.coords[i][0]);
    Serial.println(rect.coords[i][1]);
  }
  Serial.println("done");
}


void loop() {
  //Serial.println("alive");
  
  int Y = joystick.getHorizontal();
  int X = joystick.getVertical();
  //int B = joystick.getButton();
  int success = FORBIDDEN;
  
  if  (X > 575) {
    Serial.println("right");
    success = rect.moveLeft(mat);
  }
  else if (X < 450) {
    Serial.println("left");
    success = rect.moveRight(mat);
  }
  if (success == ALLOWED) {
    u8x8.clearDisplay();
    rect.draw(u8x8);
    mat.draw(u8x8);
    u8x8.refreshDisplay();
  }

  currentMillis = millis();
  if (currentMillis - previousMillis > DELAY) {
    success = rect.moveDown(mat);
    if (success == ALLOWED) {
      u8x8.clearDisplay();
      rect.draw(u8x8);
      mat.draw(u8x8);
      u8x8.refreshDisplay();
    }
    else if (success == FINAL) {
      if (mat.checkGameOver()) {
        u8x8.clearDisplay();
        u8x8.drawString(0, 0, "Game over!");
        u8x8.refreshDisplay();
        delay(5000);
        mat = Matrix();
      }
      else {
        u8x8.clearDisplay();
        mat.addRectangle(rect);
        mat.draw(u8x8);
        rect = Rectangle();
        u8x8.refreshDisplay();
        mat.checkAllLines();
      }
    }
    previousMillis = currentMillis;
  }
}
