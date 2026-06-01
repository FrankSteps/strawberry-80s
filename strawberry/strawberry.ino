/*
Criado por: Francisco Passos
Criado em:  12/08/2025
Modif. em:  01/06/2026

Descrição:

Strawberry é um projeto de computador vintage funcional que recria a experiência estética 
e prática dos computadores antigos, especialmente do Macintosh clássico. O projeto utiliza 
um Arduino Mega (ATmega 2560) como CPU, uma miniTV de 5.5 polegadas como monitor e uma 
calculadora antiga como teclado.

O Berry OS (Strawberry Operating System) é o software responsável por gerenciar a entrada do 
teclado, processar comandos e exibir a saída na tela.
*/

#include <TVout.h>
#include "TVoutfonts/fontALL.h"
#include <EEPROM.h>

TVout TV;

// Pinos
const int BUZZER_PIN = 49;

// leds de indicação 
const int greenLED = 53;

// EEPROM
const int EEPROM_ADDR = 0;


// Teclado matricial
const byte ROWS = 5;
const byte COLS = 7;

const byte rowPins[ROWS] = {30, 33, 32, 34, 35};
const byte colPins[COLS] = {23, 22, 24, 28, 27, 26, 25};

// teclas do teclado matricial
const char* hexaKeys[ROWS][COLS] = {
  {"OFF", "EX",  "+/-", " ", " ", " ",   " " },
  {"%",   "/",   "9",   "8", "7", "MU",  "->" },
  {"-",   "x",   "6",   "5", "4", "MR",  "GT" },
  {"=",   "+",   "3",   "2", "1", "M-",  "CE" },
  {" ",   " ",  ".",   "00", "0", "M+",  "ON" }
};


// estado da calculadora
char inputBuffer[16]  = "";
char lineBuffer[20]   = "";
float operandA        = 0;
char  pendingOp       = 0;
bool  freshResult     = false;
float grandTotal      = 0;
bool  errorState      = false; // true quando há ERR na tela


byte cursorY = 0;
bool infoScreenActive = false;


// lidar com a EEPROM

void eepromSaveFloat(float val) {
  EEPROM.put(EEPROM_ADDR, val);
}

float eepromLoadFloat() {
  float val;
  EEPROM.get(EEPROM_ADDR, val);
  return val;
}


// BUZZER

void sound(int quant, int delayMs) {
  for (int i = 0; i < quant; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(delayMs);
    digitalWrite(BUZZER_PIN, LOW);
    delay(delayMs);
  }
}


// TELA

void nextLine() {
  cursorY += 8;
  if (cursorY > 88) {
    TV.clear_screen();
    cursorY = 0;
  }
}


void printLine(const char* text) {
  char padded[17];
  snprintf(padded, sizeof(padded), "%-16s", text);
  TV.set_cursor(0, cursorY);
  TV.print(padded);
  nextLine();
}


void redrawCurrentLine() {
  char padded[17];
  snprintf(padded, sizeof(padded), "%-16s", lineBuffer);
  TV.set_cursor(0, cursorY);
  TV.print(padded);
}


void redrawCurrentLineWithCursor() {
  char display[17];
  snprintf(display, sizeof(display), "%-16s", lineBuffer);
  int len = strlen(lineBuffer);
  if (len < 16) display[len] = '_';
  TV.set_cursor(0, cursorY);
  TV.print(display);
}


void typeText(const char* text, int delayMs = 50) {
  TV.set_cursor(0, cursorY);
  while (*text) {
    TV.print(*text);
    delay(delayMs);
    text++;
  }
  nextLine();
}


// informações sobre a calculadora

void showInfoScreen() {
  TV.clear_screen();
  cursorY = 0;
  typeText("---------------", 20);
  typeText("Strawberry v2.0", 50);
  typeText("---------------", 20);
  typeText("CPU    :ATmega ", 30);
  typeText("Board  :Arduino", 30);
  typeText("Softw  :BerryOS", 30);
  typeText("R. date: 06/26 ", 30);
  typeText("You    : $user ", 30);
  typeText("---------------", 20);
  typeText("> Francisco P. ", 50);
  typeText("> '->' for exit", 50);
}


// CALCULADORA — helpers

void floatToStr(float val, char* buf) {
  if (isnan(val) || isinf(val)) {
    strcpy(buf, "ERR");
    return;
  }
  if (val > 9999999) val = 9999999;
  if (val < -9999999) val = -9999999;

  if (val == (long)val) {
    ltoa((long)val, buf, 10);
  } else {
    dtostrf(val, 1, 4, buf);
  }
}

// Retorna true se houve erro
bool applyOp(float a, char op, float b, float &result) {
  if (op == '/' && b == 0) {
    return true; // erro de divisão por zero
  }
  switch (op) {
    case '+': result = a + b; 
    break;

    case '-': result = a - b; 
    break;

    case '*': result = a * b; 
    break;

    case '/': result = a / b; 
    break;

    default:  result = b;     
    break;
  }
  return false;
}

