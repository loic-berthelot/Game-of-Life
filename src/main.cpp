#include <iostream>
#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <cmath>
using namespace std;
using namespace sf;

enum class GameModes {
	Gol,
	Parity,
	ModuloSum3,
	ModuloSum4,
	ModuloCount,
	GolEnclosure,
	Maze,
	Battle,
	Wave,
	Gol2,
	Gol3,
	MazeGenerator
};

//USER PARAMETERS
GameModes game_mode = GameModes::Gol; // choose a value in GameModes
bool circular_world = true; //when circular_world is set to true, cells considers cells from the other side (on x and y axis) of the world as neighbors

RenderWindow window;
Vector2i window_size(VideoMode::getDesktopMode().width, VideoMode::getDesktopMode().height);
const int g_width = 56;
const int g_height = 40;
Vector2f rect_size((window_size.x - 300) / g_width, (window_size.y - 200) / g_height);
using line = int[g_width];
using grid = line[g_height];
grid game_grid;
using colors = Color[6];
colors all_colors = { Color::Black, Color::Blue, Color(100,170,255), Color::Green, Color::Magenta, Color::Red };
bool is_paused = true;
bool key_space = false;
int frame_number = 0;
int frame_evolution = 0;
int dtime = 20;
int color_left = 1;
bool eKeyPressed = false;
Vector2i worldCorner = Vector2i(300, 100);
std::vector<std::string> instructions = {
"CONTROLS :",
	"",
"left click : draw",
"right click : gum",
"space : pause/restart",
"R : reset",
"E : evolve 1 time",
"1,2,3,4,5 : select a color",
"F1, F2, F3 : change speed"
};
sf::Font font;
RectangleShape pause_logo;
sf::Text evolutionFrameText;
sf::Text instructionsText;

