#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    LEFT, 
    RIGHT
}WhichSide;

typedef struct
{
    Rectangle rect;
    WhichSide side;
}Paddle;

typedef struct
{
    Vector2 pos;
    float radius;
    Vector2 direction;
    float speed;
    float startSpeed;
    float speedIncreaseAmount;
    float maxSpeed;
}Ball;

int update(int screenW, int screenH, Ball* ball, Paddle* leftP, Paddle* rightP, 
    float mid2SideAngle, int* rightScore, int* leftScore, int maxScore, 
    Sound const* paddleHitSound, Sound const* scoreSound);
    
//returns -1 if user presses the quit button.
//returns 1 if the user presses the reset button.
//returns 0 if the user has not yet pressed the reset or quit button.
int draw(int screenW, int screenH, Ball const* ball, Paddle const* leftP, 
    Paddle const* rightP, int leftScore, int rightScore, bool isGameOver);
    
//returns -1 if right side scored (ball went off left side of screen)
//returns 1 if left side scored (ball went off right side of screen)
//otherwise returns 0
int updateBall(Ball* ball, int screenW, int screenH, float mid2SideAngle, 
    Paddle const* leftPaddle, Paddle const* rightPaddle, Sound const* paddleHitSound);
    
void  ballCtor(Ball* this, int screenW, int screenH, float mid2SideAngle);
void  paddleCtor(Paddle* this, WhichSide side, int screenW, int screenH);
void  updatePaddles(Paddle* leftP, Paddle* rightP, int screenH);  
float calcMidToSideAngle(int screenW, int screenH);
void  resetBall(Ball* ball, int screenW, int screenH, float mid2SideAngle);
void  drawHalfCourt(int screenW, int screenH);
void  drawScore(int leftSideScore, int rightSideScore, int screenW);
int   drawEndPopup(int screenW, int screenH);
void  resetGame(int screenW, int screenH, Ball* b, float mid2SideAngle, 
    int* rightScore, int* leftScore, bool* isGameOver);

