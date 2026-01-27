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

const int XSIZE = 16;
const int YSIZE = 8;

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

class Matrix  // stores all fixed blocks
{
  public:
    Matrix();
    int data[XSIZE][YSIZE];
};

class Rectangle //TODO: make this child of abstract parent class
{
  public:
    int coords[4][2] = {
      {0, 3},
      {0, 4},
      {1, 3},
      {1, 4},
    };
    bool moveLeft();
    bool moveRight();
    bool moveDown(Matrix);
    bool moveUp();
};

bool Rectangle::moveLeft()
{
  for (int i = 0; i < 4; i++)
  {
    if (coords[i][1] - 1 < 0)
    {  
      return false;
    }
  }
  for (int i = 0; i < 4; i++)
  {
    coords[i][1]--;
  }
  return true;
}

bool Rectangle::moveRight()
{
  for (int i = 0; i < 4; i++)
  {
    if (coords[i][1] + 1 > YSIZE - 1)
    {
      return false;
    }
  }
  for (int i = 0; i < 4; i++)
  {
    coords[i][1]++;
  }
  return true;
}

bool Rectangle::moveDown(Matrix mat)
{
  // check for collision with lower boundary
  for (int i = 0; i < 4; i++)
  {
    if (coords[i][0] + 1 > XSIZE - 1)
    {
      return false;
    }
  }
  /*
  check for collision with existing pieces:
  x+1 must be free (i.e.==0) in mat, means check coords[3, 0]+1, coords[4, 0]+1
  */
  if (mat.data[coords[3][0]+1][coords[3][1]] == 1 || mat.data[coords[4][0]+1][coords[4][1]] == 1) {return false;}

  for (int i = 0; i < 4; i++)
  {
    coords[i][0]++;
  }
  return true;
}

bool Rectangle::moveUp()
{
  for (int i = 0; i < 4; i++)
  {
    if (coords[i][0] - 1 < 0)
    {
      return false;
    }
  }
  for (int i = 0; i < 4; i++)
  {
    coords[i][0]--;
  }
  return true;
}



Matrix::Matrix() {
  // fill all entries with zero
  for (int i = 0; i < XSIZE; i++)
  {
    for (int j = 0; j < YSIZE; j++)
    {
      data[i][j] = 0;
    }
  }
};

void drawMatrix(Matrix mat, U8X8_SH1106_128X64_NONAME_HW_I2C u8)
{
  return;
}

void drawRectangle(Rectangle rect, U8X8_SH1106_128X64_NONAME_HW_I2C u8)
{
  for (int i = 0; i<4;i++)
  {
    u8.drawString(rect.coords[i][0], rect.coords[i][1], "o");
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

  mat.data[14][4] = 1;
  Serial.begin(9600);
  Serial.println("Qwiic Joystick Example");

  if(joystick.begin(Wire, Address) == false)
  {
    Serial.println("Joystick does not appear to be connected. Please check wiring. Freezing...");
    while(1);
  }

  delay(2000);
  u8x8.clearDisplay();
  drawRectangle(rect, u8x8);
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
  bool success = false;
  
  if  (X > 575)
  {
    Serial.println("right");
    success = rect.moveLeft();
  }
  else if (X < 450)
  {
    Serial.println("left");
    success = rect.moveRight();
  }
  if (success) {
    u8x8.clearDisplay();
    drawRectangle(rect, u8x8);
    u8x8.refreshDisplay();
  }

  currentMillis = millis();
  if (currentMillis - previousMillis > DELAY) {
    success = rect.moveDown(mat);
    if (success) {
      u8x8.clearDisplay();
      drawRectangle(rect, u8x8);
      u8x8.refreshDisplay();
    }
    previousMillis = currentMillis;
  }
}
