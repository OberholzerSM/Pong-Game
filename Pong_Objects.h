#pragma once
#include "Pong_Classes.h"

using namespace Game;

//In welchem Menü man gerade ist.
enum Menu
{
	SCREEN_START,
	SCREEN_MAIN,
	SCREEN_OPTIONS,
	SCREEN_OPTIONSTOURNAMENT,
	SCREEN_OPTIONSENDLESS,
};

namespace Pong
{
	Menu menu{SCREEN_START};
	Obj_Background background{};
};

using namespace Pong;

//Deklariere alle namespaces mit load, damit sie sich gegenseitig laden können
namespace Screen_Start
{
	void load();
}
namespace Screen_Main
{
	void load();
}
namespace Screen_Options
{
	void load();
}
namespace Screen_OptionsTournament
{
	void load();
	bool option_new_game{ true };
	Intelligence option_intelligence{ DUMB };
}
namespace Screen_OptionsEndless
{
	void load();
	Personality option_personality{CALM};
	Intelligence option_intelligence{ DUMB };
	float option_t_react{1.0f};
}

namespace Screen_Start
{
	Text_Bitmap text{"PONG GAME", 0.0f, 0.6f, 0.1f, Colour_List::white};
	Choice_Box cb_quit{};
	constexpr int n_options{ 4 };
	const std::string menu_names[n_options] = {"TOURNAMENT MODE","ENDLESS MODE","OPTIONS","QUIT GAME"};
	enum menu_options {TOURNAMENT_MODE, ENDLESS_MODE,OPTIONS,QUIT_GAME};
	Menu_List<n_options> start_menu{menu_names, 0.0f, 0.2f, 0.5f, 0.1f, 0.005f, {0.0f, 0.0f, 0.1f}, {0.3f, 0.4f, 0.5f}, false, 0.9f};

	void load()
	{
		background.load();
		background.change_line_colour({ 0.4f,0.4f,0.4f });
		text.load();
		text.centre_text_horizontally();
		start_menu.load();
		cb_quit.load();
	}

	void process_inputs()
	{
		if (keystate[key.KEY_ESCAPE] && !keystate_old[key.KEY_ESCAPE])
		{
			if (cb_quit.active)
				window.is_open = false;
			else
			{
				cb_quit.active = true;
				start_menu.deactivate();
			}
		}

		if(cb_quit.active)
		{
			cb_quit.check_mouse();
			cb_quit.check_keyboard();

			if (cb_quit.yes_button_pressed())
				window.is_open = false;
			else if (cb_quit.no_button_pressed())
			{
				cb_quit.active = false;
				cb_quit.reset_buttons();
				start_menu.activate();
			}
		}
		else
		{
			start_menu.check_inputs();
		}

		if(start_menu.buttons[TOURNAMENT_MODE].pressed)
		{
			start_menu.reset_buttons();
			Screen_OptionsTournament::load();
			menu = SCREEN_OPTIONSTOURNAMENT;
		}
		else if(start_menu.buttons[ENDLESS_MODE].pressed)
		{
			start_menu.reset_buttons();
			Screen_OptionsEndless::load();
			menu = SCREEN_OPTIONSENDLESS;
		}
		else if(start_menu.buttons[OPTIONS].pressed)
		{
			start_menu.reset_buttons();
			Screen_Options::load();
			menu = SCREEN_OPTIONS;
		}
		else if( start_menu.buttons[QUIT_GAME].pressed )
		{
			start_menu.reset_buttons();
			cb_quit.active = true;
		}
	}

	void draw()
	{
		background.draw(1);
		text.draw();
		start_menu.draw();
		cb_quit.draw();
	}
}

namespace Screen_Main
{
	Choice_Box cb_quit{0.0f,0.0f,0.3f,0.01f,"QUIT GAME?"};
	Obj_Schlaeger player{-0.8,0.0,PLAYER};
	Obj_Schlaeger opponent{ 0.8,0.0,SMART};
	Obj_Ball ball{};
	Obj_ScoreBoard score_board{};
	
	TextBox textbox{{"ROUND 1","TURTLE: ...HASTE MAKES WASTE."}};

	//Portraits
	constexpr float portrait_width{ 0.15625f };
	Obj_Portrait portrait_left{ -1.0f + 0.5f * portrait_width,1.0f - 0.5f * portrait_width * Game::window.ratio() };
	Obj_Portrait portrait_right{ 1.0f - 0.5f * portrait_width,1.0f - 0.5f * portrait_width * Game::window.ratio() };

