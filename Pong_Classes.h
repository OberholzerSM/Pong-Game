#pragma once
#include "SDL_Game_Header.h"

//Spielrand. Gerade Linie, die entweder oben oder unten ist.
class Obj_Edge
{
public:

	Obj_Edge(bool up)
	{
		if (up)
			y = 1.0f - level_border + 0.5f*edge_shape.height;
		else
			y = -1.0f + level_border - 0.5f * edge_shape.height;
	}

	void load()
	{
		if (loaded == false)
		{
			shader.load();
			shader.change_size( edge_shape );
			shader.change_colour( colour,border_colour );
			shader.move_to(x,y);
			loaded = true;
		}
	}

	void draw()
	{
		shader.draw();
	}

	void unload()
	{
		if (loaded == true)
		{
			shader.unload();
			loaded = false;
		}
	}

	void draw() const
	{
		
	}

	~Obj_Edge()
	{
		unload();
	}

	//Variablen, die andere Objekte benötigen.
	static constexpr float level_border{ 0.1f };			//Der Punkt, wo der Ball vom Rand abprallt.
	static constexpr float edge_cutoff{ 0.05f };			//Abstand vom linken und rechten Bildrand. 
	//0.05f für ganzen Bildschirm, 0.175f damit Schläger am Tischrand sind, 0.15625f für 200 Pixel Abstand.

private:
	bool loaded{ false };

	//Farbe
	Colour colour{0.4f,0.5f,0.6f}, border_colour{ 0.3f,0.4f,0.5f };

	//Graphik-Eigenschaften
	float x{ 0.0f }, y;
	const float edge_thickness{ 0.4f };  //edge_thickness:: 1.0f entspricht Dicke von level_border bis zum Bildrand.
	const float border_thickness{ 0.004f };	//Dicke vom Rand um die Edge.
	Scale_2D edge_shape{ 2.0f - 2.0f*edge_cutoff, edge_thickness*level_border };

	Shader_Square_with_Border shader{};
};

//Hintergrund
class Obj_Background
{
public:

	Obj_Background()
	{

	}

	void load()
	{
		if(!loaded)
		{
			line_shader.load(line,2);

			edge_up.load();
			edge_down.load();
			loaded = true;
		}
	}

	void draw(bool draw_lines = true)
	{
		if(draw_lines)
			line_shader.draw();
		edge_up.draw();
		edge_down.draw();
	}

	void change_line_colour(const Colour &line_colour)
	{
		line_shader.change_colour(line_colour);
	}
	
	void unload()
	{
		if(loaded)
		{
			line_shader.unload();
			edge_up.unload();
			edge_down.unload();
			loaded = false;
		}
	}

	~Obj_Background()
	{
		unload();
	}

private:
	bool loaded{ false };
	Obj_Edge edge_up{1}, edge_down{0};

	//Tischlinien
	//0.8: Position Schläger. 0.025: Dicke Schläger.
	OpenGL_Shape line = 
	{
		{	0.0f,			+1.0f - Obj_Edge::level_border,
			0.0f,			-1.0f + Obj_Edge::level_border,
			+0.8f + 0.025f,	0.0f,
			-0.8f - 0.025f,	0.0f },

		 { 0,1,2,3 }
	};

	Shader_Lines line_shader{};
};

//Forward-Deklarationen
class Obj_Schlaeger;
class Obj_Ball;
class Obj_ScoreBoard;
class Obj_Portrait;

//K.I. Verhalten Schläger
enum Personality
{
	CALM,
	AGGRESSIVE,
	STRATEGIC,
};

enum Intelligence
{
	PLAYER,
	DUMB,
	SMART,
};

class Obj_Schlaeger
{
public:

	Obj_Schlaeger(double x = 0.0, double y = 0.0, Intelligence in = PLAYER, Personality p = CALM) : 
		x{x}, y{y}, x_old{x}, y_old{y}, x_draw{(float)x}, y_draw{(float)y}, personality{p}, intelligence{in}
	{}

