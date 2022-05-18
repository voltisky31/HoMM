#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <stdlib.h>
#define SDL_MAIN_HANDLED
#include "SDL2-2.0.20/include/SDL.h"
#include "SDL2-2.0.20/include/SDL_image.h"

/*
INSTRUCTIONS:
Left Mouse Click - Setting position
Right Mouse Click - Reset / Skip tour  < Use if neccesary (program is still a little bit buggy sometimes);

TODO:
- Implement better pathfinding (this one does have some difficulties, especialy going form right to left)
- Make that speed of characters is not dependent on FPS

*/

typedef unsigned char uchar;

// Debug mode config
int debug = 2; // 0 - none, 1 - on click, 2 - constant

// Screen resolution
const int screenWidth = 1920;
const int screenHeight = 1080;

// Battleground tiles count
const int battlegroundColumns = 15; //Battleground width
const int battlegroundRows = 11; //Battleground height

// Battleground tiles + usa-mexican border
const int battlegroundColumnsWithBorder = battlegroundColumns + 2;
const int battlegroundRowsWithBorder = battlegroundRows + 2;

// Cell pixel dimensions
const int cellWidthInPixels = screenWidth / battlegroundColumns;
const int cellHeightInPixels = screenHeight / battlegroundRows;

// Wait amount
const int wait = 100;

// Seed settings
const int seed = time(nullptr);

// Battleground config
uchar battleground[battlegroundRowsWithBorder][battlegroundColumnsWithBorder];

//For later SDL "compression" into function
SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath);

void ResetBattleground()
{
	for (int i = 0; i < battlegroundRowsWithBorder; i++)
	{
		for (int j = 0; j < battlegroundColumnsWithBorder; j++)
		{
			if (battleground[i][j] != 255)
			{
				battleground[i][j] = 0;
			}
		}
	}
}

void SetBattlegroundBorder()
{
	for (int i = 0; i < battlegroundColumnsWithBorder; i++)
	{
		battleground[0][i] = 255;
	}
	for (int i = 0; i < battlegroundRowsWithBorder; i++)
	{
		battleground[i][0] = 255;
	}
	for (int i = 0; i < battlegroundColumnsWithBorder; i++)
	{
		battleground[battlegroundRowsWithBorder - 1][i] = 255;
	}
	for (int i = 0; i < battlegroundRowsWithBorder; i++)
	{
		battleground[i][battlegroundColumnsWithBorder - 1] = 255;
	}
}

