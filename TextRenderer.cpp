//Awesome text renderer from Oscar Huang
/*
This file is based on these resources:
https://github.com/15-466/15-466-f21-base4
https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
https://www.freetype.org/freetype2/docs/tutorial/step1.html
https://learnopengl.com/In-Practice/Text-Rendering
https://github.com/GenBrg/MarryPrincess/blob/master/DrawFont.cpp
*/


#include "TextRenderer.hpp"

#define FONT_SIZE 24

TextRenderer::TextRenderer(std::string fontfile)
{
    // init & sanity check
    FT_Error ft_error;
    if ((ft_error = FT_Init_FreeType(&ft_library))) {
        std::cerr << "FT lib init fail\n";
        abort();
    }
    if ((ft_error = FT_New_Face(ft_library, fontfile.c_str(), 0, &ft_face))) {
        std::cerr << "FT face init fail\n";
        abort();
    }

    // font size we exctract from the face
    // if you want to extract bitmap at runtime, you can use this as a scaling method
    // But i choose to preload glyphs so I will set this here, and use a scale factor later to change size
    if ((ft_error = FT_Set_Char_Size(ft_face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0)))
        abort();

    // disable alignment since what we read from the face (font) is grey-scale
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // constrcut the glyph map for common ascii-key chars
    setupCharacterGlyphMap();

    // setup vao,vbo for quad
    bufferSetup();
}


