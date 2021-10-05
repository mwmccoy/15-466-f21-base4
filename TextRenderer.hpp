/*
    Header file for Oscars text renderer. This is mostly unchanged, since my implementation
    of the renderer mirrors his in terms of the interface. More information about this
    in TextRenderer.cpp
*/


#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H  

#include <map>
#include <iostream>
#include <glm/ext.hpp>
//#include "glm/gtc/matrix_transform.hpp"
#include <math.h>
#include <hb.h>
#include <hb-ft.h>
#include "data_path.hpp"
#include "Scene.hpp"
#include "ColorTextureProgram.hpp"



// This class is the main class responsible for rendering text using a ttf file.
// 1. It uses FreeType to read the ttf font file, and extracts the glyphs from the face (font).
// 2. It uses harfbuzz to shape a user-input text so that we know how to render the glyphs.
// 3. And finally it uses OpenGL to render the glyphs.
// For better performance, durting init, i read all common-used glpyphs (ie. english letters)
//  from the face and convert these glyphs into bitmap and store in the buffer.
//  A map will store the links between char and the textureID, so that during runtime it directly renders the
//  bitmap in buffer using the textureID. 
class TextRenderer {

    // FreeType varibles
    FT_Face ft_face;
    FT_Library ft_library;
    // harfbuzz variables
    hb_glyph_info_t* info;
    hb_glyph_position_t* pos;
    hb_font_t* hb_font = nullptr;
    hb_buffer_t* hb_buffer = nullptr;

    // VAO, VBO for the quad (2 triangles/ 6 vertices) on which we render a glyph.
    GLuint VAO, VBO;

    // helper functions
    void bufferSetup(); // setup vao,vbo for quad

public:
    TextRenderer() = default;

    // constructor will init everything. it reads a ttf file
    TextRenderer(std::string filename);

    // draw a user-input text
    void draw(std::string text, float x, float y, glm::vec2 scale, glm::vec3 color);

    // destructor to free resources
    ~TextRenderer() {
        FT_Done_Face(ft_face);
        FT_Done_FreeType(ft_library);
        hb_buffer_destroy(hb_buffer);
        hb_font_destroy(hb_font);
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

};