	void load()
	{
		if(!loaded)
		{
			shader.load();
			shader.move_to(x_draw,y_draw);
			shader.change_size(size);
			if(intelligence == PLAYER)
				shader.change_colour({0.9f,0.9f,0.9f});
			else
			{
				t_react = 1.0;
				switch(personality)
				{
				case CALM:
					shader.change_colour({ 0.0f,0.9f,0.0f });
					set_reaction_time(t_react);
					break;

				case AGGRESSIVE:
					shader.change_colour({ 0.9f,0.0f,0.0f });
					set_reaction_time(t_react);
					break;

				case STRATEGIC:
					shader.change_colour({ 0.0f,0.0f,0.9f });
					set_reaction_time(t_react);
					break;
				}
			}
			loaded = true;
		}
	}

	void change_personality(Personality p)
	{
		personality = p;
		if(intelligence != PLAYER)
		{
			switch (personality)
			{
			case CALM:
				shader.change_colour({ 0.0f,0.9f,0.0f });
				break;

			case AGGRESSIVE:
				shader.change_colour({ 0.9f,0.0f,0.0f });
				break;

			case STRATEGIC:
				shader.change_colour({ 0.0f,0.0f,0.9f });
				break;
			}
		}
	}

	void change_intelligence(Intelligence i)
	{
		intelligence = i;
	}

	Personality get_personality() const
	{
		return personality;
	}

	void reset_position()
	{
		y_old = 0.0;
		y = 0.0;
		update_graphics();
	}

	void draw()
	{
		if(loaded)
		{
			shader.draw();
		}
	}

	void update_physics(double dt, Obj_Schlaeger &player, Obj_Schlaeger &opponent)
	{
		x_old = x;
		y_old = y;

		bool collision_up = (y + 0.5 * size.height >= 1.0 - Obj_Edge::level_border) && v_y > 0.0;
		bool collision_down = (y - 0.5 * size.height <= -1.0 + Obj_Edge::level_border) && v_y < 0.0;

		if (collision_down || collision_up)
		{
			v_y = 0.0;
			//Linker Rand
			if (x <= -1.0)
			{
				opponent.lost_last_round = false;
				player.lost_last_round = true;
			}
			//Rechter Rand
			else if (x >= 1.0)
			{
				player.lost_last_round = false;
				opponent.lost_last_round = true;
			}
		}

		x += v_x * dt;
		y += v_y * dt;
	}

	void update_graphics(double alpha = 1.0)
	{
		x_draw = static_cast<float>(alpha * x + (1.0 - alpha) * x_old);
		y_draw = static_cast<float>(alpha * y + (1.0 - alpha) * y_old);
		shader.move_to(x_draw, y_draw);
	}

	void process_input()
	{
		using namespace Game;
		if (keystate[key.KEY_UP] || keystate[key.KEY_UP2])
			v_y = v_max;
		else if (keystate[key.KEY_DOWN] || keystate[key.KEY_DOWN2])
			v_y = -v_max;
		else
			v_y = 0.0;
	}

	double set_reaction_time(double multiplier);

	bool move_to_centre(double tiredness = 0.2, double radius = 0.001)
	{
		if (y < -radius)
		{
			v_y = tiredness * v_max;
			//std::cout << "Moving co centre up!\n";
			return false;
		}
		else if (y > radius)
		{
			v_y = -tiredness * v_max;
			//std::cout << "Moving co centre down!\n";
			return false;
		}
		else
		{
			v_y = 0.0;
			//std::cout << "Reached centre!\n";
			return true;
		}
	}

	void wiggle(double speed = 5.0, double amplitude = 0.5)
	{
		double omega = speed / amplitude;

		if (move_to_centre(1.0, amplitude + 0.1))
			v_y = -1.0 * amplitude * omega * std::sin(omega * Game::clock.get_time());
	}

	double determine_Ball_path(const Obj_Ball& ball);

	void move_to_Ball(const Obj_Ball& ball);

	void react_to_Ball(const Obj_Ball& ball);