void TextRenderer::draw(std::string text, float x, float y, glm::vec2 scale, glm::vec3 color) {

    // --------- Setup code for OpenGL, take from the original version of this renderer ----------
    // enable for text drawing
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // set the text rendering shaders
    glUseProgram(color_texture_program->program);

    // pass in uniforms
    glUniform3f(glGetUniformLocation(color_texture_program->program, "textColor"), color.x, color.y, color.z);
    // for the projection matrix we use orthognal
    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
    glUniformMatrix4fv(glGetUniformLocation(color_texture_program->program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // rendering buffers setup
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    //Initialize HarfBuzz, and use it to shape our text
    hb_font_t* hb_font;
    hb_font = hb_ft_font_create(ft_face, NULL);

    //Create hb - buffer and populate.
    hb_buffer_t * hb_buffer;
    hb_buffer = hb_buffer_create();
    hb_buffer_add_utf8(hb_buffer, text.c_str(), -1, 0, -1);
    hb_buffer_guess_segment_properties(hb_buffer);

    //Shape it!
    hb_shape(hb_font, hb_buffer, NULL, 0);

    //Get glyph information and positions out of the buffer.
    unsigned int len = hb_buffer_get_length(hb_buffer);
    hb_glyph_info_t* info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
    hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

    double current_x = 0;
    double current_y = 0;

    for (unsigned int i = 0; i < len; i++)
    {
        hb_codepoint_t gid = info[i].codepoint;
        unsigned int cluster = info[i].cluster;
        double x_advance = pos[i].x_advance / 64.;
        double y_advance = pos[i].y_advance / 64.;
        double x_offset = pos[i].x_offset / 64.;
        double y_offset = pos[i].y_offset / 64.;

        /*float xpos = x + (x_offset + ch.Bearing.x) * scale.x;
        float ypos = y + (y_offset - (ch.Size.y - ch.Bearing.y)) * scale.y;

        float x_pos = x + (float) current_x + pos[i].x_offset / 64.0f;
        float y_pos = y + (float) current_y + pos[i].y_offset / 64.0f;*/

        

        char glyphname[32];
        hb_font_get_glyph_name(hb_font, gid, glyphname, sizeof(glyphname));

        /*printf("glyph='%s'	cluster=%d	advance=(%g,%g)	offset=(%g,%g)\n",
            glyphname, cluster, x_advance, y_advance, x_offset, y_offset);*/

        FT_Error ft_error;

        if ((ft_error = FT_Load_Glyph(ft_face, info[i].codepoint, FT_LOAD_RENDER)))
        {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph with error: " << ft_error << std::endl;
        }

        float x_pos = x + (float)current_x + ft_face->glyph->bitmap_left;
        float y_pos = y + (float)current_y - (ft_face->glyph->bitmap.rows - ft_face->glyph->bitmap_top);

        /* { //Render out the bitmap
            FT_Bitmap const& bitmap = ft_face->glyph->bitmap;

            std::cout << "Bitmap (" << bitmap.width << "x" << bitmap.rows << "):\n";
            std::cout << "  pitch is " << bitmap.pitch << "\n";
            std::cout << "  pixel_mode is " << int32_t(bitmap.pixel_mode) << "; num_grays is " << bitmap.num_grays << "\n";
            if (bitmap.pixel_mode == FT_PIXEL_MODE_GRAY && bitmap.num_grays == 256 && bitmap.pitch >= 0) {
                for (uint32_t row = 0; row < bitmap.rows; ++row) {
                    std::cout << "   ";
                    for (uint32_t col = 0; col < bitmap.width; ++col) {
                        uint8_t val = bitmap.buffer[row * std::abs(bitmap.pitch) + col];
                        if (val < 128) std::cout << '.';
                        else std::cout << '#';
                    }
                    std::cout << '\n';
                }
            }
            else {
                std::cout << "  (bitmap is not FT_PIXEL_MODE_GRAY with 256 levels and upper-left origin, not dumping)" << "\n";
            }
            std::cout.flush();
        }*/

        glm::vec2 size = glm::ivec2(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows);
        glm::vec2 offset = glm::ivec2(ft_face->glyph->bitmap_left, ft_face->glyph->bitmap_top);

        /*printf("glyph='%s'	cluster=%d	advance=(%g,%g)	offset=(%g,%g)\n",
            glyphname, cluster, x_advance, y_advance, x_offset, y_offset);*/

        //Oscar's vertex code, modified to use harfbuzz
        // update VBO for each character (6 vertices to draw a quad, which holds a glyph)
        // the info for each vector is (pos_x, pox_y, texture_coord_x, texture_coord_y)
        // check my vertex shader at TextRenderProgram.cpp to see how it is used
        float vertices[6][4] = {
            { x_pos,             y_pos + size.y,  0.0f, 0.0f },
            { x_pos,             y_pos,           0.0f, 1.0f },
            { x_pos + size.x,    y_pos,           1.0f, 1.0f },

            { x_pos,             y_pos + size.y,  0.0f, 0.0f },
            { x_pos + size.x,    y_pos,           1.0f, 1.0f },
            { x_pos + size.x,    y_pos + size.y,  1.0f, 0.0f }
        };

        //Setup our texture!
        GLuint tex = 0;
        glGenTextures(1, &tex);

        //upload a 1x1 image of solid white to the texture:
        glBindTexture(GL_TEXTURE_2D, tex);
        // upload the bitmap to texure buffer
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            ft_face->glyph->bitmap.width,
            ft_face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            ft_face->glyph->bitmap.buffer
        );
        // set some texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


        // -- Render the character --
        
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // --Clean up the textures
        glDeleteTextures(1, &tex);
        tex = 0;

        current_x += pos[i].x_advance / 64.;
        current_y += pos[i].y_advance / 64.;
    }


    hb_buffer_destroy(hb_buffer);
    hb_font_destroy(hb_font);

    // unbind
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/*

void TextRenderer::draw(std::string text, float x, float y, glm::vec2 scale, glm::vec3 color) {
    // check if text diff
    if (text.compare(prevText) != 0) {
        prevText = text;
        changeTextContent();    // update shaping with harfbuzz
    }

    // enable for text drawing
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // set the text rendering shaders
    glUseProgram(color_texture_program->program);

    // pass in uniforms
    glUniform3f(glGetUniformLocation(color_texture_program->program, "textColor"), color.x, color.y, color.z);
    // for the projection matrix we use orthognal
    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
    glUniformMatrix4fv(glGetUniformLocation(color_texture_program->program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // rendering buffers setup
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // go thru all glyphs and render them
    uint16_t i = 0;

    for (char c : text)
    {
        // first get the hb shaping infos (offset & advance)
        float x_offset = pos[i].x_offset / 64.0f;
        float y_offset = pos[i].y_offset / 64.0f;
        float x_advance = pos[i].x_advance / 64.0f;
        float y_advance = pos[i].y_advance / 64.0f;

        

        // take out the glyph using char
        Glyph ch = CharacterGlyph[c];
        // calculate actual position
        float xpos = x + (x_offset + ch.Bearing.x) * scale.x;
        float ypos = y + (y_offset - (ch.Size.y - ch.Bearing.y)) * scale.y;
        float w = ch.Size.x * scale.x;
        float h = ch.Size.y * scale.y;

        // update VBO for each character (6 vertices to draw a quad, which holds a glyph)
        // the info for each vector is (pos_x, pox_y, texture_coord_x, texture_coord_y)
        // check my vertex shader at TextRenderProgram.cpp to see how it is used
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // advance to next graph, using the harfbuzz shaping info
        x += x_advance * scale.x;
        y += y_advance * scale.y;
        i++;

        
    }

    // unbind
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}*/

// called when the displaying text is changed, to use harfbuzz to reshape
// (This function is mainly based on: https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c)
void TextRenderer::changeTextContent() {

    // free previous resources
    if (hb_buffer)
        hb_buffer_destroy(hb_buffer);
    if (hb_font)
        hb_font_destroy(hb_font);

    // recreate hb resources
    hb_font = hb_ft_font_create(ft_face, NULL);
    /* Create hb-buffer and populate. */
    hb_buffer = hb_buffer_create();

    // reshape
    hb_buffer_add_utf8(hb_buffer, prevText.c_str(), -1, 0, -1);
    hb_buffer_guess_segment_properties(hb_buffer);
    hb_shape(hb_font, hb_buffer, NULL, 0);

    /* Get glyph information and positions out of the buffer. */
    info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
    pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);
}

// constrcut the glyph map for common ascii-key chars
void TextRenderer::setupCharacterGlyphMap() {
    // go over ascii key 32-126
    for (unsigned char c = 32; c < 127; c++) {
        // load character glyph (which contains bitmap)
        if (FT_Load_Char(ft_face, c, FT_LOAD_RENDER)) {
            std::cout << "Fail to load Glyph for: " << c << std::endl;
            continue;
        }
        // generate buffer
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // upload the bitmap to texure buffer
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            ft_face->glyph->bitmap.width,
            ft_face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            ft_face->glyph->bitmap.buffer
        );
        // set some texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // store into the map
        Glyph glyph = {
            texture,
            glm::ivec2(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows),
            glm::ivec2(ft_face->glyph->bitmap_left, ft_face->glyph->bitmap_top),
        };
        CharacterGlyph.insert(std::pair<char, Glyph>(c, glyph));
    }
}

// as it is
void TextRenderer::destroyCharacterGlyphMap() {
    for (unsigned char c = 0; c < 128; c++)
    {
        glDeleteTextures(1, &CharacterGlyph[c].textureID);
    }
}

void TextRenderer::bufferSetup() {
    // set up vao, vbo for the quad on which we render the glyph bitmap
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // each vertex has 4 flaot, and we need 6 vertices for a quad
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    // set attribute in location 0, which is the in-vertex for vertex shader. See the definition in (TextRenderProgram, line.13)
    glEnableVertexAttribArray(0);
    // tells vao how to read from buffer. (read 4 floats each time, which is one vertex)
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    // done
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}