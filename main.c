// Cory Pearl
// 05/02/26

// Final fixes
// - Lava not cooling into stone anymore
// - Should lava beat ice, probobly
// - impliment jimmy, bounces around amlisly, can spawn in as many as you want, dies if hits lava
// Compile wiht new changes and add jimmy

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#define totalTypes 8
#define drawSize 2
#define fps 100

typedef struct {
    char draw;
    int x;
    int y;
    char type;
    Color color;
    int life;
    int heat;
} Pixel;

int randInt(int n1, int n2) {
    return (rand() % (n2 - n1 + 1)) + n1;
}

// Write a single logical pixel's color into the screenWidth-wide pixelColors buffer
void setLogicalPixel(Color *pixelColors, int gridX, int gridY, int screenWidth, Color color) {
    int startX = gridX * drawSize;
    int startY = gridY * drawSize;
    for (int dy = 0; dy < drawSize; dy++) {
        for (int dx = 0; dx < drawSize; dx++) {
            pixelColors[(startY + dy) * screenWidth + (startX + dx)] = color;
        }
    }
}

void clearBuffers(Color *pixelColors, Pixel *pixels, int screenWidth, int screenHeight, int gridWidth, int gridHeight) {
    for (int i = 0; i < screenWidth * screenHeight; i++) {
        pixelColors[i] = (Color){0, 0, 0, 255};
    }

    for (int i = 0; i < gridWidth * gridHeight; i++) {
        pixels[i].draw = 'n';
        pixels[i].x = i % gridWidth;
        pixels[i].y = i / gridWidth;
        pixels[i].type = ' ';
        pixels[i].life = -1;
        pixels[i].heat = -1;
        pixels[i].color = (Color){0, 0, 0, 255};
    }
}

// int typeNear(Pixel *pixels, int gridWidth, int gridHeight, int x, int y, int range, char type) {
//     int step = 4;

//     for (int yy = y - range; yy <= y + range; yy += step) {
//         if (yy < 0 || yy >= gridHeight) continue;
//         for (int xx = x - range; xx <= x + range; xx += step) {
//             if (xx < 0 || xx >= gridWidth) continue;
//             int dx = xx - x;
//             int dy = yy - y;
//             if (dx * dx + dy * dy > range * range) continue;
//             if (pixels[yy * gridWidth + xx].type == type) return 1;
//         }
//     }

//     for (int d = 1; d <= range; d++) {
//         if (y - d >= 0 && pixels[(y - d) * gridWidth + x].type == type) return 1;
//         if (y + d < gridHeight && pixels[(y + d) * gridWidth + x].type == type) return 1;
//         if (x - d >= 0 && pixels[y * gridWidth + x - d].type == type) return 1;
//         if (x + d < gridWidth && pixels[y * gridWidth + x + d].type == type) return 1;
//     }

//     return 0;
// }

void explodeNapalm(Color *pixelColors, Pixel *pixels, int gridWidth, int gridHeight, int screenWidth, int x, int y) {
    int total = gridWidth * gridHeight;
    int radius = 16;
    int *stack = malloc(total * sizeof(int));
    int *cluster = malloc(total * sizeof(int));
    char *seen = calloc(total, sizeof(char));
    char *thrown = calloc(total, sizeof(char));
    int stackCount = 0;
    int clusterCount = 0;

    if (!stack || !cluster || !seen || !thrown) {
        free(stack);
        free(cluster);
        free(seen);
        free(thrown);
        return;
    }

    stack[stackCount++] = y * gridWidth + x;
    seen[y * gridWidth + x] = 1;

    while (stackCount > 0) {
        int ci = stack[--stackCount];
        int cx = ci % gridWidth;
        int cy = ci / gridWidth;
        int neighbors[] = {
            ci - gridWidth,
            ci + gridWidth,
            ci - 1,
            ci + 1
        };
        int valid[] = {
            cy > 0,
            cy < gridHeight - 1,
            cx > 0,
            cx < gridWidth - 1
        };

        cluster[clusterCount++] = ci;

        for (int n = 0; n < 4; n++) {
            if (!valid[n]) continue;
            int ni = neighbors[n];
            if (!seen[ni] && pixels[ni].type == 'p') {
                seen[ni] = 1;
                stack[stackCount++] = ni;
            }
        }
    }

    for (int c = 0; c < clusterCount; c++) {
        int centerX = cluster[c] % gridWidth;
        int centerY = cluster[c] / gridWidth;

        for (int yy = centerY - radius; yy <= centerY + radius; yy++) {
            if (yy < 0 || yy >= gridHeight) continue;
            for (int xx = centerX - radius; xx <= centerX + radius; xx++) {
                if (xx < 0 || xx >= gridWidth) continue;
                int dx = xx - centerX;
                int dy = yy - centerY;
                if (dx * dx + dy * dy > radius * radius) continue;

                Pixel *p = &pixels[yy * gridWidth + xx];
                if (thrown[yy * gridWidth + xx]) continue;
                if (p->type == 'o') continue;

                p->x = xx;
                p->y = yy;

                if (p->type == 'p' && !seen[yy * gridWidth + xx]) {
                    p->draw = 'y';
                    p->life = randInt(1, 3);
                    p->color = (Color){255, randInt(120, 180), 0, 255};
                } else if (randInt(1, 80) == 1) {
                    p->draw = 'y';
                    p->type = 'l';
                    p->life = -1;
                    p->heat = 100;
                    p->color = (Color){randInt(220, 255), randInt(40, 90), 0, 255};
                } else {
                    if (p->draw == 'y' && p->type != ' ') {
                        int pushX = dx;
                        int pushY = dy;
                        if (pushX == 0 && pushY == 0) pushY = -1;
                        if (pushX > 0) pushX = 1;
                        if (pushX < 0) pushX = -1;
                        if (pushY > 0) pushY = 1;
                        if (pushY < 0) pushY = -1;

                        int distance = randInt(radius / 2, radius + 8);
                        int targetX = xx + pushX * distance;
                        int targetY = yy + pushY * distance;
                        int blocked = 0;

                        for (int step = 1; step <= distance; step++) {
                            int checkX = xx + pushX * step;
                            int checkY = yy + pushY * step;
                            if (checkX < 0 || checkX >= gridWidth || checkY < 0 || checkY >= gridHeight) {
                                blocked = 1;
                                break;
                            }
                            if (pixels[checkY * gridWidth + checkX].type == 'o') {
                                blocked = 1;
                                break;
                            }
                        }

                        if (!blocked && targetX >= 0 && targetX < gridWidth && targetY >= 0 && targetY < gridHeight) {
                            Pixel *target = &pixels[targetY * gridWidth + targetX];
                            if (target->draw == 'n') {
                                *target = *p;
                                target->x = targetX;
                                target->y = targetY;
                                thrown[targetY * gridWidth + targetX] = 1;
                                setLogicalPixel(pixelColors, targetX, targetY, screenWidth, target->color);
                            }
                        }
                    }

                    p->draw = 'n';
                    p->type = ' ';
                    p->life = -1;
                    p->heat = -1;
                    p->color = (Color){0, 0, 0, 255};
                }

                setLogicalPixel(pixelColors, xx, yy, screenWidth, p->color);
            }
        }
    }

    free(stack);
    free(cluster);
    free(seen);
    free(thrown);
}