#pragma region utils
int mod(int a, int b) {
	while (a < 0) a += b;
	return a % b;
}
void display() {
	RectangleShape bgSquare;
	bgSquare.setSize(Vector2f(2+rect_size.x*g_width, 2+rect_size.y*g_height));
	bgSquare.setFillColor(Color(50, 50, 50));
	bgSquare.setOutlineColor(Color::White);
	bgSquare.setOutlineThickness(1);
	bgSquare.setPosition(Vector2f(worldCorner.x, worldCorner.y));
	window.draw(bgSquare);
	window.draw(evolutionFrameText);
	for (int i = 0; i < instructions.size(); ++i) {
		instructionsText.setString(instructions[i]);
		instructionsText.setPosition(Vector2f(20, 100+i*30));
		window.draw(instructionsText);
	}

	RectangleShape square;
	square.setSize(Vector2f(0.9*rect_size.x, 0.9*rect_size.y));
	for (int j = 0; j < g_height; j++) {
		for (int i = 0; i < g_width; i++) {
			square.setPosition(Vector2f(worldCorner.x+rect_size.x*i, worldCorner.y+rect_size.y*j));
			square.setFillColor(all_colors[game_grid[j][i]]);
			window.draw(square);
		}
	}
}
void initialize(bool random = false) {
	frame_evolution = 0;
	for (int j = 0; j < g_height; j++) {
		if (random) {
			for (int i = 0; i < g_width; i++) {
				game_grid[j][i] = rand() % 2;
			}
		} else {
			for (int i = 0; i < g_width; i++) {
				game_grid[j][i] = 0;
			}
		}
	}
}
void initialize(grid & g) {
	for (int j = 0; j < g_height; j++) {
		for (int i = 0; i < g_width; i++) g[j][i] = 0;
	}
}
void update() {
	Vector2i mouse_pos = Mouse::getPosition(window);
	Vector2i index = Vector2i(floor((mouse_pos.x - worldCorner.x)  / rect_size.x), floor((mouse_pos.y - worldCorner.y) / rect_size.y));
	if (index.x < 0 or index.y < 0 or index.x > g_width or index.y > g_height) return;
	if (Mouse::isButtonPressed(Mouse::Left)) game_grid[index.y][index.x] = color_left;
	if (Mouse::isButtonPressed(Mouse::Right)) game_grid[index.y][index.x] = 0;
}
vector<int> getNeighbors(int x, int y) {
	vector<int> vect(0, 0);
	for (int j = -1; j <= 1; j++) {
		for (int i = -1; i <= 1; i++) {
			if (i != 0 or j != 0) {
				if (circular_world) {
					vect.push_back(game_grid[mod(y + j, g_height)][mod(x + i, g_width)]);
				}
				else if (x + i >= 0 and x + i < g_width and y + j >= 0 and y + j < g_height) vect.push_back(game_grid[y + j][x + i]);
			}
		}
	}
	return vect;
}
vector<int> get4Neighbors(int x, int y) {
	vector<int> vect(0, 0);
	for (int j = -1; j <= 1; j++) {
		for (int i = -1; i <= 1; i++) {
			if (abs((i+j))%2 == 1) {
				if (circular_world) vect.push_back(game_grid[mod(y + j, g_height)][mod(x + i, g_width)]);
				else if (x + i >= 0 and x + i < g_width and y + j >= 0 and y + j < g_height) vect.push_back(game_grid[y + j][x + i]);
			}
		}
	}
	return vect;
}
int sumNeighbors(int x, int y, int min = -100, int max = 100) {
	int count = 0;
	vector<int> vect = getNeighbors(x, y);
	for (int i = 0; i < vect.size(); i++) {
		if (vect[i] >= min and vect[i] <= max) count += vect[i];
	}
	return count;
}
int countNeighbors(int x, int y, int min = -100, int max = 100) {
	int count = 0;
	vector<int> vect = getNeighbors(x, y);
	for (int i = 0; i < vect.size(); i++) {
		if(vect[i] >= min and vect[i] <= max) count += 1;
	}
	return count;
}
int count4Neighbors(int x, int y, int min = -100, int max = 100) {
	int count = 0;
	vector<int> vect = get4Neighbors(x, y);
	for (int i = 0; i < vect.size(); i++) {
		if (vect[i] >= min and vect[i] <= max) count += 1;
	}
	return count;
}
int maxNeighbors(int x, int y) {
	vector<int> vect = getNeighbors(x, y);
	int count = vect[0];
	for (int i = 1; i < vect.size(); i++) {
		if (vect[i] > count) count = vect[i];
	}
	return count;
}
int minNeighbors(int x, int y) {
	vector<int> vect = getNeighbors(x, y);
	int count = vect[0];
	for (int i = 1; i < vect.size(); i++) {
		if (vect[i] < count) count = vect[i];
	}
	return count;
}
void copyGrid(const grid & g) {
	for (int j = 0; j < g_height; j++) {
		for (int i = 0; i < g_width; i++) {
			game_grid[j][i] = g[j][i];
		}
	}
}
#pragma endregion utils

void gol() {
	grid g;
	int neighbors_number;
	for (int j = 0; j < g_height; j++) {
		for (int i = 0; i < g_width; i++) {
			neighbors_number = countNeighbors(i, j, 1, 100);
			if (game_grid[j][i] == 0) {
				if (neighbors_number == 3) g[j][i] = 1;
				else g[j][i] = 0;
			}
			else {
				if (neighbors_number == 2 or neighbors_number == 3) g[j][i] = 1;
				else g[j][i] = 0;
			}
		}
	}
	copyGrid(g);
}

void moduloSum(int m) {
	grid g;
	for (int j = 0; j < g_height; j++) {
		for (int i = 0; i < g_width; i++) g[j][i] = sumNeighbors(i, j) % m;
	}
	copyGrid(g);
}

void moduloCount(int m) {
	grid g;
	for (int j = 0; j < g_height; j++) {
		for (int i = 0; i < g_width; i++) g[j][i] = 2*countNeighbors(i, j, 1, 100) % m;
	}
	copyGrid(g);
}

void golEnclosure() {
	grid g;
	int neighbors_number;
	for (int j = 0; j < g_height; j++) {
		for (int i = 0; i < g_width; i++) {
			if (game_grid[j][i] <= 1) {
				neighbors_number = countNeighbors(i, j, 1, 1);
				if (game_grid[j][i] == 0) {
					if (neighbors_number == 3) g[j][i] = 1;
					else g[j][i] = 0;
				}
				else {
					if (neighbors_number == 2 or neighbors_number == 3) g[j][i] = 1;
					else g[j][i] = 0;
				}
			}
			else g[j][i] = game_grid[j][i];
		}
	}
	copyGrid(g);
}

