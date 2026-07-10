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
typedef enum
{
    SCREEN_LOGO = 0,
    SCREEN_TITLE,
    SCREEN_GAMEPLAY,
    SCREEN_ENDING
} GameScreen;

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

std::list<Vector2> snake{Vector2Zero()};

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
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update variables / Implement example logic at this point

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
            auto new_pose = Vector2Add(*snake.begin(), deltas[i]);
            snake.push_front(new_pose);
            if (!IsKeyDown(KEY_SPACE))
            {
                snake.pop_back();
            }
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

    // for (auto hex : hex_neighbors)
    // {
    //     auto cart_space1 = Vector2Add(hex_to_cartesian_space_v(hex), center);
    //     DrawPoly(cart_space1, 6, HEX_RADIUS - 1, HEX_ROT, BLACK);
    //     DrawPoly(cart_space1, 6, HEX_RADIUS - 2, HEX_ROT, WHITE);
    // }

    int n = 10;
    for (int dhx = -n; dhx < n; dhx++)
    {
        for (int dhy = -n; dhy < n; dhy++)
        {
            Vector2 pos;
            pos.x = dhx;
            pos.y = dhy;

            auto cart_space1 = Vector2Add(hex_to_cartesian_space_v(pos), center);

            if (hex_distance(pos, {0, 0}) < HEX_RADIUS * 10)
            {
                DrawPoly(cart_space1, 6, HEX_RADIUS - 1, HEX_ROT, BLACK);
                DrawPoly(cart_space1, 6, HEX_RADIUS - 2, HEX_ROT, WHITE);
            }
        }
    }

    for (auto pose_ : snake)
    {

        auto pose = Vector2Add(hex_to_cartesian_space_v(pose_), center);

        DrawPoly(pose, 6, HEX_RADIUS - 1, HEX_ROT, BLACK);
        DrawPoly(pose, 6, HEX_RADIUS - 2, HEX_ROT, GREEN);
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
}