	void unload()
	{
		if(loaded)
		{
			shader.unload();
			loaded = false;
		}
	}

	~Obj_Schlaeger()
	{
		unload();
	}

	friend class Obj_Ball;

private:
	bool loaded{ false };

	//Physik-Variablen
	double x, y, x_old, y_old;
	double v_x{0.0}, v_y{0.0};
	const double v_max{ 2.0 };
	double y_ball{ 0.0 }; //Für die Berechnung, wo der Ball ist.

	//K.I.
	Personality personality;
	Intelligence intelligence;
	bool lost_last_round{false};
	
	//Timer
	double difficulty = 1.0;
	double t_start = 0.0, t_react = 0.0, t_react_multiplier = 1.0;

	//Shader
	float x_draw, y_draw;
	const Scale_2D size{ 0.05f , 4.5f * 0.05f };
	Shader_Square shader{};
};

class Obj_Ball
{
public:
	Obj_Ball() {}

	void load()
	{
		if(!loaded)
		{
			shader.load();
			shader.change_size(shape);
			sfx_playerhit.load();
			sfx_opponenthit.load();
			sfx_upperedgehit.load();
			sfx_loweredgehit.load();
			loaded = true;
		}
	}

	//Resette alle Variablen ausser v_x
	void soft_reset()
	{
		//Warte 0.5 Sekunden lang und starte dann den Ball neu
		if (pause(1.0))
		{
			x = x_start;
			y = y_start;
			x_old = x_start;
			y_old = y_start;
			x_draw = static_cast<float>(x_start);
			y_draw = static_cast<float>(y_start);
			v_y = -v_start / sqrt(2.0);
		}
		else
		{
			//Bewege in der Zwischenzeit den Ball ausserhalb des Bildschirms
			x = 0.0;
			x_old = 0.0;
			y = 2.0;
			y_old = 2.0;
			v_y = 0.0;
		}
	}

	//Resette alle Variablen auf die Startwerte
	void reset()
	{
		soft_reset();
		v_x = -v_start / sqrt(2.0);
	}

	void check_collision_schlaeger(const Obj_Schlaeger& schlaeger)
	{
		const double damping{ 0.5 }; //Um wie viel der Ball die Geschwindigkeit vom Schläger übernimmt.
		const double off{ 0.01 };	 //Es fühlt sich besser an, wenn der Ball im Schläger etwas versinkt.

		//Teste, ob eine Kollsion von links oder rechts erfolgt.
		bool coll_y{ (y - 0.5*shape.height) <= (schlaeger.y + 0.5 * schlaeger.size.height) && (y + 0.5*shape.height) >= (schlaeger.y - 0.5 * schlaeger.size.height) };	//Ball und Schläger sind auf gleicher Höhe.
		bool coll_right{ (x - 0.5*shape.width + off) <= (schlaeger.x + 0.5 * schlaeger.size.width) && (x - 0.5*shape.width + off) >= schlaeger.x && v_x < 0.0 };	//Kollsion rechts.
		bool coll_left{ (x + 0.5*shape.width - off) >= (schlaeger.x - 0.5 * schlaeger.size.width) && (x + 0.5*shape.width - off) <= schlaeger.x && v_x > 0.0 };	//Kollision links.

		//Teste, ob eine Kollsion von oben oder unten erfolgt.
		bool coll_x{ (x - 0.5*shape.width) <= (schlaeger.x + 0.5 * schlaeger.size.width) && (x + 0.5*shape.width) >= (schlaeger.x - 0.5 * schlaeger.size.width) };		//Ball und Schläger sind übereinander.
		bool coll_up{ (y - 0.5*shape.height + off) <= (schlaeger.y + 0.5 * schlaeger.size.height) && (y - 0.5*shape.height + off) >= schlaeger.y && v_y < 0.0 };		//Kollision oben.
		bool coll_down{ (y + shape.height - off) >= (schlaeger.y - 0.5 * schlaeger.size.height) && (y + 0.5*shape.height - off) <= schlaeger.y && v_y > 0.0 };	//Kollision unten.

		//Falls es zu einer horizontalen Kollision kommt.
		if (coll_y && (coll_left || coll_right))
		{
			v_x = -v_x;
			//Falls v_max nicht überschritten wird, erhöhe v_y anhand der Geschwindigkeit des Schlägers.
			if (std::pow(v_x, 2) + std::pow(v_y, 2) <= std::pow(v_max, 2))
				v_y += damping * schlaeger.v_y;
			//std::cout << "Ball: Empfange Geschwindigkeit: " << schlaeger.v_y << '\n';

			//Spiele den Sound ab
			if (schlaeger.intelligence == PLAYER)
				sfx_playerhit.play();
			else
				sfx_opponenthit.play();
		}
		//Falls nicht, teste ob eine Kollision von oben oder unten erfolgt.
		else if (coll_x && (coll_up || coll_down))
		{
			v_y = -v_y;
			//Spiele den Sound ab
			if (schlaeger.intelligence == PLAYER)
				sfx_playerhit.play();
			else
				sfx_opponenthit.play();
		}
	}