void showError(const char* msg) {
  // Congela a linha atual e mostra ERR na próxima
  redrawCurrentLine();
  nextLine();
  printLine(msg);
  // Novo prompt
  inputBuffer[0] = '\0';
  lineBuffer[0]  = '\0';
  strcat(lineBuffer, "$ ");
  redrawCurrentLineWithCursor();
  errorState = true;
}

void rebuildLineBuffer() {
  lineBuffer[0] = '\0';
  strcat(lineBuffer, "$ ");

  if (pendingOp != 0) {
    char tmp[16];
    floatToStr(operandA, tmp);
    strcat(lineBuffer, tmp);
    strcat(lineBuffer, " ");
    char opStr[2] = {pendingOp == '*' ? 'x' : pendingOp, '\0'};
    strcat(lineBuffer, opStr);
    strcat(lineBuffer, " ");
  }

  strcat(lineBuffer, inputBuffer);
}


// reset da calculadora
void resetCalc() {
  inputBuffer[0] = '\0';
  lineBuffer[0]  = '\0';
  operandA       = 0;
  pendingOp      = 0;
  freshResult    = false;
  grandTotal     = 0;
  errorState     = false;

  TV.clear_screen();
  cursorY = 0;

  strcat(lineBuffer, "$ ");
  redrawCurrentLineWithCursor();
}


// AÇÕES DA CALCULADORA

void appendDigit(const char* digit) {
  if (errorState) return; // bloqueia entrada em estado de erro
  if (strcmp(digit, ".") == 0 && strchr(inputBuffer, '.') != NULL) return;
  if (strlen(inputBuffer) >= 14) return;

  if (freshResult) {
    operandA    = 0;
    pendingOp   = 0;
    freshResult = false;
  }

  strcat(inputBuffer, digit);
  rebuildLineBuffer();
  redrawCurrentLineWithCursor();
}

void setOperator(char op) {
  if (errorState) return;

  if (pendingOp != 0 && strlen(inputBuffer) > 0) {
    float result = 0;
    if (applyOp(operandA, pendingOp, atof(inputBuffer), result)) {
      showError("> ERR:DIV/0");
      return;
    }
    operandA = result;
  } else if (strlen(inputBuffer) > 0) {
    operandA = atof(inputBuffer);
  }

  pendingOp = op;
  inputBuffer[0] = '\0';
  freshResult = false;
  rebuildLineBuffer();
  redrawCurrentLine();
}

void doEquals() {
  if (errorState) return;

  float operandB = (strlen(inputBuffer) > 0) ? atof(inputBuffer) : operandA;
  float result   = 0;

  if (pendingOp != 0) {
    if (applyOp(operandA, pendingOp, operandB, result)) {
      // Congela linha com "="
      rebuildLineBuffer();
      strcat(lineBuffer, " =");
      redrawCurrentLine();
      nextLine();
      showError("> ERR:DIV/0");
      return;
    }
  } else {
    result = operandB;
  }

  grandTotal += result;

  rebuildLineBuffer();
  strcat(lineBuffer, " =");
  redrawCurrentLine();
  nextLine();

  char resultStr[16];
  floatToStr(result, resultStr);
  char resultLine[20] = "> ";
  strcat(resultLine, resultStr);
  printLine(resultLine);

  inputBuffer[0] = '\0';
  lineBuffer[0]  = '\0';
  strcat(lineBuffer, "$ ");
  redrawCurrentLineWithCursor();

  operandA    = result;
  pendingOp   = 0;
  freshResult = true;
}

// TECLADO
const char* getKey() {
  for (byte col = 0; col < COLS; col++) {
    digitalWrite(colPins[col], LOW);
    for (byte row = 0; row < ROWS; row++) {
      if (digitalRead(rowPins[row]) == LOW) {
        delay(50); // debounce na descida
        if (digitalRead(rowPins[row]) == LOW) {
          // Libera todas as colunas antes de esperar soltar
          for (byte c = 0; c < COLS; c++) digitalWrite(colPins[c], HIGH);
          // Espera soltar
          while (digitalRead(rowPins[row]) == LOW);
          // Delay após soltar — evita redetecção imediata
          delay(80);
          return hexaKeys[row][col];
        }
      }
    }
    digitalWrite(colPins[col], HIGH);
  }
  return NULL;
}


