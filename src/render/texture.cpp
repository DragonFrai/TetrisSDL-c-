#include "../utils.cpp"


SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path, int* width, int* height) {
    auto surface = SDL_LoadBMP(path);
    if (surface == NULL) {
        printf("Unable load textureHandle: %s!\nSDL Error: %s\n", path, SDL_GetError());
        throw std::runtime_error("Error on load textureHandle");
    }

    *width = surface->w;
    *height = surface->h;

    auto texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        printf("Unable create textureHandle: %s!\nSDL Error: %s\n", path, SDL_GetError());
        throw std::runtime_error("Error on creating textureHandle");
    }

    SDL_FreeSurface(surface);

    return texture;
}


class Texture {
private:
    SDL_Texture* textureHandle;
    const char* sourcePath;
    int sizeWidth;
    int sizeHeight;

public:
    Texture(SDL_Texture* texture, int width, int height) {
        printf("Texture <no_path> created.\n");

        {
            this->textureHandle = texture;
            this->sourcePath = NULL;
            this->sizeWidth = width;
            this->sizeHeight = height;
        }
    }

    Texture(SDL_Renderer* renderer, const char path[]) {
        int w = 0;
        int h = 0;
        auto t = loadTexture(renderer, path, &w, &h);
        printf("Texture %s created.\n", path);

        {
            this->textureHandle = t;
            this->sizeWidth = w;
            this->sizeHeight = h;
            this->sourcePath = copyStr(path);
        }
    }

    ~Texture() {
        if (this->textureHandle != NULL) {
            auto path = "<no_path>";
            if(this->sourcePath != NULL) { path = this->sourcePath; }
            printf("Texture %s deleted.\n", path);
            SDL_DestroyTexture(this->textureHandle);
        }

        {
            this->textureHandle = NULL;
            this->sizeWidth = 0;
            this->sizeHeight = 0;
            this->sourcePath = NULL;
        }
    }

    // Disable copy
    Texture(const Texture&) = delete;
    Texture(Texture&& other):
        textureHandle(other.textureHandle),
        sourcePath(other.sourcePath),
        sizeWidth(other.sizeWidth),
        sizeHeight(other.sizeHeight)
    {
        other.textureHandle = NULL;
        other.sourcePath = NULL;
        other.sizeWidth = 0;
        other.sizeHeight = 0;
    };

    SDL_Texture* sldHandle() {
        return this->textureHandle;
    }

    int width() {
        return this->sizeWidth;
    }

    int height() {
        return this->sizeHeight;
    }

    SDL_Rect rect() {
        return SDL_Rect { 0, 0, this->sizeWidth, this->sizeHeight };
    }
};