	void update_physics(double dt, Obj_Schlaeger& player, Obj_Schlaeger& opponent, Obj_ScoreBoard& score_board,
		Obj_Portrait& portrait_left, Obj_Portrait& portrait_right);

	void update_graphics(double alpha = 1.0)
	{
		x_draw = static_cast<float>(alpha * x + (1.0 - alpha) * x_old);
		y_draw = static_cast<float>(alpha * y + (1.0 - alpha) * y_old);
		shader.move_to(x_draw,y_draw);
	}

	void draw()
	{
		shader.draw();
	}

	//Warte t Sekunden lang.
	bool pause(double t)
	{
		static bool timer_start{ true };
		static double t0;

		//Beginne den Timer
		if (timer_start == true)
		{
			t0 = Game::clock.get_time();
			timer_start = false;
			return false;
		}
		//Falls die Zeit vorbei ist
		else if (Game::clock.get_time() - t0 > t)
		{
			timer_start = true;
			return true;
		}
		else
			return false;
	}

	void unload()
	{
		if(loaded)
		{
			shader.unload();
			sfx_playerhit.unload();
			sfx_opponenthit.unload();
			sfx_upperedgehit.unload();
			sfx_loweredgehit.unload();
			loaded = false;
		}
	}

	~Obj_Ball()
	{
		unload();
	}

	friend class Obj_Schlaeger;

private:
	bool loaded{ false };

	//Sounds
	const double volume{80.0};
	Sound_Effect sfx_playerhit{ "Sound_Effects\\Pong_Sound_01.mp3", volume };
	Sound_Effect sfx_opponenthit{ "Sound_Effects\\Pong_Sound_02.mp3", volume };
	Sound_Effect sfx_upperedgehit{ "Sound_Effects\\Pong_Sound_03.mp3", volume };
	Sound_Effect sfx_loweredgehit{ "Sound_Effects\\Pong_Sound_04.mp3", volume };

	//Physik-Variablen
	const double x_start{0.0}, y_start{1.0 - Obj_Edge::level_border};
	double x{x_start}, y{y_start}, x_old{x_start}, y_old{y_start};
	static constexpr double v_start{ 1.0 };
	double v_x{ - (1.0 / sqrt(2.0)) * v_start }, v_y{ - (1.0/sqrt(2.0)) * v_start };
	const double v_max{ 3.0 * v_start };

	//Shader
	float x_draw{ (float)x_start }, y_draw{ (float)y_start };
	const float radius = 0.02f;
	const Scale_2D shape{2.0f*radius,2.0f*radius};
	Shader_Square shader{};
};

class Obj_ScoreBoard
{
public:
	Obj_ScoreBoard()
	{
		txt_score.centre_text_horizontally();
	}

	void load()
	{
		if(!loaded)
		{
			txt_score.load();
			loaded = true;
		}
	}

