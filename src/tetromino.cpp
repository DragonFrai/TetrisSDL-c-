#include <optional>
#include <stdexcept>
#include "shape_lines.cpp"

#define FIELD_W 10
#define FIELD_H 24


// =================================================
// [ Плитка (1 плитка, не фигура!!!) и поле плиток ]

enum TetroColor {
    red,
    orange,
    yellow,
    green,
    blue,
    violet,
    //
    white,
    black
};
const TetroColor BASE_TILES[6] = {red, orange, yellow, green, blue, violet };

// TODO: Add class
using TetroTile = TetroColor;

class TetroField {
private:
    std::optional<TetroColor> tiles[FIELD_W][FIELD_H];

public:
    std::optional<std::optional<TetroColor>*> get(int x, int y) {
        if (x < 0 || x >= FIELD_W) { return std::optional<std::optional<TetroColor>*>(); }
        if (y < 0 || y >= FIELD_H) { return std::optional<std::optional<TetroColor>*>(); }
        return std::optional<std::optional<TetroColor>*>(&this->tiles[x][y]);
    }

    std::optional<TetroColor>* getAssured(int x, int y) {
        auto r = this->get(x, y);
        if(r.has_value()) {
            return r.value();
        } else {
            throw std::out_of_range("Out of field range");
        }
    }

    void set(int x, int y, std::optional<TetroColor> tile) {
        auto memCell = this->getAssured(x, y);
        *memCell = tile;
    }

    void clear() {
        for (int x = 0; x < FIELD_W; x++) {
            for (int y = 0; y < FIELD_H; y++) {

            }
        }
    }

    void removeLine(int line) {
        for (int y = line; y > 0; y--) {
            for (int x = 0; x < FIELD_W; x++) {
                auto upper = this->getAssured(x, y - 1);
                this->set(x, y, *upper);
            }
        }

        for (int x = 0; x < FIELD_W; x++) {
            this->set(x, 0, std::nullopt);
        }
    }

    bool lineIsFull(int line) {
        bool isFull = true;
        for (int x = 0; x < FIELD_W; x++) {
            if (!this->getAssured(x, line)->has_value()) {
                isFull = false;
                break;
            }
        }
        return isFull;
    }

    bool lineIsEmpty(int line) {
        bool isEmpty = true;
        for (int x = 0; x < FIELD_W; x++) {
            if (this->getAssured(x, line)->has_value()) {
                isEmpty = false;
                break;
            }
        }
        return isEmpty;
    }

    int removeFullLines() {
        int removedLines = 0;
        for (int line = 0; line < FIELD_H; line++) {
            if (this->lineIsFull(line)) {
                this->removeLine(line);
                removedLines += 1;
            }
        }
        return removedLines;
    }

};

// ===================================
// [ Падающая фигура из нескольких плиток! ]

enum TetroShapeClass {
    I, L, J, T, S, Z, O,
    // Extra
    Dot
};

class TetroShapePrototype {
public:
    TetroShapeClass clazz;
    int variant;

    int tilesCount;
    int offsetsX[16];
    int offsetsY[16];
    TetroColor color;

    TetroShapePrototype(const TetroShapePrototype& other) {
        this->tilesCount = other.tilesCount;
        std::copy(std::begin(other.offsetsX), std::end(other.offsetsX), this->offsetsX);
        std::copy(std::begin(other.offsetsY), std::end(other.offsetsY), this->offsetsY);
        this->color = other.color;
        this->clazz = other.clazz;
        this->variant = other.variant;
    }

    TetroShapePrototype(TetroShapeClass clazz, int variant, TetroColor color) {
        const char* line;

        switch (clazz) {
            case TetroShapeClass::O: line = SHAPE_O; break;
            case TetroShapeClass::I:
                switch (variant % 2) {
                    case 0: line = SHAPE_I_0; break;
                    case 1: line = SHAPE_I_1; break;
                }
                break;
            case TetroShapeClass::T:
                switch (variant % 4) {
                    case 0: line = SHAPE_T_0; break;
                    case 1: line = SHAPE_T_1; break;
                    case 2: line = SHAPE_T_2; break;
                    case 3: line = SHAPE_T_3; break;
                }
                break;
            case TetroShapeClass::L:
                switch (variant % 4) {
                    case 0: line = SHAPE_L_0; break;
                    case 1: line = SHAPE_L_1; break;
                    case 2: line = SHAPE_L_2; break;
                    case 3: line = SHAPE_L_3; break;
                }
                break;
            case TetroShapeClass::J:
                switch (variant % 4) {
                    case 0: line = SHAPE_J_0; break;
                    case 1: line = SHAPE_J_1; break;
                    case 2: line = SHAPE_J_2; break;
                    case 3: line = SHAPE_J_3; break;
                }
                break;
            case TetroShapeClass::S:
                switch (variant % 2) {
                    case 0: line = SHAPE_S_0; break;
                    case 1: line = SHAPE_S_1; break;
                }
                break;
            case TetroShapeClass::Z:
                switch (variant % 2) {
                    case 0: line = SHAPE_Z_0; break;
                    case 1: line = SHAPE_Z_1; break;
                }
                break;
        }

        // init
        this->clazz = clazz;
        this->variant = variant;

        int c = 0;
        for (int i = 0; i < 16; i++) {
            int x = i % 4;
            int y = i / 4;
            if (line[i] == '#') {
                this->offsetsX[c] = x;
                this->offsetsY[c] = y;
                c += 1;
            }
        }
        if (c == 0) {
            printf("EMPTY SHAPE\n");
            exit(1);
        }

        this->color = color;
        this->tilesCount = c;
    }

    TetroShapePrototype rotated() {
        return TetroShapePrototype(this->clazz, this->variant + 1, this->color);
    }
};

class TetroActiveShape {
public:
    int x, y;
    TetroShapePrototype prototype;

    TetroActiveShape(const TetroActiveShape& other): x(other.x), y(other.y), prototype(other.prototype) {}
    TetroActiveShape(int x, int y, TetroShapePrototype prototype): x(x), y(y), prototype(prototype) {}

};
