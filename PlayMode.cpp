#include "PlayMode.hpp"
#include "TextRenderer.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

#define FONT_SIZE 20
#define MARGIN (FONT_SIZE * .5)

PlayMode::PlayMode() {

	const char* fontfile = "Raleway-Black.ttf";
	textRender = new TextRenderer(data_path(fontfile));
	toPrint = "This is a test! Please show up :(";

	//Initialize starting resources
	std::list<std::string> startResources = { "safe", "norememberchute"};
	for (std::string s : startResources)
	{
		resources.push_back(s);
	}


	Room startRoom("start", "'Uh oh', you say, as you fall out of the airplane");
	startRoom.addOption(Option("Flap?", {}, {}, "flap"));
	startRoom.addOption(Option("Think, like, really hard for a second.", {}, {}, "thinking"));

	Room falling("falling", "You are falling through the air");
	falling.addOption(Option("Flap?", {}, {}, "flap"));
	falling.addOption(Option("Think, like, really hard for a second.", {}, {}, "thinking"));
	falling.addOption(Option("Pull your chute!", { "chute", "safe" }, { "nochute" }, "safeDeath"));
	falling.addOption(Option("Pull your chute!", { "chute", "nosafe" }, { "nochute" }, "win"));

	Room think("thinking", "How hard are you thinking?");
	think.addOption(Option("Sort of hard", { "norememberchute" }, { "chute" }, "remembered"));
	think.addOption(Option("Really hard", { "safe" }, { "nosafe" }, "dropsafe"));
	think.addOption(Option("Enough to see through the veil", {}, {}, "veil"));

	Room dropSafe("dropsafe", "You drop that safe you were holding.");
	dropSafe.addOption(Option("Phew, why was I holding that!?", {}, {}, "falling"));

	Room remembered("remembered", "You just remembered that you're wearing a chute!");
	remembered.addOption(Option("Oh, cool! That seems helpful.", {}, {}, "falling"));

	Room veil("veil", "You think incredibly hard. Suddenly, it's clear.");
	veil.addOption(Option("'That's it. I'm the chosen one....', you think to yourself before", {}, {}, "over"));

	Room safeDeath("safeDeath", "You parachute rips and tears apart under the weight.");
	safeDeath.addOption(Option("Wait, am I holding a safe?", {}, {}, "falling"));


	Room flap("flap", "You start flapping your arms furiously! It doesn't seem to be working well.");
	flap.addOption(Option("Flap really, really well", {}, {}, "over"));
	flap.addOption(Option("Fall instead", {}, {}, "over"));

	Room win("win", "You float gently to the ground. Congratulations! You win!");




	Room gameOver("over", "Splat! You hit the ground");

	rooms.push_back(startRoom);
	rooms.push_back(falling);
	rooms.push_back(think);
	rooms.push_back(dropSafe);
	rooms.push_back(remembered);
	rooms.push_back(veil);
	rooms.push_back(safeDeath);
	rooms.push_back(flap);
	rooms.push_back(win);
	rooms.push_back(gameOver);

	oneDown = false;
	twoDown = false;
	threeDown = false;
	fourDown = false;
	fiveDown = false;


	/*
	FT_Face face;
	FT_Library ft;

	//Start by doing some Font testing!
	{ //Harfbuzz testing

		// Initialize FreeType and create FreeType font face.
		FT_Library ft_library;
		FT_Face ft_face;
		
		FT_Error ft_error;

		if ((ft_error = FT_Init_FreeType(&ft_library)))
			abort();
		if ((ft_error = FT_New_Face(ft_library, fontfile, 0, &ft_face)))
			abort();
		if ((ft_error = FT_Set_Char_Size(ft_face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0)))
			abort();

		//Create hb-ft font.
		hb_font_t* hb_font;
		hb_font = hb_ft_font_create(ft_face, NULL);

		Create hb-buffer and populate.
		hb_buffer_t* hb_buffer;
		hb_buffer = hb_buffer_create();
		hb_buffer_add_utf8(hb_buffer, text, -1, 0, -1);
		hb_buffer_guess_segment_properties(hb_buffer);

		//Shape it!
		hb_shape(hb_font, hb_buffer, NULL, 0);

		//Get glyph information and positions out of the buffer.
		unsigned int len = hb_buffer_get_length(hb_buffer);
		hb_glyph_info_t* info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
		hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

		//Print them out as is.
		printf("Raw buffer contents:\n");
		for (unsigned int i = 0; i < len; i++)
		{
			hb_codepoint_t gid = info[i].codepoint;
			unsigned int cluster = info[i].cluster;
			double x_advance = pos[i].x_advance / 64.;
			double y_advance = pos[i].y_advance / 64.;
			double x_offset = pos[i].x_offset / 64.;
			double y_offset = pos[i].y_offset / 64.;

			char glyphname[32];
			hb_font_get_glyph_name(hb_font, gid, glyphname, sizeof(glyphname));

			printf("glyph='%s'	cluster=%d	advance=(%g,%g)	offset=(%g,%g)\n",
				glyphname, cluster, x_advance, y_advance, x_offset, y_offset);
		}

		printf("Converted to absolute positions:\n");
		//And converted to absolute positions.
		{
			double current_x = 0;
			double current_y = 0;
			for (unsigned int i = 0; i < len; i++)
			{
				hb_codepoint_t gid = info[i].codepoint;
				unsigned int cluster = info[i].cluster;
				double x_position = current_x + pos[i].x_offset / 64.;
				double y_position = current_y + pos[i].y_offset / 64.;


				char glyphname[32];
				hb_font_get_glyph_name(hb_font, gid, glyphname, sizeof(glyphname));

				printf("glyph='%s'	cluster=%d	position=(%g,%g)\n",
					glyphname, cluster, x_position, y_position);

				current_x += pos[i].x_advance / 64.;
				current_y += pos[i].y_advance / 64.;
			}
		}


		hb_buffer_destroy(hb_buffer);
		hb_font_destroy(hb_font);

		FT_Done_Face(ft_face);
		FT_Done_FreeType(ft_library);
	}

	{
		//George's code from the discord, for testing purposes
		
		if (FT_Init_FreeType(&ft))
		{
			std::cout << "ERROR::FREETYPE:: Could not init FreeType Library " << std::endl;
			
		}

		
		if (FT_New_Face(ft, fontfile, 0, &face))
		{
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
			
		}

		FT_Error ft_error;

		if ((ft_error = FT_Set_Char_Size(face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0)))
			abort();

		int errCode = FT_Load_Char(face, 'k', FT_LOAD_RENDER);
		if (errCode)
		{
			std::cout << "ERROR::FREETYPE: Failed to load Glyph with error: " << errCode << std::endl;
			
		}

		{ //Render out the bitmap
			FT_Bitmap const& bitmap = face->glyph->bitmap;

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
		}

	}
	
	//----- allocate OpenGL resources -----
	{ //vertex buffer:
		glGenBuffers(1, &vertex_buffer);
		//for now, buffer will be un-filled.

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //vertex array mapping buffer for color_texture_program:
		//ask OpenGL to fill vertex_buffer_for_color_texture_program with the name of an unused vertex array object:
		glGenVertexArrays(1, &vertex_buffer_for_color_texture_program);

		//set vertex_buffer_for_color_texture_program as the current vertex array object:
		glBindVertexArray(vertex_buffer_for_color_texture_program);

		//set vertex_buffer as the source of glVertexAttribPointer() commands:
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		//set up the vertex array object to describe arrays of PongMode::Vertex:
		glVertexAttribPointer(
			color_texture_program.Position_vec4, //attribute
			3, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 0 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Position_vec4);
		//[Note that it is okay to bind a vec3 input to a vec4 attribute -- the w component will be filled with 1.0 automatically]

		glVertexAttribPointer(
			color_texture_program.Color_vec4, //attribute
			4, //size
			GL_UNSIGNED_BYTE, //type
			GL_TRUE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 4*3 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Color_vec4);

		glVertexAttribPointer(
			color_texture_program.TexCoord_vec2, //attribute
			2, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 4*3 + 4*1 //offset
		);
		glEnableVertexAttribArray(color_texture_program.TexCoord_vec2);

		//done referring to vertex_buffer, so unbind it:
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//done setting up vertex array object, so unbind it:
		glBindVertexArray(0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //solid white texture:
		//ask OpenGL to fill white_tex with the name of an unused texture object:
		glGenTextures(1, &white_tex);

		//bind that texture object as a GL_TEXTURE_2D-type texture:
		glBindTexture(GL_TEXTURE_2D, white_tex);

		//upload a 1x1 image of solid white to the texture:
		FT_Bitmap const& bitmap = face->glyph->bitmap;
		glm::uvec2 size = glm::uvec2(bitmap.width, bitmap.rows);
		std::vector< glm::u8vec4 > data(size.x*size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		if (bitmap.pixel_mode == FT_PIXEL_MODE_GRAY && bitmap.num_grays == 256 && bitmap.pitch >= 0) {
			for (uint32_t row = 0; row < bitmap.rows; ++row) {
				std::cout << "   ";
				for (uint32_t col = 0; col < bitmap.width; ++col) {
					int val = (int) bitmap.buffer[row * std::abs(bitmap.pitch) + col];
					glm::u8vec4 new_color(val, val, val, val);
					data[row * std::abs(bitmap.pitch) + col] = new_color;
					std::cout << val << std::endl;
				}
			}
		}
		else {
			std::cout << "  (bitmap is not FT_PIXEL_MODE_GRAY with 256 levels and upper-left origin, not dumping)" << "\n";
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		//set filtering and wrapping parameters:
		//(it's a bit silly to mipmap a 1x1 texture, but I'm doing it because you may want to use this code to load different sizes of texture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//since texture uses a mipmap and we haven't uploaded one, instruct opengl to make one for us:
		glGenerateMipmap(GL_TEXTURE_2D);

		//Okay, texture uploaded, can unbind it:
		glBindTexture(GL_TEXTURE_2D, 0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	*/
}

