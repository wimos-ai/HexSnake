/*******************************************************************************************
 *
 *   raylib gamejam template
 *
 *   Code licensed under an unmodified zlib/libpng license, which is an OSI-certified,
 *   BSD-like license that allows static linking with closed source software
 *
 *   Copyright (c) 2022-2026 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h> // Emscripten library
#endif

#include <stdio.h>  // Required for: printf()
#include <stdlib.h> // Required for:
#include <string.h> // Required for:
#include <cmath>
#include <array>
#include <vector>
#include <list>
#include <cstdint>
#include <unordered_set>
#include <bit>
#include <cstring>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
enum class GameScreen
{
    SCREEN_TITLE,
    SCREEN_GAMEPLAY,
    SCREEN_ENDING
};

GameScreen state{GameScreen::SCREEN_GAMEPLAY};

// TODO: Define your custom data types here

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 720;
static const int screenHeight = 720;

static RenderTexture2D target = {0}; // Render texture to render our game
static int frameCounter = 0;

// TODO: Define global variables here, recommended to make them static

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void); // Update and Draw one frame

constexpr float HEX_RADIUS{20.0};
constexpr float HEX_ROT{30.0};
constexpr float HEX_AXIS_THETA{60.0};
constexpr float deg_to_rad(float f)
{
    return f * (3.1415 / 180.0);
}
constexpr float HEX_ROT_RAD{deg_to_rad(HEX_ROT)};
constexpr float HEX_AXIS_THETA_RAD{deg_to_rad(HEX_AXIS_THETA)};

Vector2 hex_to_cartesian_space_v(Vector2 v)
{
    float dx_of_hx = 2 * HEX_RADIUS * std::cos(HEX_ROT_RAD);
    float dx_of_hy = dx_of_hx * std::cos(HEX_AXIS_THETA_RAD);
    float dy_of_hy = dx_of_hx * std::sin(HEX_AXIS_THETA_RAD);
    return Vector2{
        .x = (dx_of_hx * v.x) + (dx_of_hy * v.y),
        .y = dy_of_hy * v.y};
}

constexpr Vector2 hex_neighbors[] = {
    {1, 0},
    {-1, 0},
    {0, 1},
    {0, -1},
    {-1, 1},
    {1, -1}};

Vector2 snake_velocity{hex_neighbors[0]};

std::list<Vector2> snake{Vector2Zero()};

uint64_t score{0};

/// @brief Returns the distance (in pixels) between two hex coordiantes
/// @param v1
/// @param v2
/// @return
float hex_distance(Vector2 v1, Vector2 v2)
{
    auto cv1 = hex_to_cartesian_space_v(v1);
    auto cv2 = hex_to_cartesian_space_v(v2);
    return Vector2Length(Vector2Subtract(cv1, cv2));
}

void render_hex(int x, int y, Color color)
{
    Vector2 pos;
    pos.x = x;
    pos.y = y;
    auto cart_space = hex_to_cartesian_space_v(pos);
    DrawPoly(cart_space, 6, HEX_RADIUS, HEX_ROT, color);
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE); // Disable raylib trace log messages
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib gamejam template");

    // TODO: Load resources / Initialize variables at this point

    // Render texture to draw, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60); // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);

    // TODO: Unload all loaded resources at this point

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------------
// Update and draw frame

void UpdateDrawFrameGameplay(void)
{
    // Update
    //----------------------------------------------------------------------------------
    //

    int control_keys[] = {KEY_D, KEY_A, KEY_C, KEY_Q, KEY_Z, KEY_E};
    Vector2 deltas[] = {{1, 0},
                        {-1, 0},
                        {0, 1},
                        {0, -1},
                        {-1, 1},
                        {1, -1}};

    for (size_t i = 0; i < std::size(control_keys); i++)
    {
        if (IsKeyPressed(control_keys[i]))
        {
            snake_velocity = deltas[i];
        }
    }

    if (frameCounter % 20 == 0)
    {
        auto new_pose = Vector2Add(*snake.begin(), snake_velocity);
        snake.push_front(new_pose);
        auto it_end = snake.end();
        if (!IsKeyDown(KEY_SPACE))
        {
            snake.pop_back();
            score++;
        }
    }

    frameCounter++;
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    // Render game screen to a texture,
    // it could be useful for scaling or further shader postprocessing
    BeginTextureMode(target);

    ClearBackground(GRAY);

    Vector2 center;
    center.x = screenWidth / 2;
    center.y = screenHeight / 2;

    int n = 10;
    for (int dhx = -n; dhx < n; dhx++)
    {
        for (int dhy = -n; dhy < n; dhy++)
        {
            Vector2 pos;
            pos.x = dhx;
            pos.y = dhy;

            auto cart_space1 = Vector2Add(hex_to_cartesian_space_v(pos), center);

            if (hex_distance(pos, {0, 0}) < (HEX_RADIUS * 10) + 10)
            {
                DrawPoly(cart_space1, 6, HEX_RADIUS - 1, HEX_ROT, BLACK);
                DrawPoly(cart_space1, 6, HEX_RADIUS - 2, HEX_ROT, WHITE);
            }
        }
    }
    Vector3 start_color{GREEN.r, GREEN.g, GREEN.b};
    Vector3 end_color{RED.r, RED.g, RED.b};
    auto delta = Vector3Scale(Vector3Subtract(end_color, start_color), 1.0 / snake.size());
    size_t idx{0};
    for (auto pose_ : snake)
    {

        auto pose = Vector2Add(hex_to_cartesian_space_v(pose_), center);

        DrawPoly(pose, 6, HEX_RADIUS - 1, HEX_ROT, BLACK);

        auto color = Vector3Add(start_color, Vector3Scale(delta, idx));

        DrawPoly(pose, 6, HEX_RADIUS - 2, HEX_ROT, {static_cast<unsigned char>(color.x), static_cast<unsigned char>(color.y), static_cast<unsigned char>(color.z), 255});
        idx++;
    }

    EndTextureMode();

    // Render to screen (main framebuffer)
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw render texture to screen, scaled if required
    DrawTexturePro(target.texture, (Rectangle){0, 0, (float)target.texture.width, -(float)target.texture.height},
                   (Rectangle){0, 0, (float)target.texture.width, (float)target.texture.height}, (Vector2){0, 0}, 0.0f, WHITE);

    EndDrawing();
    //----------------------------------------------------------------------------------

    // Test boarder collision
    if (frameCounter % 20 == 0 && hex_distance(*snake.begin(), {0, 0}) >= (HEX_RADIUS * 10) + 10)
    {
        state = GameScreen::SCREEN_ENDING;
    }
}

void UpdateDrawFrameEnding(void)
{
    BeginTextureMode(target);
    ClearBackground(RAYWHITE);

    // TODO: Draw your game screen here

    DrawRectangle(70, 90, 200, 200, BLACK);
    DrawRectangle(70 + 16, 90 + 16, 200 - 32, 200 - 32, RAYWHITE);
    DrawText("raylib", 70 + 200 - MeasureText("raylib", 40) - 32, 90 + 200 - 40 - 24, 40, BLACK);

    DrawText("6.x", 290, 90 - 26, 280, BLACK);
    DrawText("GAMEJAM", 70, 90 + 210, 120, MAROON);

    if ((frameCounter / 20) % 2)
        DrawText("are you ready?", 160, 500, 50, BLACK);

    DrawRectangleLinesEx((Rectangle){0, 0, screenWidth, screenHeight}, 16, BLACK);

    EndTextureMode();

    // Render to screen (main framebuffer)
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw render texture to screen, scaled if required
    DrawTexturePro(target.texture, (Rectangle){0, 0, (float)target.texture.width, -(float)target.texture.height},
                   (Rectangle){0, 0, (float)target.texture.width, (float)target.texture.height}, (Vector2){0, 0}, 0.0f, WHITE);

    // TODO: Draw everything that requires to be drawn at this point, maybe UI?

    EndDrawing();
}

void UpdateDrawFrame(void)
{
    switch (state)
    {
    case GameScreen::SCREEN_TITLE:
        break;
    case GameScreen::SCREEN_GAMEPLAY:
        UpdateDrawFrameGameplay();
        break;
    case GameScreen::SCREEN_ENDING:
        UpdateDrawFrameEnding();
        break;
    }
}