	bool paused{ false };
	bool tournament_mode{ true }, just_won{false}, just_lost{false};
	int progress{ 0 };
	int max_score{ 1 };

	void load()
	{
		cb_quit.load();
		cb_quit.active = false;

		portrait_left.load();

		textbox.load();

		player.load();
		player.set_reaction_time(0.0);
		player.reset_position();

		opponent.load();
		if(tournament_mode)
		{
			if( Screen_OptionsTournament::option_new_game )
				progress = 0;

			switch (progress)
			{
			case 0:
				opponent.change_personality(CALM);
				portrait_right.load(CALM);
				textbox.change_text({ "ROUND 1","TURTLE: ...HASTE MAKES WASTE." });
				break;

			case 1:
				opponent.change_personality(AGGRESSIVE);
				portrait_right.load(AGGRESSIVE);
				textbox.change_text({ "ROUND 2","PANDA: I AM GOING TO BEAT YOU!!!" });
				break;

			case 2:
				opponent.change_personality(STRATEGIC);
				portrait_right.load(STRATEGIC);
				textbox.change_text({ "ROUND 3","RAVEN: LET US BEGIN." });
				break;
			}

			opponent.change_intelligence(Screen_OptionsTournament::option_intelligence);
			if(Screen_OptionsTournament::option_intelligence == DUMB)
				opponent.set_reaction_time(1.0);
			else
				opponent.set_reaction_time(0.9);

			textbox.active = true;
			paused = true;
		}
		else
		{
			portrait_right.load(Screen_OptionsEndless::option_personality);
			portrait_right.change_character(Screen_OptionsEndless::option_personality);
			opponent.change_personality( Screen_OptionsEndless::option_personality );
			opponent.change_intelligence( Screen_OptionsEndless::option_intelligence );
			opponent.set_reaction_time( (double)Screen_OptionsEndless::option_t_react );

			textbox.active = false;
			paused = false;
		}
		opponent.reset_position();

		ball.load();
		ball.reset();
		ball.update_graphics();

		score_board.load();
		score_board.reset_score();
	}

	void process_inputs()
	{
		if ( (keystate[key.KEY_ESCAPE] && !keystate_old[key.KEY_ESCAPE]) || (keystate[key.KEY_PAUSE] && !keystate_old[key.KEY_PAUSE]))
		{
			if (cb_quit.active)
			{
				menu = SCREEN_START;
			}
			else
			{
				cb_quit.active = true;
				paused = true;
			}
		}

		if (cb_quit.active)
		{
			cb_quit.check_mouse();
			cb_quit.check_keyboard();

			if (cb_quit.yes_button_pressed())
			{
				menu = SCREEN_START;

			}
			else if (cb_quit.no_button_pressed())
			{
				cb_quit.active = false;
				paused = false;
				cb_quit.reset_buttons();
			}
		}
		else if(paused && tournament_mode)
		{
			textbox.check_inputs();
			if (textbox.active == false)
				paused = false;

			if (just_won == true && textbox.txt_position == 2)
			{
				switch (progress)
				{
				case 1:
					player.reset_position();
					opponent.reset_position();
					opponent.change_personality(AGGRESSIVE);
					portrait_right.change_character(AGGRESSIVE);
					portrait_right.change_to_neutral();
					break;

				case 2:
					player.reset_position();
					opponent.reset_position();
					opponent.change_personality(STRATEGIC);
					portrait_right.change_character(STRATEGIC);
					portrait_right.change_to_neutral();
					break;
				}
				just_won = false;
			}

			if(textbox.active == true)
			{
				bool start1 = (progress == 0 && textbox.txt_position == 0);
				bool start2 = (progress == 1 && textbox.txt_position == 2);
				bool start3 = (progress == 2 && textbox.txt_position == 2);
				bool end = (progress == 3 && textbox.txt_position == 3);
				if ( start1 || start2 || start3 || end )
				{
					textbox.centre_text();
					score_board.reset_score();
				}
				else
					textbox.uncentre_text();

				if (progress == 3 && textbox.txt_position == 2)
					portrait_right.change_to_neutral();

				ball.soft_reset();
			}
		}
		else if(!paused)
		{
			player.process_input();

			if (tournament_mode && progress == 3)
				menu = SCREEN_START;

			if(tournament_mode && score_board.score_left == max_score && !just_won)
			{
				just_won = true;
				paused = true;
				switch(progress)
				{
				case 0:
					textbox.change_text({"TURTLE: ...OH, I LOST.","WOMBAT: GG.","ROUND 2","PANDA: I AM GOING TO BEAT YOU!!!"});
					break;

				case 1:
					textbox.change_text({ "PANDA: NO, I LOST!!!","WOMBAT: GG.","ROUND 3","RAVEN: LET US BEGIN." });
					break;

				case 2:
					textbox.change_text({ "RAVEN: SO, AFTER ALL THIS TIME,\n   I FINALLY HAVE BEEN DEFEATED.","WOMBAT: GG.","RAVEN: ...GG","YOU WON!" });
					break;
				}
				textbox.active = true;
				progress += 1;
			}
			else if(tournament_mode && score_board.score_right == max_score && !just_lost)
			{
				just_lost = true;
				paused = true;
				switch (progress)
				{
				case 0:
					textbox.change_text({ "TURTLE: ...LOOKS LIKE I WON.","WOMBAT: GG." });
					break;

				case 1:
					textbox.change_text({ "PANDA: I WON!!!","WOMBAT: GG." });
					break;

				case 2:
					textbox.change_text({ "RAVEN: ...GG","WOMBAT: GG." });
					break;
				}
				textbox.active = true;
			}
			else if (tournament_mode && just_lost == true)
			{
				player.reset_position();
				opponent.reset_position();
				score_board.reset_score();
				cb_quit.active = true;
				paused = true;
				just_lost = false;
			}
		}
	}

