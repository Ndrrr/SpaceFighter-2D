#include <stdio.h>
#include <iostream>
#include <SDL_ttf.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <random>

using namespace std;

const int bulletX = 15, bulletY = 15;
const int windowX = 1200, windowY = 700;

int bullet_cnt, cooldown, damage_multiplier=1;

int speed; 
struct fire{
	SDL_Rect pos;
	int life = 100;
	int damage = 1; 
	long long id;
	SDL_Texture* texture;
};

struct enemy {
	SDL_Rect pos;
	int HP = 4;
	int hit_tex_life = 11;
	set<long long> hit_by;
	SDL_Texture* texture; // change enemy color green->yellow->red by health FT510
};

struct button {
	SDL_Rect cord;
	int state = 0; // 0-normal, 1-mouseOver, 2 - Pressed
	SDL_Texture* texture,*texture2;
	int id;
	int check_pressed(int x, int y) {
		if (cord.x <= x && cord.x + cord.w >= x && cord.y <= y && cord.y + cord.h >= y) {
			//cout << cord.x << " " << cord.w << " " << cord.y << " " << cord.h << endl;
			return id;
		}
		return -1;
	}
};

struct skill {
	SDL_Rect cord;
	vector<SDL_Texture*> textures;
	int level = 0;
};


struct luck {
	int cooldown = 10;
	int id; // 1-bullet++, 2 - cooldown--, 3 - damage++, 4 cooldown++ FT510
};


bool check_hit(fire fr, enemy en) {
	if ((fr.pos.x<en.pos.x + 30 && fr.pos.x > en.pos.x && fr.pos.y<en.pos.y && fr.pos.y >en.pos.y - 30) ||
		(fr.pos.x + bulletX<en.pos.x + 30 && fr.pos.x + bulletX > en.pos.x && fr.pos.y<en.pos.y && fr.pos.y >en.pos.y - 30)) {
		return 1;
	}
	return 0;
}


