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


	Room startRoom("start", "Uh oh, you say, as you fall out of the airplane.");
	startRoom.addOption(Option("Flap?", {}, {}, "flap"));
	startRoom.addOption(Option("Think, like, really hard for a second.", {}, {}, "thinking"));

	Room falling("falling", "You are falling through the air.");
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
	veil.addOption(Option("That's it. I'm the chosen one!, you think to yourself before...", {}, {}, "over"));

	Room safeDeath("safeDeath", "You parachute rips and tears apart under the weight.");
	safeDeath.addOption(Option("Wait, am I holding a safe?", {}, {}, "falling"));


	Room flap("flap", "You start flapping your arms furiously! It's not working.");
	flap.addOption(Option("Flap really, really well", {}, {}, "over"));
	flap.addOption(Option("Fall instead", {}, {}, "over"));

	Room win("win", "You float gently to the ground. Congratulations! You win!");




	Room gameOver("over", "Splat! You hit the ground. Game over!");

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


	//---- actual drawing ----

	//clear the color buffer:
	
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Room room = rooms[activeRoom];

	textRender->draw(room.text, 50.0f, 500.0f, glm::vec2(1, 1), glm::vec3(248.0f / 255, 222.0f / 255, 126.0f / 255));
	for (int i = 0; i < room.numActiveOptions(&resources); i++)
	{
		//std::cout << "iteration " << i << std::endl;
		textRender->draw(room.getOption(i), 50.0f, 400.0f - 100.f * i, glm::vec2(1, 1), glm::vec3(248.0f / 255, 222.0f / 255, 126.0f / 255));
	}

	// bg color
	glClearColor(135.0f / 255, 206.0f / 255, 235.0f / 255, 1.0f);
	

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