PlayMode::~PlayMode() {

}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_MOUSEMOTION) {
		//convert mouse from window pixels (top-left origin, +y is down) to clip space ([-1,1]x[-1,1], +y is up):
		glm::vec2 clip_mouse = glm::vec2(
			(evt.motion.x + 0.5f) / window_size.x * 2.0f - 1.0f,
			(evt.motion.y + 0.5f) / window_size.y *-2.0f + 1.0f
		);
		left_paddle.y = (clip_to_court * glm::vec3(clip_mouse, 1.0f)).y;
	}
	else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_SPACE)
	{
		//addBall(glm::vec2(left_paddle.x + .1f, left_paddle.y), glm::vec2(1.0f, 0.0f));
	}
	else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_1)
	{
		oneDown = true;
	}
	else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_2)
	{
		twoDown = true;
	}
	else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_3)
	{
		threeDown = true;
	}
	else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_4)
	{
		fourDown = true;
	}
	else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_5)
	{
		fiveDown = true;
	}

	return false;
}

void PlayMode::update(float elapsed) {


	int numRooms = rooms[activeRoom].numActiveOptions(&resources);

	//Good for end of game state
	/*
	if (numRooms == 0)
	{
		std::cout << "You need to fix this! Room " << numRooms << "has no valid options" << std::endl;
	}*/

	if (oneDown && numRooms >= 1)
	{
		moveRoom(0);
		
	}
	else if (twoDown && numRooms >= 2)
	{
		moveRoom(1);
	}
	else if (threeDown && numRooms >= 3)
	{
		moveRoom(2);
	}
	else if (fourDown && numRooms >= 4)
	{
		moveRoom(3);
	}
	else if (fiveDown && numRooms >= 5)
	{
		moveRoom(4);
	}


	oneDown = false;
	twoDown = false;
	threeDown = false;
	fourDown = false;
	fiveDown = false;
}