int main() {
    const int screenWidth = 1200;
    const int screenHeight = 800;
    // const int screenWidth = 2500;
    // const int screenHeight = 1300;
    const char types[] = {'s', 'w', 'S', 'l', 't', 'i', 'o', 'p'}; // s = sand, w = water, S = stone, l = lava, t = steam, i = ice, o = obsidian, p = napalm
    char listOrder[totalTypes];
    int waterMode = 1; // 1 = gap-aware (stop at cliffs) 2 = flat (scan to furthest)
    int drawAmount = 1;
    int selectedType = 's';
    int pixelCount = 0;
    int dropdownOpen = 0;
    int dropDownTimerDelay = 0;
    int reacted = 0;
    int frameCount = 0;
    int mode = 1; // 1 = draw, 2 = hammer
    Vector2 prevMouse = {-1, -1};

    // Logical grid dimensions
    const int gridWidth = screenWidth  / drawSize; // 300
    const int gridHeight = screenHeight / drawSize; // 200
    const int reactionRangeWater = 20 / drawSize;
    const int reactionRangeLava = 20 / drawSize;
    int lavaCount = 0;
    int waterCount = 0;

    InitWindow(screenWidth, screenHeight, "Sand Simulator");
    srand(time(NULL));

    Image hammerImg = LoadImage("assets/hammer.png"); // Load into ram
    // ImageResize(&hammerImg, 20, 20);
    Texture2D hammerTex = LoadTextureFromImage(hammerImg); // Convert to GPU texture
    UnloadImage(hammerImg); // Free it so no wasted ram

    // GPU texture buffer — full screen resolution
    Color *pixelColors = malloc(screenWidth * screenHeight * sizeof(Color));
    if (!pixelColors) { CloseWindow(); return 1; }
    for (int i = 0; i < screenWidth * screenHeight; i++)
        pixelColors[i] = (Color){0, 0, 0, 255};

    // To save SO much compute
    // uint8_t *lavaRaw   = calloc(gridWidth * gridHeight, sizeof(uint8_t));
    // uint8_t *waterRaw  = calloc(gridWidth * gridHeight, sizeof(uint8_t));
    // uint8_t *hasLavaInRange  = calloc(gridWidth * gridHeight, sizeof(uint8_t));
    // uint8_t *hasWaterInRange = calloc(gridWidth * gridHeight, sizeof(uint8_t));

    // Logical pixel array — one entry per grid cell
    Pixel *pixels = malloc(gridWidth * gridHeight * sizeof(Pixel));
    if (!pixels) { free(pixelColors); CloseWindow(); return 1; }
    for (int i = 0; i < gridWidth * gridHeight; i++) {
        pixels[i].draw = 'n';
        pixels[i].x = i % gridWidth;
        pixels[i].y = i / gridWidth;
        pixels[i].type = ' ';
        pixels[i].life = -1;
        pixels[i].heat = -1;
        pixels[i].color = (Color){0, 0, 0, 255};
    }

    Image img = GenImageColor(screenWidth, screenHeight, BLACK);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);

    SetTargetFPS(fps);

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);

        int textSize2 = MeasureText(TextFormat("%i", drawAmount), 24);

        int right = screenWidth - 10;

        // right button (divide)
        int btnRightMin = right - 30;
        int btnRightMax = right;

        // text
        int textRight = btnRightMin - 10;
        int textLeft = textRight - textSize2;

        // left button (multiply)
        int btnLeftMax = textLeft - 10;
        int btnLeftMin = btnLeftMax - 30;

        if (mousePos.y >= 10 && mousePos.y <= 40) {

            if (mousePos.x <= screenWidth - 10 && mousePos.x >= screenWidth - 10 - 30) { 
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

            } else if (mousePos.x <= screenWidth - 30 - 10 - 10 - textSize2 - 10 && mousePos.x >= screenWidth - 30 - 10 - 10 - textSize2 - 10 - 30) { 
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

            }
        } 
        if (mousePos.x > 10 && mousePos.x < 40 && mousePos.y > 10 && mousePos.y < 40 ) {
                if (dropdownOpen) {
                    SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                } else {
                    SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                }
            } else if (dropdownOpen) {
            int overUI = mousePos.x >= 10 && mousePos.x <= 40 && mousePos.y >= 10 && mousePos.y <= 10 + (40 * totalTypes);

            if (overUI) {
                for (int i = 1; i <= totalTypes; i++) {
                    if (mousePos.y > i * 40 + 10 && mousePos.y < i * 40 + 10 + 30) {
                        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                    }
                }
            }

        } else if (mousePos.x > screenWidth - 90 && mousePos.x < screenWidth - 10 && mousePos.y > 50 && mousePos.y < 80) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        } else if (mousePos.x > screenWidth - 50 && mousePos.x < screenWidth - 10 && mousePos.y > 90 && mousePos.y < 130) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        }

        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            // Vector2 mouse = GetMousePosition();

            if (mousePos.x > 10 && mousePos.x < 40 && mousePos.y > 10 && mousePos.y < 40 ) {
                if (dropdownOpen) {
                        dropdownOpen = 0;
                } else {
                        dropdownOpen = 1;
                }
            }

            if (dropdownOpen) {
                int overUI = mousePos.x >= 10 && mousePos.x <= 40 && mousePos.y >= 10 && mousePos.y <= 10 + (40 * totalTypes);

                if (overUI) {
                    for (int i = 1; i <= totalTypes; i++) {
                        if (mousePos.y > i * 40 + 10 && mousePos.y < i * 40 + 10 + 30) {
                            selectedType = listOrder[i];
                            dropdownOpen = 0;
                            dropDownTimerDelay = 50;
                        }
                    }
                }

            }

            int textSize2 = MeasureText(TextFormat("%i", drawAmount), 24);

            int right = screenWidth - 10;

            // right button (divide)
            int btnRightMin = right - 30;
            int btnRightMax = right;

            // text
            int textRight = btnRightMin - 10;
            int textLeft = textRight - textSize2;

            // left button (multiply)
            int btnLeftMax = textLeft - 10;
            int btnLeftMin = btnLeftMax - 30;

            if (mousePos.y >= 10 && mousePos.y <= 40) {

                if (mousePos.x <= screenWidth - 10 && mousePos.x >= screenWidth - 10 - 30 && drawAmount > 1) { 
                    drawAmount /= 2; 
                } else if (mousePos.x <= screenWidth - 30 - 10 - 10 - textSize2 - 10 && mousePos.x >= screenWidth - 30 - 10 - 10 - textSize2 - 10 - 30 && drawAmount < 128) { 
                    drawAmount *= 2; 
                }
            }

            if (mousePos.x > screenWidth - 90 && mousePos.x < screenWidth - 10 && mousePos.y > 50 && mousePos.y < 80) {
                clearBuffers(pixelColors, pixels, screenWidth, screenHeight, gridWidth, gridHeight);
            }

            if (mousePos.x > screenWidth - 50 && mousePos.x < screenWidth - 10 && mousePos.y > 90 && mousePos.y < 130) {
                if (mode == 1) {
                    mode = 2;
                } else if (mode == 2) {
                    mode = 1;
                }
            }
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            // Vector2 mouse = GetMousePosition();

            int textSize1 = MeasureText(TextFormat("%i", drawAmount), 24);

            int overUI = (mousePos.x >= 10 && mousePos.x <= 40 && mousePos.y >= 10 && mousePos.y <= 40) || (mousePos.x >= screenWidth - 30 - 10 - 10 - textSize1 - 10 - 30 && mousePos.x <= screenWidth - 10 && mousePos.y >= 10 && mousePos.y <= 40) || (mousePos.x > screenWidth - 90 && mousePos.x < screenWidth - 10 && mousePos.y > 50 && mousePos.y < 80) ||(mousePos.x > screenWidth - 50 && mousePos.x < screenWidth - 10 && mousePos.y > 90 && mousePos.y < 130);

            if (dropdownOpen) {
                overUI = mousePos.x >= 10 && mousePos.x <= 40 && mousePos.y >= 10 && mousePos.y <= 10 + (40 * totalTypes);
            }

            if (!overUI && dropDownTimerDelay == 0) { // Draw a line beetween the points and fill in every pixel
                Vector2 start = (prevMouse.x < 0) ? mousePos : prevMouse;
                Vector2 end = mousePos;

                float dx = end.x - start.x;
                float dy = end.y - start.y;
                float dist = sqrtf(dx * dx + dy * dy);
                int steps = (int)dist / drawSize + 1;

                for (int step = 0; step <= steps; step++) {
                    float t = (float)step / steps;
                    if (steps == 0) {
                        t = 0;
                    }
                    int centerX = (int)(start.x + dx * t) / drawSize;
                    int centerY = (int)(start.y + dy * t) / drawSize;
                    int half = drawAmount / 2;

                    for (int gy = centerY - half; gy < centerY - half + drawAmount; gy++) { // Dont really understand these for loops tbh
                        for (int gx = centerX - half; gx < centerX - half + drawAmount; gx++) {
                            if (gx < 0 || gx >= gridWidth || gy < 0 || gy >= gridHeight) continue;
                            Pixel *p = &pixels[gy * gridWidth + gx];
                            if (mode == 1) {
                                if (p->draw == 'n') {
                                    p->draw = 'y';
                                    p->type = selectedType;
                                    p->life = -1;
                                    p->heat = -1;

                                    switch (selectedType) {
                                        case 's':
                                            p->color = (Color){randInt(194, 210), randInt(170, 185), randInt(100, 120), 255};
                                            break;
                                        case 'w':
                                            p->color = (Color){randInt(20, 40), randInt(100, 140), randInt(200, 230), 200};
                                            p->life = 1000;
                                            break;
                                        case 'S':
                                            p->color = (Color){randInt(100, 130), randInt(100, 130), randInt(100, 130), 255};
                                            break;
                                        case 'l':
                                            p->color = (Color){randInt(200, 255), randInt(20, 60), 0, 255};
                                            p->heat = 100;
                                            break;
                                        case 't':
                                            p->color = (Color){randInt(180, 200), randInt(180, 200), randInt(180, 200), 255};
                                            p->life = randInt(200, 300);
                                            break;
                                        case 'i':
                                            p->color = (Color){randInt(180, 210), randInt(220, 240), randInt(240, 255), 255};
                                            p->heat = 0; // cold, 0 = frozen, goes up when heated
                                            break;
                                        case 'o':
                                            p->color = (Color){randInt(70, 110), randInt(25, 45), randInt(130, 180), 255};
                                            break;
                                        case 'p':
                                            p->color = (Color){randInt(230, 255), randInt(110, 160), randInt(20, 50), 255};
                                            p->life = -1;
                                            break;
                                    }
                                }
                            } else if (mode == 2) {
                                p->draw = 'n';
                                p->type = ' ';
                                p->color = (Color){0, 0, 0, 255};
                            }

                            setLogicalPixel(pixelColors, gx, gy, screenWidth, p->color);
                        }
                    }
                }
                prevMouse = mousePos;
            }
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            prevMouse = (Vector2){-1, -1};
        }

        lavaCount = 0;
        waterCount = 0;

        // memset(lavaRaw,  0, gridWidth * gridHeight);
        // memset(waterRaw, 0, gridWidth * gridHeight);

        // steam only and lava check
        for (int i = 0; i < gridWidth * gridHeight; i++) {
            if (pixels[i].type == 'l') { lavaCount++; }
            else if (pixels[i].type == 'w') { waterCount++; }
            if (pixels[i].draw == 'y' && pixels[i].type == 't') {
                pixels[i].life--;

                if (pixels[i].life <= 0) {
                    pixels[i].draw = 'n';
                    pixels[i].type = ' ';
                    pixels[i].life = -1;
                    pixels[i].color = (Color){0, 0, 0, 255};
                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                } else {
                    Pixel temp = pixels[i];
                    int col = i % gridWidth;

                    pixels[i].color.a = (unsigned char)pixels[i].life;

                    if (pixels[i].y > 0 && pixels[i - gridWidth].type != 'S' && pixels[i - gridWidth].type != 'i' && pixels[i - gridWidth].type != 'o') {

                        if (pixels[i - gridWidth].draw == 'n' || pixels[i - gridWidth].type == 'w') {
                            pixels[i] = pixels[i - gridWidth];
                            pixels[i - gridWidth] = temp;
                            pixels[i - gridWidth].x = (i - gridWidth) % gridWidth;
                            pixels[i - gridWidth].y = (i - gridWidth) / gridWidth;
                            pixels[i].x = i % gridWidth;
                            pixels[i].y = i / gridWidth;

                            setLogicalPixel(pixelColors, pixels[i - gridWidth].x, pixels[i - gridWidth].y, screenWidth, pixels[i - gridWidth].color);
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, pixels[i].color);

                        } else if (col > 0 && col < gridWidth - 1 && pixels[i - gridWidth - 1].draw == 'n' && pixels[i - gridWidth + 1].draw == 'n') {
                            int target = (randInt(1, 2) == 1) ? i - gridWidth - 1 : i - gridWidth + 1;

                            pixels[i] = pixels[target];
                            pixels[target] = temp;
                            pixels[target].x = target % gridWidth;
                            pixels[target].y = target / gridWidth;
                            pixels[i].x = i % gridWidth;
                            pixels[i].y = i / gridWidth;

                            setLogicalPixel(pixelColors, pixels[target].x, pixels[target].y, screenWidth, pixels[target].color);
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                        } else if (col > 0 && pixels[i - gridWidth - 1].draw == 'n') {
                            pixels[i] = pixels[i - gridWidth - 1];
                            pixels[i - gridWidth - 1] = temp;
                            pixels[i - gridWidth - 1].x = (i - gridWidth - 1) % gridWidth;
                            pixels[i - gridWidth - 1].y = (i - gridWidth - 1) / gridWidth;
                            pixels[i].x = i % gridWidth;
                            pixels[i].y = i / gridWidth;

                            setLogicalPixel(pixelColors, pixels[i - gridWidth - 1].x, pixels[i - gridWidth - 1].y, screenWidth, pixels[i - gridWidth - 1].color);
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                        } else if (col < gridWidth - 1 && pixels[i - gridWidth + 1].draw == 'n') {
                            pixels[i] = pixels[i - gridWidth + 1];
                            pixels[i - gridWidth + 1] = temp;
                            pixels[i - gridWidth + 1].x = (i - gridWidth + 1) % gridWidth;
                            pixels[i - gridWidth + 1].y = (i - gridWidth + 1) / gridWidth;
                            pixels[i].x = i % gridWidth;
                            pixels[i].y = i / gridWidth;

                            setLogicalPixel(pixelColors, pixels[i - gridWidth + 1].x, pixels[i - gridWidth + 1].y, screenWidth, pixels[i - gridWidth + 1].color);
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                        } else {
                            if (col > 0 && col < gridWidth - 1 && pixels[i - 1].draw == 'n' && pixels[i + 1].draw == 'n') {
                                int target = (randInt(1, 2) == 1) ? i - 1 : i + 1;

                                pixels[i] = pixels[target];
                                pixels[target] = temp;
                                pixels[target].x = target % gridWidth;
                                pixels[target].y = target / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[target].x, pixels[target].y, screenWidth, pixels[target].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                            } else if (col < gridWidth - 1 && pixels[i + 1].draw == 'n') {
                                pixels[i] = pixels[i + 1];
                                pixels[i + 1] = temp;
                                pixels[i + 1].x = (i + 1) % gridWidth;
                                pixels[i + 1].y = (i + 1) / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[i + 1].x, pixels[i + 1].y, screenWidth, pixels[i + 1].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                            } else if (col > 0 && pixels[i - 1].draw == 'n') {
                                pixels[i] = pixels[i - 1];
                                pixels[i - 1] = temp;
                                pixels[i - 1].x = (i - 1) % gridWidth;
                                pixels[i - 1].y = (i - 1) / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[i - 1].x, pixels[i - 1].y, screenWidth, pixels[i - 1].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});
                            }
                        }

                    } else {
                        // at top or blocked — just redraw with updated alpha
                        setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, pixels[i].color);
                    }
                }
            }
        }

        float lavaDominance = 0.5f;
        if (waterCount + lavaCount > 0) {
            lavaDominance = (float)lavaCount / (float)(lavaCount + waterCount);
        }

        pixelCount = 0;
        for (int row = gridHeight - 1; row >= 0; row--) {
        for (int col2 = 0; col2 < gridWidth; col2++) {
            int flip = (frameCount % 2 == 0) ^ (rand() % 8 == 0);
            int actualCol = flip ? col2 : (gridWidth - 1 - col2);
            int i = row * gridWidth + actualCol;
            if (pixels[i].draw == 'y') {
                pixelCount++;
                Pixel temp = pixels[i];
                int col = i % gridWidth;

                switch (pixels[i].type) {
                    case 's': // Sand rules: If can go down, go down, if can go left or right, choose random, if can only go one way do that, else stay still
                        if (pixels[i].y < gridHeight - 1 && pixels[i + gridWidth].type != 'S' && pixels[i + gridWidth].type != 'i' && pixels[i + gridWidth].type != 'o') {
                            
                            if (pixels[i + gridWidth].draw == 'n') {
                                pixels[i] = pixels[i + gridWidth];
                                pixels[i + gridWidth] = temp;
                                pixels[i + gridWidth].x = (i + gridWidth) % gridWidth;
                                pixels[i + gridWidth].y = (i + gridWidth) / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[i + gridWidth].x, pixels[i + gridWidth].y, screenWidth, pixels[i + gridWidth].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                            } else if (col > 0 && col < gridWidth - 1 && pixels[i + gridWidth - 1].draw == 'n' && pixels[i + gridWidth + 1].draw == 'n') {
                                int choice = randInt(1, 2);
                                int target = target = i + gridWidth - 1;
                                if (choice == 2) {
                                    target = i + gridWidth + 1;
                                }

                                pixels[i] = pixels[target];
                                pixels[target] = temp;
                                pixels[target].x = target % gridWidth;
                                pixels[target].y = target / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[target].x, pixels[target].y, screenWidth, pixels[target].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                            } else if (col > 0 && pixels[i + gridWidth - 1].draw == 'n') {
                                pixels[i] = pixels[i + gridWidth - 1];
                                pixels[i + gridWidth - 1] = temp;
                                pixels[i + gridWidth - 1].x = (i + gridWidth - 1) % gridWidth;
                                pixels[i + gridWidth - 1].y = (i + gridWidth - 1) / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[i + gridWidth - 1].x, pixels[i + gridWidth - 1].y, screenWidth, pixels[i + gridWidth - 1].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                            } else if (col < gridWidth - 1 && pixels[i + gridWidth + 1].draw == 'n') {
                                pixels[i] = pixels[i + gridWidth + 1];
                                pixels[i + gridWidth + 1] = temp;
                                pixels[i + gridWidth + 1].x = (i + gridWidth + 1) % gridWidth;
                                pixels[i + gridWidth + 1].y = (i + gridWidth + 1) / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[i + gridWidth + 1].x, pixels[i + gridWidth + 1].y, screenWidth, pixels[i + gridWidth + 1].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});
                            }
                        }
                    break;
                    
                    case 'w': // Water rules: If can go down, go down, if can go left or right choose random, Slide sideways if can't fall at all, try left or right or random,option 2 - find the furthest free pxiel next to another water pixel and fill that to make like fluid
                        reacted = 0;
                        int waterNeighbors[] = {
                            i - gridWidth,
                            i + gridWidth,
                            i - 1,
                            i + 1
                        };
                        int waterValid[] = {
                            pixels[i].y > 0,
                            pixels[i].y < gridHeight - 1,
                            col > 0,
                            col < gridWidth - 1
                        };

                        if (pixels[i].life < 0) pixels[i].life = 1000;

                        // spread heat from hot neighbors directly
                        for (int n = 0; n < 4; n++) {
                            if (!waterValid[n]) continue;
                            int ni = waterNeighbors[n];

                            // direct lava contact: fast drain
                            if (pixels[ni].type == 'l') {
                                int drain = (int)(2 + lavaDominance * 6);
                                pixels[i].life -= drain;
                                reacted = 1;
                            }
                            // neighbor water is hotter (lower life): spread the damage
                            else if (pixels[ni].type == 'w' && pixels[ni].life >= 0 && pixels[ni].life < pixels[i].life - 30) {
                                int spread = (pixels[i].life - pixels[ni].life) / 3;
                                if (spread < 1) spread = 1;
                                pixels[i].life -= spread;
                                reacted = 1;
                            }
                            // hot stone contact
                            else if (pixels[ni].type == 'S' && pixels[ni].heat > 50) {
                                pixels[i].life -= (pixels[ni].heat / 30) + 1;
                                reacted = 1;
                            }
                        }

                        if (pixels[i].life <= 0) {
                            pixels[i].type = 't';
                            pixels[i].life = randInt(200, 400);
                            pixels[i].heat = -1;
                            pixels[i].color = (Color){randInt(180, 220), randInt(180, 220), randInt(180, 220), 255};
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, pixels[i].color);
                            break;
                        }

                        if (reacted) {
                            pixels[i].color = (Color){randInt(40, 70), randInt(130, 160), randInt(210, 240), 200};
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, pixels[i].color);
                            temp = pixels[i];
                        }

                        if (pixels[i].y < gridHeight - 1 && pixels[i + gridWidth].type != 'S' && pixels[i + gridWidth].type != 'i' && pixels[i + gridWidth].type != 'o') {

                            int fallingThroughSteam = pixels[i + gridWidth].type == 't';

                            if (pixels[i + gridWidth].draw == 'n' || fallingThroughSteam) {
                                pixels[i] = pixels[i + gridWidth];
                                pixels[i + gridWidth] = temp;
                                pixels[i + gridWidth].x = (i + gridWidth) % gridWidth;
                                pixels[i + gridWidth].y = (i + gridWidth) / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[i + gridWidth].x, pixels[i + gridWidth].y, screenWidth, pixels[i + gridWidth].color);
                                if (fallingThroughSteam) {
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, pixels[i].color);
                                } else {
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});
                                }

                            } else if (col > 0 && col < gridWidth - 1 && pixels[i + gridWidth - 1].draw == 'n' && pixels[i + gridWidth + 1].draw == 'n') {
                                int target = (randInt(1, 2) == 1) ? i + gridWidth - 1 : i + gridWidth + 1;

                                pixels[i] = pixels[target];
                                pixels[target] = temp;
                                pixels[target].x = target % gridWidth;
                                pixels[target].y = target / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[target].x, pixels[target].y, screenWidth, pixels[target].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                            } else if (col > 0 && pixels[i + gridWidth - 1].draw == 'n') {
                                pixels[i] = pixels[i + gridWidth - 1];
                                pixels[i + gridWidth - 1] = temp;
                                pixels[i + gridWidth - 1].x = (i + gridWidth - 1) % gridWidth;
                                pixels[i + gridWidth - 1].y = (i + gridWidth - 1) / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[i + gridWidth - 1].x, pixels[i + gridWidth - 1].y, screenWidth, pixels[i + gridWidth - 1].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                            } else if (col < gridWidth - 1 && pixels[i + gridWidth + 1].draw == 'n') {
                                pixels[i] = pixels[i + gridWidth + 1];
                                pixels[i + gridWidth + 1] = temp;
                                pixels[i + gridWidth + 1].x = (i + gridWidth + 1) % gridWidth;
                                pixels[i + gridWidth + 1].y = (i + gridWidth + 1) / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[i + gridWidth + 1].x, pixels[i + gridWidth + 1].y, screenWidth, pixels[i + gridWidth + 1].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                            } else if (waterMode == 1) {
                                // Mode 1: original one-cell sideways
                                if (col > 0 && col < gridWidth - 1 && pixels[i - 1].draw == 'n' && pixels[i + 1].draw == 'n') {
                                    int target = (randInt(1, 2) == 1) ? i - 1 : i + 1;

                                    pixels[i] = pixels[target];
                                    pixels[target] = temp;
                                    pixels[target].x = target % gridWidth;
                                    pixels[target].y = target / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[target].x, pixels[target].y, screenWidth, pixels[target].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                                } else if (col < gridWidth - 1 && pixels[i + 1].draw == 'n') {
                                    pixels[i] = pixels[i + 1];
                                    pixels[i + 1] = temp;
                                    pixels[i + 1].x = (i + 1) % gridWidth;
                                    pixels[i + 1].y = (i + 1) / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[i + 1].x, pixels[i + 1].y, screenWidth, pixels[i + 1].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                                } else if (col > 0 && pixels[i - 1].draw == 'n') {
                                    pixels[i] = pixels[i - 1];
                                    pixels[i - 1] = temp;
                                    pixels[i - 1].x = (i - 1) % gridWidth;
                                    pixels[i - 1].y = (i - 1) / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[i - 1].x, pixels[i - 1].y, screenWidth, pixels[i - 1].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});
                                }

                            } else {
                                // Mode 2: scan to furthest empty cell, stop at cliffs
                                int leftTarget = -1;
                                int rightTarget = -1;

                                for (int s = 1; col - s >= 0; s++) {
                                    if (pixels[i - s].draw == 'n') leftTarget = i - s;
                                    else break;
                                    int below = i - s + gridWidth;
                                    if (pixels[i - s].y < gridHeight - 1 && pixels[below].draw == 'n') break;
                                }
                                for (int s = 1; col + s < gridWidth; s++) {
                                    if (pixels[i + s].draw == 'n') rightTarget = i + s;
                                    else break;
                                    int below = i + s + gridWidth;
                                    if (pixels[i + s].y < gridHeight - 1 && pixels[below].draw == 'n') break;
                                }

                                int target = -1;
                                if (leftTarget != -1 && rightTarget != -1)
                                    target = (randInt(1, 2) == 1) ? leftTarget : rightTarget;
                                else if (leftTarget != -1)
                                    target = leftTarget;
                                else if (rightTarget != -1)
                                    target = rightTarget;

                                if (target != -1) {
                                    pixels[i] = pixels[target];
                                    pixels[target] = temp;
                                    pixels[target].x = target % gridWidth;
                                    pixels[target].y = target / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[target].x, pixels[target].y, screenWidth, pixels[target].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});
                                }
                            }

                        } else {
                            // Bottom row — slide sideways only
                            if (waterMode == 1) {
                                if (col > 0 && col < gridWidth - 1 && pixels[i - 1].draw == 'n' && pixels[i + 1].draw == 'n') {
                                    int target = (randInt(1, 2) == 1) ? i - 1 : i + 1;

                                    pixels[i] = pixels[target];
                                    pixels[target] = temp;
                                    pixels[target].x = target % gridWidth;
                                    pixels[target].y = target / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[target].x, pixels[target].y, screenWidth, pixels[target].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                                } else if (col < gridWidth - 1 && pixels[i + 1].draw == 'n') {
                                    pixels[i] = pixels[i + 1];
                                    pixels[i + 1] = temp;
                                    pixels[i + 1].x = (i + 1) % gridWidth;
                                    pixels[i + 1].y = (i + 1) / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[i + 1].x, pixels[i + 1].y, screenWidth, pixels[i + 1].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                                } else if (col > 0 && pixels[i - 1].draw == 'n') {
                                    pixels[i] = pixels[i - 1];
                                    pixels[i - 1] = temp;
                                    pixels[i - 1].x = (i - 1) % gridWidth;
                                    pixels[i - 1].y = (i - 1) / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[i - 1].x, pixels[i - 1].y, screenWidth, pixels[i - 1].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});
                                }

                            } else {
                                int leftTarget = -1;
                                int rightTarget = -1;

                                for (int s = 1; col - s >= 0; s++) {
                                    if (pixels[i - s].draw == 'n') leftTarget = i - s;
                                    else break;
                                }
                                for (int s = 1; col + s < gridWidth; s++) {
                                    if (pixels[i + s].draw == 'n') rightTarget = i + s;
                                    else break;
                                }

                                int target = -1;
                                if (leftTarget != -1 && rightTarget != -1)
                                    target = (randInt(1, 2) == 1) ? leftTarget : rightTarget;
                                else if (leftTarget != -1)
                                    target = leftTarget;
                                else if (rightTarget != -1)
                                    target = rightTarget;

                                if (target != -1) {
                                    pixels[i] = pixels[target];
                                    pixels[target] = temp;
                                    pixels[target].x = target % gridWidth;
                                    pixels[target].y = target / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[target].x, pixels[target].y, screenWidth, pixels[target].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});
                                }
                            }
                        }
                    break;

                    case 'S': // Stone rules: conduct heat from lava neighbors, melt to lava if fully heated, cool water if hot
                    {
                        if (pixels[i].heat <= 0) break; // cold stone does nothing

                        int stoneNeighbors[] = {
                            i - gridWidth,
                            i + gridWidth,
                            i - 1,
                            i + 1
                        };
                        int stoneValid[] = {
                            pixels[i].y > 0,
                            pixels[i].y < gridHeight - 1,
                            col > 0,
                            col < gridWidth - 1
                        };

                        for (int n = 0; n < 4; n++) {
                            if (!stoneValid[n]) continue;
                            int ni = stoneNeighbors[n];

                            if (pixels[ni].type == 'w') {
                                // hot stone heats water → evaporate it faster
                                pixels[ni].life -= randInt(15, 25);
                                pixels[i].heat -= randInt(8, 14);
                                if (pixels[ni].life <= 0) {
                                    pixels[ni].type = 't';
                                    pixels[ni].life = randInt(200, 400);
                                    pixels[ni].color = (Color){randInt(180,220), randInt(180,220), randInt(180,220), 255};
                                    setLogicalPixel(pixelColors, pixels[ni].x, pixels[ni].y, screenWidth, pixels[ni].color);
                                }

                            } else if (pixels[ni].type == 'S' && pixels[ni].heat >= 0 && pixels[ni].heat < pixels[i].heat) {
                                // conduct heat into adjacent cooler stone, faster transfer
                                int transfer = 1;
                                pixels[ni].heat += transfer /2;
                                pixels[i].heat -= transfer / 2;
                                if (pixels[ni].heat > 300) pixels[ni].heat = 300;

                            } else if (pixels[ni].type == 'l') {
                                int reheat = waterCount == 0 ? 15 : 1;
                                pixels[i].heat += reheat;
                                if (pixels[i].heat > 300) pixels[i].heat = 300;
                            }
                        }

                        // stone fully heated by lava → melts back into lava
                        if (pixels[i].heat >= 300) {
                            pixels[i].type = 'l';
                            pixels[i].heat = 100;
                            pixels[i].color = (Color){randInt(200, 255), randInt(20, 60), 0, 255};
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, pixels[i].color);
                            break;
                        }

                        int waterNearby = 0;
                        for (int n = 0; n < 4; n++) {
                            if (!stoneValid[n]) continue;
                            if (pixels[stoneNeighbors[n]].type == 'w') { waterNearby = 1; break; }
                        }
                        if (!waterNearby && pixels[i].heat > 150) {
                            pixels[i].heat += 2;
                            if (pixels[i].heat > 300) pixels[i].heat = 300;
                        }

                        // update stone color based on heat (grey to orange)
                        {
                            float t = pixels[i].heat / 300.0f;
                            if (t > 1) t = 1;
                            int r = (int)(110 + (230 - 110) * t);
                            int g = (int)(110 + (40  - 110) * t);
                            int b = (int)(110 + (0   - 110) * t);
                            pixels[i].color = (Color){r, g, b, 255};
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, pixels[i].color);
                        }

                        if (waterCount > 0) pixels[i].heat -= 1; // slowly cool down over time
                        if (pixels[i].heat <= 0) {
                            pixels[i].heat = -1;
                            pixels[i].color = (Color){randInt(100, 130), randInt(100, 130), randInt(100, 130), 255};
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, pixels[i].color);
                        }
                        break;
                    }
                    
                    case 'l': // Lava rules: react with water to cool and solidify, heat adjacent stone, flow like slow water
                        reacted = 0;
                        int touchedWater = 0;
                        int neighbors[] = {
                            i - gridWidth,
                            i + gridWidth,
                            i - 1,
                            i + 1
                        };
                        int neighborValid[] = {
                            pixels[i].y > 0,
                            pixels[i].y < gridHeight - 1,
                            col > 0,
                            col < gridWidth - 1
                        };

                        // spread cooling from cold neighbors directly
                        for (int n = 0; n < 4; n++) {
                            if (!neighborValid[n]) continue;
                            int ni = neighbors[n];

                            // direct water contact: strong cooling scaled by dominance
                            if (pixels[ni].type == 'w') {
                                float waterDominance = 1.0f - lavaDominance;
                                int drain = (int)(-1 + waterDominance * 8);
                                if (drain > 0) {  // only cool if water actually dominates
                                    pixels[i].heat -= drain;
                                    reacted = 1;
                                    touchedWater = 1;
                                }
                            }
                            // neighbor lava is cooler: pull heat from this one (chain cooling)
                            else if (pixels[ni].type == 'l' && pixels[ni].heat < pixels[i].heat - 5) {
                                int spread = (pixels[i].heat - pixels[ni].heat) / 4;
                                if (spread < 1) spread = 1;
                                pixels[i].heat  -= spread;
                                pixels[ni].heat += spread;
                            }
                            // cold stone drains heat inward
                            else if (pixels[ni].type == 'S' && pixels[ni].heat >= 0 && pixels[ni].heat < 100) {
                                if (lavaDominance < 0.4f) {
                                    // only cool lava through stone if that stone has water directly touching it
                                    int stoneHasWater = 0;
                                    int snx = ni % gridWidth;
                                    int sny = ni / gridWidth;
                                    int stoneNeigh[] = { ni - gridWidth, ni + gridWidth, ni - 1, ni + 1 };
                                    int stoneNeighValid[] = { sny > 0, sny < gridHeight - 1, snx > 0, snx < gridWidth - 1 };
                                    for (int sn = 0; sn < 4; sn++) {
                                        if (!stoneNeighValid[sn]) continue;
                                        if (pixels[stoneNeigh[sn]].type == 'w') { stoneHasWater = 1; break; }
                                    }
                                    if (stoneHasWater) {
                                        pixels[i].heat -= 1;
                                        touchedWater = 1;
                                    }
                                }
                            }
                        }

                        for (int n = 0; n < 4; n++) {
                            if (!neighborValid[n]) continue;

                            // lava melts adjacent ice directly into water
                            if (pixels[neighbors[n]].type == 'i') {
                                pixels[neighbors[n]].type = 'w';
                                pixels[neighbors[n]].life = 100;
                                pixels[neighbors[n]].heat = -1;
                                pixels[neighbors[n]].color = (Color){randInt(20, 40), randInt(100, 140), randInt(200, 230), 200};
                                setLogicalPixel(pixelColors, pixels[neighbors[n]].x, pixels[neighbors[n]].y, screenWidth, pixels[neighbors[n]].color);
                                pixels[i].heat -= randInt(1, 3); // lava cools slightly from melting ice
                                reacted = 1;
                                break;
                            }

                            // cooler lava pulls heat out through the group
                           if (touchedWater && pixels[neighbors[n]].type == 'l' && pixels[neighbors[n]].heat < pixels[i].heat) {
                                pixels[i].heat -= 1;
                            }

                            // fresh stone or obsidian keeps cooling lava inward
                            if (touchedWater && ((pixels[neighbors[n]].type == 'S' && pixels[neighbors[n]].heat >= 0 && pixels[neighbors[n]].heat <= 80) || pixels[neighbors[n]].type == 'o')) {
                                pixels[i].heat -= 1;
                                continue;
                            }

                            // lava heats adjacent stone directly
                            if (pixels[neighbors[n]].type == 'S') {
                                if (pixels[neighbors[n]].heat < 0) pixels[neighbors[n]].heat = 0;
                                pixels[neighbors[n]].heat += 1;
                                if (pixels[neighbors[n]].heat > 300) pixels[neighbors[n]].heat = 300;
                            }
                        }

                        if (pixels[i].heat <= 0) {
                            if (touchedWater && randInt(1, 10) == 1) {
                                pixels[i].type = 'o';
                                pixels[i].heat = -1;
                                pixels[i].color = (Color){randInt(70, 110), randInt(25, 45), randInt(130, 180), 255};
                            } else {
                                pixels[i].type = 'S';
                                pixels[i].heat = randInt(20, 40); // starts warm so it can conduct through the barrier
                                pixels[i].color = (Color){randInt(100, 130), randInt(100, 130), randInt(100, 130), 255};
                            }
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, pixels[i].color);
                            break;
                        }

                        if (reacted) {
                            float t = pixels[i].heat / 100.0f;
                            if (t < 0) t = 0;
                            if (t > 1) t = 1;
                            int r = (int)(110 * (1 - t) + 230 * t);
                            int g = (int)(110 * (1 - t) + 40 * t);
                            int b = (int)(110 * (1 - t) + 0 * t);
                            pixels[i].color = (Color){r, g, b, 255};
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, pixels[i].color);
                            temp = pixels[i];
                        }

                        {
                            // lava flows like water, always mode 1 (one cell at a time, slower than water)
                            // fixed: check below (i + gridWidth) not above (i - gridWidth)
                            if (pixels[i].y < gridHeight - 1 && pixels[i + gridWidth].type != 'S' && pixels[i + gridWidth].type != 'i' && pixels[i + gridWidth].type != 'o') {

                                if (pixels[i + gridWidth].draw == 'n') {
                                    pixels[i] = pixels[i + gridWidth];
                                    pixels[i + gridWidth] = temp;
                                    pixels[i + gridWidth].x = (i + gridWidth) % gridWidth;
                                    pixels[i + gridWidth].y = (i + gridWidth) / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[i + gridWidth].x, pixels[i + gridWidth].y, screenWidth, pixels[i + gridWidth].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                                } else if (col > 0 && col < gridWidth - 1 && pixels[i + gridWidth - 1].draw == 'n' && pixels[i + gridWidth + 1].draw == 'n') {
                                    int target = (randInt(1, 2) == 1) ? i + gridWidth - 1 : i + gridWidth + 1;

                                    pixels[i] = pixels[target];
                                    pixels[target] = temp;
                                    pixels[target].x = target % gridWidth;
                                    pixels[target].y = target / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[target].x, pixels[target].y, screenWidth, pixels[target].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                                } else if (col > 0 && pixels[i + gridWidth - 1].draw == 'n') {
                                    pixels[i] = pixels[i + gridWidth - 1];
                                    pixels[i + gridWidth - 1] = temp;
                                    pixels[i + gridWidth - 1].x = (i + gridWidth - 1) % gridWidth;
                                    pixels[i + gridWidth - 1].y = (i + gridWidth - 1) / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[i + gridWidth - 1].x, pixels[i + gridWidth - 1].y, screenWidth, pixels[i + gridWidth - 1].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                                } else if (col < gridWidth - 1 && pixels[i + gridWidth + 1].draw == 'n') {
                                    pixels[i] = pixels[i + gridWidth + 1];
                                    pixels[i + gridWidth + 1] = temp;
                                    pixels[i + gridWidth + 1].x = (i + gridWidth + 1) % gridWidth;
                                    pixels[i + gridWidth + 1].y = (i + gridWidth + 1) / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[i + gridWidth + 1].x, pixels[i + gridWidth + 1].y, screenWidth, pixels[i + gridWidth + 1].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                                } else if (col > 0 && col < gridWidth - 1 && pixels[i - 1].draw == 'n' && pixels[i + 1].draw == 'n') {
                                    int target = (randInt(1, 2) == 1) ? i - 1 : i + 1;

                                    pixels[i] = pixels[target];
                                    pixels[target] = temp;
                                    pixels[target].x = target % gridWidth;
                                    pixels[target].y = target / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[target].x, pixels[target].y, screenWidth, pixels[target].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                                } else if (col < gridWidth - 1 && pixels[i + 1].draw == 'n') {
                                    pixels[i] = pixels[i + 1];
                                    pixels[i + 1] = temp;
                                    pixels[i + 1].x = (i + 1) % gridWidth;
                                    pixels[i + 1].y = (i + 1) / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[i + 1].x, pixels[i + 1].y, screenWidth, pixels[i + 1].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                                } else if (col > 0 && pixels[i - 1].draw == 'n') {
                                    pixels[i] = pixels[i - 1];
                                    pixels[i - 1] = temp;
                                    pixels[i - 1].x = (i - 1) % gridWidth;
                                    pixels[i - 1].y = (i - 1) / gridWidth;
                                    pixels[i].x = i % gridWidth;
                                    pixels[i].y = i / gridWidth;

                                    setLogicalPixel(pixelColors, pixels[i - 1].x, pixels[i - 1].y, screenWidth, pixels[i - 1].color);
                                    setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});
                                }
                            }
                        }
                        break;

                    case 'p': // Napalm rules: falls like sand, explodes when lava lights it, chains into nearby napalm
                    {
                        int napalmNeighbors[] = {
                            i - gridWidth,
                            i + gridWidth,
                            i - 1,
                            i + 1
                        };
                        int napalmValid[] = {
                            pixels[i].y > 0,
                            pixels[i].y < gridHeight - 1,
                            col > 0,
                            col < gridWidth - 1
                        };

                        for (int n = 0; n < 4; n++) {
                            if (!napalmValid[n]) continue;
                            if (pixels[napalmNeighbors[n]].type == 'l') {
                                pixels[i].life = randInt(1, 3);
                            }
                        }

                        if (pixels[i].life > 0) {
                            pixels[i].life--;
                            pixels[i].color = (Color){255, randInt(120, 180), 0, 255};
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, pixels[i].color);
                            if (pixels[i].life <= 0) {
                                explodeNapalm(pixelColors, pixels, gridWidth, gridHeight, screenWidth, pixels[i].x, pixels[i].y);
                            }
                            break;
                        }

                        if (pixels[i].y < gridHeight - 1 && pixels[i + gridWidth].type != 'S' && pixels[i + gridWidth].type != 'i' && pixels[i + gridWidth].type != 'o') {
                            
                            if (pixels[i + gridWidth].draw == 'n') {
                                pixels[i] = pixels[i + gridWidth];
                                pixels[i + gridWidth] = temp;
                                pixels[i + gridWidth].x = (i + gridWidth) % gridWidth;
                                pixels[i + gridWidth].y = (i + gridWidth) / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[i + gridWidth].x, pixels[i + gridWidth].y, screenWidth, pixels[i + gridWidth].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                            } else if (col > 0 && col < gridWidth - 1 && pixels[i + gridWidth - 1].draw == 'n' && pixels[i + gridWidth + 1].draw == 'n') {
                                int target = (randInt(1, 2) == 1) ? i + gridWidth - 1 : i + gridWidth + 1;

                                pixels[i] = pixels[target];
                                pixels[target] = temp;
                                pixels[target].x = target % gridWidth;
                                pixels[target].y = target / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[target].x, pixels[target].y, screenWidth, pixels[target].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                            } else if (col > 0 && pixels[i + gridWidth - 1].draw == 'n') {
                                pixels[i] = pixels[i + gridWidth - 1];
                                pixels[i + gridWidth - 1] = temp;
                                pixels[i + gridWidth - 1].x = (i + gridWidth - 1) % gridWidth;
                                pixels[i + gridWidth - 1].y = (i + gridWidth - 1) / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[i + gridWidth - 1].x, pixels[i + gridWidth - 1].y, screenWidth, pixels[i + gridWidth - 1].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});

                            } else if (col < gridWidth - 1 && pixels[i + gridWidth + 1].draw == 'n') {
                                pixels[i] = pixels[i + gridWidth + 1];
                                pixels[i + gridWidth + 1] = temp;
                                pixels[i + gridWidth + 1].x = (i + gridWidth + 1) % gridWidth;
                                pixels[i + gridWidth + 1].y = (i + gridWidth + 1) / gridWidth;
                                pixels[i].x = i % gridWidth;
                                pixels[i].y = i / gridWidth;

                                setLogicalPixel(pixelColors, pixels[i + gridWidth + 1].x, pixels[i + gridWidth + 1].y, screenWidth, pixels[i + gridWidth + 1].color);
                                setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, (Color){0, 0, 0, 255});
                            }
                        }
                        break;
                    }

                    case 'i': // Ice rules: static like stone, freezes adjacent water, melts to water from lava or steam, heat tracks warmth
                    {
                        int iceNeighbors[] = {
                            i - gridWidth,
                            i + gridWidth,
                            i - 1,
                            i + 1
                        };
                        int iceValid[] = {
                            pixels[i].y > 0,
                            pixels[i].y < gridHeight - 1,
                            col > 0,
                            col < gridWidth - 1
                        };

                        for (int n = 0; n < 4; n++) {
                            if (!iceValid[n]) continue;
                            int ni = iceNeighbors[n];

                            if (pixels[ni].type == 't') {
                                // steam touching ice → instantly condenses to water
                                pixels[ni].type = 'w';
                                pixels[ni].life = 100;
                                pixels[ni].heat = -1;
                                pixels[ni].color = (Color){randInt(20, 40), randInt(100, 140), randInt(200, 230), 200};
                                setLogicalPixel(pixelColors, pixels[ni].x, pixels[ni].y, screenWidth, pixels[ni].color);

                            } else if (pixels[ni].type == 'w' && randInt(1, 180) == 1) {
                                // ice slowly freezes adjacent water into more ice
                                pixels[ni].type = 'i';
                                pixels[ni].heat = 0;
                                pixels[ni].life = -1;
                                pixels[ni].color = (Color){randInt(180, 210), randInt(220, 240), randInt(240, 255), 255};
                                setLogicalPixel(pixelColors, pixels[ni].x, pixels[ni].y, screenWidth, pixels[ni].color);

                            } else if (pixels[ni].type == 'l' || pixels[ni].type == 'S') {
                                // lava or hot stone heats up the ice
                                if (pixels[ni].type == 'l') pixels[i].heat += randInt(35, 55);
                                if (pixels[ni].type == 'S' && pixels[ni].heat > 0) pixels[i].heat += randInt(3, 8);
                                if (pixels[i].heat > 100) pixels[i].heat = 100;
                            }
                        }

                        // ice melts to water when heated enough
                        if (pixels[i].heat >= 100) {
                            pixels[i].type = 'w';
                            pixels[i].life = 100;
                            pixels[i].heat = -1;
                            pixels[i].color = (Color){randInt(20, 40), randInt(100, 140), randInt(200, 230), 200};
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, pixels[i].color);
                            break;
                        }

                        // update ice color slightly warmer as it heats (stays icy blue but shifts)
                        if (pixels[i].heat > 0) {
                            float t = pixels[i].heat / 100.0f;
                            int r = (int)(195 + (230 - 195) * t);
                            int g = (int)(230 + (230 - 230) * t);
                            int b = (int)(247 + (200 - 247) * t);
                            pixels[i].color = (Color){r, g, b, 255};
                            setLogicalPixel(pixelColors, pixels[i].x, pixels[i].y, screenWidth, pixels[i].color);
                        }

                        break;
                    }
                }
            }
        }
    }   

    // Heat propagation pass