	void update_physics(double dt)
	{
		if(!paused)
		{
			//player.react_to_Ball(ball);
			player.update_physics(dt,player,opponent);
			opponent.react_to_Ball(ball);
			opponent.update_physics(dt, player, opponent);
			ball.update_physics(dt,player,opponent,score_board,portrait_left,portrait_right);
		}
	}

	void update_graphics(double alpha)
	{
		if(!paused)
		{
			player.update_graphics(alpha);
			opponent.update_graphics(alpha);
			ball.update_graphics(alpha);
		}
	}

	void draw()
	{
		background.draw();
		score_board.draw();
		player.draw();
		opponent.draw();
		ball.draw();
		textbox.draw();
		portrait_left.draw();
		portrait_right.draw();
		cb_quit.draw();
	}
}

namespace Screen_Options
{
	Text_Bitmap slider_txt{"VOLUME",0.0f,0.35f,0.05f,{0.9f,0.9f,0.9f}};
	Slider slider_volume{0.0f,0.2f};
	Button button_goback{0.0f,-0.7f,0.3f,0.15f,0.005f,{0.0f, 0.0f, 0.1f},{0.3f, 0.4f, 0.5f},false,"GO BACK"};

	int position{ 0 };
	const int n{2};
	bool mouse_active{ true }, position_moved{false};

	void load()
	{
		slider_txt.load();
		slider_txt.centre_text_horizontally();
		button_goback.load();
		slider_volume.load();
	}

	void process_inputs()
	{
		if (Game::mouse.moved)
		{
			position = 0;
			mouse_active = true;
		}

		if( (keystate[key.KEY_UP] && !keystate_old[key.KEY_UP]) || (keystate[key.KEY_UP2] && !keystate_old[key.KEY_UP2]))
		{
			position_moved = true;
			mouse_active = false;
			if (position > 0)
				position -= 1;
			else
				position = n - 1;
		}
		else if ((keystate[key.KEY_DOWN] && !keystate_old[key.KEY_DOWN]) || (keystate[key.KEY_DOWN2] && !keystate_old[key.KEY_DOWN2]))
		{
			position_moved = true;
			mouse_active = false;
			if (position < n-1)
				position += 1;
			else
				position = 0;
		}

		if(mouse_active)
			button_goback.check_mouse();
		else
		{
			button_goback.selected_old = button_goback.selected;
			button_goback.pressed_old = button_goback.pressed;
		}

		switch(position)
		{
		case 0:
			if(position_moved)
			{
				position_moved = false;
				button_goback.selected = false;
				button_goback.check_input();
			}
			slider_volume.check_mouse();
			slider_volume.check_keyboard();
			Game::global_volume = 2.0 * (double)slider_volume.get_slider_position();
			break;

		case 1:
			if(!mouse_active)
			{
				button_goback.selected = true;
				if (keystate[key.KEY_CONFIRM] && !keystate_old[key.KEY_CONFIRM])
					button_goback.pressed = true;
				button_goback.check_input();
			}
			break;
		}

		if(button_goback.pressed || keystate[key.KEY_ESCAPE])
		{
			button_goback.selected = false;
			button_goback.pressed = false;
			button_goback.check_input();
			menu = SCREEN_START;
		}

	}