void PlayMode::moveRoom(int opt)
{
	//std::cout << "Evaluating " << opt << std::endl;
	std::string newRoom = rooms[activeRoom].EvaluateOption(opt, &resources);

	int index = 0;

	//std::cout << "Looking for " << newRoom << std::endl;
	for (Room r : rooms)
	{
		if (r.name.compare(newRoom) == 0)
		{
			activeRoom = index;
			return;
		}
		index++;
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//some nice colors from the course web page:
	#define HEX_TO_U8VEC4( HX ) (glm::u8vec4( (HX >> 24) & 0xff, (HX >> 16) & 0xff, (HX >> 8) & 0xff, (HX) & 0xff ))
	const glm::u8vec4 bg_color = HEX_TO_U8VEC4(0x193b59ff);
	const glm::u8vec4 fg_color = HEX_TO_U8VEC4(0xf2d2b6ff);
	const glm::u8vec4 shadow_color = HEX_TO_U8VEC4(0xf2ad94ff);
	const std::vector< glm::u8vec4 > trail_colors = {
		HEX_TO_U8VEC4(0xf2ad9488),
		HEX_TO_U8VEC4(0xf2897288),
		HEX_TO_U8VEC4(0xbacac088),
	};
	#undef HEX_TO_U8VEC4


	//---- actual drawing ----

	//clear the color buffer:
	
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Room room = rooms[activeRoom];

	textRender->draw(room.text, 50.0f, 500.0f, glm::vec2(1, 1), glm::vec3(250.0f / 255, 192.0f / 255, 137.0f / 255));
	for (int i = 0; i < room.numActiveOptions(&resources); i++)
	{
		//std::cout << "iteration " << i << std::endl;
		textRender->draw(room.getOption(i), 50.0f, 400.0f - 100.f * i, glm::vec2(1, 1), glm::vec3(250.0f / 255, 192.0f / 255, 137.0f / 255));
	}

	// bg color
	glClearColor(174.0f / 255, 136.0f / 255, 184.0f / 255, 1.0f);
	

	GL_ERRORS(); //PARANOIA: print errors just in case we did something wrong.

}
/*
void PongMode::addBall(glm::vec2 position, glm::vec2 velocity)
{
	PongBall ball = PongBall();
	ball.ball = position;
	ball.ball_velocity = velocity;

	ball.ball_trail.clear();
	ball.ball_trail.emplace_back(ball.ball, ball.trail_length);
	ball.ball_trail.emplace_back(ball.ball, 0.0f);

	balls.push_back(ball);
}*/
