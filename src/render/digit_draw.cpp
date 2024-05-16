//#include <SDL.h>
//#include "texture.cpp"

#define DIGIT_TEX_W 24
#define DIGIT_TEX_H 32

void drawNumber(SDL_Renderer* renderer, Texture* texture, int x, int y, int number, int length) {
    if (length > 10) {
        printf("Invalid number length");
        exit(1);
    }

    int w = texture->width();
    int h = texture->height();

    int digits[10]; // reversed sequence

    int digitsCount = 0;
    {
        int n = number;
        int i = 0;
        while (n > 0) {
            digits[i] = n % 10;
            n = n / 10;
            i += 1;
        }
        for (int j = i; j < length; j++) { digits[j] = 0; }
        digitsCount = i > length ? i : length;
    }

    int dx = 0;
    for (int i = 0; i < digitsCount; i++) {
        int j = digitsCount - i - 1;
        int d = digits[j];

        int texX = DIGIT_TEX_W * d;
        int texY = 0;

        SDL_Rect srcRect = SDL_Rect { texX, texY, DIGIT_TEX_W, DIGIT_TEX_H };
        SDL_Rect dstRect = SDL_Rect { x + dx, y, DIGIT_TEX_W, DIGIT_TEX_H };

        SDL_RenderCopy(renderer, texture->sldHandle(), &srcRect, &dstRect);

        dx += DIGIT_TEX_W;
    }
}