void maze() {
	grid g;
	for (int j = 0; j < g_height; j++) {
		for (int i = 0; i < g_width; i++) {
			if (game_grid[j][i] == 0) {
				if (countNeighbors(i, j, 1, 1) > 0) g[j][i] = 1;
				else g[j][i] = 0;
			}
			else if (game_grid[j][i] == 1) {
				if (countNeighbors(i, j, 2, 2) > 0) g[j][i] = 2;
				else g[j][i] = 2;
			}
			else if (game_grid[j][i] == 2) g[j][i] = 0;
			else g[j][i] = game_grid[j][i];
		}
	}
	copyGrid(g);
}

void battle() {
	grid g;
	for (int j = 0; j < g_height; j++) {
		for (int i = 0; i < g_width; i++) {
			if (game_grid[j][i] == 0 or game_grid[j][i] == 1 or game_grid[j][i] == 5) {
				if (countNeighbors(i, j, 0, 0) + min(2, countNeighbors(i, j, 3, 3)) < 5) g[j][i] = 0;
				else if (countNeighbors(i, j, 1, 1) > countNeighbors(i, j, 5, 5)) g[j][i] = 1; 
				else if (countNeighbors(i, j, 5, 5) > countNeighbors(i, j, 1, 1)) g[j][i] = 5;
				else g[j][i] = game_grid[j][i];
			}
			else g[j][i] = game_grid[j][i];
		}
	}
	copyGrid(g);
}

void wave() {
	grid g;
	int neighbors_number;
	for (int j = 0; j < g_height; j++) {
		for (int i = 0; i < g_width; i++) g[j][i] = (game_grid[j][i]+maxNeighbors(i, j)-minNeighbors(i, j))%6;
	}
	copyGrid(g);
}

void gol2() {
	grid g;
	int neighbors_number;
	for (int j = 0; j < g_height; j++) {
		for (int i = 0; i < g_width; i++) {
			neighbors_number = countNeighbors(i, j, 1, 100);
			if (game_grid[j][i] == 0) {
				if (neighbors_number == 3) {
					if (countNeighbors(i, j, 1, 1) >= 2) g[j][i] = 1;
					else g[j][i] = 5;
				}
				else g[j][i] = 0;
			}
			else {
				if (neighbors_number == 2 or neighbors_number == 3) g[j][i] = game_grid[j][i];
				else g[j][i] = 0;
			}
		}
	}
	copyGrid(g);
}

void gol3() {
	grid g;
	for (int j = 0; j < g_height; j++) {
		for (int i = 0; i < g_width; i++) {
			if (game_grid[j][i] == 0) {
				if (countNeighbors(i, j, 1, 100) >= 3) {
					if (countNeighbors(i, j, 1, 1) > countNeighbors(i, j, 3, 3) and countNeighbors(i, j, 1, 1) > countNeighbors(i, j, 5, 5)) g[j][i] = 1;
					else if (countNeighbors(i, j, 3, 3) > countNeighbors(i, j, 1, 1) and countNeighbors(i, j, 3, 3) > countNeighbors(i, j, 5, 5)) g[j][i] = 3;
					else if (countNeighbors(i, j, 5, 5) > countNeighbors(i, j, 3, 3) and countNeighbors(i, j, 5, 5) > countNeighbors(i, j, 1, 1)) g[j][i] = 5;
					else g[j][i] = 0;
				}
				else g[j][i] = 0;
			}
			else {
				int predator, predators, friends, enemies;
				if (game_grid[j][i] == 1) predator = 3;
				if (game_grid[j][i] == 3) predator = 5;
				if (game_grid[j][i] == 5) predator = 1;
				predators = countNeighbors(i, j, predator, predator);
				friends = countNeighbors(i, j, game_grid[j][i], game_grid[j][i]);
				enemies = 8 - friends - countNeighbors(i, j, 0, 0);
				if (enemies + predators > friends) {
					if (predators >= 2) g[j][i] = predator;
					else g[j][i] = 0;
				}
				else g[j][i] = game_grid[j][i];
			}
		}
	}
	copyGrid(g);
}

