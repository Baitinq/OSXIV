/*
    OSXIV - Overly Simple X Image Viewer
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define DEBUG 0

int main(int argc, char** argv);
int render(double zoom, double move_x, double move_y);
int handle_events();
void cleanup();

#define DBG(fmt, ...) do { if (DEBUG) printf(fmt, __VA_ARGS__); } while (0)

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        fprintf(stderr, "Please provide a filename (or '-' to read from stdin).\n");
        return 1;
    }

    if(strcmp(argv[1], "-") && access(argv[1], F_OK ) == -1)
    {
        fprintf(stderr, "Filename doesnt exist.\n");
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    atexit(cleanup);

    char* windowtitle = malloc(sizeof(char) * (9 + strlen(strcmp(argv[1], "-") ? argv[1] : "stdin"))); // +9 bcz "OSXIV - "
    sprintf(windowtitle, "OSXIV - %s", strcmp(argv[1], "-") ? argv[1] : "stdin");

    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);

    window = SDL_CreateWindow(
            windowtitle,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            dm.w / 2, //w
            dm.h / 2, //h
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );

    free(windowtitle);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);

    SDL_RWops* img;
    if(!strcmp(argv[1], "-"))
        img = SDL_RWFromFP(stdin, 0);
    else
        img = SDL_RWFromFile(argv[1], "rb");
    texture = IMG_LoadTexture_RW(renderer, img, 0);
    if(!texture)
    {
        SDL_FreeRW(img);
        fprintf(stderr, "Problem while loading the image: (%s).\n", argv[1]);
        return 1;
    }

    SDL_FreeRW(img);

    if(handle_events())
    {
        fprintf(stderr, "Event handling exception.\n");
        return 1;
    }

    return 0;
}

int render(double zoom, double move_x, double move_y)
{
    SDL_Rect src = {0};
    SDL_Rect dst = {0};
    int screen_h, screen_w;

    SDL_RenderClear(renderer);
    SDL_QueryTexture(texture, NULL, NULL, &src.w , &src.h);

    DBG("\nsrc w: %d h: %d x: %d y: %d\n", src.w, src.h, src.x, src.y);

    SDL_GetWindowSize(window, &dst.w, &dst.h);
    screen_h = dst.h;
    screen_w = dst.w;

    DBG("scr w: %d h: %d\n", screen_w, screen_h);

    /* APPLY ZOOM */
    dst.h *= zoom;
    dst.w *= zoom;

    DBG("zoom: %f\n", zoom);

    /* REVERT IMAGE FILL */
    double ratio = (double) src.w / src.h;
    if(dst.h >= dst.w / ratio)
        dst.h = dst.w / ratio;
    else
        dst.w = ratio * dst.h;

    /* CENTER IN SCREEN */
    dst.x = (screen_w - dst.w) / 2;
    dst.y = (screen_h - dst.h) / 2;

    DBG("dst w: %d h: %d x: %d y: %d\n\n", dst.w, dst.h, dst.x, dst.y);

    /* APPLY MOVE */
    dst.x -= move_x * dst.w;
    dst.y -= move_y * dst.h;

    DBG("Moved by x_perc: %f y_perc: %f\n", move_x, move_y);

    SDL_RenderCopy(renderer, texture, &src, &dst);
    SDL_RenderPresent(renderer);

    return 0;
}

int handle_events()
{
    SDL_Event event;

    double zoom_multiplier = 0.1; // 10%
    double move_multipler = 0.1; // 10%

    double zoom = 1.0; // 100% initially
    double move_x = 0.0; // initial x pos (centered)
    double move_y = 0.0; // initial y pos (centered)

    while(SDL_WaitEvent(&event))
    {
        //DBG("Event: 0x%x\n", event.type);
        switch(event.type)
        {
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym)
                {
                    case '+': // zoom in
                    case 61: // +
                        zoom += zoom_multiplier;
                        render(zoom, move_x, move_y);
                        break;
                    case '-': // zoom out
                        zoom -= zoom - zoom_multiplier < 0 ? 0 : zoom_multiplier;
                        render(zoom, move_x, move_y);
                        break;
                    case 1073741906: // up arrow - move up
                        move_y -= move_multipler / zoom;
                        render(zoom, move_x, move_y);
                        break;
                    case 1073741905: // down arrow - move down
                        move_y += move_multipler / zoom;
                        render(zoom, move_x, move_y);
                        break;
                    case 1073741904: // left arrow - move left
                        move_x -= move_multipler / zoom;
                        render(zoom, move_x, move_y);
                        break;
                    case 1073741903: // right arrow - move right
                        move_x += move_multipler / zoom;
                        render(zoom, move_x, move_y);
                        break;
                    case 'q':
                        exit(0);
                }
                break;
             case SDL_WINDOWEVENT:
                render(zoom, move_x, move_y);
                break;
            case SDL_QUIT:
                exit(0);
        }
    }

    return 1;
}

void cleanup()
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}