	void update_physics(double dt)
	{
		slider_volume.update_physics(dt);
	}

	void update_graphics(double alpha)
	{
		slider_volume.update_graphics(alpha);
	}

	void draw()
	{
		background.draw();
		slider_txt.draw();
		slider_volume.draw();
		button_goback.draw();
	}
}

namespace Screen_OptionsTournament
{
	constexpr float text_size{0.05f};

	std::string menu_names[3] = { "NEW GAME","CONTINUE", "GO BACK" };
	Menu_List<3> options_menu{ menu_names,-0.5f, 0.7f, 0.5f, 0.1f, 0.005f, {0.0f, 0.0f, 0.1f}, {0.3f, 0.4f, 0.5f},
		false, 1.0f, {0.8f,0.8f,0.8f}, "Sound_Effects\\Menu_Click.mp3", "Sound_Effects\\Menu_Clap.mp3", HORIZONTAL };

	Text_Bitmap text_difficultyoption{ "DIFFICULTY",0.0, 0.25f, text_size, Colour_List::white };

	std::string intelligence_names[2] = { "EASY","HARD" };
	Menu_List<2> options_intelligence{ intelligence_names,-0.275f, 0.1f, 0.5f, 0.1f, 0.005f, {0.0f, 0.05f, 0.01f}, {0.3f, 0.5f, 0.4f},
		true, 1.0f, {0.8f,0.8f,0.8f}, "Sound_Effects\\Menu_Click.mp3", "Sound_Effects\\Menu_Clap.mp3", HORIZONTAL };

	Text_Bitmap text_pointoption{"POINTS NEEDED TO WIN",0.0,-0.25f, text_size, Colour_List::white};

	std::string point_names[3] = { "1","3", "7"};
	Menu_List<3> options_points{ point_names,-0.5f, -0.4f, 0.5f, 0.1f, 0.005f, {0.0f, 0.05f, 0.01f}, {0.3f, 0.5f, 0.4f},
		true, 1.0f, {0.8f,0.8f,0.8f}, "Sound_Effects\\Menu_Click.mp3", "Sound_Effects\\Menu_Clap.mp3", HORIZONTAL };

	int position{ 0 };
	constexpr int n{ 3 };
	bool switching_menu{ false };

	void load()
	{
		options_menu.load();
		options_menu.select_button(0);

		text_pointoption.load();
		text_pointoption.centre_text_horizontally();

		text_difficultyoption.load();
		text_difficultyoption.centre_text_horizontally();

		options_intelligence.load();
		options_intelligence.keyboard_active = false;
		options_intelligence.buttons[0].pressed = true;
		options_intelligence.buttons[0].check_input();

		options_points.load();
		options_points.keyboard_active = false;
		options_points.buttons[0].pressed = true;
		options_points.buttons[0].check_input();
	}

