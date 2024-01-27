// --------------------- Базовые настройки для игры ----------------------

#define logo_delay 5000 // время показа логотипа
#define pre_game_delay 2000 // задержка начала игры после логотипа
#define limit 5 // максимальное колл-во очков
#define paddle_height 15 // размер ракетки
#define ball_speed 16 // скорость шарика (чем меньше число, тем быстрее)
#define paddle_speed 64 //скороть ракетки (чем меньше число, тем быстрее)

// -----------------------------------------------------------------------

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define UP_BUTTON 2
#define DOWN_BUTTON 3

const unsigned long PADDLE_RATE = 64;
const unsigned long BALL_RATE = ball_speed;
const uint8_t PADDLE_HEIGHT = paddle_height;
const uint8_t SCORE_LIMIT = limit;

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);

bool game_over, win;

uint8_t player_score, mcu_score;
uint8_t ball_x = 53, ball_y = 26;
uint8_t ball_dir_x = 1, ball_dir_y = 1;

unsigned long ball_update;
unsigned long paddle_update;

const uint8_t MCU_X = 12;
uint8_t mcu_y = 16;

const uint8_t PLAYER_X = 115;
uint8_t player_y = 16;

void setup() {
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    drawCustomLogo(); // Отображаем лого в начале

    pinMode(UP_BUTTON, INPUT_PULLUP);
    pinMode(DOWN_BUTTON, INPUT_PULLUP);

    display.clearDisplay();
    drawCourt();

    display.display();
    delay(pre_game_delay); // Задержка началом игры 

    ball_update = millis();
    paddle_update = ball_update;
    display.setTextSize(1);
    
}

void loop() 
{
    bool update_needed = false;
    unsigned long time = millis();

    static bool up_state = false;
    static bool down_state = false;
    
    up_state |= (digitalRead(UP_BUTTON) == LOW);
    down_state |= (digitalRead(DOWN_BUTTON) == LOW);

    if(time > ball_update) 
    {
        uint8_t new_x = ball_x + ball_dir_x;
        uint8_t new_y = ball_y + ball_dir_y;

        // Check if we hit the vertical walls
        if(new_x == 0 || new_x == 127) 
        {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;

            if (new_x < 64)
            {
                player_score++;
            }
            else
            {
                mcu_score++;
            }

            if (player_score == SCORE_LIMIT || mcu_score == SCORE_LIMIT)
            {
                win = player_score > mcu_score;
                game_over = true;
            }
        }

        // Check if we hit the horizontal walls.
        if(new_y == 0 || new_y == 53) 
        {
            ball_dir_y = -ball_dir_y;
            new_y += ball_dir_y + ball_dir_y;
        }

        // Check if we hit the CPU paddle
        if(new_x == MCU_X && new_y >= mcu_y && new_y <= mcu_y + PADDLE_HEIGHT) 
        {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        // Check if we hit the player paddle
        if(new_x == PLAYER_X && new_y >= player_y && new_y <= player_y + PADDLE_HEIGHT)
        {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        display.drawPixel(ball_x, ball_y, BLACK);
        display.drawPixel(new_x, new_y, WHITE);
        ball_x = new_x;
        ball_y = new_y;

        ball_update += BALL_RATE;

        update_needed = true;
    }

    if(time > paddle_update) 
    {
        paddle_update += PADDLE_RATE;

        // настройки для псевдо ии
        display.drawFastVLine(MCU_X, mcu_y, PADDLE_HEIGHT, BLACK);
        const uint8_t half_paddle = PADDLE_HEIGHT >> 1;

        if(mcu_y + half_paddle > ball_y)
        {
            int8_t dir = ball_x > MCU_X ? -1 : 1;
            mcu_y += dir;
        }

        if(mcu_y + half_paddle < ball_y)
        {
            int8_t dir = ball_x > MCU_X ? 1 : -1;
            mcu_y += dir;
        }

        if(mcu_y < 1) 
        {
            mcu_y = 1;
        }

        if(mcu_y + PADDLE_HEIGHT > 53)
        {
            mcu_y = 53 - PADDLE_HEIGHT;
        }

        // настройки игрока
        display.drawFastVLine(MCU_X, mcu_y, PADDLE_HEIGHT, WHITE);
        display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, BLACK);

        if(up_state) 
        {
            player_y -= 1;
        }

        if(down_state) 
        {
            player_y += 1;
        }

        up_state = down_state = false;

        if(player_y < 1) 
        {
            player_y = 1;
        }

        if(player_y + PADDLE_HEIGHT > 53) 
        {
            player_y = 53 - PADDLE_HEIGHT;
        }

        display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, WHITE);

        update_needed = true;
    }

    if(update_needed)
    {
        if (game_over)
        {
            const char* text = win ? "YOU WIN!!" : "YOU LOSE!";
            display.clearDisplay();
            display.setCursor(40, 28);
            display.print(text);
            display.display();

            delay(5000);

            display.clearDisplay();
            ball_x = 53;
            ball_y = 26;
            ball_dir_x = 1;
            ball_dir_y = 1;
            mcu_y = 16;
            player_y = 16;
            mcu_score = 0;
            player_score = 0;
            game_over = false;
            drawCourt();
        }

        display.setTextColor(WHITE, BLACK);
        display.setCursor(0, 56);
        display.print(mcu_score);
        display.setCursor(122, 56);
        display.print(player_score);
        display.display();
    }
}

void drawCourt() 
{
    display.drawRect(0, 0, 128, 54, WHITE);
}

void drawCustomLogo() {
    display.clearDisplay();
    display.setTextSize(1.5);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(" ");
    display.println(" ");
    display.println("PING-PONG GAME");
    display.println("ON ARDUINO");
    display.println(" ");
    display.println("develop by: Bagai35");
    display.println("Github: Bagai35");
    display.display();
    delay(logo_delay);
}
