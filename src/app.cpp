#include <stdexcept>
#include <SDL2/SDL.h>
#include <vector>
#include <optional>
#include "render/texture.cpp"
#include "render/digit_draw.cpp"
#include "tetromino.cpp"

#define GAME_SPEED 1.0
#define FALL_BASE_T 0.5

#define VIEWABLE_FIELD_H 20
#define VIEWABLE_FIELD_Y (FIELD_H - VIEWABLE_FIELD_H)
#define TILE_SIZE 16

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// [COLORS]
#define COL_WHITE SDL_Color { 255, 255, 255, 255 }
#define COL_BLACK SDL_Color { 0, 0, 0, 255 }
#define COL_GRAY SDL_Color { 128, 128, 128, 255 }
#define COL_GRAY_LIGHT SDL_Color { 192, 192, 192, 255 }
#define COL_GRAY_DARK SDL_Color { 64, 64, 64, 255 }

#define COL_RED SDL_Color { 255, 0, 0, 255 }
#define COL_ORANGE SDL_Color { 255, 128, 0, 255 }
#define COL_YELLOW SDL_Color { 255, 255, 0, 255 }
#define COL_LIME SDL_Color { 128, 255, 0, 255 }
#define COL_GREEN SDL_Color { 0, 255, 0, 255 }
#define COL_GREEN_CYAN SDL_Color { 0, 255, 128, 255 }
#define COL_CYAN SDL_Color { 0, 255, 255, 255 }
#define COL_BLUE_LIGHT SDL_Color { 0, 128, 255, 255 }
#define COL_BLUE SDL_Color { 0, 0, 255, 255 }
#define COL_PURPLE SDL_Color { 128, 0, 255, 255 }
#define COL_PINK SDL_Color { 255, 0, 255, 255 }
#define COL_PINK_DARK SDL_Color { 255, 0, 128, 255 }

// ==============
// Resources

class Resources {
public:
    Texture texBlock;

    Texture menuNewGame;
    Texture menuExit;

    Texture scoreText;
    Texture digits;
    Texture gameOver;

    Resources(const Resources&) = delete;
    Resources(Resources&&) = default;
    ~Resources() = default;
};

Resources loadResources(SDL_Renderer* renderer) {
    auto texBlock = Texture(renderer, "assets/textures/t-block-s.bmp");

    auto menuNewGame = Texture(renderer, "assets/textures/menu/new-game.bmp");
    auto menuExit = Texture(renderer, "assets/textures/menu/exit.bmp");

    auto scoreText = Texture(renderer, "assets/textures/common_ui/score-l.bmp");
    auto digits = Texture(renderer, "assets/textures/common_ui/digits.bmp");
    auto gameOver = Texture(renderer, "assets/textures/common_ui/game-over.bmp");

    return Resources {
        std::move(texBlock),

        std::move(menuNewGame),
        std::move(menuExit),

        std::move(scoreText),
        std::move(digits),
        std::move(gameOver)
    };
};

void destroyResources(SDL_Renderer* renderer, Resources* res) {

}

// ===========
// InputState

class KeyState {
private:
    bool f_isDown;
    bool f_isPressed;

public:
    KeyState() {
        this->f_isDown = false;
        this->f_isPressed = false;
    }

    bool isDown() {
        return this->f_isDown;
    }

    bool isUp() {
        return !this->f_isDown;
    }

    bool isPressed() {
        return this->f_isPressed;
    }

    void press() {
        if (this->f_isDown) {
            this->f_isDown = true;
        } else {
            this->f_isDown = true;
            this->f_isPressed = true;
        }
    }

    void release() {
        this->f_isDown = false;
        this->f_isPressed = false;
    }

    void update() {
        if (this->f_isPressed) {
            this->f_isPressed = false;
        }
    }
};

class InputState {
public:
    KeyState keyR; // right
    KeyState keyU; // isUp
    KeyState keyL; // left
    KeyState keyD; // isDown
    KeyState keyAction; // z or space
    KeyState keyBack; // x or esc
    bool exitRequired;