	void update_score(bool player_win)
	{
		//Update den Punktestand
		if (player_win)
			score_left += 1;
		else
			score_right += 1;

		//Update den Punktestand im Text
		std::stringstream new_score;
		new_score << score_left << ' ' << score_right;
		txt_score.change_text(new_score.str());

		//Rezentriere den Text, falls der Punktestand ein Vielfaches von 10 ist
		if ((score_left != 0 && score_left % 10 == 0) || (score_right != 0 && score_right % 10 == 0))
		{
			if (!text_recentred)
			{
				txt_score.centre_text_horizontally();
				text_recentred = true;
			}
		}
		else
			text_recentred = false;
	}

	void reset_score()
	{
		score_left = 0;
		score_right = 0;
		txt_score.change_text("0 0");
		txt_score.centre_text_horizontally();
	}

	void draw()
	{
		txt_score.draw();
	}

	void unload()
	{
		if(loaded)
		{
			txt_score.unload();
			loaded = false;
		}
	}

	~Obj_ScoreBoard()
	{
		unload();
	}

	int score_left{ 0 }, score_right{ 0 };

private:
	bool loaded{ false }, text_recentred{true};
	Text_Bitmap txt_score{"0 0",0.0f,0.55f,0.1f,Colour_List::white};
};

class Obj_Portrait
{
public:
	Obj_Portrait(float x = -1.0, float y = 0.0) : x{x}, y{y}
	{

	}

	void change_character(int personality = 10)
	{
		switch (personality)
		{
		case CALM:
			y_texture = 2.0f / texture_height;
			break;

		case AGGRESSIVE:
			y_texture = 1.0f / texture_height;
			break;

		case STRATEGIC:
			y_texture = 0.0f;
			break;

		default:
			y_texture = 3.0f / texture_height;
			break;
		}
		shader.move_texture_square(0.0f, y_texture);
	}

	void load(int personality = 10)
	{
		if(!loaded)
		{
			shader.load("Textures\\Pong_Reactions.png");
			loaded = true;
		}
		shader.move_to(x, y);
		shader.change_size(shape);
		shader.change_size_texture_square(1.0f / texture_width, 1.0f / texture_height);
		change_character(personality);
		if (x > 0.0f)
			shader.flip();
	}

	void draw()
	{
		shader.draw();
	}

	void change_background_colours(const Colour &colour1, const Colour &colour2)
	{
		shader.change_background_colours(colour1,colour2);
	}

	void change_to_neutral()
	{
		shader.move_texture_square(0.0f, y_texture);
		change_background_colours(colour_neutral_1,colour_neutral_2);
	}

	void change_to_happy()
	{
		shader.move_texture_square(1.0f/texture_width, y_texture);
		change_background_colours(colour_happy_1, colour_happy_2);
	}

	void change_to_sad()
	{
		shader.move_texture_square(2.0f / texture_width, y_texture);
		change_background_colours(colour_sad_1, colour_sad_2);
	}

	void change_to_angry()
	{
		shader.move_texture_square(2.0f / texture_width, y_texture);
		change_background_colours(colour_angry_1, colour_angry_2);
	}

	void unload()
	{
		if(loaded)
		{
			loaded = false;
		}
	}

	~Obj_Portrait()
	{
		unload();
	}

private:
	const float texture_width{3.0f}, texture_height{4.0f};

	//Farben
	Colour colour_neutral_1{ 0.5f,0.78f,0.87f }, colour_neutral_2{ 0.94f,0.94f,0.75f };
	Colour colour_happy_1{ 1.0f,1.0f,0.69f }, colour_happy_2{ 1.0f,0.91f,0.47f };
	Colour colour_sad_1{ 0.47f,0.53f,0.72f }, colour_sad_2{ 0.56f,0.81f,0.75f };
	Colour colour_angry_1{ 0.97f,0.4f,0.53f }, colour_angry_2{ 0.87f,0.5f,0.69f };