int main(void)
{
    srand((unsigned)time(NULL));
    int const screenW = 868, screenH = 1024;
    InitWindow(screenW, screenH, "Pong");
    InitAudioDevice();
    Sound paddleHitSound = LoadSound("./sounds/paddleHit.wav");
    Sound scoreSound = LoadSound("./sounds/score.wav");
    SetTargetFPS(144);

    float const mid2SideAngle = calcMidToSideAngle(screenW, screenH);

    Paddle leftPaddle, rightPaddle;
    paddleCtor(&leftPaddle, LEFT, screenW, screenH);
    paddleCtor(&rightPaddle, RIGHT, screenW, screenH);
    
    Ball ball;
    ballCtor(&ball, screenW, screenH, mid2SideAngle);
    
    int leftScore = 0;
    int rightScore = 0;
    int const maxScore = 11;
    
    bool isGameOver = false;
    
    while( ! WindowShouldClose() )
    {
        int const endPopupResult = draw(screenW, screenH, &ball, &leftPaddle, &rightPaddle, 
            leftScore, rightScore, isGameOver);
        
        if( ! isGameOver )
        {
            int const updateResult = update(screenW, screenH, &ball, &leftPaddle, &rightPaddle,
                mid2SideAngle, &rightScore, &leftScore, maxScore, &paddleHitSound, &scoreSound);
                
            if(updateResult != 0) { isGameOver = true; }
        }
        else
        {
            if(endPopupResult == -1) { break; }
            else if(endPopupResult == 1)
                resetGame(screenW, screenH, &ball, mid2SideAngle, &rightScore, &leftScore, &isGameOver);
        }
    }
    
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void resetGame(int screenW, int screenH, Ball* b, float mid2SideAngle, int* rightScore, int* leftScore, bool* isGameOver)
{
    *isGameOver = false;
    *rightScore = 0;
    *leftScore = 0;
    resetBall(b, screenW, screenH, mid2SideAngle);
}

//returns -1 if user presses the quit button.
//returns 1 if the user presses the reset button.
//returns 0 if the user has not yet pressed the reset or quit button.
int draw(int screenW, int screenH, Ball const* ball, Paddle const* leftP, 
    Paddle const* rightP, int leftScore, int rightScore, bool isGameOver)
{   
    BeginDrawing();
    ClearBackground(BLACK);

    DrawRectangleRec(leftP->rect, RAYWHITE);
    DrawRectangleRec(rightP->rect, RAYWHITE);
    DrawCircleV(ball->pos, ball->radius, RAYWHITE);
    drawHalfCourt(screenW, screenH);
    drawScore(leftScore, rightScore, screenW);
    
    int endPopupResult = 0;
    if(isGameOver) { endPopupResult = drawEndPopup(screenW, screenH); }
    
    EndDrawing();
    
    return endPopupResult;
}

//returns -1 if left side won, 1 if right side won, and 0 if neither side has won.
int update(int screenW, int screenH, Ball* ball, Paddle* leftP, Paddle* rightP, 
    float mid2SideAngle, int* rightScore, int* leftScore, int maxScore, 
    Sound const* paddleHitSound, Sound const* scoreSound)
{
    updatePaddles(leftP, rightP, screenH);
    
    int const newScore = updateBall(ball, screenW, screenH,
        mid2SideAngle, leftP, rightP, paddleHitSound);
        
    if(newScore == -1) 
    { 
        ++(*rightScore);
        PlaySound(*scoreSound);
    }
    else if(newScore == 1) 
    { 
        ++(*leftScore);
        PlaySound(*scoreSound);
    }
    
    if( (*rightScore) >= maxScore ) { return 1; }
    else if( (*leftScore) >= maxScore) { return -1; }
    
    return 0;
}

bool isMouseOverRect(Vector2 rectSize, Vector2 rectPos)
{
    Vector2 const mousePos = GetMousePosition();
    return mousePos.x > rectPos.x &&
           mousePos.x < rectPos.x + rectSize.x &&
           mousePos.y > rectPos.y &&
           mousePos.y < rectPos.y + rectSize.y;
}

//returns -1 if user presses the quit button.
//returns 1 if the user presses the reset button.
//returns 0 if the user has not yet pressed the reset or quit button.
int drawEndPopup(int screenW, int screenH)
{
    Vector2 const popupSize = {screenW / 5.0f, screenH / 5.0f};
    Vector2 const popupPos = {screenW / 2 - popupSize.x / 2, screenH / 2 - popupSize.y / 2};
    
    Vector2 const buttonSize = {popupSize.x * .8f, popupSize.y / 3};
    Vector2 const quitPos = {popupPos.x + popupSize.x * .1f, popupPos.y + buttonSize.y * 0.33f};
    Vector2 const resetPos = {quitPos.x, popupPos.y + buttonSize.y * 1.66f};
    
    char const* resetTxt = "Reset";
    char const* quitTxt = "Quit";
    float const fontSize = 22.0f;
    int const resetTxtSize = MeasureText(resetTxt, fontSize);
    int const quitTxtSize = MeasureText(quitTxt, fontSize);
    
    bool const isMouseOverReset = isMouseOverRect(buttonSize, resetPos);
    bool const isMouseOverQuit = isMouseOverRect(buttonSize, quitPos);
    
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if(isMouseOverReset) { return 1; }
        else if(isMouseOverQuit) { return -1; }
    }
    
    Color const quitColor = isMouseOverQuit ? GRAY : LIGHTGRAY;
    Color const resetColor = isMouseOverReset ? GRAY : LIGHTGRAY;
    
    DrawRectangleV(popupPos, popupSize, DARKGRAY);
    DrawRectangleV(resetPos, buttonSize, resetColor);
    DrawRectangleV(quitPos, buttonSize, quitColor);
    
    DrawText(resetTxt, resetPos.x + buttonSize.x / 2 - resetTxtSize / 2,
        resetPos.y + buttonSize.y / 2 - fontSize / 2, fontSize, BLACK);
    
    DrawText(quitTxt, quitPos.x + buttonSize.x / 2 - quitTxtSize / 2,
        quitPos.y + buttonSize.y / 2 - fontSize / 2, fontSize, BLACK);
    
    return 0;
}

