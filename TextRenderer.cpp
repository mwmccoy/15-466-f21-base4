/*
    Awesome text renderer from Oscar Huang, of which the drawing code has been mostly rewritten.
    A good bit of his OpenGL formatting and setup is still here, though!
    
    I struggled a lot to get the pipeline initially working, so I used Oscars to
    develop a game, and then came back and rewrote the drawing code.

    The original stored some characters in an array, and exclusively used those characters.
    This one shapes and reloads all the textures every time draw is called. This is needlessly
    ineffecient, but nice from a debugging and pipeline viewpoint. I did this both because
    it was easier and because it let me try out the full pipeline.

    All credit to Oscar for this, his pipeline was hugely instructional and a big
    portion of why this works now.
*/






// -- Comment from original --
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

//Setup code is mostly unchanged from the original
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


    // setup vao,vbo for quad
    bufferSetup();
}

//The draw function that renders text out on the screen
//This one has been mostly rewritten, so that it uses harfbuzz and freetype on every frame. It still
//makes use of some of the orginal rendering code, though, mostly in terms of OpenGl calls.
//It also has a good bit of code from the harfbuzz tutorial: https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
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

        glm::vec2 size = glm::ivec2(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows);

        //Oscar's vertex code, modified to use harfbuzz coordinate things instead
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

        //Setup our texture! We do this for every character
        GLuint tex = 0;
        glGenTextures(1, &tex);

        //Bind our texture and load the bitmap
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

        // set some texture options -- VERY IMPORTANT, NOT HAVING THIS BREAKS EVERYTHING
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


        // -- Render the character -- uses mostly same code as oscar's original
        
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

//Buffer setup from Oscars original program. Rewriting this didn't seem worth it, since it's mostly boilerplate
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