void PrintArray()
{
	for (int i = 0; i < battlegroundRowsWithBorder; i++)
	{
		for (int j = 0; j < battlegroundColumnsWithBorder; j++)
		{
			printf("%d  ", battleground[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

// Grassfire (kinda?)
void Grassfire()
{
	bool gf_run = true;

	while (gf_run)
	{
		gf_run = false;
		for (int i = 1; i < battlegroundRowsWithBorder - 1; i++)
		{
			for (int j = 1; j < battlegroundColumnsWithBorder - 1; j++)
			{
				uchar a = battleground[i][j];

				if (battleground[i][j] != 0 && battleground[i][j] != 255)
				{
					uchar b = a + 1;
					uchar arrayDown = battleground[i + 1][j];
					uchar arrayRight = battleground[i][j + 1];

					if (i < battlegroundRows && (arrayDown != 255 && arrayDown < b))
					{
						if (battleground[i + 1][j] == 0)
						{
							battleground[i + 1][j] = b;
							gf_run = true;
						};
					}
					if (j < battlegroundColumns && (arrayRight != 255 && arrayRight < b))
					{
						if (battleground[i][j + 1] == 0)
						{
							battleground[i][j + 1] = b;
							gf_run = true;
						};
					}
				}
			}
		}
		for (int i = battlegroundRowsWithBorder - 1; i > 1; i--)
		{
			for (int j = battlegroundColumnsWithBorder - 1; j > 1; j--)
			{
				uchar a = battleground[i][j];

				if (battleground[i][j] != 0 && battleground[i][j] != 255)
				{
					uchar b = a + 1;
					uchar arrayLeft = battleground[i][j - 1];
					uchar arrayUp = battleground[i - 1][j];

					if (i > 0 && (arrayUp != 255 && arrayUp < b))
					{
						if (battleground[i - 1][j] == 0)
						{
							battleground[i - 1][j] = b;
							gf_run = true;
						};
					}
					if (j > 0 && (arrayLeft != 255 && arrayLeft < b))
					{
						if (battleground[i][j - 1] == 0)
						{
							battleground[i][j - 1] = b;
							gf_run = true;
						};
					}
				}
			}
		}
	}
}

struct Vector2i
{
	int x;
	int y;
};

Vector2i MouseToArrayPos(Vector2i mousePosition)
{
	Vector2i arrayPos = { mousePosition.x / cellWidthInPixels ,  mousePosition.y / cellHeightInPixels };
	return arrayPos;
}

struct Character
{
	SDL_Texture* texture;
	Vector2i position;
	Vector2i currentArray = {0, 0};
	Vector2i previousArray = {0, 0};
	Vector2i destinationArray;

	Character(Vector2i pos, SDL_Surface* sur, SDL_Renderer* rend, const char* imagePath);
	void PlaceCharacter(Vector2i pos);
	void UpdatePreviousPosition();
	void Move(Vector2i dest);
};

void Character::PlaceCharacter(Vector2i pos)
{
	position.x = (pos.x - 1) * cellWidthInPixels + cellWidthInPixels / 2;
	position.y = (pos.y - 1) * cellHeightInPixels + cellHeightInPixels / 2;
}

void Character::UpdatePreviousPosition()
{
	previousArray = currentArray;
}

Character::Character(Vector2i pos, SDL_Surface* sur, SDL_Renderer* rend, const char* imagePath)
{
	PlaceCharacter(pos);
	battleground[MouseToArrayPos(position).y + 1][MouseToArrayPos(position).x + 1] = 255;
	texture = SetTexture(sur, rend, imagePath);
}

void Character::Move(Vector2i dest)
{
	destinationArray = dest;
	currentArray = { position.x / cellWidthInPixels ,position.y / cellHeightInPixels };
	destinationArray.x += 1;
	destinationArray.y += 1;
	currentArray.x += 1;
	currentArray.y += 1;

	if (battleground[destinationArray.y][destinationArray.x] != 255 && battleground[destinationArray.y][destinationArray.x] != 254)
	{
		battleground[destinationArray.y][destinationArray.x] = 1;
	}

	uchar destination = battleground[destinationArray.y][destinationArray.x];
	uchar playerPosition = battleground[currentArray.y][currentArray.x];
	uchar down = battleground[currentArray.y + 1][currentArray.x];
	uchar up = battleground[currentArray.y - 1][currentArray.x];
	uchar right = battleground[currentArray.y][currentArray.x + 1];
	uchar left = battleground[currentArray.y][currentArray.x - 1];

	Grassfire();

	if (destination != 255 && destination != 254)
	{

		if (playerPosition > up)
		{
			if (position.y - cellHeightInPixels >= cellHeightInPixels / 2)
			{
				position.y -= cellHeightInPixels;
				currentArray.y -= 1;
			}
		}
		else if (playerPosition > down)
		{
			if (position.y + cellHeightInPixels <= (screenHeight - cellHeightInPixels / 2))
			{
				position.y += cellHeightInPixels;
				currentArray.y += 1;
			}
		}

		else if (playerPosition > left)
		{
			if (position.x - cellWidthInPixels >= cellWidthInPixels / 2)
			{
				position.x -= cellWidthInPixels;
				currentArray.x -= 1;
			}
		}
		else if (playerPosition > right)
		{
			if (position.x + cellWidthInPixels <= (screenWidth - cellWidthInPixels / 2))
			{
				position.x += cellWidthInPixels;
				currentArray.x += 1;
			}
		}
		Sleep(wait);
	}
}

struct Obstacle
{
	SDL_Texture* texture;

	Vector2i position;

	Obstacle(Vector2i pos, SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath);
	void PlaceObstacle();
};

Obstacle::Obstacle(Vector2i pos, SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath)
{
	position = pos;
	PlaceObstacle();
	battleground[MouseToArrayPos(position).y + 1][MouseToArrayPos(position).x + 1] = 255;
	texture = SetTexture(surface, renderer, imagePath);
}

void Obstacle::PlaceObstacle()
{
	position.x = (position.x - 1) * cellWidthInPixels + cellWidthInPixels / 2;
	position.y = (position.y - 1) * cellHeightInPixels + cellHeightInPixels / 2;
}

uint32_t DeltaTime(uint32_t* lastTickTime, uint32_t* tickTime)
{
	*tickTime = SDL_GetTicks();
	uint32_t deltaTime = *tickTime - *lastTickTime;
	*lastTickTime = *tickTime;
	return deltaTime;
}

SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath)
{
	surface = IMG_Load(imagePath);
	if (!surface)
	{
		printf("Unable to load an image %s. Error: %s", imagePath, IMG_GetError());
		return NULL;
	}

	// Now we use the renderer and the surface to create a texture which we later can draw on the screen.
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture)
	{
		printf("Unable to create a texture. Error: %s", SDL_GetError());
		return NULL;
	}
	SDL_FreeSurface(surface);

	return texture;
}

void SetRect(SDL_Rect* rect, Vector2i position)
{
	rect->x = (int)round(position.x - cellWidthInPixels / 2); // Counting from the image's center but that's up to you
	rect->y = (int)round(position.y - cellHeightInPixels / 2); // Counting from the image's center but that's up to you
	rect->w = (int)cellWidthInPixels;
	rect->h = (int)cellHeightInPixels;
}

void DrawImage(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect rect)
{
	SDL_RenderCopyEx(renderer, // Already know what is that
		texture, // The image
		nullptr, // A rectangle to crop from the original image. Since we need the whole image that can be left empty (nullptr)
		&rect, // The destination rectangle on the screen.
		0, // An angle in degrees for rotation
		nullptr, // The center of the rotation (when nullptr, the rect center is taken)
		SDL_FLIP_NONE); // We don't want to flip the image
}

bool InitSDL(SDL_Renderer** renderer, SDL_Window** window)
{
	// Init SDL libraries
	SDL_SetMainReady(); // Just leave it be
	int result = 0;
	result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Init of the main SDL library
	if (result) // SDL_Init returns 0 (false) when everything is OK
	{
		printf("Can't initialize SDL. Error: %s", SDL_GetError()); // SDL_GetError() returns a string (as const char*) which explains what went wrong with the last operation
		return -1;
	}

	result = IMG_Init(IMG_INIT_PNG); // Init of the Image SDL library. We only need to support PNG for this project
	if (!(result & IMG_INIT_PNG)) // Checking if the PNG decoder has started successfully
	{
		printf("Can't initialize SDL image. Error: %s", SDL_GetError());
		return -1;
	}

	// Creating the window 1920x1080 (could be any other size)
	*window = SDL_CreateWindow("HoMM",
		0, 0,
		screenWidth, screenHeight,
		SDL_WINDOW_SHOWN);

	if (!*window)
		return -1;

	// Creating a renderer which will draw things on the screen
	*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
	if (!*renderer)
		return -1;

	// Setting the color of an empty window (RGBA). You are free to adjust it.
	SDL_SetRenderDrawColor(*renderer, 20, 150, 39, 200);

	return 1;
}

// Rand
int GetRandom(int min, int max)
{
	int tmp;
	if (max >= min)
		max -= min;
	else
	{
		tmp = min - max;
		min = max;
		max = tmp;
	}
	return max ? (rand() % max + min) : min;
}

Vector2i GetRandomArray()
{
	int randomX = GetRandom(3, 13);
	int randomY = GetRandom(2, 11);
	if (battleground[randomX][randomY] != 255)
	{
		Vector2i vector = { randomX, randomY };
		return vector;
	}
	else
	{
		GetRandomArray();
	}
}

void PlayTour(Character* playerCharacter, Character* aiCharacter, bool* playerIsMoving, bool* playerFinishedMoving, bool* aiIsMoving, int* tour, int nextTour, Vector2i mousePosition)
{
	if (*playerIsMoving)
	{
		playerCharacter->UpdatePreviousPosition();
		playerCharacter->Move(MouseToArrayPos(mousePosition));
		battleground[playerCharacter->previousArray.y][playerCharacter->previousArray.x] = 254;
		if (playerCharacter->currentArray.x == playerCharacter->destinationArray.x && playerCharacter->currentArray.y == playerCharacter->destinationArray.y)
		{
			playerCharacter->destinationArray.x = 0;
			playerCharacter->destinationArray.y = 0;
			*playerIsMoving = false;
			*playerFinishedMoving = true;
			*aiIsMoving = true;
			ResetBattleground();
			battleground[playerCharacter->currentArray.y][playerCharacter->currentArray.x] = 255;
		}
	}
	if (*playerFinishedMoving && *aiIsMoving)
	{
		aiCharacter->UpdatePreviousPosition();
		aiCharacter->Move(GetRandomArray());
		battleground[aiCharacter->previousArray.y][aiCharacter->previousArray.x] = 254;
		if (aiCharacter->currentArray.x == aiCharacter->destinationArray.x && aiCharacter->currentArray.y == aiCharacter->destinationArray.y)
		{
			aiCharacter->destinationArray.x = 0;
			aiCharacter->destinationArray.y = 0;
			*aiIsMoving = false;
			ResetBattleground();
			battleground[aiCharacter->currentArray.y][aiCharacter->currentArray.x] = 255;
			*tour = nextTour;
		}
	}
}

int main()
{
	int tour = 0;
	srand(seed);

	// Lööps management (some of them used for smooth movement which is disabled)
	bool playerIsMoving = false;
	bool playerFinishedMoving = false;
	bool aiIsMoving = false;

	Vector2i mousePosition = { 0, 0 };

	SDL_Event sdlEvent;
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;
	SDL_Surface* surface = nullptr;

	bool initSDLResult = InitSDL(&renderer, &window);
	if (!initSDLResult)
	{
		return -1;
	}

	// Player Characters
	Character guide({ 1,2 }, surface, renderer, "image/Guide.png");
	Character merchant({ 1,3 }, surface, renderer, "image/Merchant.png");
	Character nurse({ 1,4 }, surface, renderer, "image/Nurse.png");
	Character tavernkeep({ 1,5 }, surface, renderer, "image/Tavernkeep.png");
	Character angler({ 1,6 }, surface, renderer, "image/Angler.png");
	Character demolitionist({ 1,7 }, surface, renderer, "image/Demolitionist.png");
	Character dryad({ 1,8 }, surface, renderer, "image/Dryad.png");
	Character goblinTinkerer({ 1,9 }, surface, renderer, "image/GoblinTinkerer.png");

	// Enemy Characters
	Character abone({ 15,2 }, surface, renderer, "image/AB.png");
	Character pigron({ 15,3 }, surface, renderer, "image/CorruptionPigron.png");
	Character darkCaster({ 15,4 }, surface, renderer, "image/DarkCaster.png");
	Character demon({ 15,5 }, surface, renderer, "image/Demon.png");
	Character goblinScout({ 15,6 }, surface, renderer, "image/GoblinScout.png");
	Character tim({ 15,7 }, surface, renderer, "image/Tim.png");
	Character undead({ 15,8 }, surface, renderer, "image/UndeadViking.png");
	Character zombie({ 15,9 }, surface, renderer, "image/Zombie.png");

	// Obstacles
	Obstacle obstacle1({ GetRandom(3, 13), GetRandom(2, 10) }, surface, renderer, "image/Gandalf.png");
	Obstacle obstacle2({ GetRandom(3, 13), GetRandom(2, 10) }, surface, renderer, "image/Gandalf.png");
	Obstacle obstacle3({ GetRandom(3, 13), GetRandom(2, 10) }, surface, renderer, "image/Gandalf.png");
	Obstacle obstacle4({ GetRandom(3, 13), GetRandom(2, 10) }, surface, renderer, "image/Gandalf.png");

	SDL_FreeSurface(surface);

	SetBattlegroundBorder();

	int texWidth = cellWidthInPixels;
	int texHeight = cellHeightInPixels;

	// The main loop
	// Every iteration is a frame
	bool done = false;
	while (!done)
	{
		// Polling the messages from the OS.
		// That could be key downs, mouse movement, ALT+F4 or many others
		while (SDL_PollEvent(&sdlEvent))
		{
			if (sdlEvent.type == SDL_QUIT) // The user wants to quit
			{
				done = true;
			}
			else if (sdlEvent.type == SDL_KEYDOWN) // A key was pressed
			{
				switch (sdlEvent.key.keysym.sym) // Which key?
				{
				case SDLK_ESCAPE: // Posting a quit message to the OS queue so it gets processed on the next step and closes the game
					SDL_Event event;
					event.type = SDL_QUIT;
					event.quit.type = SDL_QUIT;
					event.quit.timestamp = SDL_GetTicks();
					SDL_PushEvent(&event);
					break;
					// Other keys here
				default:
					break;
				}
			}
			else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN)
			{
				if (sdlEvent.button.button == SDL_BUTTON_LEFT)
				{
					playerIsMoving = true;
					ResetBattleground();
					SDL_GetMouseState(&mousePosition.x, &mousePosition.y);

					if (debug == true)
					{
						PrintArray();
					}
				}
				if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
				{
					playerIsMoving = false;
					playerFinishedMoving = false;
					aiIsMoving = false;
					ResetBattleground();
					tour += 1;
					if (debug == true)
					{
						printf("RESET!!!");
					}
				}
			}
		}

		// Clearing the screen
		SDL_RenderClear(renderer);
		// All drawing goes here

		// Let's draw a sample image

		switch (tour)
		{
		case 0:
			PlayTour(&guide, &abone, &playerIsMoving, &playerFinishedMoving, &aiIsMoving, &tour, 1, mousePosition);
			break;
		case 1:
			PlayTour(&merchant, &pigron, &playerIsMoving, &playerFinishedMoving, &aiIsMoving, &tour, 2, mousePosition);
			break;
		case 2:
			PlayTour(&nurse, &darkCaster, &playerIsMoving, &playerFinishedMoving, &aiIsMoving, &tour, 3, mousePosition);
			break;
		case 3:
			PlayTour(&tavernkeep, &demon, &playerIsMoving, &playerFinishedMoving, &aiIsMoving, &tour, 4, mousePosition);
			break;
		case 4:
			PlayTour(&angler, &goblinScout, &playerIsMoving, &playerFinishedMoving, &aiIsMoving, &tour, 5, mousePosition);
			break;
		case 5:
			PlayTour(&demolitionist, &tim, &playerIsMoving, &playerFinishedMoving, &aiIsMoving, &tour, 6, mousePosition);
			break;
		case 6:
			PlayTour(&dryad, &undead, &playerIsMoving, &playerFinishedMoving, &aiIsMoving, &tour, 7, mousePosition);
			break;
		case 7:
			PlayTour(&goblinTinkerer, &zombie, &playerIsMoving, &playerFinishedMoving, &aiIsMoving, &tour, 0, mousePosition);
			break;
		default:
			break;
		}

		// Rendering etc.
		// Here is the rectangle where the image will be on the screen
		// PC
		SDL_Rect rectGuide;
		SDL_Rect rectMerchant;
		SDL_Rect rectNurse;
		SDL_Rect rectTavernkeep;
		SDL_Rect rectAngler;
		SDL_Rect rectDemolitionist;
		SDL_Rect rectDryad;
		SDL_Rect rectGoblinTinkerer;

		SetRect(&rectGuide, guide.position);
		SetRect(&rectMerchant, merchant.position);
		SetRect(&rectNurse, nurse.position);
		SetRect(&rectTavernkeep, tavernkeep.position);
		SetRect(&rectAngler, angler.position);
		SetRect(&rectDemolitionist, demolitionist.position);
		SetRect(&rectDryad, dryad.position);
		SetRect(&rectGoblinTinkerer, goblinTinkerer.position);

		DrawImage(renderer, guide.texture, rectGuide);
		DrawImage(renderer, merchant.texture, rectMerchant);
		DrawImage(renderer, nurse.texture, rectNurse);
		DrawImage(renderer, tavernkeep.texture, rectTavernkeep);
		DrawImage(renderer, angler.texture, rectAngler);
		DrawImage(renderer, demolitionist.texture, rectDemolitionist);
		DrawImage(renderer, dryad.texture, rectDryad);
		DrawImage(renderer, goblinTinkerer.texture, rectGoblinTinkerer);


		// EC
		SDL_Rect rectABone;
		SDL_Rect rectPigron;
		SDL_Rect rectDarkCaster;
		SDL_Rect rectDemon;
		SDL_Rect rectGoblinScout;
		SDL_Rect rectTim;
		SDL_Rect rectUndead;
		SDL_Rect rectZombie;

		SetRect(&rectABone, abone.position);
		SetRect(&rectPigron, pigron.position);
		SetRect(&rectDarkCaster, darkCaster.position);
		SetRect(&rectDemon, demon.position);
		SetRect(&rectGoblinScout, goblinScout.position);
		SetRect(&rectTim, tim.position);
		SetRect(&rectUndead, undead.position);
		SetRect(&rectZombie, zombie.position);

		DrawImage(renderer, abone.texture, rectABone);
		DrawImage(renderer, pigron.texture, rectPigron);
		DrawImage(renderer, darkCaster.texture, rectDarkCaster);
		DrawImage(renderer, demon.texture, rectDemon);
		DrawImage(renderer, goblinScout.texture, rectGoblinScout);
		DrawImage(renderer, tim.texture, rectTim);
		DrawImage(renderer, undead.texture, rectUndead);
		DrawImage(renderer, zombie.texture, rectZombie);

		// Ob.
		SDL_Rect rectObstacle1;
		SDL_Rect rectObstacle2;
		SDL_Rect rectObstacle3;
		SDL_Rect rectObstacle4;

		SetRect(&rectObstacle1, obstacle1.position);
		SetRect(&rectObstacle2, obstacle2.position);
		SetRect(&rectObstacle3, obstacle3.position);
		SetRect(&rectObstacle4, obstacle4.position);

		DrawImage(renderer, obstacle1.texture, rectObstacle1);
		DrawImage(renderer, obstacle2.texture, rectObstacle2);
		DrawImage(renderer, obstacle3.texture, rectObstacle3);
		DrawImage(renderer, obstacle4.texture, rectObstacle4);

		// Showing the screen to the player
		SDL_RenderPresent(renderer);

		// next frame...

		if (debug == 2)
		{
			PrintArray();
		}
	}
	// If we reached here then the main loop stoped
	// That means the game wants to quit

	// Shutting down the renderer
	SDL_DestroyRenderer(renderer);

	// Shutting down the window
	SDL_DestroyWindow(window);

	// Quitting the Image SDL library
	IMG_Quit();
	// Quitting the main SDL library
	SDL_Quit();

	// Done.
	return 0;
}