void drawScore(int leftSideScore, int rightSideScore, int screenW)
{
    //The maximum possible score is 11 so 2 chars + '\n' is all that is needed
    char leftScoreText[3] = {0};
    char rightScoreText[3] = {0};
    
    sprintf_s(leftScoreText, sizeof(leftScoreText), "%d", leftSideScore);
    sprintf_s(rightScoreText, sizeof(rightScoreText), "%d", rightSideScore);
    
    float const fontSize = 50.0f;
    Color textColor = (Color){200, 200, 200, 230};
    textColor.a = 190;
    
    int const leftScoreX = screenW / 5;
    DrawText(leftScoreText, leftScoreX, 30, fontSize, textColor);
    
    int const rightScoreX = screenW - leftScoreX - MeasureText(rightScoreText, fontSize);
    DrawText(rightScoreText, rightScoreX, 30, fontSize, textColor);
}

void drawHalfCourt(int screenW, int screenH)
{
    //draw the dotted line that represents the half court
    float const lineSegmentHeight = 30.0f, lineSegmentWidth = 4.0f, gap = lineSegmentHeight * 0.45f;
    for(float f = 0.0f; f < screenH; f += lineSegmentHeight + gap)
    {
        DrawRectangle(screenW / 2 - lineSegmentWidth / 2, f, 
            lineSegmentWidth, lineSegmentHeight, (Color){200, 200, 200, 180});
    }
}

//call 1 time in main before the game loop. this should only need to be re called if the window gets resized
float calcMidToSideAngle(int screenW, int screenH)
{
    Vector2 const mid = {screenW * 0.5f, screenH * 0.5f};
    Vector2 const mid2TopRight = {screenW - mid.x, 0 - mid.y};
    Vector2 const mid2BottomRight = {screenW - mid.x, screenH - mid.y};
    
    float const dot = mid2TopRight.x * mid2BottomRight.x + mid2TopRight.y * mid2BottomRight.y;
    float const magTop = sqrt(mid2TopRight.x * mid2TopRight.x + mid2TopRight.y * mid2TopRight.y);
    float const magBottom = sqrt(mid2BottomRight.x * mid2BottomRight.x + mid2BottomRight.y * mid2BottomRight.y);
    float const radians = acos(dot / (magTop * magBottom));

    return radians;
}

void paddleCtor(Paddle* this, WhichSide side, int screenW, int screenH)
{
    float const paddleGap = 16.0f;
    this->rect.width = 10.0f;
    this->rect.height = screenH / 13.0f;
    this->rect.y = screenH / 2 - this->rect.height / 2;
    this->side = side;
    this->rect.x = (side == LEFT ? paddleGap : (screenW - this->rect.width - paddleGap));
}

void updatePaddles(Paddle* leftP, Paddle* rightP, int screenH)
{
    float const dt = GetFrameTime();
    float const paddleSpeed = 1300.0f;
    
    if(IsKeyDown(KEY_W)) //left paddle up
    {
        leftP->rect.y -= dt * paddleSpeed;
        if(leftP->rect.y < 0)
            leftP->rect.y = 0;
    }
 
    if(IsKeyDown(KEY_S)) //left paddle down
    {
        leftP->rect.y += dt * paddleSpeed;
        if(leftP->rect.y > screenH - leftP->rect.height)
            leftP->rect.y = screenH - leftP->rect.height;
    }
    
    if(IsKeyDown(KEY_UP)) //right paddle up
    {
        rightP->rect.y -= dt * paddleSpeed;
        if(rightP->rect.y < 0)
            rightP->rect.y = 0;
    }
    
    if(IsKeyDown(KEY_DOWN)) //right paddle down
    {
        rightP->rect.y += dt * paddleSpeed;
        if(rightP->rect.y > screenH - rightP->rect.height)
            rightP->rect.y = screenH - rightP->rect.height;
    }
}

//mid2SideAngle: get with calcMidToSideAngle() in main so it only needs to be calculated once.
void setBallInitialDirection(Ball* ball, int screenW, int screenH, float mid2SideAngle)
{
    //calc random angle in [0, mid2SideAngle]
    float randRadians = (float)rand() / RAND_MAX * mid2SideAngle;
    
    ball->direction.x = cos(randRadians - mid2SideAngle * 0.5f);
    ball->direction.y = sin(randRadians - mid2SideAngle * 0.5f);
    
    //if rand() picks an odd number, then the ball flies towards the left side of the screen
    if(rand() & 1) 
        ball->direction.x = -ball->direction.x;
}