    InputState():
            keyR(KeyState()),
            keyU(KeyState()),
            keyL(KeyState()),
            keyD(KeyState()),
            keyAction(KeyState()),
            keyBack(KeyState()),
            exitRequired(false) {}
};

// =============
// Application



SDL_Point point(int x, int y) {
    return SDL_Point { x, y };
}

enum AppState { menu = 0, game = 1 };

enum MenuElement { newGame = 0, quit = 1 };

enum CleaningMode { line = 0, color = 1 };

enum ExtraTilesMode { off = 0, on = 1 };

SDL_Color tileSdlColor(TetroColor color) {
    switch(color) {
        case TetroColor::red: return COL_RED;
        case TetroColor::orange: return COL_ORANGE;
        case TetroColor::yellow: return COL_YELLOW;
        case TetroColor::green: return COL_GREEN;
        case TetroColor::blue: return COL_CYAN;
        case TetroColor::violet: return COL_BLUE;
        case TetroColor::white: return COL_GRAY_LIGHT;
        case TetroColor::black: return COL_GRAY_DARK;
    }
}

void fillShapeBag(std::vector<TetroShapeClass>* bag) {
    bag->clear();
    TetroShapeClass base[7] = {
            TetroShapeClass::L,
//            TetroShapeClass::L,
            TetroShapeClass::J,
//            TetroShapeClass::J,
            TetroShapeClass::I,
//            TetroShapeClass::I,
            TetroShapeClass::T,
//            TetroShapeClass::T,
            TetroShapeClass::O,
//            TetroShapeClass::O,
            TetroShapeClass::Z,
//            TetroShapeClass::Z,
            TetroShapeClass::S,
//            TetroShapeClass::S
    };
    for (int i = 0; i < 7; i++) { bag->push_back(base[i]); }

    for (int i = 0; i < 7; i++) {
        int i1 = i;
        int i2 = std::rand() % 7;

        auto class1 = bag->at(i1);
        auto class2 = bag->at(i2);

        bag->at(i1) = class2;
        bag->at(i2) = class1;
    }
}

void fillColorBag(std::vector<TetroColor>* bag) {
    bag->clear();
    for (int i = 0; i < 6; i++) { bag->push_back(BASE_TILES[i]); }

    int size = bag->size();
    for (int i = 0; i < size; i++) {
        int i1 = i;
        int i2 = std::rand() % size;

        auto class1 = bag->at(i1);
        auto class2 = bag->at(i2);

        bag->at(i1) = class2;
        bag->at(i2) = class1;
    }
}

class App {
public:

    SDL_Window* window;
    SDL_Renderer* renderer;
    Resources resources;

    AppState __state;
    InputState input;

    // =================
    // [menu state part]

    MenuElement menuElement;
    CleaningMode cleaningMode;
    ExtraTilesMode extraTilesMode;

    // [menu state part]
    // =================
    // [game state part]

    TetroField field;
    std::optional<TetroActiveShape> activeShape;
    float tickAccDown;
    float tickAccSide;
    int score;
    float gameSpeed;
    std::vector<TetroShapeClass> shapeBag;
    std::vector<TetroColor> colorBag;
    bool isLose;

    // [game state part]
    // =================

    App() = delete;
    App(const App&) = delete;
    App(App&&) = default;
    ~App() = default;

    App(SDL_Window* window, SDL_Renderer* renderer, Resources res):
            window(window),
            renderer(renderer),
            resources(std::move(res)),
            __state(AppState::menu),

            // [menu]
            menuElement(MenuElement::newGame),
            cleaningMode(CleaningMode::line),
            extraTilesMode(ExtraTilesMode::off),

            // [game]
            field(TetroField()),
            activeShape(std::nullopt),
            tickAccDown(0.0),
            tickAccSide(0.0),
            score(0),
            gameSpeed(GAME_SPEED),
            shapeBag(std::vector<TetroShapeClass>()),
            colorBag(std::vector<TetroColor>()),
            isLose(false)
    {}