	void process_inputs()
	{
		if (keystate[key.KEY_ESCAPE])
		{
			menu = SCREEN_START;
		}
		else if ((keystate[key.KEY_DOWN] && !keystate_old[key.KEY_DOWN]) || (keystate[key.KEY_DOWN2] && !keystate_old[key.KEY_DOWN2]))
		{
			switching_menu = true;
			if (position < n - 1)
				position += 1;
			else
				position = 0;
		}
		else if ((keystate[key.KEY_UP] && !keystate_old[key.KEY_UP]) || (keystate[key.KEY_UP2] && !keystate_old[key.KEY_UP2]))
		{
			switching_menu = true;
			if (position > 0)
				position -= 1;
			else
				position = n-1;
		}

		if (switching_menu)
		{
			switching_menu = false;
			switch (position)
			{
			case 0:

				options_menu.keyboard_active = true;
				options_menu.select_button(0);

				options_intelligence.keyboard_active = false;
				for (int i = 0; i < 2; i++)
				{
					options_intelligence.buttons[i].selected = false;
					options_intelligence.buttons[i].check_input();
				}

				options_points.keyboard_active = false;
				for (int i = 0; i < 2; i++)
				{
					options_points.buttons[i].selected = false;
					options_points.buttons[i].check_input();
				}
				break;

			case 1:
				options_menu.keyboard_active = false;
				options_menu.reset_buttons();

				options_intelligence.select_button(0);
				options_intelligence.keyboard_active = true;

				options_points.keyboard_active = false;
				for (int i = 0; i < 2; i++)
				{
					options_points.buttons[i].selected = false;
					options_points.buttons[i].check_input();
				}
				break;

			case 2:
				options_menu.keyboard_active = false;
				options_menu.reset_buttons();

				options_intelligence.keyboard_active = false;
				for (int i = 0; i < 2; i++)
				{
					options_intelligence.buttons[i].selected = false;
					options_intelligence.buttons[i].check_input();
				}

				options_points.select_button(0);
				options_points.keyboard_active = true;
				break;
			}
		}

		options_menu.check_inputs();
		options_intelligence.check_inputs();
		options_points.check_inputs();

		if (options_intelligence.buttons[0].pressed)
			option_intelligence = DUMB;
		else if (options_intelligence.buttons[1].pressed)
			option_intelligence = SMART;

		if (options_points.buttons[0].pressed)
			Screen_Main::max_score = 1;
		else if (options_points.buttons[1].pressed)
			Screen_Main::max_score = 3;
		else if (options_points.buttons[2].pressed)
			Screen_Main::max_score = 7;

		if (options_menu.buttons[0].pressed)
		{
			options_menu.reset_buttons();
			option_new_game = true;
			Screen_Main::tournament_mode = true;
			Screen_Main::load();
			menu = SCREEN_MAIN;
		}
		else if (options_menu.buttons[1].pressed)
		{
			options_menu.reset_buttons();
			option_new_game = false;
			Screen_Main::tournament_mode = true;
			Screen_Main::load();
			menu = SCREEN_MAIN;
		}
		else if (options_menu.buttons[2].pressed)
		{
			options_menu.reset_buttons();
			menu = SCREEN_START;
		}
	}

	void draw()
	{
		background.draw();
		options_menu.draw();
		options_intelligence.draw();
		options_points.draw();
		text_pointoption.draw();
		text_difficultyoption.draw();
	}
}

namespace Screen_OptionsEndless
{
	std::string menu_names[2] = {"START","GO BACK"};
	Menu_List<2> options_menu{ menu_names,-0.275f, 0.7f, 0.5f, 0.1f, 0.005f, {0.0f, 0.0f, 0.1f}, {0.3f, 0.4f, 0.5f},
		false, 1.0f, {0.8f,0.8f,0.8f}, "Sound_Effects\\Menu_Click.mp3", "Sound_Effects\\Menu_Clap.mp3", HORIZONTAL };

	std::string personality_names[3] = {"CALM","AGGRESSIVE","STRATEGIC"};
	Menu_List<3> options_personality{ personality_names,-0.5f, 0.4f, 0.5f, 0.1f, 0.005f, {0.0f, 0.05f, 0.01f}, {0.3f, 0.5f, 0.4f}, 
		true, 1.0f, {0.8f,0.8f,0.8f}, "Sound_Effects\\Menu_Click.mp3", "Sound_Effects\\Menu_Clap.mp3", HORIZONTAL};

	std::string intelligence_names[2] = { "DUMB","SMART"};
	Menu_List<2> options_intelligence{ intelligence_names,-0.275f, 0.1f, 0.5f, 0.1f, 0.005f, {0.0f, 0.05f, 0.01f}, {0.3f, 0.5f, 0.4f},
		true, 1.0f, {0.8f,0.8f,0.8f}, "Sound_Effects\\Menu_Click.mp3", "Sound_Effects\\Menu_Clap.mp3", HORIZONTAL };

	Text_Bitmap slider_text{"REACTION TIME\n\n\nHARD     EASY",0.0f,-0.2f,0.1f,Colour_List::white};
	Slider slider_treact{0.0f,-0.35f};

	int position{ 0 };
	constexpr int n{ 4 };
	bool switching_menu{ false };

	void load()
	{
		options_menu.load();
		options_menu.select_button(0);

		options_personality.load();
		options_personality.keyboard_active = false;
		options_personality.buttons[0].pressed = true;
		options_personality.buttons[0].check_input();

		options_intelligence.load();
		options_intelligence.keyboard_active = false;
		options_intelligence.buttons[0].pressed = true;
		options_intelligence.buttons[0].check_input();

		slider_text.load();
		slider_treact.load();
		slider_treact.keyboard_active = false;

		Scale_2D max_glyph_size = options_personality.get_glyph_size();
		options_menu.change_glyph_size(max_glyph_size);
		options_intelligence.change_glyph_size(max_glyph_size);
		slider_text.change_glyph_size(max_glyph_size);
		slider_text.centre_text_horizontally();
	}

