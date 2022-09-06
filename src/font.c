#include <razor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define DICTSIZE 128

RZfont* rzFontLoad(const char* path, const unsigned int size)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        printf("razor could not initiate freetype font library.\n");
        return NULL;
    } 
	
    FT_Face face;
    if (FT_New_Face(ft, path, 0, &face)) {
        printf("razor failed to load font file '%s'.\n", path);
        return NULL;
    }

    FT_Set_Pixel_Sizes(face, 0, size);
    RZfont* font = malloc(sizeof(RZfont) * DICTSIZE);

    for (unsigned char c = 0; c < DICTSIZE; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            printf("razor failed to load freetype glyph of character '%c'.'%s'\n", c, path);
            continue;
        }

        font[c] = (RZfont){
            malloc(face->glyph->bitmap.width * face->glyph->bitmap.rows),
            {face->glyph->bitmap.width, face->glyph->bitmap.rows},
            {face->glyph->bitmap_left, face->glyph->bitmap_top},
            (unsigned int)face->glyph->advance.x
        };
        
        memcpy(font[c].pixmap, face->glyph->bitmap.buffer, face->glyph->bitmap.width * face->glyph->bitmap.rows);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return font;
}

void rzFontDrawChar(bmp_t* bmp, const RZfont font, const Px color, const ivec2 pos)
{
    const int xend = pos.x + font.size.x;
    const int yend = pos.y + font.size.y;
    for (int y = pos.y; y < yend; ++y) {
        for (int x = pos.x; x < xend; ++x) {
            Px* p = (Px*)bmp->pixels + y * bmp->width + x;
            unsigned char c = font.pixmap[(font.size.y - 1 - (y - pos.y)) * font.size.x + (x - pos.x)];
            *p = pxlerp(p[0], color, (float)c / 255.0);
        }
    }
}

void rzFontDrawText(bmp_t* bmp, const RZfont* font, const char* text, const Px color, ivec2 pos)
{
    for (int i = 0; text[i]; i++) {
        rzFontDrawChar(bmp, font[(int)text[i]], color, pos);
        pos.x += (font[(int)text[i]].advance >> 6);
    }
}

void rzFontFree(RZfont* font)
{
    if (font) {
        for (int i = 0; i < DICTSIZE; ++i) {
            free(font[i].pixmap);
        }
        free(font);
    }
}