    void drawTextureCopyColored(Texture& texture, SDL_Point point, SDL_Color color) {
        auto textureRect = texture.rect();
        SDL_Rect destRect = SDL_Rect { point.x, point.y, textureRect.w, textureRect.h };

        SDL_SetTextureColorMod(texture.sldHandle(), color.r, color.g, color.b);
        SDL_RenderCopy(this->renderer, texture.sldHandle(), &textureRect, &destRect);
    }

    void drawTextureCopy(Texture& texture, SDL_Point point) {
        drawTextureCopyColored(texture, point, SDL_Color { 255, 255, 255, 255});
    }

    TetroShapeClass nextShapeClass(bool remove) {
        if (this->shapeBag.empty()) {
            fillShapeBag(&this->shapeBag);
        }

        int last = this->shapeBag.size() - 1;
        auto shapeClass = this->shapeBag.at(last);
        if (remove) { this->shapeBag.pop_back(); }

        return shapeClass;
    }

    TetroColor nextShapeColor(bool remove) {
        if (this->colorBag.empty()) {
            fillColorBag(&this->colorBag);
        }

        int last = this->colorBag.size() - 1;
        auto shapeColor = this->colorBag.at(last);

        if (remove) { this->colorBag.pop_back(); }

        return shapeColor;
    }

    TetroShapePrototype peekNextShape() {
        auto shapeClass = this->nextShapeClass(false);
        auto shapeColor = this->nextShapeColor(false);
        return TetroShapePrototype(shapeClass, 0, shapeColor);
    }

    void spawnNextShape() {
        auto shapeClass = this->nextShapeClass(true);
        auto shapeColor = this->nextShapeColor(true);
        this->activeShape = std::optional(
            TetroActiveShape(
                3,
                0,
                TetroShapePrototype(shapeClass, 0, shapeColor)
            )
        );
    }

    void resetGameState() {
        this->field = TetroField();
        this->tickAccDown = 0.0;
        this->score = 0;
        this->isLose = false;
        this->activeShape = std::nullopt;
        this->shapeBag.clear();
        this->colorBag.clear();
        this->field.clear();
    }

    void setMainState(AppState state) {
        if (this->__state == state) {
            printf("Change app __state to same");
            exit(1);
        }
        if (this->__state == AppState::menu && state == AppState::game) {
            this->__state = state;
            this->resetGameState();
        }
        if (this->__state == AppState::game && state == AppState::menu) {
            this->__state = state;
        }
    }

