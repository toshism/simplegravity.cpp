#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "raylib.h"
#include <vector>
#include <cmath>
#include <raymath.h>  
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <algorithm>

using namespace std;
const int ScreenWidth = 1920;
const int ScreenHeight = 1080;
class gravitySource {
public:
    int radius;
    float cx;
    float cy;
    float xv;
    float yv;
    float mass;
    vector<Vector2> trail;
    gravitySource(int radius, float cx, float cy, float xv, float yv, float mass) :
        radius(radius),
        cx(cx), cy(cy),
        xv(xv), yv(yv),
        mass(mass) {
    }
};

class gravityPlain {
public:
    vector<gravitySource> Objects;
    void addObject(int radius, float cx, float cy, float xv, float yv, float mass) {
        Objects.emplace_back(radius, cx, cy, xv, yv, mass);
    }
    float gravitationalConstant = 1000.0f;
    float distanceBetweenCenters(float cx1, float cy1, float cx2, float cy2) {
        float dx = cx2 - cx1;
        float dy = cy2 - cy1;
        return sqrt(dx * dx + dy * dy);
    }
    Vector2 directionBetweenCenters(gravitySource src1, gravitySource src2) {
        return { src2.cx - src1.cx,src2.cy - src1.cy };
    }
    float calculateGravitationalPull(gravitySource src1, gravitySource src2) {
        float distance = distanceBetweenCenters(src1.cx, src1.cy, src2.cx, src2.cy);
        if (distance < 1.0f) {
            return 0;
        }
        return (gravitationalConstant * src1.mass * src2.mass) / (distance * distance);
    }
    Vector2 calculateAcceleration(gravitySource src1, gravitySource src2) {
        Vector2 direction = directionBetweenCenters(src1, src2);
        float distance = Vector2Length(direction);
        if (src1.mass <= 0) {
            return { 0, 0 };
        }
        direction = Vector2Normalize(direction);
        float force = calculateGravitationalPull(src1, src2);
        float acceleration = force / src1.mass;
        return Vector2Scale(direction, acceleration);
    }

    void update(float deltatime) {
        vector<Vector2> accelerations(Objects.size());
        for (int i = 0; i < Objects.size(); i++) {
            Vector2 totalAcceleration = { 0, 0 };
            for (int j = 0; j < Objects.size(); j++) {
                if (i == j) {
                    continue;
                }
                Vector2 acceleration = calculateAcceleration(Objects[i], Objects[j]);
                totalAcceleration = Vector2Add(totalAcceleration, acceleration);
            }
            accelerations[i] = totalAcceleration;
        }
        for (int i = 0; i < Objects.size(); i++) {
            Objects[i].trail.push_back({ Objects[i].cx, Objects[i].cy });
            Objects[i].xv += accelerations[i].x * deltatime;
            Objects[i].yv += accelerations[i].y * deltatime;
            Objects[i].cx += Objects[i].xv * deltatime;
            Objects[i].cy += Objects[i].yv * deltatime;
        }
        for (int i = (int)Objects.size() - 1; i >= 0; i--) {
            if (Objects[i].cx + Objects[i].radius < 0 ||
                Objects[i].cx - Objects[i].radius > ScreenWidth ||
                Objects[i].cy + Objects[i].radius < 0 ||
                Objects[i].cy - Objects[i].radius > ScreenHeight) {
                Objects.erase(Objects.begin() + i);
            }
        }
    }
    void draw() {
        for (int i = 0; i < Objects.size(); i++) {
            int brightness = clamp((int)(Objects[i].mass / 5000.0f * 255.0f),30, 255);
            Color baseColor = { brightness, brightness, brightness, 255};
            DrawCircle((int)Objects[i].cx, (int)Objects[i].cy, Objects[i].radius, baseColor);
            if (Objects[i].trail.size() >= 2) {
                for (int j = 0; j < Objects[i].trail.size() - 1; j++) {
                    DrawLineV(Objects[i].trail[j], Objects[i].trail[j + 1], WHITE);
                    if (Objects[i].trail.size() > 500) {
                        Objects[i].trail.erase(Objects[i].trail.begin());
                    }
                }
            }
        }
    }
};
int main() {
    InitWindow(ScreenWidth, ScreenHeight, "Gravity sim");
    SetTargetFPS(60);
    gravityPlain world;
    // Play around, this is just a simple somewhat stable configuration
    world.addObject(30, 900, 540, 0, 0, 1300);
    world.addObject(12, 1100, 540, 0, 75, 100);
    float volume = 50.0f;
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        world.update(deltaTime);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionRecs(Rectangle(GetMouseX(),GetMouseY(), 1, 1), { 60, 60, 200, 20 })) {
            world.addObject(20, GetMouseX(), GetMouseY(), 0, 0, volume);
        }
        BeginDrawing();
        ClearBackground(BLACK);
        GuiSlider(Rectangle{ 60, 60, 200, 20 }, "100 Mass", "5000 Mass",&volume, 100.0, 5000.0);
        world.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