for (int row = gridHeight - 1; row >= 0; row--) {
    for (int c = 0; c < gridWidth; c++) {
        int i = row * gridWidth + c;
        if (pixels[i].draw != 'y') continue;

        int neighbors[] = { i - gridWidth, i + gridWidth, i - 1, i + 1 };
        int valid[] = {
            row > 0,
            row < gridHeight - 1,
            c > 0,
            c < gridWidth - 1
        };

        for (int n = 0; n < 4; n++) {
            if (!valid[n]) continue;
            int ni = neighbors[n];
            if (pixels[ni].draw != 'y') continue;

            // Lava conducts heat to adjacent lava
            if (pixels[i].type == 'l' && pixels[ni].type == 'l' && lavaDominance < 0.5f) {
                if (pixels[ni].heat > pixels[i].heat + 2) {
                    int transfer = (pixels[ni].heat - pixels[i].heat) / 4;
                    if (transfer < 1) transfer = 1;
                    pixels[i].heat  += transfer;
                    pixels[ni].heat -= transfer;
                }
            }

            // Water spreads heat damage inward
            if (pixels[i].type == 'w' && pixels[ni].type == 'w') {
                if (pixels[ni].life < pixels[i].life - 20) {
                    int transfer = (pixels[i].life - pixels[ni].life) / 4;
                    if (transfer < 1) transfer = 1;
                    pixels[ni].life -= transfer;
                    // no rebound — just let it spread cleanly
                }
            }

            // Stone drains heat into adjacent lava
            if (pixels[i].type == 'l' && pixels[ni].type == 'S' && pixels[ni].heat > 20 && lavaDominance < 0.4f) {
                int stoneHasWater = 0;
                int snx = ni % gridWidth;
                int sny = ni / gridWidth;
                int stoneNeigh[] = { ni - gridWidth, ni + gridWidth, ni - 1, ni + 1 };
                int stoneNeighValid[] = { sny > 0, sny < gridHeight - 1, snx > 0, snx < gridWidth - 1 };
                for (int sn = 0; sn < 4; sn++) {
                    if (!stoneNeighValid[sn]) continue;
                    if (pixels[stoneNeigh[sn]].type == 'w') { stoneHasWater = 1; break; }
                }
                if (stoneHasWater) {
                    int transfer = (pixels[ni].heat / 10) + 1;
                    pixels[i].heat  -= transfer;
                    pixels[ni].heat -= 1;
                }
            }

            // Hot stone conducts into adjacent stone chain
            if (pixels[i].type == 'S' && pixels[ni].type == 'S' && pixels[i].heat > 0) {
                if (pixels[i].heat > pixels[ni].heat + 3) {
                    int transfer = (pixels[i].heat - pixels[ni].heat) / 4;
                    if (transfer < 1) transfer = 1;
                    pixels[i].heat  -= transfer;
                    pixels[ni].heat += transfer;
                    if (pixels[ni].heat > 300) pixels[ni].heat = 300;
                }
            }
        }
    }
}

        BeginDrawing();
            UpdateTexture(tex, pixelColors);
            DrawTexture(tex, 0, 0, WHITE);

            // Display selected type
            switch (selectedType) {
                
                case 's':
                    DrawRectangle(10, 10, 30, 30, (Color){210, 185, 120, 255});
                    DrawRectangleLinesEx((Rectangle){10, 10, 30, 30}, 3, LIGHTGRAY);
                    DrawText("Sand", 50, 14, 24, LIGHTGRAY);
                    break;

                case 'w':
                    DrawRectangle(10, 10, 30, 30, (Color){40, 140, 230, 200});
                    DrawRectangleLinesEx((Rectangle){10, 10, 30, 30}, 3, LIGHTGRAY);
                    DrawText("Water", 50, 14, 24, LIGHTGRAY);
                    break;

                case 'S':
                    DrawRectangle(10, 10, 30, 30, (Color){130, 130, 130, 255});
                    DrawRectangleLinesEx((Rectangle){10, 10, 30, 30}, 3, LIGHTGRAY);
                    DrawText("Stone", 50, 14, 24, LIGHTGRAY);
                    break;

                case 'l':
                    DrawRectangle(10, 10, 30, 30, (Color){255, 60, 0, 255});
                    DrawRectangleLinesEx((Rectangle){10, 10, 30, 30}, 3, LIGHTGRAY);
                    DrawText("Lava", 50, 14, 24, LIGHTGRAY);
                    break;
                
                case 't':
                    DrawRectangle(10, 10, 30, 30, (Color){200, 200, 200, 255});
                    DrawRectangleLinesEx((Rectangle){10, 10, 30, 30}, 3, LIGHTGRAY);
                    DrawText("Steam", 50, 14, 24, LIGHTGRAY);
                    break;

                case 'i':
                    DrawRectangle(10, 10, 30, 30, (Color){195, 230, 247, 255});
                    DrawRectangleLinesEx((Rectangle){10, 10, 30, 30}, 3, LIGHTGRAY);
                    DrawText("Ice", 50, 14, 24, LIGHTGRAY);
                    break;

                case 'o':
                    DrawRectangle(10, 10, 30, 30, (Color){95, 35, 165, 255});
                    DrawRectangleLinesEx((Rectangle){10, 10, 30, 30}, 3, LIGHTGRAY);
                    DrawText("Obsidian", 50, 14, 24, LIGHTGRAY);
                    break;

                case 'p':
                    DrawRectangle(10, 10, 30, 30, (Color){255, 145, 35, 255});
                    DrawRectangleLinesEx((Rectangle){10, 10, 30, 30}, 3, LIGHTGRAY);
                    DrawText("Napalm", 50, 14, 24, LIGHTGRAY);
                    break;
            
            }
            
            if (dropdownOpen) {
                int offset = 40;
                int index = 1;
                listOrder[0] = selectedType;
                for (int i = 0; i < totalTypes; i++) {
                    if (types[i] != selectedType) {
                        listOrder[index] = types[i];
                        index++;
                        switch (types[i]) {
                
                            case 's':
                                DrawRectangle(10, 10 + offset, 30, 30, (Color){210, 185, 120, 255});
                                DrawRectangleLinesEx((Rectangle){10, 10 + offset, 30, 30}, 3, LIGHTGRAY);
                                DrawText("Sand", 50, 14 + offset, 24, LIGHTGRAY);
                                offset += 40;
                                break;

                            case 'w':
                                DrawRectangle(10, 10 + offset, 30, 30, (Color){40, 140, 230, 200});
                                DrawRectangleLinesEx((Rectangle){10, 10 + offset, 30, 30}, 3, LIGHTGRAY);
                                DrawText("Water", 50, 14 + offset, 24, LIGHTGRAY);
                                offset += 40;
                                break;

                            case 'S':
                                DrawRectangle(10, 10 + offset, 30, 30, (Color){130, 130, 130, 255});
                                DrawRectangleLinesEx((Rectangle){10, 10 + offset, 30, 30}, 3, LIGHTGRAY);
                                DrawText("Stone", 50, 14 + offset, 24, LIGHTGRAY);
                                offset += 40;
                                break;
                            
                            case 'l':
                                DrawRectangle(10, 10 + offset, 30, 30, (Color){255, 60, 0, 255});
                                DrawRectangleLinesEx((Rectangle){10, 10 + offset, 30, 30}, 3, LIGHTGRAY);
                                DrawText("Lava", 50, 14 + offset, 24, LIGHTGRAY);
                                offset += 40;
                                break;
                            
                            case 't':
                                DrawRectangle(10, 10 + offset, 30, 30, (Color){200, 200, 200, 255});
                                DrawRectangleLinesEx((Rectangle){10, 10 + offset, 30, 30}, 3, LIGHTGRAY);
                                DrawText("Steam", 50, 14 + offset, 24, LIGHTGRAY);
                                offset += 40;
                                break;

                            case 'i':
                                DrawRectangle(10, 10 + offset, 30, 30, (Color){195, 230, 247, 255});
                                DrawRectangleLinesEx((Rectangle){10, 10 + offset, 30, 30}, 3, LIGHTGRAY);
                                DrawText("Ice", 50, 14 + offset, 24, LIGHTGRAY);
                                offset += 40;
                                break;

                            case 'o':
                                DrawRectangle(10, 10 + offset, 30, 30, (Color){95, 35, 165, 255});
                                DrawRectangleLinesEx((Rectangle){10, 10 + offset, 30, 30}, 3, LIGHTGRAY);
                                DrawText("Obsidian", 50, 14 + offset, 24, LIGHTGRAY);
                                offset += 40;
                                break;

                            case 'p':
                                DrawRectangle(10, 10 + offset, 30, 30, (Color){255, 145, 35, 255});
                                DrawRectangleLinesEx((Rectangle){10, 10 + offset, 30, 30}, 3, LIGHTGRAY);
                                DrawText("Napalm", 50, 14 + offset, 24, LIGHTGRAY);
                                offset += 40;
                                break;
                        }
                    }
                }
            }

            if (dropDownTimerDelay > 0) {
                dropDownTimerDelay--;
            }

            // Plus minus buttons
            int textSize = MeasureText(TextFormat("%i", drawAmount), 24);
            DrawRectangleLinesEx((Rectangle){screenWidth - 30 - 10, 10, 30, 30}, 3, LIGHTGRAY);
            DrawText("-", screenWidth - 31, 11, 30, WHITE);
            DrawText(TextFormat("%i", drawAmount), screenWidth - 30 - 10 - textSize - 10, 14, 24, WHITE);
            DrawText("+", screenWidth - 32 - textSize - 50, 11, 30, WHITE);
            DrawRectangleLinesEx((Rectangle){screenWidth - 30 - 10 - 10 - textSize - 10 - 30, 10, 30, 30}, 3, LIGHTGRAY);

            // Restart butotn
            DrawText("Reset", screenWidth - 83, 53, 24, WHITE);
            DrawRectangleLinesEx((Rectangle){screenWidth - 90, 50, 80, 30}, 3, LIGHTGRAY);

            // Hammer toggle
            if (mode == 1) {
                ShowCursor();
                DrawRectangleLinesEx((Rectangle){screenWidth - 50, 90, 40, 40}, 3, RED);
            } else if (mode == 2) {
                HideCursor();
                DrawTexture(hammerTex, mousePos.x - 16, mousePos.y, WHITE);
                DrawRectangleLinesEx((Rectangle){screenWidth - 50, 90, 40, 40}, 3, GREEN);
            }

            DrawTexture(hammerTex, screenWidth - 46, 93, WHITE); 

            // Cursor box - top-left = (centerX - half) * drawSize, size = drawAmount * drawSize
            Vector2 mouse = GetMousePosition();
            int centerX = (int)mouse.x / drawSize;
            int centerY = (int)mouse.y / drawSize;
            int half = drawAmount / 2;
            int boxSize = drawAmount * drawSize;
            int snapX = (centerX - half) * drawSize;
            int snapY = (centerY - half) * drawSize;

            DrawRectangleLinesEx((Rectangle){snapX, snapY, boxSize, boxSize}, 1, (Color){186, 186, 186, 255}); // Pixel selector

        EndDrawing();

        SetWindowTitle(TextFormat("Sand Simulator - %i FPS - %i Particles - %i Pixels", GetFPS(), pixelCount, pixelCount * (drawSize * drawSize)));
        frameCount++;
    }

    UnloadTexture(hammerTex);
    // free(lavaRaw);
    // free(waterRaw);
    // free(hasLavaInRange);
    // free(hasWaterInRange);
    free(pixels);
    free(pixelColors);
    CloseWindow();
    return 0;
}