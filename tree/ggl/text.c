
#include "glad.h"
#include <GLFW/glfw3.h>
#include <cglm/struct.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "text.h"
#include "ggl.h"
#include "util.h"

// lets go 128 bytes of ascii
ft_char charmap[128] = {0};
shader_pgm_id text_pgm;
vao text_vao;
vbo text_vbo;

void text_init(gg_context *g) {
    printf("Initializing text...\n");
    FT_Library ft;
    FT_Face face;
    if (FT_Init_FreeType(&ft))
        printf("Error initing freetype\n");

    if (FT_New_Face(ft, "Hack-Regular.ttf", 0, &face))
        printf("Error loading font\n");

    FT_Set_Pixel_Sizes(face, 0, 48);


    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            printf("Error loading char %d (%c)\n", c, c);
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        charmap[c] = (ft_char) {
            texture,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            face->glyph->bitmap_left,
            face->glyph->bitmap_top,
            face->glyph->advance.x
        };
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader_id text_vert = ggl_make_shader(g, slurp("shaders/text.vert"), GL_VERTEX_SHADER);
    shader_id text_frag = ggl_make_shader(g, slurp("shaders/text.frag"), GL_FRAGMENT_SHADER);
    text_pgm = ggl_make_shader_pgm(g, text_vert, text_frag);

    mat4s projection = glms_ortho(0, g->xres, 0, g->yres, 0,100);
    // upload the uniform as well
    glUseProgram(text_pgm);
    glUniformMatrix4fv(glGetUniformLocation(text_pgm, "projection"), 1, GL_FALSE, projection.raw[0]);


    // set up vao
    glGenVertexArrays(1, &text_vao);
    glGenBuffers(1, &text_vbo);
    glBindVertexArray(text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void draw_text(const char* text, float x, float y, text_style style) {
    
    // use shader
    glUseProgram(text_pgm);
    glUniform3f(glGetUniformLocation(text_pgm, "textColor"), style.colour.x, style.colour.y, style.colour.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(text_vao);
    
    // draw each char
    int i = 0;
    char c;
    while ((c = text[i++]) != '\0') {
        ft_char ch = charmap[(int)c];
        
        float xpos = x + ch.bearing_x * style.scale;
        float ypos = y - (ch.size_y - ch.bearing_y) * style.scale;

        float w = ch.size_x * style.scale;
        float h = ch.size_y * style.scale;

        // update vbo for each char
        float vertices[6][4] = {
            {xpos, ypos + h, 0, 0},
            {xpos, ypos, 0, 1},
            {xpos + w, ypos, 1, 1},

            {xpos, ypos + h, 0, 0},
            {xpos + w, ypos, 1, 1},
            {xpos + w, ypos + h, 1, 0}
        };
        
        // render glyph
        glBindTexture(GL_TEXTURE_2D, ch.texture);

        glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.advance >> 6) * style.scale;
        
    }

}