	bool loaded{ false };
	float x, y;
	float y_texture{ 3.0f / texture_height };
	Scale_2D shape{ 0.15625f, 0.15625f }; //Grösse, damit das Fenster 200px x 200 px gross ist.
	Shader_Portrait shader{};
};

//Funktionen, die andere Objekte benötigen
double Obj_Schlaeger::set_reaction_time(double multiplier)
{
	const double speed_ratio = abs( Obj_Ball::v_start / (std::sqrt(2.0) * v_max) );

	//Vertical Distance Schlaeger to Edge
	const double distance_zero_to_edge = 1.0 - Obj_Edge::level_border - 0.5 * size.height;
	const double distance_edge_to_edge = 2.0 - 2.0*Obj_Edge::level_border - size.height;

	//Horizontal Distance Ball to Schlaeger
	const double distance_ball_to_self = speed_ratio * distance_zero_to_edge;
	const double distance_ball_from_opponent = 2.0 * abs(x) - size.width - distance_ball_to_self;

	//Maximal erlaubte Distanz für t_react_min
	const double max_distance_ball_to_self = speed_ratio * distance_edge_to_edge;
	const double max_distance_ball_from_opponent = 2.0 * abs(x) - size.width - max_distance_ball_to_self;

	//Minimal erlaubte Distanz für t_react_max
	const double min_distance_ball_to_self = 0.2;
	const double min_distance_ball_from_opponent = 2.0 * abs(x) - size.width - min_distance_ball_to_self;

	//Reaktionszeit
	t_react_multiplier = multiplier;
	t_react = distance_ball_from_opponent / (Obj_Ball::v_start / std::sqrt(2.0));
	const double t_react_min = max_distance_ball_from_opponent / (Obj_Ball::v_start / std::sqrt(2.0));
	const double t_react_max = min_distance_ball_from_opponent / (Obj_Ball::v_start / std::sqrt(2.0));

	t_react = multiplier * t_react;
	if (t_react <= t_react_min)
		t_react = t_react_min;
	else if (t_react >= t_react_max)
		t_react = t_react_max;

	return t_react;
}

double Obj_Schlaeger::determine_Ball_path(const Obj_Ball& ball)
{
	using namespace Game;
	double x_Ball = ball.x, y_Ball = ball.y, v_Ball = ball.v_y;

	while (abs(x_Ball) < abs(x - 0.5 * (double)size.width) )
	{
		x_Ball += ball.v_x * dt;

		bool collision_up = (y_Ball + 0.5 * ball.shape.height >= 1.0 - Obj_Edge::level_border) && v_Ball > 0.0;
		bool collision_down = (y_Ball - 0.5 * ball.shape.height <= -1.0 + Obj_Edge::level_border) && v_Ball < 0.0;

		if (collision_down || collision_up)
			v_Ball = -1.0 * v_Ball;

		y_Ball += v_Ball * dt;
	}
	return y_Ball;
}

void Obj_Schlaeger::move_to_Ball(const Obj_Ball& ball)
{
	//Bewege dich direkt zum Ball hin.
	if (abs(x - ball.x) > 0.01 + 0.5 * size.width)
	{
		//Bewege dich direkt zum Ball hin.
		if(intelligence == DUMB)
		{
			y_ball = ball.y;
		}

		if (y + 0.3 * size.height < y_ball)
		{
			v_y = v_max;
		}
		else if (y - 0.3 * size.height > y_ball)
		{
			v_y = -v_max;
		}
		else
		{
			v_y = 0.0;
		}
	}
	//Beschleunige oder verlangsame den Ball, je nach Persönlichkeit
	else
	{
		double sign = ball.v_y / abs(ball.v_y);
		switch (personality)
		{
		case CALM:
			v_y = -1.0 * sign * v_max;
			break;

		case AGGRESSIVE:
			v_y = sign * v_max;
			break;

		case STRATEGIC:
			double v_middle = (ball.v_max + ball.v_start) / 2.0;
			if (ball.v_y > v_middle)
				v_y = -1.0 * sign * v_max;
			else
				v_y = sign * v_max;
			break;
		}
	}
}

