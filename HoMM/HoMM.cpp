#include <stdio.h>
#include <Windows.h>
#define SDL_MAIN_HANDLED
#include "SDL2-2.0.20/include/SDL.h"
#include "SDL2-2.0.20/include/SDL_image.h"

int main()
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

	int scrWidth = 1920;
	int scrHeight = 1056;

	// Creating the window scrWidthxscrHeight (could be any other size)
	SDL_Window* window = SDL_CreateWindow("FirstSDL",
		0, 0,
		scrWidth, scrHeight,
		SDL_WINDOW_SHOWN);

	if (!window)
		return -1;

	// Creating a renderer which will draw things on the screen
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
		return -1;

	// Setting the color of an empty window (RGBA). You are free to adjust it.
	SDL_SetRenderDrawColor(renderer, 20, 150, 39, 255);

	// Loading an image
	char image_path[] = "image/char.png";
	char background_path[] = "image/gridx.png";
	// Here the surface is the information about the image. It contains the color data, width, height and other info.
	SDL_Surface* surface = IMG_Load(image_path);
	if (!surface)
	{
		printf("Unable to load an image %s. Error: %s", image_path, IMG_GetError());
		return -1;
	}

	SDL_Surface* background_surface = IMG_Load(background_path);
	if (!background_surface)
	{
		printf("Unable to load an image %s. Error: %s", background_path, IMG_GetError());
		return -1;
	}

	// Now we use the renderer and the surface to create a texture which we later can draw on the screen.
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture)
	{
		printf("Unable to create a texture. Error: %s", SDL_GetError());
		return -1;
	}

	SDL_Texture* background_texture = SDL_CreateTextureFromSurface(renderer, background_surface);
	if (!background_texture)
	{
		printf("Unable to create a texture. Error: %s", SDL_GetError());
		return -1;
	}

	// In a moment we will get rid of the surface as we no longer need that. But let's keep the image dimensions.
	int tex_width = surface->w;
	int tex_height = surface->h;

	int background_width = background_surface->w;
	int background_height = background_surface->h;

	// Bye-bye the surface
	SDL_FreeSurface(surface);
	SDL_FreeSurface(background_surface);

	bool done = false;
	SDL_Event sdl_event;

	//
	int xtg, ytg; // x to go to, y to go to
	int xtg_f, ytg_f;

	// Lööps management
	bool move = false;
	bool mouse_move = false;
	bool gf_run = true;
	bool mouse_lock = false;
	bool move_finished = true;
	bool lock_rest = false;

	// Wait amount
	int wait = 100;

	// Battleground tiles count
	static int b_columns = 15;
	static int b_rows = 11;

	// Background pixel coordinates
	int background_x = scrWidth / 2;
	int background_y = scrHeight / 2;

	// Cell pixel dimensions
	int cell_w = scrWidth / b_columns;
	int cell_h = scrHeight / b_rows;

	// Moving part pixel coordinates
	int x = scrWidth / 2;
	int y = scrHeight / 2;

	// Desirable cell coordinates
	int x_cell = -1;
	int y_cell = -1;

	// Moving part battleground tile coordinates
	int y_tc = (x / cell_w);
	int x_tc = (y / cell_h);

	// For ticks checker
	unsigned int end = 0, start;

	unsigned char battleground[11][15] =
					{
					{255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},
					{255,255,000,000,000,000,000,000,000,000,255,000,000,000,255},
					{255,000,255,000,000,000,000,000,000,000,255,000,000,000,255},
					{255,000,000,000,000,000,000,000,000,000,000,000,000,000,255},
					{255,000,000,000,000,000,000,000,255,000,000,000,000,000,255},
					{255,000,000,000,255,000,000,000,255,000,000,000,000,000,255},
					{255,000,000,000,255,000,000,000,000,000,000,000,000,000,255},
					{255,000,000,000,255,000,000,000,000,000,255,000,000,000,255},
					{255,000,000,000,255,000,000,000,000,000,255,000,000,000,255},
					{255,000,000,000,000,000,000,000,000,000,255,000,000,000,255},
					{255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},
					};

	// The main loop
	// Every iteration is a frame
	while (!done)
	{
		SDL_PumpEvents();

		start = SDL_GetTicks();
		// Polling the messages from the OS.
		// That could be key downs, mouse movement, ALT+F4 or many others
		while (SDL_PollEvent(&sdl_event) && mouse_lock == false)
		{
			if (sdl_event.type == SDL_QUIT) // The user wants to quit
			{
				done = true;
			}
			else if (sdl_event.type == SDL_KEYDOWN) // A key was pressed
			{
				switch (sdl_event.key.keysym.sym) // Which key?
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
			// More events here?
			if (sdl_event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (sdl_event.button.button == SDL_BUTTON_LEFT)
				{
					SDL_GetMouseState(&xtg, &ytg);
					mouse_move = true;
					gf_run = true;
					mouse_lock = true;
				}
			}
		}

		// Clearing the screen
		SDL_RenderClear(renderer);
		// All drawing goes here
		// Let's draw a sample image

		// Checking battleground cell by pixels designated with mouse
		if (mouse_move == true && lock_rest == false)
		{
			for (int i = 1; i < b_columns; i++)
			{
				if (xtg > cell_w * (i - 1) && xtg < cell_w * i)
				{
					y_cell = i - 1;
				}
			}
			for (int j = 1; j < b_rows; j++)
			{
				if (ytg > cell_h * (j - 1) && ytg < cell_h * j)
				{
					x_cell = j - 1;
				}
			}
			if (x_cell > 0 && y_cell > 0)
			{
				mouse_move = false;
				move = true;
			}
		}
			// Marking desirable cell on the battleground if available	
			if (move == true && lock_rest == false)
			{
				if (battleground[x_cell][y_cell] != 255)
				{
					battleground[x_cell][y_cell] = 1;
				}
				move = false;
			}

			// Grassfire
			while (gf_run == true && lock_rest == false)
			{
				gf_run = false;
				if (battleground[x_tc][y_tc] != 0)
				{
					gf_run = true;
					break;
				}
				for (int i = 0; i < b_rows; i++)
				{
					for (int j = 0; j < b_columns; j++)
					{
						if (battleground[i][j] != 0 && battleground[i][j] != 255)
						{
							battleground[i][j] += 1;
							if (battleground[i - 1][j] != 255 && battleground[i - 1][j] < battleground[i][j])
							{
								battleground[i - 1][j] = battleground[i][j] - 1;
							}
							if (battleground[i][j - 1] != 255 && battleground[i][j - 1] < battleground[i][j])
							{
								battleground[i][j - 1] = battleground[i][j] - 1;
							}
						}
					}
				}
				for (int g = b_rows; g > 0; g--)
				{
					for (int k = b_columns; k > 0; k--)
					{
						if (battleground[k][g] != 255)
						{
							if (battleground[k + 1][g] != 255 && battleground[k + 1][g] < battleground[k][g])
							{
								battleground[k + 1][g] = battleground[k][g] - 1;
							}
							if (battleground[k][g + 1] != 255 && battleground[k][g + 1] < battleground[k][g])
							{
								battleground[k][g + 1] = battleground[k][g] - 1;
							}
						}
					}
				}
			}
			gf_run = true;

			//end = SDL_GetTicks();
			//int delta = (end - start);

			// Moving our "hero" to destination
			if (start > end + wait)
			{
				if (battleground[x_tc][y_tc] < battleground[x_tc - 1][y_tc] && battleground[x_tc - 1][y_tc] != 255)
				{
					y = cell_h * (x_tc - 1) + (cell_h / 2);
					x_tc -= 1;
				}
				else if (battleground[x_tc][y_tc] < battleground[x_tc + 1][y_tc] && battleground[x_tc + 1][y_tc] != 255)
				{
					y = cell_h * (x_tc + 1) + (cell_h / 2);
					x_tc += 1;
				}
				else if (battleground[x_tc][y_tc] < battleground[x_tc][y_tc - 1] && battleground[x_tc][y_tc - 1] != 255)
				{
					x = cell_w * (y_tc - 1) + (cell_w / 2);
					y_tc -= 1;
				}
				else if (battleground[x_tc][y_tc] < battleground[x_tc][y_tc + 1] && battleground[x_tc][y_tc + 1] != 255)
				{
					x = cell_w * (y_tc + 1) + (cell_w / 2);
					y_tc += 1;
				}
				// Clearing table after alogrithm is complete
				else if (battleground[x_tc][y_tc] == battleground[x_cell][y_cell])
				{
					for (int i = 0; i < b_rows; i++)
					{
						for (int j = 0; j < b_columns; j++)
						{
							if (battleground[i][j] != 255)
							{
								battleground[i][j] = 0;
							}
						}
					}
					move_finished = true;
					gf_run = false;
					mouse_lock = false;
				} end = start;
			}

			/*if (move_finished == false)
			{
				lock_rest = true;
				if (x != xtg_f)
				{
					if (x > xtg_f)
					{
						x -= wait * delta;
					}
					if (x < xtg_f)
					{
						x += wait * delta;
					}
				}
				if (y != ytg_f)
				{
					if (y > ytg_f)
					{
						y -= wait * delta;
					}
					if (y < ytg_f)
					{
						y += wait * delta;
					}
				} else
				if (x == xtg_f && y == ytg_f)
				{
					move_finished = true;
					lock_rest = false;
				}

			}*/

			// Here is the rectangle where the image will be on the screen
			SDL_Rect rect;
			rect.x = (int)round(x - tex_width / 2); // Counting from the image's center but that's up to you
			rect.y = (int)round(y - tex_height / 2); // Counting from the image's center but that's up to you
			rect.w = (int)tex_width;
			rect.h = (int)tex_height;

			SDL_Rect background_rect;
			background_rect.x = (int)round(background_x - background_width / 2); // Counting from the image's center but that's up to you
			background_rect.y = (int)round(background_y - background_height / 2); // Counting from the image's center but that's up to you
			background_rect.w = (int)background_width;
			background_rect.h = (int)background_height;

			SDL_RenderCopyEx(renderer, // Already know what is that
				background_texture, // The image
				nullptr, // A rectangle to crop from the original image. Since we need the whole image that can be left empty (nullptr)
				&background_rect, // The destination rectangle on the screen.
				0, // An angle in degrees for rotation
				nullptr, // The center of the rotation (when nullptr, the rect center is taken)
				SDL_FLIP_NONE); // We don't want to flip the image

			SDL_RenderCopyEx(renderer, // Already know what is that
				texture, // The image
				nullptr, // A rectangle to crop from the original image. Since we need the whole image that can be left empty (nullptr)
				&rect, // The destination rectangle on the screen.
				0, // An angle in degrees for rotation
				nullptr, // The center of the rotation (when nullptr, the rect center is taken)
				SDL_FLIP_NONE); // We don't want to flip the image

	// Showing the screen to the player
			SDL_RenderPresent(renderer);
			// next frame...
			for (int i = 0; i < b_rows; i++)
			{
				for (int j = 0; j < b_columns; j++)
				{
					printf("%d ", battleground[i][j]);
				}printf("\n");
			}
			printf("\n");
	}
		

	SDL_DestroyTexture(texture);
	SDL_DestroyTexture(background_texture);

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