// PROCESSAMENTO DE TECLAS
void handleKey(const char* key) {
  if (key == NULL) return;
  if (infoScreenActive && strcmp(key, "->") != 0) return;

  Serial.println(key);

  // Em estado de erro só aceita ON, OFF e EX
  if (errorState) {
    if (strcmp(key, "ON")  == 0 || strcmp(key, "OFF") == 0) {
      infoScreenActive = false;
      resetCalc();
    } else if (strcmp(key, "EX") == 0) {
      errorState = false;
      inputBuffer[0] = '\0';
      lineBuffer[0]  = '\0';
      strcat(lineBuffer, "$ ");
      operandA  = 0;
      pendingOp = 0;
      redrawCurrentLineWithCursor();
    }
    return;
  }

  // dígitos
  if ((key[0] >= '0' && key[0] <= '9') || strcmp(key, ".") == 0) {
    appendDigit(key);
    return;
  }

  if (strcmp(key, "00") == 0) {
    if (strlen(inputBuffer) > 0) {
      appendDigit("0");
      appendDigit("0");
    }
    return;
  }

  if (strcmp(key, "+") == 0) { 
    setOperator('+'); 
    return; 
  }
  if (strcmp(key, "-") == 0) { 
    setOperator('-'); 
    return; 
  }
  if (strcmp(key, "x") == 0) { 
    setOperator('*'); 
    return; 
  }
  if (strcmp(key, "/") == 0) { 
    setOperator('/'); 
    return; 
  }
  if (strcmp(key, "=") == 0) { 
    doEquals(); 
    return; 
  }


  if (strcmp(key, "%") == 0) {
    float val = atof(inputBuffer) / 100.0;
    dtostrf(val, 1, 4, inputBuffer);
    rebuildLineBuffer();
    redrawCurrentLineWithCursor();
    return;
  }


  if (strcmp(key, "+/-") == 0) {
    if (strlen(inputBuffer) > 0) {
      float val = atof(inputBuffer) * -1;
      dtostrf(val, 1, 4, inputBuffer);
      rebuildLineBuffer();
      redrawCurrentLineWithCursor();
    }
    return;
  }

  // --- CE: apaga último dígito ---
  if (strcmp(key, "CE") == 0) {
    int len = strlen(inputBuffer);
    if (len > 0) inputBuffer[len - 1] = '\0';
    rebuildLineBuffer();
    redrawCurrentLineWithCursor();
    return;
  }

  // --- EX: limpa entrada atual ---
  if (strcmp(key, "EX") == 0) {
    inputBuffer[0] = '\0';
    lineBuffer[0]  = '\0';
    strcat(lineBuffer, "$ ");
    operandA  = 0;
    pendingOp = 0;
    redrawCurrentLineWithCursor();
    return;
  }


  if (strcmp(key, "M+") == 0) {
    float mem = eepromLoadFloat();
    if (isnan(mem) || isinf(mem)) mem = 0.0;
    eepromSaveFloat(mem + atof(inputBuffer));
    sound(1, 100);
    return;
  }


  if (strcmp(key, "M-") == 0) {
    float mem = eepromLoadFloat();
    if (isnan(mem) || isinf(mem)) mem = 0.0;
    eepromSaveFloat(mem - atof(inputBuffer));
    sound(1, 100);
    return;
  }


  if (strcmp(key, "MR") == 0) {
    float mem = eepromLoadFloat();
    if (isnan(mem) || isinf(mem)) {
      eepromSaveFloat(0.0);
      strcpy(inputBuffer, "0");
    } else {
      floatToStr(mem, inputBuffer);
    }
    rebuildLineBuffer();
    redrawCurrentLineWithCursor();
    return;
  }

  if (strcmp(key, "MU") == 0) {
    float markup = atof(inputBuffer);
    if (markup < 100 && operandA != 0) {
      float result = operandA / (1.0 - markup / 100.0);
      floatToStr(result, inputBuffer);
      rebuildLineBuffer();
      redrawCurrentLineWithCursor();
    }
    return;
  }


  if (strcmp(key, "GT") == 0) {
    char gtLine[20] = "> GT:";
    char tmp[12];
    floatToStr(grandTotal, tmp);
    strcat(gtLine, tmp);
    printLine(gtLine);
    lineBuffer[0] = '\0';
    strcat(lineBuffer, "$ ");
    redrawCurrentLineWithCursor();
    return;
  }


  if (strcmp(key, "ON") == 0 || strcmp(key, "OFF") == 0) {
    infoScreenActive = false;
    resetCalc();
    return;
  }


  if (strcmp(key, "->") == 0) {
    if (!infoScreenActive) {
      showInfoScreen();
      infoScreenActive = true;
    } else {
      infoScreenActive = false;
      resetCalc();
    }
    return;
  }
}



void setup() {
  TV.begin(NTSC, 128, 96);
  TV.select_font(font8x8);
  TV.clear_screen();
  Serial.begin(9600);
  
  // led de indicação 
  pinMode(greenLED, OUTPUT);
  digitalWrite(greenLED, HIGH);

  // Para caso de emergência com memória EEPROM corrompida
  // eepromSaveFloat(0.0);

  pinMode(BUZZER_PIN, OUTPUT);
  sound(3, 200);

  for (byte row = 0; row < ROWS; row++) {
    pinMode(rowPins[row], INPUT_PULLUP);
  }

  for (byte col = 0; col < COLS; col++) {
    pinMode(colPins[col], OUTPUT);
    digitalWrite(colPins[col], HIGH);
  }

  float mem = eepromLoadFloat();
  if (isnan(mem) || isinf(mem)) eepromSaveFloat(0.0);

  resetCalc();
}


void loop() {
  const char* key = getKey();
  handleKey(key);
}