void Obj_Schlaeger::react_to_Ball(const Obj_Ball& ball)
{
	//Am Start der Runde, wenn der Ball ausserhalb des Bildschirms ist
	if (abs(ball.y) >= 1.0)
	{
		//Setzte temporär die Reaktionszeit auf 0.5, damit am Start der Runde der Schläger den Ball erwischt.
		if (lost_last_round)
			t_react = 0.5;

		switch (personality)
		{
		case CALM:
			v_y = 0.0;
			break;

		case AGGRESSIVE:
			wiggle(4.0 * v_max, 0.2);
			break;

		case STRATEGIC:
			move_to_centre();
			break;
		}
	}
	//Falls sich der Ball zum Schläger hin bewegt
	else if ((ball.x < x && ball.v_x > 0.0) || (ball.x > x && ball.v_x < 0.0))
	{
		if (t_start == 0.0)
		{
			if(intelligence == SMART)
				y_ball = determine_Ball_path(ball);
			t_start = Game::clock.get_time();
		}
		else if (Game::clock.get_time() - t_start >= t_react)
		{
			//std::cout << "Going after the ball!\n";
			move_to_Ball(ball);
		}
		else
		{
			switch (personality)
			{
			case CALM:
				v_y = 0.0;
				break;

			case AGGRESSIVE:
				wiggle(v_max, 0.5);
				break;

			case STRATEGIC:
				move_to_centre();
				break;
			}
		}
	}
	else
	{
		t_start = 0.0;
		if (lost_last_round)
		{
			t_react = set_reaction_time(t_react_multiplier);
			lost_last_round = false;
		}
		//Was der Schläger tut, wenn der Ball nicht zu ihm geht.
		switch (personality)
		{
		case CALM:
			v_y = 0.0;
			break;

		case AGGRESSIVE:
			wiggle(v_max, 0.5);
			break;

		case STRATEGIC:
			move_to_centre();
			break;
		}
	}
}

void Obj_Ball::update_physics(double dt, Obj_Schlaeger& player, Obj_Schlaeger& opponent, Obj_ScoreBoard& score_board, 
	Obj_Portrait &portrait_left, Obj_Portrait &portrait_right)
{
	x_old = x;
	y_old = y;

	static bool changed_to_neutral{ true };

	//Kollision oberer oder unterer Rand
	bool collision_up = (y + 0.5 * shape.height >= 1.0 - Obj_Edge::level_border) && v_y > 0.0;
	bool collision_down = (y - 0.5 * shape.height <= -1.0 + Obj_Edge::level_border) && v_y < 0.0;

	if (collision_down || collision_up)
	{
		v_y = -v_y;
		if (collision_down)
			sfx_loweredgehit.play();
		else if (collision_up)
			sfx_upperedgehit.play();
	}

	//Falls der Ball aus dem Bildschirm herausfliegt oder die horizontale Geschwindigkeit 0 ist.
	if (abs(x) >= 1.0 || abs(y) >= 1.0 || v_x == 0)
	{
		//Linker Rand
		if (x <= -1.0)
		{
			score_board.update_score(0);
			opponent.lost_last_round = false;
			portrait_right.change_to_happy();
			player.lost_last_round = true;
			portrait_left.change_to_sad();
		}
		//Rechter Rand
		else if (x >= 1.0)
		{
			score_board.update_score(1);
			player.lost_last_round = false;
			portrait_left.change_to_happy();
			opponent.lost_last_round = true;
			if(opponent.get_personality() == AGGRESSIVE)
				portrait_right.change_to_angry();
			else
				portrait_right.change_to_sad();
		}
		changed_to_neutral = false;
		soft_reset();
	}
	else if(!changed_to_neutral)
	{
		portrait_left.change_to_neutral();
		portrait_right.change_to_neutral();
		changed_to_neutral = true;
	}

	//Kollision Schläger
	check_collision_schlaeger(player);
	check_collision_schlaeger(opponent);


	x += v_x * dt;
	y += v_y * dt;
}