	void process_inputs()
	{
		if( keystate[key.KEY_ESCAPE] )
		{
			menu = SCREEN_START;
		}
		else if( (keystate[key.KEY_DOWN] && !keystate_old[key.KEY_DOWN]) || (keystate[key.KEY_DOWN2] && !keystate_old[key.KEY_DOWN2]) )
		{
			switching_menu = true;
			if (position < n - 1)
				position += 1;
			else
				position = 0;
		}
		else if ( (keystate[key.KEY_UP] && !keystate_old[key.KEY_UP]) || (keystate[key.KEY_UP2] && !keystate_old[key.KEY_UP2]) )
		{
			switching_menu = true;
			if (position > 0)
				position -= 1;
			else
				position = n - 1;
		}

		if(switching_menu)
		{
			switching_menu = false;
			switch(position)
			{
			case 0:
				slider_treact.keyboard_active = false;

				options_menu.keyboard_active = true;
				options_menu.select_button(0);

				options_personality.keyboard_active = false;
				for(int i=0;i<3;i++)
				{
					options_personality.buttons[i].selected = false;
					options_personality.buttons[i].check_input();
				}

				options_intelligence.keyboard_active = false;
				for (int i = 0; i < 2; i++)
				{
					options_intelligence.buttons[i].selected = false;
					options_intelligence.buttons[i].check_input();
				}
				break;

			case 1:
				slider_treact.keyboard_active = false;

				options_menu.keyboard_active = false;
				options_menu.reset_buttons();

				options_personality.select_button(0);
				options_personality.keyboard_active = true;

				options_intelligence.keyboard_active = false;
				for (int i = 0; i < 2; i++)
				{
					options_intelligence.buttons[i].selected = false;
					options_intelligence.buttons[i].check_input();
				}
				break;

			case 2:
				slider_treact.keyboard_active = false;

				options_menu.keyboard_active = false;
				options_menu.reset_buttons();

				options_personality.keyboard_active = false;
				for (int i = 0; i < 3; i++)
				{
					options_personality.buttons[i].selected = false;
					options_personality.buttons[i].check_input();
				}

				options_intelligence.select_button(0);
				options_intelligence.keyboard_active = true;
				break;

			case 3:
				slider_treact.keyboard_active = true;

				options_menu.keyboard_active = false;
				options_menu.reset_buttons();

				options_personality.keyboard_active = false;
				for (int i = 0; i < 3; i++)
				{
					options_personality.buttons[i].selected = false;
					options_personality.buttons[i].check_input();
				}

				options_intelligence.keyboard_active = false;
				for (int i = 0; i < 2; i++)
				{
					options_intelligence.buttons[i].selected = false;
					options_intelligence.buttons[i].check_input();
				}

				break;
			}
		}

		options_menu.check_inputs();
		options_personality.check_inputs();
		options_intelligence.check_inputs();
		slider_treact.check_mouse();
		slider_treact.check_keyboard();

		//1.23f: Maximal erlaubter Multiplier. Slider sollte bei x=0.5 einen Multiplier von 1.0 geben.
		option_t_react = 2.0f*(1.23f - 1.0f) * slider_treact.get_slider_position() + 2.0f - 1.23f;
		
		if (options_personality.buttons[CALM].pressed)
			option_personality = CALM;
		else if (options_personality.buttons[AGGRESSIVE].pressed)
			option_personality = AGGRESSIVE;
		else if (options_personality.buttons[STRATEGIC].pressed)
			option_personality = STRATEGIC;

		if (options_intelligence.buttons[0].pressed)
			option_intelligence = DUMB;
		else if (options_intelligence.buttons[1].pressed)
			option_intelligence = SMART;

		if (options_menu.buttons[0].pressed)
		{
			options_menu.reset_buttons();
			Screen_Main::tournament_mode = false;
			Screen_Main::load();
			menu = SCREEN_MAIN;
		}
		else if (options_menu.buttons[1].pressed)
		{
			options_menu.reset_buttons();
			menu = SCREEN_START;
		}
	}

	void update_physics(double dt)
	{
		slider_treact.update_physics(dt);
	}

	void update_graphics(double alpha)
	{
		slider_treact.update_graphics(alpha);
	}

	void draw()
	{
		background.draw();
		options_menu.draw();
		options_personality.draw();
		options_intelligence.draw();
		slider_text.draw();
		slider_treact.draw();
	}
}