void ballCtor(Ball* this, int screenW, int screenH, float const mid2SideAngle)
{
    this->pos = (Vector2){screenW * 0.5f, screenH * 0.5f};
    this->startSpeed = 500.0f;
    this->maxSpeed = 800.0f;
    this->speed = this->startSpeed;
    this->radius = 8.0f;
    this->speedIncreaseAmount = 40.0f;
    setBallInitialDirection(this, screenW, screenH, mid2SideAngle);
}

void resetBall(Ball* ball, int screenW, int screenH, float mid2SideAngle)
{
    ball->speed = ball->startSpeed;
    ball->pos.x = screenW * 0.5f;
    ball->pos.y = screenH * 0.5f;
    setBallInitialDirection(ball, screenW, screenH, mid2SideAngle);
}

bool ballPaddleCollision(Paddle const* p, Ball const* b)
{
    Vector2 const closestPoint = 
    {
        fmin(p->rect.width + p->rect.x, fmax(b->pos.x, p->rect.x)),
        fmin(p->rect.height + p->rect.y, fmax(b->pos.y, p->rect.y))
    };
    Vector2 const center2ClosestPoint = 
    { 
        closestPoint.x - b->pos.x, 
        closestPoint.y - b->pos.y 
    };
    
    float const center2ClosestPointLengthSqrd = center2ClosestPoint.x * center2ClosestPoint.x +
        center2ClosestPoint.y * center2ClosestPoint.y;
        
    return center2ClosestPointLengthSqrd <= b->radius * b->radius;
}

float mapRange2Range(float min0, float max0, float min1, float max1, float input)
{
    float const delta0 = max0 - min0;
    float const delta1 = max1 - min1;
    return (((input - min0) / delta0 * delta1)) + min1;
}

//returns -1 if right side scored (ball went off left side of screen)
//returns 1 if left side scored (ball went off right side of screen)
//otherwise returns 0
int updateBall(Ball* ball, int screenW, int screenH, float mid2SideAngle, 
    Paddle const* leftPaddle, Paddle const* rightPaddle, Sound const* paddleHitSound)
{
    float const dt = GetFrameTime();
    ball->pos.x += ball->direction.x * ball->speed * dt;
    ball->pos.y += ball->direction.y * ball->speed * dt;
    
    //if the left side has scored
    if(ball->pos.x - ball->radius > screenW)
    {
        resetBall(ball, screenW, screenH, mid2SideAngle);
        return 1;
    }//if the right side has scored
    else if(ball->pos.x + ball->radius < 0)
    {
        resetBall(ball, screenW, screenH, mid2SideAngle);
        return -1;
    }
    
    //if we hit the floor or ceil then flip the y direction
    if(ball->pos.y + ball->radius >= screenH || ball->pos.y - ball->radius <= 0)
    {
        ball->direction.y = -ball->direction.y;
    }
    else if(ballPaddleCollision(leftPaddle, ball))
    {
        float const newAngle = mapRange2Range(0, 
            leftPaddle->rect.height, -55, 55, ball->pos.y - leftPaddle->rect.y);
        
        ball->direction.x = cos(PI / 180 * newAngle);
        ball->direction.y = sin(PI / 180 * newAngle);
        
        PlaySound(*paddleHitSound);
        
        if(ball->speed < ball->maxSpeed)
        {
            ball->speed += (ball->maxSpeed - ball->speed >= ball->speedIncreaseAmount) ? 
                ball->speedIncreaseAmount : ball->maxSpeed - ball->speed;
        }
    }
    else if(ballPaddleCollision(rightPaddle, ball))
    {
        float const newAngle = mapRange2Range(0,
            rightPaddle->rect.height, -55, 55, ball->pos.y - rightPaddle->rect.y);
        
        ball->direction.x = -cos(PI / 180 * newAngle);
        ball->direction.y = sin(PI / 180 * newAngle);
        
        PlaySound(*paddleHitSound);
        
        if(ball->speed < ball->maxSpeed)
        {
            ball->speed += (ball->maxSpeed - ball->speed >= ball->speedIncreaseAmount) ? 
                ball->speedIncreaseAmount : ball->maxSpeed - ball->speed;
        }
    }
    
    return 0;
}