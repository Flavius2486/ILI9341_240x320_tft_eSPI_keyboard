#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

#define GRAY 0x8410
#define WHITE 0xFFFF
#define BLACK 0x0000
#define DARKGREY 0x7BEF
//------------------------------------------------------------------------------------------
#define tft_backlight 17

TFT_eSPI_Button keyboard_keys[30];

struct keyboard_struct {
  const char (*keys)[13];  // Pointer to array of characters
};

const char lower_letters_keys[3][13] PROGMEM = {
  { 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '\0' },
  { 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\0' },
  { 'z', 'x', 'c', 'v', 'b', 'n', 'm', '\0' },
};

const char capital_letters_keys[3][13] PROGMEM = {
  { 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '\0' },
  { 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '\0' },
  { 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '\0' },
};

const char numbers_keys[3][13] PROGMEM = {
  { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\0' },
  { '-', '/', ':', ';', '(', ')', '€', '&', '@', '"', '\0' },
  { '.', ',', '?', '!', '\'', '\0' }
};

const char symbols_keys[3][13] PROGMEM = {
  { '[', ']', '{', '}', '#', '%', '^', '*', '+', '=', '\0' },
  { '_', '\\', '|', '~', '<', '>', '$', '£', '¥', '•', '\0' },
  { '.', ',', '?', '!', '\'', '\0' }
};

// Define the keyboard instances
keyboard_struct keyboard[] = {
  { lower_letters_keys },
  { capital_letters_keys },
  { numbers_keys },
  { symbols_keys }
};

struct other_keyboard_buttons {
  int x, y, width, height;
  char text[10];
};

other_keyboard_buttons switch_keyboard_buttons[6] = {
  { 32, 64 + 110, 64, 32, "CAPS" },
  { 32, 96 + 110, 64, 32, "#+=" },
  { 120, 96 + 110, 112, 32, "SPACE" },
  { 216, 96 + 110, 80, 32, "Cancel" },
  { 288, 96 + 110, 64, 32, "Save" },
  { 285, 68, 45, 25, "<-" }
};


int keyboard_page = 0;
char keyboard_data[50] = "";

void draw_keyboard() {
  tft.fillRect(0, 110, 320, 240, BLACK);
  int characters_counter = 6;
  for (int i = 0; i < 3; i++) {
    int numberOfCharactersPerLine = strlen(keyboard[keyboard_page].keys[i]);
    for (int j = 0; j < numberOfCharactersPerLine; j++) {
      int buttonXPos = ((10 - numberOfCharactersPerLine) / 2 + j) * 32;
      if (numberOfCharactersPerLine % 2 != 0)
        buttonXPos += 16;
      if (i == 2) buttonXPos = 64 + j * 32;
      char temp_character[2] = { keyboard[keyboard_page].keys[i][j], '\0' };
      keyboard_keys[characters_counter].initButton(&tft,
                                                   buttonXPos + 15,  // X Cord
                                                   i * 32 + 110,     // Y CORD
                                                   32,               // WIDTH
                                                   32,               // HEIGHT
                                                   GRAY,             //bg color
                                                   DARKGREY,         // OUTLINE
                                                   WHITE,            // TEXT COLOR
                                                   temp_character,   // TEXT TO PRINT
                                                   2);               // TEXT SIZE: SEE ABOVE Line 23
      keyboard_keys[characters_counter].drawButton();
      characters_counter++;
    }
  }
  for (int i = 0; i < 6; i++) {
    keyboard_keys[i].initButton(&tft,
                                switch_keyboard_buttons[i].x,       // X Cord
                                switch_keyboard_buttons[i].y,       // Y CORD
                                switch_keyboard_buttons[i].width,   // WIDTH
                                switch_keyboard_buttons[i].height,  // HEIGHT
                                GRAY,                               //bg color
                                DARKGREY,                           // OUTLINE
                                WHITE,                              // TEXT COLOR
                                switch_keyboard_buttons[i].text,    // TEXT TO PRINT
                                2);                                 // TEXT SIZE
    keyboard_keys[i].drawButton();
  }
}

void drawBorderedRect(int x, int y, int width, int height, uint16_t borderColor, uint16_t innerColor) {
  tft.drawRect(x, y, width, height, borderColor);
  tft.fillRect(x + 1, y + 1, width - 2, height - 2, innerColor);
}

void draw_text(String text, int x, int y, int size, u_int16_t bg_color) {
  tft.setTextSize(size);
  tft.setTextColor(WHITE, bg_color);
  tft.setCursor(x, y);
  tft.print(text);
}

void setup() {
  Serial.begin(115200);
  Serial.println("HBJHJBKHJBK");
  tft.begin();
  tft.setRotation(3);
  tft.fillRect(0, 0, 320, 240, BLACK);
  pinMode(tft_backlight, OUTPUT);
  digitalWrite(tft_backlight, HIGH);
  tft.fillRect(0, 0, 320, 40, GRAY);
  keyboard_page = 0;
  draw_keyboard();
  drawBorderedRect(5, 50, 250, 35, GRAY, BLACK);
  draw_text(keyboard_data, 10, 65, 2, BLACK);
}

void loop() {
  uint16_t t_x = 0, t_y = 0;
  boolean pressed = tft.getTouch(&t_x, &t_y);
  t_x = (t_x - 320) * -1;  // Fix touch coordinates issue
  if (pressed) {
    for (int i = 0; i < 6; i++) {
      keyboard_keys[i].press(keyboard_keys[i].contains(t_x, t_y));
      if (keyboard_keys[i].justPressed()) {
        switch (i) {
          case 0:
            keyboard_page = keyboard_page == 1 ? 0 : 1;
            draw_keyboard();
            break;
          case 1:
            keyboard_page = keyboard_page == 0 || keyboard_page == 1 || keyboard_page == 3 ? 2 : 3;
            draw_keyboard();
            break;
          case 2:
            if (strlen(keyboard_data) < 20) {
              strcat(keyboard_data, " ");
              draw_text(keyboard_data, 10, 65, 2, BLACK);
            }
            break;
          case 3:
            // Go to the Wi-Fi pages and delete string
            break;
          case 4:
            // Connect to the network
            break;
          case 5:
            if (strlen(keyboard_data) > 0) {
              keyboard_data[strlen(keyboard_data) - 1] = '\0';
              strcat(keyboard_data, " ");
              draw_text(keyboard_data, 10, 65, 2, BLACK);
              keyboard_data[strlen(keyboard_data) - 1] = '\0';
            }
            break;
        }
        keyboard_keys[i].press(false);
        break;
      }
    }
    int characters_counter = 6;
    bool exit_loop = false;
    for (int i = 0; i < 3 && !exit_loop; i++) {
      for (int j = 0; j < strlen(keyboard[keyboard_page].keys[i]); j++) {
        keyboard_keys[characters_counter].press(keyboard_keys[characters_counter].contains(t_x, t_y));
        if (keyboard_keys[characters_counter].justPressed()) {
          // Serial.println(keyboard[keyboard_page].keys[i][j]);
          char temp_string[2] = { keyboard[keyboard_page].keys[i][j], '\0' };
          if (strlen(keyboard_data) < 20) {
            strcat(keyboard_data, temp_string);
            draw_text(keyboard_data, 10, 65, 2, BLACK);
          }
          keyboard_keys[characters_counter].press(false);
          exit_loop = true;
          break;
        }
        characters_counter++;
      }
    }
  }
  delay(50);
}