int main(int argc, char* argv[])
{
	mt19937 mt(time(NULL));

	// retutns zero on success else non-zero
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("error initializing SDL: %s\n", SDL_GetError());
	}
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
	}
	if (TTF_Init() == -1)
	{
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
	}

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* win = SDL_CreateWindow("GAME", // creates a window
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowX, windowY, 0);

	// triggers the program that controls
	// your graphics hardware and sets flags
	Uint32 render_flags = SDL_RENDERER_ACCELERATED;

	// creates a renderer to render our images
	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);

	// creates a surface to load an image into the main memory
	SDL_Surface* surface;
	
	//
	map<string,button> buttons;
	map<string, skill> skills;
	button tmp;
	SDL_Texture* tmpTex;
	//import textures
	surface = IMG_Load("Textures\\spaceFighter.png");
	SDL_Texture* spaceship_texture = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	
	surface = IMG_Load("Textures\\sky.jpg");
	SDL_Texture* background_image = SDL_CreateTextureFromSurface(rend, surface), *background_image2 = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	surface = IMG_Load("Textures\\laser.jpg");
	SDL_Texture* laser_tex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	surface = IMG_Load("Textures\\green_enemies.png");
	SDL_Texture* green_enemy = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	surface = IMG_Load("Textures\\yellow_enemies.png");
	SDL_Texture* yellow_enemy = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	surface = IMG_Load("Textures\\red_enemies.png");
	SDL_Texture* red_enemy = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	surface = IMG_Load("Textures\\lucky_enemy.png");
	SDL_Texture* lucky_enemy = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	surface = IMG_Load("Textures\\white.jpg");
	SDL_Texture* on_hit_tex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	


	// add information about keybinding to pause screen FT510
	surface = IMG_Load("Textures\\UI\\background.png");
	SDL_Texture* pauseBackgroundTexture = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	// import button textures and initialize buttons
	surface = IMG_Load("Textures\\UI\\playButton.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["play"].texture = tmpTex;
	buttons["play"].cord.x = buttons["play"].cord.y = 50;
	buttons["play"].cord.w = 250; buttons["play"].cord.h = 100;
	buttons["play"].id = 0;
	surface = IMG_Load("Textures\\UI\\playButton2.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["play"].texture2 = tmpTex;

	surface = IMG_Load("Textures\\UI\\settingsButton.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["settings"].texture = tmpTex;
	buttons["settings"].texture = tmpTex;
	buttons["settings"].cord.x = 50,  buttons["settings"].cord.y = 175;
	buttons["settings"].cord.w = 250; buttons["settings"].cord.h = 100;
	buttons["settings"].id = 1;
	surface = IMG_Load("Textures\\UI\\settingsButton2.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["settings"].texture2 = tmpTex;

	surface = IMG_Load("Textures\\UI\\quitButton.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["quit"].texture = tmpTex;
	buttons["quit"].texture = tmpTex;
	buttons["quit"].cord.x =50,  buttons["quit"].cord.y = 300;
	buttons["quit"].cord.w = 250; buttons["quit"].cord.h = 100;
	buttons["quit"].id = 2;
	surface = IMG_Load("Textures\\UI\\quitButton2.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["quit"].texture2 = tmpTex;

	surface = IMG_Load("Textures\\UI\\continueButton.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["continue"].texture = tmpTex;
	buttons["continue"].cord.x = buttons["continue"].cord.y = 50;
	buttons["continue"].cord.w = 250; buttons["continue"].cord.h = 100;
	buttons["continue"].id = 3;
	surface = IMG_Load("Textures\\UI\\continueButton2.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["continue"].texture2 = tmpTex;

	
	skills["cooldown"].cord.x = windowX - 70;
	skills["cooldown"].cord.y = windowY - 70;
	skills["cooldown"].cord.h = skills["cooldown"].cord.w = 50;
	surface = IMG_Load("Textures\\Skills\\Cooldown\\red.png");
	skills["cooldown"].textures.push_back(SDL_CreateTextureFromSurface(rend,surface));
	surface = IMG_Load("Textures\\Skills\\Cooldown\\yellow.png");
	skills["cooldown"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	surface = IMG_Load("Textures\\Skills\\Cooldown\\green.png");
	skills["cooldown"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	surface = IMG_Load("Textures\\Skills\\Cooldown\\blue.png");
	skills["cooldown"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	

	TTF_Font* font = TTF_OpenFont("Textures\\evilEmpire.ttf", 100);
	if (font == NULL) {
		printf("dfd%s", TTF_GetError());
	}
	SDL_Color White = { 255,255,255,255 };

	long score = 0;
	string scr;
	SDL_Texture* scoreTextTexture;
	// let us control our image position
	// so that we can move it with our keyboard.
	SDL_Rect main_pos, cord,scoreTextPos,background1,background2;
	
	background1.x = background1.y = background2.x = 0; background2.y = -windowY;
	background1.w = background2.w= windowX, background1.h = background2.h = windowY;

	cord.x = cord.y = 0;
	
	scoreTextPos.x = 1000, scoreTextPos.y = 0;
	scoreTextPos.w = 150;
	scoreTextPos.h = 60;
	// connects our texture with main_pos to control position
	SDL_QueryTexture(spaceship_texture, NULL, NULL, &main_pos.w, &main_pos.h);

	// adjust height and width of our image box.
	main_pos.w = 80;
	main_pos.h = 80;

	// sets initial x-position of object
	main_pos.x = (windowX - main_pos.w) / 2;

	// sets initial y-position of object
	main_pos.y = (windowY - main_pos.h) / 2;

	// controls annimation loop
	int close = 0;

	

	vector <fire> fires,fires_tmp; 
	
	fire fr;

	enemy en;
	vector<enemy> enemies,enemies_tmp;
	
	int counter = -1; 
	long long fire_id = 0; // for assigning unique id
	int pause = 1, stop_debug=0;
	bool startFlag = 0;

	speed = 600; // spaceship
	//skills
	//add these skills -> bullet cnt ++, cooldown --, movement speed ++, bullet damage ++( multiple bullet under 1 image max 3) FT510
	// add these extras -> lucky enemies with special ammos - 4 bullet in one , unlucky bullet increase enemy health, big bullet 
	bullet_cnt = 0; // 3 bullet
	cooldown = 25; 
	damage_multiplier = 1; // explosion 
	int cooldown_cnt = 30; 
	int bullet_crd[4][4] = {{33,0,0,0},{0,66,0,0},{0,33,66,0},{-8,18,44,70}};
	//animation loop
	while (!close) {
		cooldown_cnt++;
		SDL_Event event;
		if (!pause) {
			if (counter > 10000) counter = -1;
			counter++;
			if (counter % 100 == 0) {

				
				en.pos = cord;
				en.pos.x = mt() % (windowX - 200) + 15;
				en.texture = green_enemy;
				
				// luck block enemies FT510
				//if (mt() % 10 == 0) en.texture = lucky_enemy;
				
				
				enemies.push_back(en);
				
				SDL_QueryTexture(en.texture, NULL, NULL, &enemies[enemies.size() - 1].pos.w, &enemies[enemies.size() - 1].pos.h);
				enemies[enemies.size() - 1].pos.w = 30;
				enemies[enemies.size() - 1].pos.h = 30;
			}
			// Events management
			while (SDL_PollEvent(&event)) {
				switch (event.type) {

				case SDL_QUIT:
					// handling of close button
					close = 1;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT && cooldown_cnt>=cooldown) {
						for (int i = 0; i < bullet_cnt%4+1; i++) {
							for (int j = 0; j < damage_multiplier; j++) {
								fr.pos = main_pos;
								fr.pos.x += bullet_crd[bullet_cnt % 4][i];
								fires.push_back(fr);
								fr.id = fire_id++;
								SDL_QueryTexture(laser_tex, NULL, NULL, &fires[fires.size() - 1].pos.w, &fires[fires.size() - 1].pos.h);
								fires[fires.size() - 1].pos.w = bulletX;
								fires[fires.size() - 1].pos.h = bulletY;
							}
						}
						cooldown_cnt = 0;
					}
					break;


				case SDL_KEYDOWN:
					switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_E:
						stop_debug = 1 - stop_debug; //only for debug
 						break;
					case SDL_SCANCODE_ESCAPE:
					case SDL_SCANCODE_P:
						pause = 1-pause;
						break;
					case SDL_SCANCODE_W:
					case SDL_SCANCODE_UP:
						main_pos.y -= speed / 30;
						break;
					case SDL_SCANCODE_A:
					case SDL_SCANCODE_LEFT:
						main_pos.x -= speed / 30;
						break;
					case SDL_SCANCODE_S:
					case SDL_SCANCODE_DOWN:
						main_pos.y += speed / 30;
						break;
					case SDL_SCANCODE_D:
					case SDL_SCANCODE_RIGHT:
						main_pos.x += speed / 30;
						break;
					case SDL_SCANCODE_1:
						bullet_cnt++;
						break;
					case SDL_SCANCODE_2:
						if (skills["cooldown"].level < 4) {
							skills["cooldown"].level++;
							cooldown -= 5;
						}
						break;
					case SDL_SCANCODE_3:
						speed += 30;
						break;
					case SDL_SCANCODE_4:
						damage_multiplier++;
						break;
					default:
						break;
					}
				}
			}

			// right boundary
			if (main_pos.x + main_pos.w > windowX)
				main_pos.x = windowX - main_pos.w;

			// left boundary
			if (main_pos.x < 0)
				main_pos.x = 0;

			// bottom boundary
			if (main_pos.y + main_pos.h > windowY)
				main_pos.y = windowY - main_pos.h;

			// upper boundary
			if (main_pos.y < 0)
				main_pos.y = 0;

			// clears the screen
			if (!stop_debug) {
				SDL_RenderClear(rend);

				//Background movement
				background1.y++;
				background2.y++;

				if (background1.y == windowY) background1.y = -windowY;
				if (background2.y == windowY) background2.y = -windowY;

				SDL_RenderCopy(rend, background_image, NULL, &background1);
				SDL_RenderCopy(rend, background_image2, NULL, &background2);


				SDL_RenderCopy(rend, spaceship_texture, NULL, &main_pos);
				//score update
				scr = "Score: " + to_string(score);
				const char* scoreText = scr.c_str();
				surface = TTF_RenderText_Solid(font, scoreText, White);
				scoreTextTexture = SDL_CreateTextureFromSurface(rend, surface);
				SDL_FreeSurface(surface);
				SDL_RenderCopy(rend, scoreTextTexture, NULL, &scoreTextPos);


				for (int i = 0; i < fires.size(); i++) {
					if (fires[i].life > 0) {
						SDL_RenderCopy(rend, laser_tex, NULL, &fires[i].pos);
						for (int j = 0; j < enemies.size(); j++) {
							if (check_hit(fires[i], enemies[j])) {
								//cout << enemies[j].hit_by.size() << endl;
								enemies[j].hit_tex_life = 10;
								enemies[j].hit_by.insert(fires[i].id);
							}
						}
						//fires[i].pos.x += 10;
						fires[i].pos.y -= 10;
						fires[i].life--;
						if (fires[i].life > 0)
							fires_tmp.push_back(fires[i]);
					}
				}

				fires.clear();
				for (int i = 0; i < fires_tmp.size(); i++) fires.push_back(fires_tmp[i]);
				fires_tmp.clear();
				int escaped_enemies = 0;
				for (int i = 0; i < enemies.size(); i++) {
					if (enemies[i].pos.y >= 800) escaped_enemies++;
					if (enemies[i].hit_by.size() < enemies[i].HP && enemies[i].pos.y < 800) {

						// change color by health
						if (enemies[i].hit_by.size() >= enemies[i].HP / 3 * 2) enemies[i].texture = red_enemy;
						else if (enemies[i].hit_by.size() >= enemies[i].HP / 3) enemies[i].texture = yellow_enemy;
						
						

						if (enemies[i].hit_tex_life <= 10 && enemies[i].hit_tex_life > 0) {
							SDL_RenderCopy(rend, on_hit_tex, NULL, &enemies[i].pos);
							enemies[i].hit_tex_life--;
						}
						else if (enemies[i].hit_tex_life <= 0 || enemies[i].hit_tex_life == 11) {
							enemies[i].hit_tex_life = 11;
							SDL_RenderCopy(rend, enemies[i].texture, NULL, &enemies[i].pos);
						}
						//fires[i].pos.x += 10;
						enemies[i].pos.y += 2;
						enemies_tmp.push_back(enemies[i]);
					}
					else if (enemies[i].hit_by.size() >= 4 && enemies[i].texture == lucky_enemy) {
						//lucky_func();
					}

				}
				score += (enemies.size() - enemies_tmp.size()-escaped_enemies) * 10;
				enemies.clear();
				for (int i = 0; i < enemies_tmp.size(); i++) {
					enemies.push_back(enemies_tmp[i]);
				}
				enemies_tmp.clear();

				// triggers the double buffers
				// for multiple rendering
				SDL_RenderCopy(rend, skills["cooldown"].textures[skills["cooldown"].level], NULL, &skills["cooldown"].cord);

				SDL_RenderPresent(rend);
			}

			// calculates to 60 fps
			SDL_Delay(1000 / 60);
		}
		else {
			while (SDL_PollEvent(&event)) {
				
				switch (event.type) {
					case SDL_QUIT:
					// handling of close button
					close = 1;
					break;
					case SDL_MOUSEBUTTONUP:
						if (event.button.button == SDL_BUTTON_LEFT) {
							int x, y;
							SDL_GetMouseState(&x, &y);
							map<string, button>::iterator itr;
							//printf("%d %d\n", x, y);
							for (itr = buttons.begin(); itr != buttons.end(); itr++) {
								if ((*itr).second.check_pressed(x, y) != -1) {
									//cout << (*itr).second.check_pressed(x, y) << " "<<(*itr).second.id << endl;
									switch ((*itr).second.check_pressed(x, y))
									{
									case 0:
										if (startFlag == 0) {
											startFlag++;
											buttons["play"].texture = buttons["continue"].texture;
											buttons["play"].texture2 = buttons["continue"].texture2;
										}
										pause = pause - 1;
										break;
									case 2:
										close = 1;
										break;
									default:
										break;
									}
								}
							}
						}
						break;
					case SDL_KEYDOWN:
						switch (event.key.keysym.scancode) {
						case SDL_SCANCODE_P:
							pause = 1 - pause;
							break;
						default:
							break;
						}
					default:
						break;
				}

			}
			SDL_RenderClear(rend);
			SDL_RenderCopy(rend, pauseBackgroundTexture, NULL, NULL);
			SDL_RenderCopy(rend, buttons["play"].texture, NULL, &buttons["play"].cord);
			SDL_RenderCopy(rend, buttons["settings"].texture, NULL, &buttons["settings"].cord);
			SDL_RenderCopy(rend, buttons["quit"].texture, NULL, &buttons["quit"].cord);
			int x, y;
			SDL_GetMouseState(&x, &y);
			for (auto itr : buttons) {
				switch (itr.second.check_pressed(x, y))
				{
				case 0:
					SDL_RenderCopy(rend, buttons["play"].texture2, NULL, &buttons["play"].cord);
					break;
				case 1:
					SDL_RenderCopy(rend, buttons["settings"].texture2, NULL, &buttons["settings"].cord);
					break;
				case 2:
					SDL_RenderCopy(rend, buttons["quit"].texture2, NULL, &buttons["quit"].cord);
					break;
				default:
					break;
				}
			}
			SDL_RenderPresent(rend);

			// calculates to 60 fps
			SDL_Delay(windowX / 60);
		}
	}

	// posroy texture
	SDL_DestroyTexture(spaceship_texture);
	SDL_DestroyTexture(background_image);
	SDL_DestroyTexture(background_image2);
	SDL_DestroyTexture(lucky_enemy);
	SDL_DestroyTexture(green_enemy);
	SDL_DestroyTexture(laser_tex);
	SDL_DestroyTexture(on_hit_tex);
	for (auto itr : buttons) {
		SDL_DestroyTexture(itr.second.texture);
	}
	// destroy renderer
	SDL_DestroyRenderer(rend);
	// destroy window
	SDL_DestroyWindow(win);

	// close SDL
	SDL_Quit();

	return 0;
}
