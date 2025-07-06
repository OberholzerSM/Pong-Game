#include "Pong_Objects.h"

int main(int argc, char *args[])
{
	//Lade zu Beginn den Startbildschirm
	Screen_Start::load();

	while(window.is_open)
	{
		//Alte Events
		update_old_mouse();
		for(int i=0; i<256; i++)
			keystate_old[i] = keystate[i];

		//Behandle Events.
		while(SDL_PollEvent(&event))
		{
			//Standardmässig: Maus hat sich nicht bewegt.
			mouse.moved = false;

			switch (event.type)
			{
			//Beende das Spiel, wenn das Fenster geschlossen wird.
			case SDL_QUIT:
				window.is_open = false;
				break;

			//Update den keystate
			 //Taste wird gedrückt
			case SDL_KEYDOWN:
				keystate[event.key.keysym.scancode] = true;
				break;
			 //Taste wird wieder losgelassen
			case SDL_KEYUP:
				keystate[event.key.keysym.scancode] = false;
				break;

			//Behandle Fenster-Events
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					window.update_size();
					break;

				case SDL_WINDOWEVENT_MINIMIZED:
					window.is_minimized = true;
					break;

				case SDL_WINDOWEVENT_RESTORED:
					window.is_minimized = false;
					break;
				}
				break;

			//Update die Maus-Koordinaten
			case SDL_MOUSEMOTION:
				mouse.moved = true;
				mouse.update_coordinates();
				break;

			//Welche Maustaste gedrückt wurde und ob via Doppelklick.
			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button)
				{
				case SDL_BUTTON_LEFT:
					mouse.left_click = true;
					if (event.button.clicks == 2)
						mouse.left_doubleclick = true;
					break;

				case SDL_BUTTON_RIGHT:
					mouse.right_click = true;
					if (event.button.clicks == 2)
						mouse.right_doubleclick = true;
					break;

				case SDL_BUTTON_MIDDLE:
					mouse.mousewheel_click = true;
					break;
				}
				break;

			//Wenn man die Maustaste nicht länger drückt.
			case SDL_MOUSEBUTTONUP:
				mouse.left_click = false;
				mouse.left_doubleclick = false;
				mouse.right_click = false;
				mouse.right_doubleclick = false;
				mouse.mousewheel_click = false;
				break;

			//Um wie viel das Mausrad gedreht wurde.
			case SDL_MOUSEWHEEL:
				mouse.mousewheel = Game::event.wheel.y;
				break;
			}
		}
		
		//Verarbeite Inputs vom Spieler.
		switch (Pong::menu)
		{
		case SCREEN_START:
			Screen_Start::process_inputs();
			break;

		case SCREEN_MAIN:
			Screen_Main::process_inputs();
			break;

		case SCREEN_OPTIONS:
			Screen_Options::process_inputs();
			break;

		case SCREEN_OPTIONSTOURNAMENT:
			Screen_OptionsTournament::process_inputs();
			break;

		case SCREEN_OPTIONSENDLESS:
			Screen_OptionsEndless::process_inputs();
			break;
		}

		//Update die Physik in delta-Zeit.
		time_new = Game::clock.get_time();
		rest_time += time_new - time_old;
		time_old = time_new;
		while (rest_time >= dt)
		{
			switch (Pong::menu)
			{
			case SCREEN_MAIN:
				Screen_Main::update_physics(dt);
				break;

			case SCREEN_OPTIONS:
				Screen_Options::update_physics(dt);
				break;

			case SCREEN_OPTIONSENDLESS:
				Screen_OptionsEndless::update_physics(dt);
				break;
			}
			rest_time -= dt;
		}

		//Bestimme die Position auf dem Bildschirm anhand der Restzeit
		alpha = rest_time / dt;
		switch (Pong::menu)
		{
		case SCREEN_MAIN:
			Screen_Main::update_graphics(alpha);
			break;

		case SCREEN_OPTIONS:
			Screen_Options::update_graphics(alpha);
			break;

		case SCREEN_OPTIONSENDLESS:
			Screen_OptionsEndless::update_graphics(alpha);
			break;
		}

		//Behandle Render-Funktionen
		if (!Game::window.is_minimized) //Zeichne nur, falls das Fenster nicht minimiert wurde.
		{
			glClear(GL_COLOR_BUFFER_BIT);
			switch (Pong::menu)
			{
			case SCREEN_START:
				Screen_Start::draw();
				break;

			case SCREEN_MAIN:
				Screen_Main::draw();
				break;

			case SCREEN_OPTIONS:
				Screen_Options::draw();
				break;

			case SCREEN_OPTIONSTOURNAMENT:
				Screen_OptionsTournament::draw();
				break;

			case SCREEN_OPTIONSENDLESS:
				Screen_OptionsEndless::draw();
				break;
			}
			SDL_GL_SwapWindow(window.get_window());
		}
	}

	Game::window.close_SDL();

	//Damit der Kompiler keine Warnung darüber gibt, dass man argc und args nicht genutzt hat.
	(void)argc;
	(void)args;
	return 0;
}