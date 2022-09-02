#define SPXE_APPLICATION
#include <razor.h>
#include <stdlib.h>
#include <stdio.h>

void plot(Px* dest, const Px src, const int x, const int y)
{
    memcpy(dest + ((y * spxe.scrres.width) + x), &src, sizeof(Px));
}

int main(const int argc, char** argv)
{
    int width = 200, height = 150, mousex, mousey;
    for (int i = 1; i < argc - 1; ++i) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'w') {
                width = atoi(argv[++i]);
            }
            else if (argv[i][1] == 'h') {
                height = atoi(argv[++i]);
            }
        }
    }

    //bmp_t bmp = bmp_load("image.png");
    bmp_t bmp = bmp_new(2, 2, 4);
    bmp.pixels[0] = 255, bmp.pixels[1] = 0, bmp.pixels[2] = 0, bmp.pixels[3] = 255;
    bmp.pixels[4] = 0, bmp.pixels[5] = 255, bmp.pixels[6] = 0, bmp.pixels[7] = 255;
    bmp.pixels[8] = 0, bmp.pixels[9] = 0, bmp.pixels[10] = 255, bmp.pixels[11] = 255;
    bmp.pixels[12] = 255, bmp.pixels[13] = 255, bmp.pixels[14] = 255,
    bmp.pixels[15] = 255;

    Px* pixbuf = spxeStart("razor", 800, 600, width, height);
    if (!pixbuf) {
        printf("razor could not initiate spxe pixel engine.\n");
        return EXIT_FAILURE;
    }

    while (spxeRun(pixbuf)) {
        
        spxeMousePos(&mousex, &mousey);
        if (spxeKeyPressed(ESCAPE)) {
            break;
        }

        Vertex tri[] = {
            {{10.0, 10.0}, {0.0, 0.0}},
            {{10.0, 100.0}, {0.0, 1.0}},
            {{100.0, 100.0}, {1.0, 1.0}},
            {{10.0, 10.0}, {0.0, 0.0}},
            {{100.0, 100.0}, {1.0, 1.0}},
            {{100.0, 10.0}, {1.0, 0.0}}
        };

        if (mousex >= 0 && mousey >= 0 && mousex < width && mousey < height) {
            tri[0].pos.x = mousex;
            tri[0].pos.y = mousey;
            tri[3].pos.x = mousex;
            tri[3].pos.y = mousey;
        }
        
        memset(pixbuf, 155, width * height * sizeof(Px));
        rasterize(pixbuf, &bmp, tri);
        rasterize(pixbuf, &bmp, tri + 3);
    }

    bmp_free(&bmp);
    return spxeEnd(pixbuf);
}