void mazeGenerator() {
	for (int j = 0; j < g_height; j++) {
		for (int i = 0; i < g_width; i++) {
			if (count4Neighbors(i, j, 1, 1) == 1 and rand() % countNeighbors(i, j, 1, 1) <= 0) game_grid[j][i] = 1;
		}
	}
}

void evolve() {
	switch (game_mode) {
		case GameModes::Gol :
			gol();
		break;
		case GameModes::Parity :
			moduloSum(2);
		break;
		case GameModes::ModuloSum3 :
			moduloSum(3);
		break;
		case GameModes::ModuloSum4 :
			moduloSum(4);
		break;
		case GameModes::ModuloCount :
			moduloSum(4);
		break;
		case GameModes::GolEnclosure :
			moduloSum(4);
		break;
		case GameModes::Maze :
			moduloSum(4);
		break;
		case GameModes::Battle :
			moduloSum(4);
		break;
		case GameModes::Wave :
			moduloSum(4);
		break;
		case GameModes::Gol2 :
			moduloSum(4);
		break;
		case GameModes::Gol3 :
			moduloSum(4);
		break;
		case GameModes::MazeGenerator :
			moduloSum(4);
		break;
		default:
			gol();
	}
	frame_evolution++;
}

void inputs() {
	if (Keyboard::isKeyPressed(Keyboard::Space)) {
		if (not key_space) {
			key_space = true;
			is_paused = not is_paused;
		}
	}
	else key_space = false;
	if(Keyboard::isKeyPressed(Keyboard::R)) initialize();
	if(Keyboard::isKeyPressed(Keyboard::E)) {
		if (! eKeyPressed) evolve();
		eKeyPressed = true;
	} else {
		eKeyPressed = false;
	}
	if (Keyboard::isKeyPressed(Keyboard::Num1)) color_left = 1;
	if (Keyboard::isKeyPressed(Keyboard::Num2)) color_left = 2;
	if (Keyboard::isKeyPressed(Keyboard::Num3)) color_left = 3;
	if (Keyboard::isKeyPressed(Keyboard::Num4)) color_left = 4;
	if (Keyboard::isKeyPressed(Keyboard::Num5)) color_left = 5;
	if (Keyboard::isKeyPressed(Keyboard::F1)) dtime = 2;
	if (Keyboard::isKeyPressed(Keyboard::F2)) dtime = 10;
	if (Keyboard::isKeyPressed(Keyboard::F3)) dtime = 30;
}

void initWindow() {
	window.create(VideoMode(window_size.x, window_size.y), "Game of Life");
	window.setPosition(Vector2i(0, 0));
	window.setFramerateLimit(60);
	pause_logo.setFillColor(Color::Red);
	pause_logo.setSize(Vector2f(30, 30));
	pause_logo.setOrigin(Vector2f(15, 15));
	pause_logo.setPosition(Vector2f(30, 30));
	if (!font.loadFromFile("../resources/fonts/arial.TTF")) {
		std::cerr << "Error when loading the font\n";
	}
	evolutionFrameText.setFont(font);
	evolutionFrameText.setCharacterSize(20);
	evolutionFrameText.setFillColor(sf::Color::White);
	evolutionFrameText.setPosition(100, 20);
	instructionsText.setFont(font);
	instructionsText.setCharacterSize(20);
	instructionsText.setFillColor(sf::Color::White);
}

int main() {
	srand(time(0));
	initWindow();
	grid g;
	initialize();
	while (window.isOpen()) {
		window.clear(Color::Black);
		update();
		inputs();
		pause_logo.setFillColor(all_colors[color_left]);
		if (is_paused) pause_logo.setRotation(0);
		else {
			frame_number++;
			pause_logo.setRotation(45);
			if (frame_number % dtime == 0) evolve();
		}
		window.draw(pause_logo);
		evolutionFrameText.setString(std::to_string(frame_evolution));
		display();
		window.display();
		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) window.close();
		}
	}
	return 0;
}