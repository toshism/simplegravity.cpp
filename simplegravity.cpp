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
Vector2 sim2screen(float cx, float cy, float factor) {
    return Vector2{ (cx/factor), (cy/factor) };
}
Vector2 screen2sim(float cx, float cy, float factor) {
    return Vector2{ (cx * factor), (cy * factor) };
}
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
    Vector2 giveScreenCoords(float factor) {
        return sim2screen(cx, cy, factor);
    }
};

class gravityPlain {
public:
    vector<gravitySource> Objects;
    void addObject(int radius, float cx, float cy, float xv, float yv, float mass) {
        Objects.emplace_back(radius, cx, cy, xv, yv, mass);
    }
    float gravitationalConstant = 1000.0f;
    int margins;
    int trailWidth=500;
    float factor = 5.0;
    float maxFactor = 5.0;
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
            if (Objects[i].cx + Objects[i].radius < 0 - margins ||
                Objects[i].cx - Objects[i].radius > (ScreenWidth + margins)*maxFactor ||
                Objects[i].cy + Objects[i].radius < 0 - margins||
                Objects[i].cy - Objects[i].radius > (ScreenHeight + margins)*maxFactor) {
                Objects.erase(Objects.begin() + i);
            }
        }
    }
    void draw() {
        for (int i = 0; i < Objects.size(); i++) {
            int brightness = clamp((int)(Objects[i].mass / 10000.0f * 255.0f),30, 255);
            Color baseColor = { brightness, brightness, brightness, 200};
            Vector2 displayCoords = Objects[i].giveScreenCoords(factor);
            DrawCircleV(displayCoords, Objects[i].radius/factor, baseColor);
            if (Objects[i].trail.size() >= 2) {
                for (int j = 0; j < Objects[i].trail.size() - 1; j++) {
                    Vector2 displayLine1 = sim2screen(Objects[i].trail[j].x, Objects[i].trail[j].y, factor);
                    Vector2 displayLine2 = sim2screen(Objects[i].trail[j+1].x, Objects[i].trail[j+1].y, factor);
                    DrawLineV(displayLine1,displayLine2, WHITE);
                    if (Objects[i].trail.size() > trailWidth) {
                        Objects[i].trail.erase(Objects[i].trail.begin());
                    }
                }
            }
        }
    }
};
int main() {
    InitWindow(1920, 1080, "Gravity sim");
    SetTargetFPS(60);
    gravityPlain world;
    world.margins = 200;
    world.trailWidth = 500;
    world.factor = 5.0;
    world.addObject(60, 1000, 1000, 100, 10, 1000);
    float clickMass = 50.0f;
    float clickRadius = 20;
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        world.update(deltaTime);
        Vector2 mousePos = screen2sim((float)GetMouseX(),(float)GetMouseY(), world.factor);
        Rectangle massSliderArea = { 60, 50, 400, 10 };
        Rectangle radiusSliderArea = { 60, 80, 200, 10 };
        Rectangle zoomSliderArea = { 60, 110, 200, 10 };
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(Vector2{ (float)GetMouseX(),(float)GetMouseY()}, massSliderArea) && !CheckCollisionPointRec(Vector2{ (float)GetMouseX(),(float)GetMouseY() }, radiusSliderArea) && !CheckCollisionPointRec(Vector2{ (float)GetMouseX(),(float)GetMouseY() }, zoomSliderArea)) {
            world.addObject(clickRadius, mousePos.x, mousePos.y, 0, 0, clickMass);
        }
        BeginDrawing();
        ClearBackground(BLACK);
        GuiSlider(massSliderArea, "50 Mass", "10000 Mass",&clickMass, 50.0, 10000.0);
        GuiSlider(radiusSliderArea, "10 Radius", "250 Radius", &clickRadius, 10.0, 250.0);
        GuiSlider(zoomSliderArea, "5x zoom", "1x zoom", &world.factor, 1.0, 5.0);
        world.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