    void updateInput() {
        {
            this->input.keyAction.update();
            this->input.keyBack.update();
            this->input.keyR.update();
            this->input.keyU.update();
            this->input.keyL.update();
            this->input.keyD.update();
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                this->input.exitRequired = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_z || event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_RETURN) { this->input.keyAction.press(); }
                if (event.key.keysym.sym == SDLK_x || event.key.keysym.sym == SDLK_ESCAPE) { this->input.keyBack.press(); }
                if (event.key.keysym.sym == SDLK_RIGHT) { this->input.keyR.press(); }
                if (event.key.keysym.sym == SDLK_UP) { this->input.keyU.press(); }
                if (event.key.keysym.sym == SDLK_LEFT) { this->input.keyL.press(); }
                if (event.key.keysym.sym == SDLK_DOWN) { this->input.keyD.press(); }
            } else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_z || event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_RETURN) { this->input.keyAction.release(); }
                if (event.key.keysym.sym == SDLK_x || event.key.keysym.sym == SDLK_ESCAPE) { this->input.keyBack.release(); }
                if (event.key.keysym.sym == SDLK_RIGHT) { this->input.keyR.release(); }
                if (event.key.keysym.sym == SDLK_UP) { this->input.keyU.release(); }
                if (event.key.keysym.sym == SDLK_LEFT) { this->input.keyL.release(); }
                if (event.key.keysym.sym == SDLK_DOWN) { this->input.keyD.release(); }
            } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                this->input.keyAction.release();
                this->input.keyBack.release();
                this->input.keyR.release();
                this->input.keyU.release();
                this->input.keyL.release();
                this->input.keyD.release();
            }
        }
    }

    void updateStateMenu() {
        if (this->input.keyBack.isPressed()) {
            this->input.exitRequired = true;
        }

        if (this->input.keyAction.isPressed()) {
            switch (this->menuElement) {
                case MenuElement::newGame:
                    this->setMainState(AppState::game);
                    break;
                case MenuElement::quit:
                    this->input.exitRequired = true;
                    break;
                default:
                    printf("UNREACHABLE\n");
                    exit(1);
            }
        }

        if (this->input.keyD.isPressed()) {
            this->menuElement = static_cast<MenuElement>(this->menuElement < 1 ? this->menuElement + 1 : 1);
        }
        if (this->input.keyU.isPressed()) {
            this->menuElement = static_cast<MenuElement>(this->menuElement > 0 ? this->menuElement - 1 : 0);
        }
    }

    void updateStateGame(float dt) {
        // Проверка выхода
        if (this->input.keyBack.isPressed()) { this->setMainState(AppState::menu); return; }
        // Проверка проигрыша
        if (this->isLose) {
            if (this->input.keyAction.isPressed()) {
                this->resetGameState();
            }
            return;
        }

        // Определение управление фигурой
        this->tickAccDown += dt;

        if (this->tickAccSide > 0.0) {
            this->tickAccSide -= dt;
        }
        auto tS = this->tickAccSide;
        auto tD = this->tickAccDown;

        float fallT = FALL_BASE_T / this->gameSpeed;
        float forceFallT = fallT / 12.0;
        float sideT = FALL_BASE_T / 8.0;

        bool downPressed = this->input.keyD.isDown();
        bool upPressed = this->input.keyU.isDown();
        bool upJustPressed = this->input.keyU.isPressed();
        bool leftPressed = this->input.keyL.isDown();
        bool rightPressed = this->input.keyR.isDown();

        bool handleMove = false;
        bool resetT = false;
        bool left = false;
        bool right = false;
        bool down = false;
        bool rotate = false;

        if (downPressed ? tD >= forceFallT : tD >= fallT) {
            down = true;
            handleMove = true;
            resetT = true;
        }

        if (leftPressed && tS <= 0.0) {
            left = true;
            handleMove = true;
            if (downPressed) { down = true; }
        }

        if (rightPressed && tS <= 0.0) {
            right = true;
            handleMove = true;
            if (downPressed) { down = true; }
        }

        if (upJustPressed) {
            rotate = true;
        }

        if (resetT) {
            this->tickAccDown = 0.0;// this->tickAccDown - fallT * floor(this->tickAccDown / fallT); // Skipping many ticks on lag fix
        }

        // Обработка вращения фигуры
        if (rotate && this->activeShape.has_value()) {
            TetroActiveShape shape = this->activeShape.value();
            auto rotatedPrototype = shape.prototype.rotated();
            shape.prototype = rotatedPrototype;

            if (shapeCanPlaced(shape)) {
                this->activeShape.value() = shape;
            }
        }

        // Обработка движения фигуры
        if (handleMove) {
            if(this->activeShape.has_value()) {
                // handle down
                {
                    TetroActiveShape& shape = this->activeShape.value();
                    TetroActiveShape movedShape = shape;
                    if (down) { movedShape.y += 1; }

                    bool canMove = this->shapeCanPlaced(movedShape);
                    if(canMove) {
                        this->activeShape.value() = movedShape;
                    } else {
                        // Здесь нам нужна еще не сдвинутая фигура.
                        for (int i = 0; i < shape.prototype.tilesCount; i++) {
                            int x = shape.x + shape.prototype.offsetsX[i];
                            int y = shape.y + shape.prototype.offsetsY[i];
                            this->field.set(x, y, std::optional(shape.prototype.color));
                        }
                        this->spawnNextShape();
                    }
                }
                // handle left/right
                {
                    TetroActiveShape &shape = this->activeShape.value();
                    TetroActiveShape movedShape = shape;
                    if (left) { movedShape.x -= 1; }
                    if (right) { movedShape.x += 1; }

                    bool canMove = this->shapeCanPlaced(movedShape);
                    if (canMove) {
                        this->activeShape.value() = movedShape;
                        this->tickAccSide = sideT;
                    }
                }
            } else {
                this->spawnNextShape();
            }
        }

        // обработка полных линий
        int removed = this->field.removeFullLines();
        if (removed > 0) {
            int dScore = 0;
            switch (removed) {
                case 1: dScore = 10 + 0; break;
                case 2: dScore = 20 + 5; break;
                case 3: dScore = 30 + 15; break;
                case 4: dScore = 40 + 20; break;
                default: dScore = 13 * removed; break;
            }
            this->score += dScore;
        }

        // Обработка проигрыша
        {
            bool isLose = false;
            for (int y = 0; y < VIEWABLE_FIELD_Y; y++) {
                if (!this->field.lineIsEmpty(y)) {
                    isLose = true;
                    break;
                }
            }
            if (isLose) { this->isLose = true; }
        }
    }

    // Game logic
    void updateState(float dt) {
        switch (this->__state) {
            case AppState::menu:
                this->updateStateMenu();
                break;
            case AppState::game:
                this->updateStateGame(dt);
                break;
            default:
                printf("UNREACHABLE\n");
                exit(1);
        }
    }

    bool shapeCanPlaced(TetroActiveShape& shape) {
        int count = shape.prototype.tilesCount;
        for (int i = 0; i < count; i++) {
            int x = shape.x + shape.prototype.offsetsX[i];
            int y = shape.y + shape.prototype.offsetsY[i];
            if (x < 0 || x >= FIELD_W || y < 0 || y >= FIELD_H) { return false; }
            if (this->field.getAssured(x, y)->has_value()) { return false; }
        }
        return true;
    }

    void drawMenu() {
        auto activeColor = COL_WHITE;
        auto unActiveColor = COL_GRAY;

        auto baseX = (SCREEN_WIDTH - 256) / 2;
        auto baseY = (SCREEN_HEIGHT - 32 * 2) / 2;

        {
            auto color = this->menuElement == MenuElement::newGame ? activeColor : unActiveColor;
            this->drawTextureCopyColored(this->resources.menuNewGame, point(baseX, baseY + 32*0), color);
        }

        {
            auto color = this->menuElement == MenuElement::quit ? activeColor : unActiveColor;
            this->drawTextureCopyColored(this->resources.menuExit, point(baseX, baseY + 32*1), color);
        }
    }

    void drawDynamicShape(TetroShapePrototype shape, int x, int y, int clipping) {
        for(int i = 0; i < shape.tilesCount; i++) {
            int xp = shape.offsetsX[i];
            int yp = shape.offsetsY[i];
            if (yp <= clipping) { continue; }
            this->drawTextureCopyColored(
                    this->resources.texBlock,
                    point(x + xp * TILE_SIZE, y + yp * TILE_SIZE),
                    tileSdlColor(shape.color)
            );
        }
    }

    void drawDynamicShape(TetroShapePrototype shape, int x, int y) {
        drawDynamicShape(shape, x, y, -2000000000);
    }

    void drawGame() {

        int fieldMinX = SCREEN_WIDTH / 2 - TILE_SIZE*FIELD_W / 2;
        int fieldMinY = SCREEN_HEIGHT / 2 - TILE_SIZE*VIEWABLE_FIELD_H / 2;
        int fieldW = TILE_SIZE * FIELD_W;
        int fieldH = TILE_SIZE * VIEWABLE_FIELD_H;
        {
            int x1 = fieldMinX - 1;
            int y1 = fieldMinY - 1;
            int x2 = fieldMinX + fieldW;
            int y2 = fieldMinY + fieldH;
            SDL_SetRenderDrawColor(this->renderer, 128, 128, 128, 255);
            SDL_RenderDrawLine(this->renderer, x1, y1, x1, y2);
            SDL_RenderDrawLine(this->renderer, x1, y1, x2, y1);
            SDL_RenderDrawLine(this->renderer, x2, y2, x1, y2);
            SDL_RenderDrawLine(this->renderer, x2, y2, x2, y1);
            SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
        }

        // xp, yp - Визуальные координаты поля
        // xi, yi - Координаты в массиве поля
        for (int xp = 0; xp < FIELD_W; xp++) {
            for (int yp = 0; yp < VIEWABLE_FIELD_H; yp++) {
                int xi = xp;
                int yi = yp + VIEWABLE_FIELD_Y;

                auto tileOpt = this->field.getAssured(xi, yi);
                if (!tileOpt->has_value()) { continue; }
                auto tile = tileOpt->value();

                auto sdlColor = tileSdlColor(tile);
                this->drawTextureCopyColored(
                    this->resources.texBlock,
                    point(fieldMinX + xp * TILE_SIZE, fieldMinY + yp * TILE_SIZE),
                    sdlColor
                );
            }
        }

        if (this->activeShape.has_value()) {
            auto shape = this->activeShape.value();
            auto x = fieldMinX + shape.x * TILE_SIZE;
            auto y = fieldMinY + (shape.y - VIEWABLE_FIELD_Y) * TILE_SIZE;
            drawDynamicShape(shape.prototype, x, y, VIEWABLE_FIELD_Y - shape.y - 1);
//            drawDynamicShape(shape.prototype, x, y);
        }

        // next shape
        auto nextShape = this->peekNextShape();
        int shapeX = fieldMinX + fieldW + 16;
        int shapeY = fieldMinY + 16;
        int shapeW = TILE_SIZE * 4;
        int shapeH = TILE_SIZE * 4;

        drawDynamicShape(nextShape, shapeX, shapeY);
        {
            int x1 = shapeX - 1;
            int y1 = shapeY - 1;
            int x2 = shapeX + shapeW;
            int y2 = shapeY + shapeH;
            SDL_SetRenderDrawColor(this->renderer, 128, 128, 128, 255);
            SDL_RenderDrawLine(this->renderer, x1, y1, x1, y2);
            SDL_RenderDrawLine(this->renderer, x1, y1, x2, y1);
            SDL_RenderDrawLine(this->renderer, x2, y2, x1, y2);
            SDL_RenderDrawLine(this->renderer, x2, y2, x2, y1);
            SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
        }

        // score
        int titleX = shapeX;
        int titleY = shapeY + shapeH;
        SDL_Rect dstRect = SDL_Rect{titleX, titleY, 80, 32};
        SDL_RenderCopy(this->renderer, this->resources.scoreText.sldHandle(), NULL, &dstRect);

        int scoreX = titleX + 0;
        int scoreY = titleY + 32;
        drawNumber(this->renderer, &this->resources.digits, scoreX, scoreY, this->score, 3);

        // lose
        if (this->isLose) {
            int w = 80;
            int h = 64;
            int x = SCREEN_WIDTH / 2 - w / 2;
            int y = SCREEN_HEIGHT / 2 - h / 2;

            SDL_Rect dstRect = SDL_Rect { x, y, w, h };
            SDL_RenderCopy(this->renderer, this->resources.gameOver.sldHandle(), NULL, &dstRect);
        }
    }

    void drawState() {
        SDL_RenderClear(this->renderer);

        switch (this->__state) {
            case AppState::menu:
                this->drawMenu();
                break;
            case AppState::game:
                this->drawGame();
                break;
        }

        SDL_RenderPresent(this->renderer);
    }

    // While true: Do game loop
    bool tick(float dt) {
        updateInput();
        updateState(dt);
        drawState();

        auto error = SDL_GetError();
        if (error && strcmp(error, "") != 0) {
            printf("> SDL ERROR: %s\n", error);
        }

        return !this->input.exitRequired;
    }
};

App Tetris_initApplication() {
    if(SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        throw std::runtime_error("Unable init SDL");
    }

    SDL_Window* window;
    SDL_Renderer* renderer;
    auto createWindow = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN, &window, &renderer);
    if(createWindow) {
        printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        throw std::runtime_error("Unable create Window");
    }
    SDL_SetWindowTitle(window, "TetrisSDL");
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    auto resources = loadResources(renderer);

    return App(window, renderer, std::move(resources));
}

void Tetris_closeApplication(App* app) {
    destroyResources(app->renderer, &app->resources);

    SDL_DestroyWindow(app->window);
    app->window = NULL;

    SDL_DestroyRenderer(app->renderer);
    app->renderer = NULL;

    SDL_Quit();
}
