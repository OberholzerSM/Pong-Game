#pragma once

//C++ Libraries.
#include <iostream>		//Für Input/Output via der Konsole
#include <fstream>		//Für file input/output. Benötigt auch iostream.
#include <string>		//Für String-Klasse.
#include <sstream>		//Für stringstream: Erlaubt es Text zu haben, wo man leicht mehr Text hinzufügen kann.
#include <vector>		//Für Vektor-Klasse (dynamische Arrays).
#include <chrono>		//Um die Zeit zu messen.

//SDL and GLAD libraries.
#include <SDL.h>		//SDL Hauptheader.
#include <glad/glad.h>	//Um OpenGL mit Shadern zu nutzen.
#include <SDL_image.h>	//Um Bilddateien wie .png etc. laden zu können.
#include <SDL_mixer.h>	//Für Sound.

//Teste ob ein glad-Fehler vorgekommen ist. Quelle: https://learnopengl.com/In-Practice/Debugging
inline GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode{ glGetError() };
	if (errorCode != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cerr << error << " | " << file << " (" << line << ")\n";
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

//Stoppuhr-Klasse zum Zeitmessen. Braucht chrono.
class Stoppuhr
{
public:
	Stoppuhr() : paused{ false }
	{
		start_time = clock::now();
	}

	void reset()
	{
		start_time = clock::now();
	}

	void pause()
	{
		pause_time = clock::now();
		paused = true;
	}

	void resume()
	{
		if (paused)
			start_time += clock::now() - pause_time;
		paused = false;
	}

	double get_time() const
	{
		if (paused)
		{
			return std::chrono::duration_cast<double_second>(pause_time - start_time).count();
		}
		else
		{
			return std::chrono::duration_cast<double_second>(clock::now() - start_time).count();
		}
	}

private:
	using clock = std::chrono::steady_clock;
	using time_point = std::chrono::time_point<clock>;
	using double_second = std::chrono::duration<double, std::ratio<1> >;  //std::ratio<1> repräsentiert Sekunden, 1:1000 Millisekunden etc.
	time_point start_time, pause_time;
	bool paused;
};

//Initalisere SDL mit allen Subsystemen und das Hauptfenster.
class MainWindowClass
{
public:
	MainWindowClass(int window_width = 1280, int window_height = 720, const std::string& window_icon_path = "Textures\\Window_Icon.png") :
		window_width{ window_width }, window_height{ window_height }
	{
		initialize_mainwindow();
		set_window_icon(window_icon_path.c_str());
		glViewport(0, 0, window_width, window_height);
		glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

		//Für die Shader: w/h als float.
		window_ratio = static_cast<float>(window_width) / static_cast<float>(window_height);
	}

	int width() const
	{
		return window_width;
	}
	
	int height() const
	{
		return window_height;
	}
	
	float ratio() const
	{
		return window_ratio;
	}

	SDL_Window* get_window() const
	{
		return window;
	}

	void update_size()
	{
		SDL_GL_GetDrawableSize(window, &window_width, &window_height); //Achtung! SDL_GL_GetDrawableSize ist verschieden von event.window.data1 !
		glViewport(0, 0, window_width, window_height);
		window_ratio = static_cast<float>(window_width) / static_cast<float>(window_height);
	}

	void close_SDL() const
	{
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(window);
		Mix_Quit();
		IMG_Quit();
		SDL_Quit();
	}

	~MainWindowClass()
	{
		close_SDL();
	}

	bool is_minimized{ false }, is_open{ true };

private:
	int window_width, window_height;
	float window_ratio;
	SDL_Window* window{ NULL };
	SDL_GLContext context;
	void initialize_mainwindow()
	{
		//Initialisiere SDL
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		{
			std::cerr << "Error: Could not initialize SDL! SDL_Error: " << SDL_GetError() << '\n';
			SDL_Quit();
			exit(-1);
		}

		//Initalisiere OpenGL
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		//Initalisiere Hauptfenster
		window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		if (window == NULL)
		{
			std::cerr << "Error: Could not create Window! SDL_Error: " << SDL_GetError() << '\n';
			SDL_Quit();
			exit(-1);
		}

		//SDL muss noch separat ein OpenGL-Context erstellen.
		context = SDL_GL_CreateContext(window);
		(void)context;

		//Initalisiere GLAD
		if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
		{
			std::cerr << "Error: Could not initialize Glad!\n";
			SDL_Quit();
			exit(-1);
		}

		//Initalisiere SDL_image für jpg und png
		int img_formats{ IMG_INIT_JPG | IMG_INIT_PNG };
		if ((IMG_Init(img_formats) & img_formats) != img_formats)
		{
			std::cerr << "Error: Could not initialize SDL_image! SDL_Error: " << IMG_GetError() << '\n';
			SDL_Quit();
			exit(-1);
		}

		//SDL_Mixer für Sound
		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) //Sound-Frequenz, Format, Anzahl Soundkanäle, Grösse Sounds in bytes (hier 2kb). 
		{
			std::cerr << "Error: Could not initialize SDL_mix! SDL_Error: " << Mix_GetError() << '\n';
			SDL_Quit();
			IMG_Quit();
			exit(-1);
		}
	}
	void set_window_icon(const char* icon_path) const
	{
		SDL_Surface* window_icon{ IMG_Load(icon_path) };
		if (window_icon == NULL)
			std::cerr << "Could not load window icon! " << IMG_GetError() << '\n';
		else
			SDL_SetWindowIcon(window, window_icon);
		SDL_FreeSurface(window_icon);
	}
};

//Uhr und Hauptfenster (Fenstergrösse wird von Maus benötigt)
namespace Game
{
	Stoppuhr clock{};					//Stoppuhr-Objekt, um die Zeit zu messen.
	MainWindowClass window{ 1280,720 };	//Hauptfenster
};

//Maus-Klasse
class Mouse
{
public:

	Mouse()
	{
		update_coordinates();
	}

	double x{}, y{};															//Maus-Koordinaten
	bool left_click{ false }, right_click{ false }, mousewheel_click{ false };	//Ob die Maustasten gedrückt wurden.
	bool left_doubleclick{ false }, right_doubleclick{ false };					//Doppelklick.
	bool moved{ false };														//Ob die Maus bewegt wurde.
	int mousewheel{ 0 };														//Um wie viel das Mausrad das letzte mal gedreht wurde.

	double x_old{}, y_old{};																//Maus-Koordinaten
	bool left_click_old{ false }, right_click_old{ false }, mousewheel_click_old{ false };	//Ob die Maustasten gedrückt wurden.
	bool left_doubleclick_old{ false }, right_doubleclick_old{ false };						//Doppelklick.
	bool moved_old{ false };																//Ob die Maus bewegt wurde.
	int mousewheel_old{ 0 };																//Um wie viel das Mausrad das letzte mal gedreht wurde.

	void update_coordinates()
	{
		int x_int, y_int;	//Absolute Koordinaten mit (0,0) in der linken oberen Ecke
		SDL_GetMouseState(&x_int, &y_int);

		//Wandle die Koordinaten von absoluten in relative Koordinaten mit (0,0) im Zentrum um
		x_old = x;
		y_old = y;
		x = static_cast<double>(+2 * x_int) / static_cast<double>(Game::window.width()) - 1.0;
		y = static_cast<double>(-2 * y_int) / static_cast<double>(Game::window.height()) + 1.0;
	}

};

//Farb-Klasse
class Colour
{
public:
	enum Colour_Modi
	{
		RGBA,
		HSVA,
	};

	Colour(float x = 0.0f, float y = 0.0f, float z = 0.0f, float a = 1.0f, Colour_Modi modus = RGBA) : a{a}
	{
		if(modus == RGBA)
		{
			r = x;
			g = y;
			b = z;
		}
		else if(modus == HSVA)
		{
			Colour temp_colour = get_hsva_colour(x,y,z,a);
			r = temp_colour.r;
			g = temp_colour.g;
			b = temp_colour.b;
		}
	}

	float r{}, g{}, b{}, a;

	float get_hue() const
	{
		float c{ get_chroma() };
		//Grauer Farbton
		if (c == 0.0f)
			return 0.0f;
		else if (r >= b && r >= g)
			return 60.0f * std::fmodf((g - b) / c, 6.0f);
		else if (g >= b)
			return 60.0f * ((b - r) / c + 2.0f);
		else
			return 60.0f * ((r - g) / c + 4.0f);
	}

	float get_brightness()
	{
		return get_max();
	}

	float get_saturation()
	{
		if (r == g && r == b)
			return 0.0f;
		else
			return  get_chroma() / get_max();
	}

	//Returns an RGBA Colour that is the same as this colour but with increased brightness.
	Colour change_brightness(float V_new)
	{
		return get_hsva_colour(get_hue(), get_saturation(), std::min(V_new * get_brightness(), 1.0f),a);
	}

	//Returns an RGBA Colour that is the same as this colour but with increased saturation.
	Colour change_saturation(float S_new)
	{
		return get_hsva_colour(get_hue(), std::min(S_new * get_saturation(), 1.0f), get_brightness(),a);
	}

private:
	float get_max() const
	{
		if (r >= b && r >= g)
			return r;
		else if (b >= g)
			return b;
		else
			return g;
	}

	float get_min() const
	{
		if (r <= b && r <= g)
			return r;
		else if (b <= g)
			return b;
		else
			return g;
	}

	float get_chroma() const
	{
		return get_max() - get_min();
	}

	//Returns an RGBA Colour that corresponds to the HSVA values.
	Colour get_hsva_colour(float H, float S, float V, float A) const
	{
		Colour output_colour{};
		float h{ H / 60.0f }, c{ V * S };
		float x{ c * (1.0f - std::fabsf(std::fmodf(h,2.0f) - 1.0f)) }, m{ V - c };

		if (0.0f <= h && h < 1.0f)
		{
			output_colour.r = c + m;
			output_colour.g = x + m;
			output_colour.b = m;
		}
		else if (1.0f <= h && h < 2.0f)
		{
			output_colour.r = x + m;
			output_colour.g = c + m;
			output_colour.b = m;
		}
		else if (2.0f <= h && h < 3.0f)
		{
			output_colour.r = m;
			output_colour.g = c + m;
			output_colour.b = x + m;
		}
		else if (3.0f <= h && h < 4.0f)
		{
			output_colour.r = m;
			output_colour.g = x + m;
			output_colour.b = c + m;
		}
		else if (4.0f <= h && h < 5.0f)
		{
			output_colour.r = x + m;
			output_colour.g = m;
			output_colour.b = c + m;
		}
		else if (5.0f <= h && h < 6.0f)
		{
			output_colour.r = c + m;
			output_colour.g = m;
			output_colour.b = x + m;
		}
		else
			std::cerr << "Warning: h is not between 0 and 6, brightness will not be changed!\n";

		output_colour.a = A;
		return output_colour;
	}
};

namespace Colour_List
{
	Colour red{1.0f,0.0f,0.0f};
	Colour green{ 0.0f,1.0f,0.0f };
	Colour blue{0.0f,0.0f,1.0f};
	Colour yellow{1.0f,1.0f,0.0f};
	Colour magenta{1.0f,0.0f,1.0f};
	Colour cyan{0.0f,1.0f,1.0f};
	Colour white{1.0f,1.0f,1.0f};
	Colour black{ 0.0f,0.0f,0.0f };
	Colour grey{0.5f,0.5f,0.5f};
};

//Andere wichtige Objekte für das Spiel
namespace Game
{
	//Maus
	Mouse mouse;

	//Globales Lautstärke Objekt. 0.0 entspricht kein Sound, 1.0 normalen Sound, und 2.0 doppelt so lautem Sound.
	double global_volume{ 1.0 };

	//Input-Handling
	SDL_Event event;
	//Alle Scancode-Tasten, die genutzt werden.
	struct Keybindings
	{
		//Richtungstasten
		SDL_Scancode KEY_UP = SDL_SCANCODE_UP;
		SDL_Scancode KEY_UP2 = SDL_SCANCODE_W;
		SDL_Scancode KEY_DOWN = SDL_SCANCODE_DOWN;
		SDL_Scancode KEY_DOWN2 = SDL_SCANCODE_S;
		SDL_Scancode KEY_LEFT = SDL_SCANCODE_LEFT;
		SDL_Scancode KEY_LEFT2 = SDL_SCANCODE_A;
		SDL_Scancode KEY_RIGHT = SDL_SCANCODE_RIGHT;
		SDL_Scancode KEY_RIGHT2 = SDL_SCANCODE_D;

		//Auswahl-Tasten
		SDL_Scancode KEY_ESCAPE = SDL_SCANCODE_ESCAPE;
		SDL_Scancode KEY_CONFIRM = SDL_SCANCODE_RETURN;
		SDL_Scancode KEY_PAUSE = SDL_SCANCODE_SPACE;
	};
	Keybindings key;
	bool keystate[256]{}; //Ist 0 oder 1, je nachdem ob eine Tastatur-Taste gedrückt wird.
	bool keystate_old[256]{};

	//Variablen, damit das Spiel immer gleich schnell läuft
	constexpr int target_fps{ 60 };
	const double dt{ 1.0 / static_cast<double>(target_fps) };
	double time_new{ 0.0 }, time_old{ 0.0 }, rest_time{ 0.0 }, alpha{ 1.0 };

	//Update Maus-Variablen
	void update_old_mouse()
	{
		mouse.left_click_old = mouse.left_click;
		mouse.right_click_old = mouse.right_click;
		mouse.left_doubleclick_old = mouse.left_doubleclick;
		mouse.right_doubleclick_old = mouse.right_doubleclick;
		mouse.mousewheel_old = mouse.mousewheel;
		mouse.moved_old = mouse.moved;
	}
};

//Sound-Klassen

//Sound Effekte
class Sound_Effect
{
public:

	Sound_Effect(const std::string& file_path, double volume = 128.0) : file_path{ file_path }, volume{volume}
	{}

	void initialize(const std::string& path, int new_volume = 128)
	{
		file_path = path;
		volume = new_volume;
	}
	
	void load()
	{
		sound_effect = Mix_LoadWAV(file_path.c_str());
		if (sound_effect == nullptr)
			std::cerr << "Error: Could not load sound effect! " << '\n' << file_path.c_str() << '\n';
	}

	void update_volume()
	{
		game_volume = static_cast<int>(Game::global_volume * volume);
		if (game_volume > 128)
			game_volume = 128;
		else if (game_volume < 0)
			game_volume = 0;
	}

	void play(int n_repeats = 0, int channel = -1)
	{
		update_volume();
		Mix_Volume(channel, game_volume);
		Mix_PlayChannel(channel, sound_effect, n_repeats);
	}

	void unload()
	{
		Mix_FreeChunk(sound_effect);
		sound_effect = nullptr;
	}

	~Sound_Effect()
	{
		unload();
	}

private:
	Mix_Chunk* sound_effect{ nullptr };
	std::string file_path;
	int game_volume{128};
	double volume;
};

//Musik
class Music
{
public:
	Music(const std::string& file_path, double volume = 128.0) : file_path{ file_path }
	{
		change_volume(volume);
	}

	void load()
	{
		music = Mix_LoadMUS(file_path.c_str());
		if (music == nullptr)
			std::cerr << "Error: Could not load music!" << '\n' << file_path << '\n';
	}

	void change_volume(double new_volume)
	{
		volume = new_volume;
		int game_volume = static_cast<int>(Game::global_volume * volume);
		if (game_volume > 128)
			game_volume = 128;
		else if (game_volume < 0)
			game_volume = 0;
	}

	//n_repeats = -1: Endloss-Schlaufe
	void play(int n_repeats = -1)
	{
		Mix_VolumeMusic(game_volume);
		//Falls die Musik nicht pausiert wurde
		if (Mix_PausedMusic == 0)
			Mix_PlayMusic(music, n_repeats);
		else
			Mix_ResumeMusic();
	}

	void pause()
	{
		if (Mix_PausedMusic == 0)
			Mix_PauseMusic();
	}

	void stop()
	{
		Mix_HaltMusic();
	}

	void unload()
	{
		Mix_HaltMusic();
		Mix_FreeMusic(music);
		music = nullptr;
	}

private:
	Mix_Music* music{ nullptr };
	std::string file_path;
	double volume;
	int game_volume;
};

//Shader-Klassen

//Einfache Formen für den Shader
struct OpenGL_Shape
{
	std::vector<float> vertex_data;
	std::vector<unsigned int> indices;
};

namespace Shapes
{
	//Quadrat der Länge 1 mit Koordinatenmittelpunkt im Zentrum.
	OpenGL_Shape square
	{
		//Vertex Data.
		{ -0.5f, -0.5f,
		 +0.5f, -0.5f,
		 -0.5f, +0.5f,
		 +0.5f, +0.5f },

		 //Indices
		 { 0,1,2,
		  3,1,2 }
	};

	//Quadrat der Länge 1 mit Koordinatenmittelpunkt im Zentrum und Texturdaten.
	OpenGL_Shape square_textured
	{
		//Vertex und Textur Data.
			//Koordinaten Bildschirm	//Textur-Koordinaten
		{   -0.5f, -0.5f,				0.0f,	0.0f,
			+0.5f, -0.5f,				1.0f,	0.0f,
			-0.5f, +0.5f,				0.0f,	1.0f,
			+0.5f, +0.5f ,				1.0f,	1.0f
		},

		//Indices
		{ 0,1,2,
		  3,1,2 }
	};

	//Glyph für Bitmap-Text
	OpenGL_Shape glyph
	{
		//Vertex und Textur Data.
			//Koordinaten Bildschirm	//Textur-Koordinaten
		{   0.0f,	-0.25f,				0.0f,	0.0f,
			1.0f,	-0.25f,				1.0f,	0.0f,
			0.0f,	+0.75f,				0.0f,	1.0f,
			1.0f,	+0.75f,				1.0f,	1.0f
		},

		//Indices
		{ 0,1,2,
		  3,1,2 }
	};

};

//Massstab, der die Höhe automatisch mit der Fenstergrösse skaliert
struct Scale_2D
{
	Scale_2D(float w = 0.0f, float h = 0.0f, bool scaled = true): width{w}
	{
		if (scaled)
			height = h * Game::window.ratio();
		else
			height = h;
	}

	float width;
	float height;
};

//Generische Shader-Klasse. Beinhaltet keine Vertex-Objekte.
class Shader
{
public:
	Shader() {};

	void load(const std::vector<float>& vertex_array, const std::vector<unsigned int>& index_array, const std::string& shader_name)
	{
		if (loaded == false)
		{
			load_program(vertex_array, index_array, shader_name);
			loaded = true;
		}
	}

	void unload()
	{
		if(loaded)
		{
			glDeleteProgram(program);
			vertex_shader_name.str("");
			vertex_shader_name.clear();
			fragment_shader_name.str("");
			fragment_shader_name.clear();
			loaded = false;
		}
	}

	~Shader()
	{
		unload();
	}

protected:
	unsigned int program{};
	std::vector<float> vertex_data{};
	std::vector<unsigned int> indices{};

private:
	bool loaded{false};
	std::stringstream vertex_shader_name{}, fragment_shader_name{};
	std::string vertex_shader_source{}, fragment_shader_source{};
	unsigned int vertex_shader{}, fragment_shader{};

	void read_shader(const std::string& file_name, std::string& shader_code) //Speichert den Shader in shader_code
	{
		std::ifstream file_input{ file_name };
		if (!file_input)													//Falls man die Datei nicht finden konnte.
		{
			std::cerr << "Error: Could not find the following shader source-file: " << file_name << '\n';
			exit(-1);
		}
		else
		{
			std::stringstream file_stream;
			file_stream << file_input.rdbuf();
			shader_code = file_stream.str();
		}

		file_input.close();
	}

	void initalize_vertexshader()
	{
		int  success{};
		char infoLog[512]{};

		read_shader(vertex_shader_name.str().c_str(), vertex_shader_source); //Lese den Shader-Code ein.
		const GLchar* shader_file[] = { vertex_shader_source.c_str() }; //Konvertiere von string zu const GLchar*

		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, shader_file, NULL);
		glCompileShader(vertex_shader);
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
			std::cerr << "Error: Vertex-Shader compilation failed!\n" << infoLog << 'n';
		}
	}

	void initalize_fragmentshader()
	{
		int  success{};
		char infoLog[512]{};

		read_shader(fragment_shader_name.str().c_str(), fragment_shader_source); //Lese den Shader-Code ein.
		const GLchar* shader_file[] = { fragment_shader_source.c_str() }; //Konvertiere von string zu const GLchar*

		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, shader_file, NULL);
		glCompileShader(fragment_shader);
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
			std::cerr << "Error: Fragment-Shader compilation failed!\n" << infoLog << 'n';
		}
	}
	
	void create_shaderprogram()
	{
		int  success{};
		char infoLog[512]{};
		program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(program, 512, NULL, infoLog);
			std::cerr << "Error: Could not link shader program!\n" << infoLog << 'n';
		}
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		glUseProgram(program);
	}

	void load_program(const std::vector<float>& vertex_array, const std::vector<unsigned int>& index_array, const std::string& shader_name)
	{
		vertex_data = vertex_array;
		indices = index_array;
		vertex_shader_name << "Shaders\\" << shader_name.c_str() << ".vert";
		fragment_shader_name << "Shaders\\" << shader_name.c_str() << ".frag";
		initalize_vertexshader();
		initalize_fragmentshader();
		create_shaderprogram();
	}
};

//Shader-Template für 2D-Objekte ohne Textur- oder Farbdaten.
class Shader_2D : public Shader
{
public:
	Shader_2D() : Shader() {}

	void load(const OpenGL_Shape &shape, const std::string& shader_name)
	{
		if (loaded == false)
		{
			Shader::load(shape.vertex_data, shape.indices, shader_name);
			create_vertexobjects();
			loaded = true;
		}
	}

	void unload()
	{
		if (loaded == true)
		{
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &vbo);
			glDeleteBuffers(1, &ebo);
			loaded = false;
		}
	}

	~Shader_2D()
	{
		unload();
	}

protected:
	unsigned int vao{}, vbo{}, ebo{};

private:
	bool loaded{ false };
	void create_vertexobjects()
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), vertex_data.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
};

//Shader-Template für 2D-Objekte mit Textur aber ohne Farbdaten.
class Shader_2D_Textured : public Shader
{
public:
	Shader_2D_Textured(bool mipmap = true) : mipmap{mipmap} {}

	void load(const std::string &texture_path, const OpenGL_Shape &shape, const std::string &shader_name)
	{
		if (loaded == false)
		{
			Shader::load(shape.vertex_data, shape.indices, shader_name);
			create_vertexobjects();
			load_texture(texture_path.c_str());
			loaded = true;
		}
	}

	void unload()
	{
		if (loaded == true)
		{
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &vbo);
			glDeleteBuffers(1, &ebo);
			glDeleteTextures(1, &texture);
			loaded = false;
		}
	}

	~Shader_2D_Textured()
	{
		unload();
	}

protected:
	unsigned int texture{}, vao{}, vbo{}, ebo{};
	float texcel_width{}, texcel_height{};

private:
	bool loaded{ false }, mipmap;
	void create_vertexobjects()
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), vertex_data.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		//2D-Koordinaten
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		//Textur-Koordinaten
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	//Um das Bild korrekt herum zu drehen.
	//Code von StackOVerflow übernommen: https://stackoverflow.com/questions/65815332/flipping-a-surface-vertically-in-sdl2
	void flip_surface(SDL_Surface* surface)
	{
		SDL_LockSurface(surface);

		int pitch = surface->pitch; // row size
		char* temp = new char[pitch]; // intermediate buffer
		char* pixels = (char*)surface->pixels;

		for (int i = 0; i < surface->h / 2; ++i) {
			// get pointers to the two rows to swap
			char* row1 = pixels + i * pitch;
			char* row2 = pixels + (surface->h - i - 1) * pitch;

			// swap rows
			memcpy(temp, row1, pitch);
			memcpy(row1, row2, pitch);
			memcpy(row2, temp, pitch);
		}

		delete[] temp;

		SDL_UnlockSurface(surface);
	}

	//Lade die Textur
	void load_texture(const char* font_path = nullptr)
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		//Bestimmt, wie die Textur vortgesetzt wird, sobald man über den Rand hinaus geht.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//Skalierungs-Optionen. Mipmaps nur für MIN_FILTER.
		if(mipmap)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		else
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//Lade die Textur hoch
		SDL_Surface* texture_surface;
		if (font_path == nullptr)
			texture_surface = IMG_Load("Fonts\\Font_Digital.png"); //Default-Font
		else
			texture_surface = IMG_Load(font_path);

		if (texture_surface == NULL)
		{
			std::cerr << "Error: Could not load Font texture!\n";
		}
		else
		{
			flip_surface(texture_surface);
			int modus{ GL_RGB };
			if (texture_surface->format->BytesPerPixel == 4)
				modus = GL_RGBA;
			glTexImage2D(GL_TEXTURE_2D, 0, modus, texture_surface->w, texture_surface->h, 0, modus, GL_UNSIGNED_BYTE, texture_surface->pixels);
			if(mipmap)
				glGenerateMipmap(GL_TEXTURE_2D);

			texcel_width = 1.0f / static_cast<float>(texture_surface->w);
			texcel_height = 1.0f / static_cast<float>(texture_surface->h);
		}

		SDL_FreeSurface(texture_surface);
	}
};

//Einfacher Shader für ein Rechteck
class Shader_Square : public Shader_2D
{
public:

	Shader_Square(){}

	void load(const OpenGL_Shape &shape = Shapes::square)
	{
		if(!loaded)
		{
			Shader_2D::load(shape, "monocolour_shader");

			//Position Uniform-Variablen.
			x_loc = glGetUniformLocation(program, "x");
			scaling_loc = glGetUniformLocation(program, "scaling");
			input_colour_loc = glGetUniformLocation(program, "input_colour");

			loaded = true;
		}
	}

	void draw() const
	{
		if (loaded)
		{
			glUseProgram(program);
			glBindVertexArray(vao);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glUseProgram(0);
			glBindVertexArray(0);
		}
	}

	void move_to(float x_draw, float y_draw) const
	{
		if(loaded)
		{
			glUseProgram(program);
			glUniform2f(x_loc, x_draw, y_draw);
			glUseProgram(0);
		}
	}

	void change_size(const Scale_2D &shape) const
	{
		if(loaded)
		{
			glUseProgram(program);
			glUniform2f(scaling_loc, shape.width, shape.height);
			glUseProgram(0);
		}
	}

	void change_colour(const Colour& input_colour) const
	{
		if(loaded)
		{
			glUseProgram(program);
			glUniform3f(input_colour_loc, input_colour.r, input_colour.g, input_colour.b);
			glUseProgram(0);
		}
	}	  

	void unload()
	{
		if(loaded)
		{
			Shader_2D::unload();
			x_loc = 0;
			scaling_loc = 0;
			input_colour_loc = 0;
			loaded = false;
		}
	}

	~Shader_Square()
	{
		unload();
	}

private:
	bool loaded{false};

	//Uniform-Variablen Ort im Shader
	int x_loc{}, scaling_loc{}, input_colour_loc{};
};

//Einfacher Shader für ein Rechteck mit Rand
class Shader_Square_with_Border : public Shader_2D
{
public:

	Shader_Square_with_Border() {}

	void load(const OpenGL_Shape& shape = Shapes::square)
	{
		if (!loaded)
		{
			Shader_2D::load(shape, "monocolour_border_shader");

			//Position Uniform-Variablen.
			x_loc = glGetUniformLocation(program, "x");
			scaling_loc = glGetUniformLocation(program, "scaling");
			input_colour_loc = glGetUniformLocation(program, "input_colour");
			border_colour_loc = glGetUniformLocation(program, "border_colour");
			border_thickness_loc = glGetUniformLocation(program, "border_thickness");

			loaded = true;
		}
	}

	void draw() const
	{
		if (loaded)
		{
			glUseProgram(program);
			glBindVertexArray(vao);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glUseProgram(0);
			glBindVertexArray(0);
		}
	}

	void move_to(float x_draw, float y_draw) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(x_loc, x_draw, y_draw);
			glUseProgram(0);
		}
	}

	void change_size(const Scale_2D &shape, const Scale_2D& border = {0.005f,0.005f}) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(scaling_loc, shape.width, shape.height);
			glUniform2f(border_thickness_loc, border.width, border.height);
			glUseProgram(0);
		}
	}

	void change_colour(const Colour& input_colour, const Colour& border_colour = {0.0f,0.0f,0.0f}) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform3f(input_colour_loc, input_colour.r, input_colour.g, input_colour.b);
			glUniform3f(border_colour_loc, border_colour.r, border_colour.g, border_colour.b);
			glUseProgram(0);
		}
	}

	void unload()
	{
		if (loaded)
		{
			Shader_2D::unload();
			x_loc = 0;
			scaling_loc = 0;
			input_colour_loc = 0;
			border_colour_loc = 0;
			border_thickness_loc = 0;
			loaded = false;
		}
	}

	~Shader_Square_with_Border()
	{
		unload();
	}

private:
	bool loaded{ false };

	//Uniform-Variablen Ort im Shader
	unsigned int x_loc{}, scaling_loc{}, input_colour_loc{};
	unsigned int border_colour_loc{}, border_thickness_loc{};
};

//Einfacher Shader für ein Rechteck mit einfarbiger Textur.
class Shader_Square_Textured_Monocolour : public Shader_2D_Textured
{
public:

	Shader_Square_Textured_Monocolour() {}

	void load(const std::string &texture_path, const OpenGL_Shape& shape = Shapes::square_textured)
	{
		if (!loaded)
		{
			Shader_2D_Textured::load(texture_path, shape, "monocolour_texture_shader");

			//Position Uniform-Variablen.
			x_loc = glGetUniformLocation(program, "x");
			x_texture_loc = glGetUniformLocation(program, "x_texture");
			scaling_loc = glGetUniformLocation(program, "scaling");
			scaling_texture_loc = glGetUniformLocation(program, "scaling_texture");
			input_colour_loc = glGetUniformLocation(program, "input_colour");
			loaded = true;
		}
	}

	void draw() const
	{
		if (loaded)
		{
			glUseProgram(program);
			glBindVertexArray(vao);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glUseProgram(0);
			glBindVertexArray(0);
		}
	}

	void move_to(float x_draw, float y_draw) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(x_loc, x_draw, y_draw);
			glUseProgram(0);
		}
	}

	//Bewege das Quadrat, welches den Ausschnitt der Textur zeigt, den es zu zeichnen gilt.
	void move_texture_square(float x_new, float y_new) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(x_texture_loc, x_new + 0.5f*texcel_width, y_new + 0.5f * texcel_height);
			glUseProgram(0);
		}
	}

	void change_size(const Scale_2D& shape) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(scaling_loc, shape.width, shape.height);
			glUseProgram(0);
		}
	}

	//Ändere die Grösse des Quadrats, welches den Ausschnitt der Textur zeigt, den es zu zeichnen gilt.
	void change_size_texture_square(float width, float height) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(scaling_texture_loc, width, height);
			glUseProgram(0);
		}
	}

	void change_colour(const Colour& input_colour) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform3f(input_colour_loc, input_colour.r, input_colour.g, input_colour.b);
			glUseProgram(0);
		}
	}

	void unload()
	{
		if (loaded)
		{
			Shader_2D_Textured::unload();
			x_loc = 0;
			scaling_loc = 0;
			input_colour_loc = 0;
			loaded = false;
		}
	}

	~Shader_Square_Textured_Monocolour()
	{
		unload();
	}

private:
	bool loaded{ false };

	//Uniform-Variablen Ort im Shader
	int x_loc{}, x_texture_loc{}, scaling_loc{}, scaling_texture_loc{}, input_colour_loc{};
};

//Einfacher Shader für ein Rechteck mit Textur.
class Shader_Square_Textured : public Shader_2D_Textured
{
public:

	Shader_Square_Textured() {}

	void load(const std::string& texture_path, const OpenGL_Shape& shape = Shapes::square_textured)
	{
		if (!loaded)
		{
			Shader_2D_Textured::load(texture_path, shape, "texture_shader");

			//Position Uniform-Variablen.
			x_loc = glGetUniformLocation(program, "x");
			x_texture_loc = glGetUniformLocation(program, "x_texture");
			scaling_loc = glGetUniformLocation(program, "scaling");
			scaling_texture_loc = glGetUniformLocation(program, "scaling_texture");
			flip_loc = glGetUniformLocation(program,"flip");
			loaded = true;
		}
	}

	void draw() const
	{
		if (loaded)
		{
			glUseProgram(program);
			glBindVertexArray(vao);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glUseProgram(0);
			glBindVertexArray(0);
		}
	}

	void move_to(float x_draw, float y_draw) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(x_loc, x_draw, y_draw);
			glUseProgram(0);
		}
	}

	//Bewege das Quadrat, welches den Ausschnitt der Textur zeigt, den es zu zeichnen gilt.
	void move_texture_square(float x_new, float y_new) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(x_texture_loc, x_new + 0.5f * texcel_width, y_new + 0.5f * texcel_height);
			glUseProgram(0);
		}
	}

	void change_size(const Scale_2D& shape) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(scaling_loc, shape.width, shape.height);
			glUseProgram(0);
		}
	}

	//Ändere die Grösse des Quadrats, welches den Ausschnitt der Textur zeigt, den es zu zeichnen gilt.
	void change_size_texture_square(float width, float height) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(scaling_texture_loc, width, height);
			glUseProgram(0);
		}
	}

	void flip(bool flip=true)
	{
		glUseProgram(program);
		if(flip)
			glUniform1i(flip_loc, true);
		else
			glUniform1i(flip_loc, false);
		glUseProgram(0);
	}

	void unload()
	{
		if (loaded)
		{
			Shader_2D_Textured::unload();
			x_loc = 0;
			scaling_loc = 0;
			loaded = false;
		}
	}

	~Shader_Square_Textured()
	{
		unload();
	}

private:
	bool loaded{ false };

	//Uniform-Variablen Ort im Shader
	int x_loc{}, x_texture_loc{}, scaling_loc{}, scaling_texture_loc{}, flip_loc{};
};

//Shader mit Textur, wo der Hintergrund von einer Farbe in die andere übergeht.
class Shader_Portrait : public Shader_2D_Textured
{
public:

	Shader_Portrait() {}

	void load(const std::string& texture_path, const OpenGL_Shape& shape = Shapes::square_textured)
	{
		if (!loaded)
		{
			Shader_2D_Textured::load(texture_path, shape, "portrait_shader");

			//Position Uniform-Variablen.
			x_loc = glGetUniformLocation(program, "x");
			x_texture_loc = glGetUniformLocation(program, "x_texture");
			scaling_loc = glGetUniformLocation(program, "scaling");
			scaling_texture_loc = glGetUniformLocation(program, "scaling_texture");
			flip_loc = glGetUniformLocation(program, "flip");
			colour1_loc = glGetUniformLocation(program, "colour1");
			colour2_loc = glGetUniformLocation(program, "colour2");
			loaded = true;
		}
	}

	void draw() const
	{
		if (loaded)
		{
			glUseProgram(program);
			glBindVertexArray(vao);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glUseProgram(0);
			glBindVertexArray(0);
		}
	}

	void move_to(float x_draw, float y_draw) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(x_loc, x_draw, y_draw);
			glUseProgram(0);
		}
	}

	//Bewege das Quadrat, welches den Ausschnitt der Textur zeigt, den es zu zeichnen gilt.
	void move_texture_square(float x_new, float y_new) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(x_texture_loc, x_new + 0.5f * texcel_width, y_new + 0.5f * texcel_height);
			glUseProgram(0);
		}
	}

	void change_size(const Scale_2D& shape) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(scaling_loc, shape.width, shape.height);
			glUseProgram(0);
		}
	}

	//Ändere die Grösse des Quadrats, welches den Ausschnitt der Textur zeigt, den es zu zeichnen gilt.
	void change_size_texture_square(float width, float height) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(scaling_texture_loc, width, height);
			glUseProgram(0);
		}
	}

	void change_background_colours(const Colour &colour1, const Colour &colour2)
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform3f(colour1_loc, colour1.r, colour1.g, colour1.b);
			glUniform3f(colour2_loc, colour2.r, colour2.g, colour2.b);
			glUseProgram(0);
		}
	}

	void flip(bool flip = true)
	{
		glUseProgram(program);
		if (flip)
			glUniform1i(flip_loc, true);
		else
			glUniform1i(flip_loc, false);
		glUseProgram(0);
	}

	void unload()
	{
		if (loaded)
		{
			Shader_2D_Textured::unload();
			x_loc = 0;
			scaling_loc = 0;
			loaded = false;
		}
	}

	~Shader_Portrait()
	{
		unload();
	}

private:
	bool loaded{ false };

	//Uniform-Variablen Ort im Shader
	int x_loc{}, x_texture_loc{}, scaling_loc{}, scaling_texture_loc{}, flip_loc{}, colour1_loc{}, colour2_loc{};
};

//Shader, der Linien zeichnet
class Shader_Lines : public Shader_2D
{
public:

	Shader_Lines() {}

	void load(const OpenGL_Shape& shape, int n=1)
	{
		if (!loaded)
		{
			n_lines = n;
			Shader_2D::load(shape, "monocolour_shader");

			//Position Uniform-Variablen.
			x_loc = glGetUniformLocation(program, "x");
			scaling_loc = glGetUniformLocation(program, "scaling");
			input_colour_loc = glGetUniformLocation(program, "input_colour");

			loaded = true;
		}
	}

	void draw() const
	{
		if (loaded)
		{
			glUseProgram(program);
			glBindVertexArray(vao);
			glDrawElements(GL_LINES, 2*n_lines, GL_UNSIGNED_INT, 0);
			glUseProgram(0);
			glBindVertexArray(0);
		}
	}

	void move_to(float x_draw, float y_draw) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(x_loc, x_draw, y_draw);
			glUseProgram(0);
		}
	}

	void change_size(const Scale_2D& shape) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform2f(scaling_loc, shape.width, shape.height);
			glUseProgram(0);
		}
	}

	void change_colour(const Colour& input_colour) const
	{
		if (loaded)
		{
			glUseProgram(program);
			glUniform3f(input_colour_loc, input_colour.r, input_colour.g, input_colour.b);
			glUseProgram(0);
		}
	}

	void unload()
	{
		if (loaded)
		{
			Shader_2D::unload();
			x_loc = 0;
			scaling_loc = 0;
			input_colour_loc = 0;
			loaded = false;
		}
	}

	~Shader_Lines()
	{
		unload();
	}

private:
	bool loaded{ false };
	int n_lines{ 1 };

	//Uniform-Variablen Ort im Shader
	int x_loc{}, scaling_loc{}, input_colour_loc{};
};

//Klassen, die Shader benötigen

//Bitmap-Struct, für Text
struct Bitmap
{
	int n_columns{};
	int n_rows{};
	std::vector<char> glyph_list{};
};

namespace Bitmap_List
{
	Bitmap custom_simple{13,7,
		{ '0','1','2','3','4','5','6','7','8','9','0','0',' ',
		'A','B','C','D','E','F','G','H','I','J','K','L','M',
		'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
		'a','b','c','d','e','f','g','h','i','j','k','l','m',
		'n','o','p','q','r','s','t','u','v','w','x','y','z',
		'.',':',',',';','!','?','\"','_','%','+','-','*','=',
		'(',')','[',']','{','}','<','>','/','\\','$','&','~' } };
};

//Um Text via einer Bitmap darzustellen. Beinhaltet seinen eigenen Shader.
class Text_Bitmap : public Shader_2D_Textured
{
public:
	Text_Bitmap(const std::string& text = "TEST", float x = 0.0f, float y = 0.0f, float text_size = 0.1f, const Colour &colour = Colour_List::black, 
		const std::string& font_path = "Fonts\\Font_Digital.png", Bitmap bitmap = Bitmap_List::custom_simple) : 
		text{ text }, x{x}, y{y}, glyph{text_size, 2.0f*text_size}, text_colour{colour}, font_path{ font_path }, bitmap{bitmap} {}

	void load()
	{
		if (!loaded)
		{
			shader.load(font_path, Shapes::glyph);
			update_glyph_size();
			update_glyph_coordinates();

			//Inputs
			move_to(x,y);
			change_glyph_size(glyph);
			change_colour(text_colour);

			loaded = true;
		}
	}

	void draw()
	{
		if (loaded)
		{
			//Zeichne den Text
			int i{ 0 }, j{ 0 };
			for (int k{ 0 }; k < text.size(); k++)
			{
				//Zeichne den nächsten Glyphen, falls kein Absatz kommt.
				if (text[k] != '\n')
				{
					//Setze die Textur-Koordinaten des nächsten Glyphen
					shader.move_texture_square(x_array[k], y_array[k]);

					//Zeichne den nächsten Glyphen
					shader.move_to(x + i * glyph.width, y - j * glyph.height);
					shader.draw();

					//Teste, ob der nächste Glyph über den rechten Rand hinaus geht.
					if (x + static_cast<float>(i + 2) * glyph.width < 1.0f)
					{
						i++;
					}
					else
					{
						i = 0;
						j++;
					}

				}
				else //Falls ein Absatz kommt
				{
					i = 0;
					j++;
				}
			}
		}
	}

	void move_to(float x_new, float y_new)
	{
		x = x_new;
		y = y_new;
		shader.move_to(x_new,y_new);
	}

	void change_text(const std::string &new_text)
	{
		text = new_text;
		update_glyph_coordinates();
	}

	void change_bitmap(const Bitmap &bitmap_new)
	{
		bitmap = bitmap_new;
		update_glyph_size();
		update_glyph_coordinates();
	}

	void change_glyph_size(const Scale_2D &new_glyph)
	{
		glyph = new_glyph;
		shader.change_size(glyph);
	}

	void change_text_size(const Scale_2D& new_glyph)
	{
		glyph = new_glyph;
		glyph.width = glyph.width / static_cast<float>(get_number_of_letters());
		glyph.height = glyph.height / static_cast<float>(get_number_of_paragraphs());
		shader.change_size(glyph);
	}

	void change_colour(const Colour& input_colour)
	{
		text_colour = input_colour;
		shader.change_colour(input_colour);
	}

	std::string string()
	{
		return text;
	}

	//Bestimmt wie viele Zeichen die erste Zeile des Texts hat.
	int get_number_of_letters() const
	{
		int n_letters{ 0 };
		bool test{ true };
		while (test)
		{
			if (text[n_letters] == '\n' || text[n_letters] == '\0')
			{
				test = false;
				break;
			}
			n_letters++;
		}
		return n_letters;
	}

	//Bestimme, wie viele Absätze der Text hat, inklusive automatisch eingefügte Absätze.
	int get_number_of_paragraphs() const
	{
		//Gehe durch den gesamten Text durch und teste auf Absätze
		int i{ 0 }, j{ 1 };
		for (int k{ 0 }; k < text.size(); k++)
		{
			//Falls kein Absatz kommt
			if (text[k] != '\n')
			{
				//Teste, ob der nächste Glyph über den rechten Rand hinaus geht.
				if (x + static_cast<float>(i + 2) * glyph.width < 1.0f)
				{
					i++;
				}
				else
				{
					i = 0;
					j++;
				}

			}
			else //Falls ein Absatz kommt
			{
				i = 0;
				j++;
			}
		}
		return j;
	}

	//Bestimme, wie viele "echte" Absätze der Text hat.
	int get_number_of_real_paragraphs() const
	{
		//Gehe durch den gesamten Text durch und teste auf Absätze
		int j{ 1 };
		for (int k{ 0 }; k < text.size(); k++)
		{
			//Falls ein Absatz kommt
			if (text[k] == '\n')
				j++;
		}
		return j;
	}

	//Bestimme, wie lange die erste Zeile des Textes ist.
	float get_length() const
	{
		return static_cast<float>(get_number_of_letters()) * glyph.width;
	}

	//Bestimme, die hoch der Text ist
	float get_height(bool only_real = 0) const
	{
		if(only_real)
			return static_cast<float>(get_number_of_real_paragraphs()) * glyph.height;
		else
			return static_cast<float>(get_number_of_paragraphs()) * glyph.height;
	}

	std::string get_text() const
	{
		return text;
	}

	//Zentriere den Text horizontal zur Bildmitte anhand der ersten Zeile.
	void centre_text_horizontally()
	{
		move_to(0.0f - 0.5f * get_length(), y);
	}

	//Zentriere den Text vertikal zur Bildmitte.
	void centre_text_vertically()
	{
		//0.25, da der Mittelpunkt vom Glyphen etwas über dem Rand liegt.
		move_to(x, 0.0f - 0.25f * get_height());
	}

	void unload()
	{
		if (loaded)
		{
			shader.unload();
			loaded = false;
		}
	}

	~Text_Bitmap()
	{
		unload();
	}

private:
	bool loaded{ false };

	//Eigenschaften Text
	std::string text;
	std::string font_path;
	float x, y;
	Scale_2D glyph;
	Colour text_colour;

	//Arrays, um die Textur-Koordinaten der Glyphen zu speichern.
	std::vector<float> x_array;
	std::vector<float> y_array;

	//Bitmap Eigenschaften
	Bitmap bitmap;
	float glyph_width{}, glyph_height{};

	//Shader
	Shader_Square_Textured_Monocolour shader{};

	//Ändert die Grösse des Glyphs anhand der Bitmap-Eigenschaften
	void update_glyph_size()
	{
		glyph_width = 1.0f / static_cast<float>(bitmap.n_columns);
		glyph_height = 1.0f / static_cast<float>(bitmap.n_rows);
		shader.change_size_texture_square(glyph_width, glyph_height);
	}

	//Bestimmt, wo auf der Bitmap sich ein Glyph befindet.
	void subroutine_get_glyph_coordinates(char glyph, float& x_texture, float& y_texture)
	{
		bool found_glyph{ false };
		int i{ 0 }, j{ 1 };
		for (int k{ 0 }; k < (bitmap.n_columns * bitmap.n_rows); k++)
		{
			//Zeichne den Glyphen
			if (glyph == bitmap.glyph_list[k])
			{
				x_texture = i * glyph_width;
				y_texture = 1.0f - j * glyph_height;
				found_glyph = true;
				break;
			}

			//Falls der Glyph auf der Bitmap auf einer neuen Zeile ist
			if ((k + 1) >= j * bitmap.n_columns)
			{
				i = 0;
				j += 1;
			}
			else
			{
				i++;
			}
		}

		//Default-Symbol, falls das Zeichen nicht gefunden wurde.
		if (!found_glyph)
		{
			x_texture = 10.0f * glyph_width;
			y_texture = 1.0f - glyph_height;
		}
	}

	//Setze die Textur-Koordinaten für jeden Glyphen im Text
	void update_glyph_coordinates()
	{
		float x_texture{}, y_texture{};
		x_array.resize(text.size());
		y_array.resize(text.size());

		for (int k{ 0 }; k < text.size(); k++)
		{
			subroutine_get_glyph_coordinates(text[k], x_texture, y_texture);
			x_array[k] = x_texture;
			y_array[k] = y_texture;
		}
	}
};

//Knopf, den man drücken kann.
class Button
{
public:
	Button( float x = 0.0f, float y = 0.0f, float w = 0.3f, float h = 0.15f, float border_thickness = 0.005f,
		const Colour &button_colour = {0.0f,0.0f,0.5f}, const Colour &border_colour = {0.0f,0.0f,0.0f}, bool stays_pressed = false,
		std::string button_text = " ", float text_size = 1.0f, const Colour &text_colour = {0.8f,0.8f,0.8f},
		const std::string &sfx_selected_path = "Sound_Effects\\Menu_Click.mp3", const std::string &sfx_pressed_path = "Sound_Effects\\Menu_Clap.mp3") :
		stays_pressed{stays_pressed}, x{x}, y{y}, shape{w,h}, border{ border_thickness, border_thickness },
		button_colour{button_colour}, button_colour_start{button_colour}, border_colour{border_colour},
		text{button_text,x,y,text_size}, text_shape{text_size,2.0f*text_size}, text_colour{text_colour},
		sfx_selected{sfx_selected_path,60}, sfx_pressed{sfx_pressed_path,20}
	{
		text_exists = (text.string() == " " ? false : true);
	}

	void load()
	{
		if(!loaded)
		{
			shader.load();
			shader.move_to(x,y);
			shader.change_size(shape,border);
			shader.change_colour(button_colour, border_colour);

			if (text_exists)
			{
				text.load();
				text.change_colour(text_colour);
				fit_text_into_button();
			}

			sfx_pressed.load();
			sfx_selected.load();

			loaded = true;
		}
	}

	void initialize(float xi = 0.0f, float yi = 0.0f, float w = 0.3f, float h = 0.15f, float border_thickness = 0.005f,
		const Colour& colour = { 0.0f,0.0f,0.5f }, const Colour& b_colour = { 0.0f,0.0f,0.0f }, bool stays_pressed_i = false,
		std::string button_text = " ", float text_size = 1.0f, const Colour& txt_colour = { 0.8f,0.8f,0.8f },
		const std::string& sfx_selected_path = "Sound_Effects\\Menu_Click.mp3", const std::string& sfx_pressed_path = "Sound_Effects\\Menu_Clap.mp3")
	{
		x = xi;
		y = yi;
		shape.width = w;
		shape.height = h * Game::window.ratio();
		border.width = border_thickness;
		border.height = border_thickness * Game::window.ratio();
		button_colour = colour;
		button_colour_start = colour;
		border_colour = b_colour;
		stays_pressed = stays_pressed_i;
		text_exists = (button_text == " " ? false : true);
		text_colour = txt_colour;
		sfx_selected.initialize(sfx_selected_path,60);
		sfx_pressed.initialize(sfx_pressed_path, 20);

		unload();
		load();

		text.move_to(x, y);
		text.change_text(button_text);
		text.change_glyph_size({ text_size, 2.0f * text_size });
		fit_text_into_button();	
	}

	//Gibt die Dimensionen des ganzen Textes zurück.
	Scale_2D get_text_size() const
	{
		return text_shape;
	}

	Scale_2D get_glyph_size() const
	{
		Scale_2D glyph_shape{};
		glyph_shape.width = text_shape.width / (float)text.get_number_of_letters();
		glyph_shape.height = text_shape.height / (float)text.get_number_of_paragraphs();
		return glyph_shape;
	}

	//Bewege den Text in den Knopf hinein. Ändere falls nötigt die Textgrösse, damit sie in den Knopf hinein passt.
	Scale_2D fit_text_into_button()
	{
		float d = 0.01f;
		float dx = shape.width - 2.0f * border.width - d;
		float dy = shape.height - 2.0f * border.height - d*Game::window.ratio();

		if(text.get_length() > dx || text.get_height() > dy)
		{
			if( 2.0f * dx / (float)text.get_number_of_letters() < dy )
			{
				text_shape.width = dx;
				text_shape.height = 2.0f * dx / (float)text.get_number_of_letters();
			}
			else
			{
				text_shape.height = dy;
				text_shape.width = (float)text.get_number_of_letters() * 0.5f * dy;
			}

			text.change_text_size(text_shape);
		}
		
		text.move_to(x - 0.5f * text.get_length(), y - 0.5f * text.get_height() + 2.0f * border.height);
		return text_shape;
	}
	
	void move_to(float x_new, float y_new)
	{
		x = x_new;
		y = y_new;
		shader.move_to(x,y);
		text.move_to(x - 0.5f * text.get_length(), y - 0.5f * text.get_height() + 2.0f * border.height);
	}

	void change_text(const std::string &text_new)
	{
		text_exists = (text_new == " " ? false : true);
		if(text_exists)
		{
			text.load();
			text.change_text(text_new);
			fit_text_into_button();
		}
	}

	void change_text_size(Scale_2D new_text_shape)
	{
		text_shape = new_text_shape;
		text.change_text_size(text_shape);

		glyph_shape.width = text_shape.width / (float)text.get_number_of_letters();
		glyph_shape.height = text_shape.height / (float)text.get_number_of_paragraphs();

		text.move_to(x - 0.5f * text.get_length(), y - 0.5f * text.get_height() + 2.0f * border.height);
	}

	void change_glyph_size(Scale_2D new_glyph_shape)
	{
		glyph_shape = new_glyph_shape;
		text.change_glyph_size(glyph_shape);

		text_shape.width = glyph_shape.width * (float)text.get_number_of_letters();
		text_shape.height = glyph_shape.height * (float)text.get_number_of_paragraphs();

		text.move_to(x - 0.5f * text.get_length(), y - 0.5f * text.get_height() + 2.0f * border.height);
	}

	void change_size(const Scale_2D &shape_new, const Scale_2D &border_shape_new = {0.005f,0.005f})
	{
		shape = shape_new;
		border = border_shape_new;
		shader.change_size(shape, border);
	}

	void change_colour(const Colour &new_colour, const Colour &new_border_colour = {0.0f,0.0f,0.0f}, const Colour &text_colour_new = { 0.8f,0.8f,0.8f })
	{
		button_colour = new_colour;
		border_colour = new_border_colour;
		text_colour = text_colour_new;
		shader.change_colour(button_colour, border_colour);
		text.change_colour(text_colour);
	}

	//Passt Sound und Farbe je nachdem an, ob der Knopf ausgewählt/gedrückt wurde.
	void check_input()
	{
		constexpr float V{ 5.0f };
		if(active)
		{
			if (selected && !selected_old)
			{
				sfx_selected.play();
				change_colour(button_colour.change_brightness(V),border_colour,text_colour);
			}
			else if (!selected && selected_old)
			{
				if(pressed)
					change_colour(button_colour_start.change_brightness(V), border_colour, text_colour);
				else
					change_colour(button_colour_start, border_colour, text_colour);
			}

			if (pressed && !pressed_old)
			{
				sfx_pressed.play();
				if (stays_pressed == true)
					change_colour(button_colour.change_brightness(V), border_colour, text_colour);
			}
			else if (!pressed && pressed_old && stays_pressed == true)
			{
				sfx_pressed.play();
				if(selected)
					change_colour(button_colour_start.change_brightness(V), border_colour, text_colour);
				else
					change_colour(button_colour_start, border_colour, text_colour);
			}
		}
	}

	bool check_mouse()
	{
		selected_old = selected;
		pressed_old = pressed;

		//Setze standardmässig den Knopf als nicht gedrückt.
		if(stays_pressed == false)
			pressed = false;

		//Teste, ob die Maus über dem Knopf ist
		bool x_col{ x - 0.5f * shape.width  <= Game::mouse.x && Game::mouse.x <= x + 0.5f * shape.width };
		bool y_col{ y - 0.5f * shape.height <= Game::mouse.y && Game::mouse.y <= y + 0.5f * shape.height };

		if (active && loaded)
		{
			if (x_col && y_col)
			{
				selected = true;

				//Teste, ob der Knopf gedrückt wurde
				if (Game::mouse.left_click && !Game::mouse.left_click_old && stays_pressed == false)
					pressed = true;

				//Wechsle zwischen gedrückt und nicht gedrückt.
				if(Game::mouse.left_click == true && Game::mouse.left_click_old == false && stays_pressed == true)
				{
					if (pressed)
						pressed = false;
					else
						pressed = true;
				}
			}
			else
			{
				selected = false;
			}

			check_input();
		}

		return x_col && y_col;
	}

	void draw()
	{
		shader.draw();
		if(text_exists)
			text.draw();
	}

	void unload()
	{
		if(loaded)
		{
			shader.unload();
			if (text_exists)
				text.unload();
			sfx_pressed.unload();
			sfx_selected.unload();
			loaded = false;
		}
	}

	~Button()
	{
		unload();
	}

	friend class Choice_Box;
	bool active{true};
	bool selected{ false }, selected_old{ false };	//Ob der Knopf mit dem Cursor ausgewählt wurde.
	bool pressed{ false }, pressed_old{ false };	//Ob der Knopf gedrückt wurde.

private:
	bool loaded{ false };
	bool stays_pressed;
	Shader_Square_with_Border shader{};

	//Eigenschaften Knopf
	float x, y;
	Scale_2D shape, border;
	Colour button_colour, button_colour_start, border_colour;

	//Text
	bool text_exists{true};
	Text_Bitmap text;
	Scale_2D text_shape{}, glyph_shape{};
	Colour text_colour;

	//Sounds um Knöpfe auszuwählen
	Sound_Effect sfx_selected;
	Sound_Effect sfx_pressed;
};

//Fenster mit einer Ja oder Nein Option.
class Choice_Box
{
public:
	Choice_Box(float x = 0.0f, float y = 0.0f, float box_size = 0.3f, float border_thickness = 0.01f,
		const std::string &text = "QUIT GAME?", const std::string &text_yes = "YES", const std::string &text_no = "NO",
		const Colour &box_colour = { 0.0f,0.5f,0.5f }, const Colour &border_colour = { 0.0f,0.2f,0.2f }, 
		const Colour &button_colour = { 0.0f,0.0f,0.2f }, const Colour &button_border_colour = { 0.3f, 0.4f, 0.5f },
		const Colour &text_colour = Colour_List::black) :
		x{x}, y{y}, box_shape{2.5f*box_size, box_size}, border_shape{border_thickness,border_thickness},
		txt_message{text}, button_txt_yes{text_yes}, button_txt_no{text_no},
		box_colour{box_colour}, border_colour{border_colour}, button_colour{button_colour}, button_border_colour{button_border_colour}
	{}

	void load()
	{
		if(!loaded)
		{
			shader.load();
			shader.change_size(box_shape, border_shape);
			shader.change_colour(box_colour,border_colour);

			constexpr float button_size = 0.3f;
			constexpr float button_offset = 0.01f;

			button_yes.load();
			button_yes.change_colour(button_colour,button_border_colour);
			button_yes.change_size( { button_size *box_shape.width, 0.5f * button_size * box_shape.width } );
			button_yes.change_text(button_txt_yes);
			button_yes.move_to(x - 0.5f * box_shape.width + border_shape.width + 0.5f * button_yes.shape.width + button_offset,
				y - 0.5f * box_shape.height + border_shape.height + 0.5f * button_yes.shape.height + button_offset);
			
			button_no.load();
			button_no.change_colour(button_colour, button_border_colour);
			button_no.change_size({ button_size * box_shape.width, 0.5f * button_size * box_shape.width });
			button_no.change_text(button_txt_no);
			button_no.move_to(x + 0.5f * box_shape.width - border_shape.width - 0.5f * button_yes.shape.width - button_offset,
				y - 0.5f * box_shape.height + border_shape.height + 0.5f * button_yes.shape.height + button_offset);

			//Passe die Textgrösse der Knöpfe aneinander an
			float button_yes_size = button_yes.get_glyph_size().width;
			float button_no_size = button_no.get_glyph_size().width;

			if (button_yes_size > button_no_size)
				button_yes.change_glyph_size( button_no.get_glyph_size());
			else
				button_no.change_glyph_size(button_yes.get_glyph_size());

			txt_message.load();
			txt_message.change_glyph_size( button_yes.get_glyph_size() );
			txt_message.move_to(0.0f, y + 0.5f*box_shape.height - border_shape.height - txt_message.get_height());
			txt_message.centre_text_horizontally();
			loaded = true;
		}
	}

	void reset_buttons()
	{
		button_yes.selected = false;
		button_yes.pressed = false;
		button_yes.check_input();

		button_no.selected = false;
		button_no.pressed = false;
		button_no.check_input();
	}

	void check_mouse()
	{
		if (Game::mouse.moved)
			mouse_active = true;

		if(active && loaded && mouse_active)
		{
			button_yes.check_mouse();
			button_no.check_mouse();
		}
	}

	void check_keyboard()
	{
		using namespace Game;

		if(active && loaded)
		{
			//Update die Variablen nur, falls das nicht schon die Maus macht.
			if(mouse_active == false)
			{
				button_yes.selected_old = button_yes.selected;
				button_yes.pressed_old = button_yes.pressed;
				button_no.selected_old = button_no.selected;
				button_no.pressed_old = button_no.pressed;

				button_yes.pressed = false;
				button_no.pressed = false;
			}

			if(keystate[key.KEY_LEFT] || keystate[key.KEY_LEFT2])
			{
				mouse_active = false;
				button_yes.selected = true;
				button_no.selected = false;
				button_yes.check_input();
				button_no.check_input();
			}
			else if (keystate[key.KEY_RIGHT] || keystate[key.KEY_RIGHT2] )
			{
				mouse_active = false;
				button_yes.selected = false;
				button_no.selected = true;
				button_yes.check_input();
				button_no.check_input();
			}

			if( keystate[key.KEY_CONFIRM] && !keystate_old[key.KEY_CONFIRM] )
			{
				if(button_yes.selected)
				{
					button_yes.pressed = true;
					button_yes.check_input();
				}
				else if (button_no.selected)
				{
					button_no.pressed = true;
					button_no.check_input();
				}
			}
			else if(!keystate[key.KEY_CONFIRM] && keystate_old[key.KEY_CONFIRM])
			{
				reset_buttons();
			}
		}
	}

	void check_inputs()
	{
		check_mouse();
		check_keyboard();
	}

	bool yes_button_pressed() const
	{
		return button_yes.pressed;
	}

	bool no_button_pressed() const
	{
		return button_no.pressed;
	}

	void draw()
	{
		if(active && loaded)
		{
			shader.draw();
			button_yes.draw();
			button_no.draw();
			txt_message.draw();
		}
	}

	void unload()
	{
		if(loaded)
		{
			shader.unload();
			button_yes.unload();
			button_no.unload();
			txt_message.unload();
			loaded = false;
		}
	}

	~Choice_Box()
	{
		unload();
	}

	bool active{false};

private:
	bool loaded{ false }, mouse_active{false};

	//Eigenschaften Box
	float x, y;
	Scale_2D box_shape;
	Scale_2D border_shape;
	Colour box_colour, border_colour;

	//Buttons
	Scale_2D button_shape;
	Button button_yes, button_no;
	Colour button_colour, button_border_colour;
	std::string button_txt_yes, button_txt_no;

	//Text
	Scale_2D text_shape;
	Text_Bitmap txt_message;

	Shader_Square_with_Border shader{};
};

enum Orientation
{
	VERTICAL,
	HORIZONTAL,
};

//Menü mit n Knopfen.
template<int n>
class Menu_List
{
public:
	Menu_List( const std::string *button_name_list, float x = 0.0f, float y = 0.0f, float w = 0.5f, float h = 0.2f, float border_thickness = 0.005f,
		const Colour& button_colour = { 0.0f,0.0f,0.5f }, const Colour& border_colour = { 0.0f,0.0f,0.0f }, 
		bool stays_pressed = false, float text_size = 0.05f, const Colour& text_colour = { 0.8f,0.8f,0.8f },
		const std::string& sfx_selected_path = "Sound_Effects\\Menu_Click.mp3", const std::string& sfx_pressed_path = "Sound_Effects\\Menu_Clap.mp3",
		Orientation modus = VERTICAL ) : modus{modus}, stays_pressed{stays_pressed}
	{
		//Bestimme den Abstand der Knöpfe voneinander
		if (modus == VERTICAL)
			dy = 0.1f * Game::window.ratio();
		else
			dx = 0.1f;

		float fmodus = (float)modus;
		std::size_t max_txt_len{ 0 }, max_txt_pos{ 0 };
		for(int i=0; i<n; i++)
		{
			//Bestimme die Textgrösse für alle Knöpfe.
			if (max_txt_len < button_name_list[i].length())
			{
				max_txt_len = button_name_list[i].length();
				max_txt_pos = i;
			}

			//Initialisiere die einzelnen Knöpfe
			button_names[i] = button_name_list[i];
			buttons[i].initialize(x + fmodus * i * (1.0f+dx) * w, y + (fmodus-1.0f) * i * (1.0f+dy) * h * Game::window.ratio(),
				w, h, border_thickness,
				button_colour, border_colour, stays_pressed,
				button_names[i], text_size, text_colour,
				sfx_selected_path, sfx_pressed_path);	
		}

		//Passe die Textgrösse an
		glyph_size = buttons[max_txt_pos].get_glyph_size();
		for(int i=0; i<n; i++)
		{
			buttons[i].change_glyph_size(glyph_size);
		}

		//std::cout << button_name_list[0] << ": " << d_text.width << ", " << d_text.height / Game::window.ratio() << '\n';
	}

	void load()
	{
		if(!loaded)
		{
			for(int i=0; i<n; i++)
			{
				buttons[i].load();
			}
			loaded = true;
		}
	}

	void draw()
	{
		if(loaded)
		{
			for(int i=0; i<n; i++)
			{
				buttons[i].draw();
			}
		}
	}

	Scale_2D get_glyph_size() const
	{
		return glyph_size;
	}

	void change_glyph_size(const Scale_2D &d_text)
	{
		for (int i = 0; i < n; i++)
		{
			buttons[i].change_glyph_size(d_text);
		}
	}

	void check_inputs()
	{
		position_old = position;
		check_mouse();
		check_keyboard();
	}

	void check_mouse()
	{
		if (Game::mouse.moved)
			mouse_active = true;

		if(mouse_active && loaded && active)
		{
			for(int i=0; i<n; i++)
			{
				buttons[i].check_mouse();

				if (buttons[i].pressed && i != position_old)
				{
					position = i;
				}
			}
			
			for(int i=0; i<n; i++)
			{
				if(i!=position)
				{
					buttons[i].pressed = false;
					buttons[i].check_input();
				}
			}
		}
	}

	void check_keyboard()
	{
		using namespace Game;
		if(loaded && active && keyboard_active)
		{
			bool up_button_pressed{};
			bool down_button_pressed{};
			if (modus == VERTICAL)
			{
				up_button_pressed = (keystate[key.KEY_UP] && !keystate_old[key.KEY_UP] ) 
									|| (keystate[key.KEY_UP2] && !keystate_old[key.KEY_UP2]);
				down_button_pressed = (keystate[key.KEY_DOWN] && !keystate_old[key.KEY_DOWN])
									|| (keystate[key.KEY_DOWN2] && !keystate_old[key.KEY_DOWN2]);
			}
			else if (modus == HORIZONTAL)
			{
				up_button_pressed = (keystate[key.KEY_LEFT] && !keystate_old[key.KEY_LEFT])
					|| (keystate[key.KEY_LEFT2] && !keystate_old[key.KEY_LEFT2]);
				down_button_pressed = (keystate[key.KEY_RIGHT] && !keystate_old[key.KEY_RIGHT])
					|| (keystate[key.KEY_RIGHT2] && !keystate_old[key.KEY_RIGHT2]);
			}
		
			//Update die Variablen nur, falls das nicht schon die Maus macht.
			if (mouse_active == false)
			{
				for(int i=0; i<n; i++)
				{
					buttons[i].selected_old = buttons[i].selected;
					buttons[i].pressed_old = buttons[i].pressed;
				}
			}

			if(up_button_pressed)
			{
				if (mouse_active)
				{
					position = 1;
					mouse_active = false;
				}

				buttons[position].selected = false;
				buttons[position].check_input();
				if (position > 0)
					position -= 1;
				else
					position = n - 1;
				buttons[position].selected = true;
				buttons[position].check_input();
			}
			else if(down_button_pressed)
			{
				if (mouse_active)
				{
					position = n-1;
					mouse_active = false;
				}

				buttons[position].selected = false;
				buttons[position].check_input();
				if (position < n-1)
					position += 1;
				else
					position = 0;
				buttons[position].selected = true;
				buttons[position].check_input();
			}
			else if( keystate[key.KEY_CONFIRM] && !keystate_old[key.KEY_CONFIRM])
			{
				for(int i=0; i<n; i++)
				{
					if(i!=position)
					{
						buttons[i].pressed = false;
						buttons[i].check_input();
					}
				}

				if(stays_pressed && buttons[position].pressed)
					buttons[position].pressed = false;
				else if(stays_pressed && !buttons[position].pressed)
					buttons[position].pressed = true;
				else
					buttons[position].pressed = true;

				buttons[position].check_input();
			}
			else if(!keystate[key.KEY_CONFIRM] && keystate_old[key.KEY_CONFIRM] && !stays_pressed)
			{
				buttons[position].pressed = false;
				buttons[position].check_input();
			}
		}
	}

	void select_button(int i)
	{
		mouse_active = false;
		position = i;
		buttons[i].selected = true;
		buttons[i].selected_old = false;
		buttons[i].check_input();
	}

	void activate()
	{
		active = true;
		for (int i = 0; i < n; i++)
		{
			buttons[i].active = true;
		}
	}

	void deactivate()
	{
		active = false;
		for (int i = 0; i < n; i++)
		{
			buttons[i].active = false;
			buttons[i].pressed = false;
			buttons[i].selected = false;
			buttons[i].check_input();
		}
	}

	void reset_buttons()
	{
		for(int i=0; i<n; i++)
		{
			buttons[i].pressed = false;
			buttons[i].selected = false;
			buttons[i].check_input();
		}
	}

	void unload()
	{
		if(loaded)
		{
			loaded = false;
		}
	}

	~Menu_List()
	{
		unload();
	}

	Button buttons[n]{};
	bool keyboard_active{ true };
	int position{0}, position_old{0};

private:
	bool loaded{false}, mouse_active{false}, stays_pressed{false}, active{true};
	Orientation modus;

	//Button-Eigenschaften
	std::string button_names[n]{};
	Scale_2D glyph_size{};
	float dx{0.0f}, dy{0.0f}; //Abstand Knöpfe voneinander
};

//Slider-Objekt für Menüs
class Slider
{
public:
	Slider(float x = 0.0f, float y = 0.0f, float width = 0.5f, float height = 0.05f,
		Colour colour_box = { 0.3f,0.3f,0.3f }, Colour border_colour_box = { 0.4f,0.4f,0.4f },
		Colour colour_slider = {0.9f,0.9f,0.9f}, Colour border_colour_slider = {0.0f,0.0f,0.0f}):
		x_box{x}, y_box{y}, x_slider{x}, y_slider{y}, x_slider_old{ x }, y_slider_old{ y }, x_slider_draw{x}, y_slider_draw{y},
		shape_box{width,height}, shape_slider{2.0f*height,height},
		colour_box{colour_box}, border_colour_box{border_colour_box}, colour_slider{colour_slider}, border_colour_slider{border_colour_slider}
	{}

	void load()
	{
		if(!loaded)
		{
			shader_box.load();
			shader_box.move_to(x_box,y_box);
			shader_box.change_size(shape_box);
			shader_box.change_colour(colour_box,border_colour_box);

			shader_slider.load();
			shader_slider.move_to(x_slider_draw, y_slider_draw);
			shader_slider.change_size(shape_slider);
			shader_slider.change_colour(colour_slider, border_colour_slider);

			loaded = true;
		}
	}

	void draw()
	{
		shader_box.draw();
		shader_slider.draw();
	}

	void check_mouse()
	{
		bool x_col = (x_box - 0.5f * shape_box.width < Game::mouse.x) && (Game::mouse.x < x_box + 0.5f * shape_box.width);
		bool y_col = (y_box - 0.5f * shape_box.height < Game::mouse.y) && (Game::mouse.y < y_box + 0.5f * shape_box.height);

		if(x_col && y_col && Game::mouse.left_click)
		{
			x_slider = Game::mouse.x;
		}
	}

	void check_keyboard()
	{
		using namespace Game;
		if(keyboard_active)
		{
			bool coll_left = x_box - 0.5f * shape_box.width < x_slider  - 0.5f * shape_slider.width;
			bool coll_right = x_box + 0.5f * shape_box.width > x_slider + 0.5f * shape_slider.width;
			if ((keystate[key.KEY_LEFT] || keystate[key.KEY_LEFT2]) && coll_left)
			{
				v_slider = -v_slider_max;
			}
			else if ((keystate[key.KEY_RIGHT] || keystate[key.KEY_RIGHT2]) && coll_right)
			{
				v_slider = +v_slider_max;
			}
			else
				v_slider = 0.0;
		}
	}

	void update_physics(double dt)
	{
		x_slider_old = x_slider;
		x_slider += v_slider * dt;
	}

	void update_graphics(double alpha = 1.0)
	{
		x_slider_draw = static_cast<float>(alpha * x_slider + (1.0 - alpha) * x_slider_old);
		y_slider_draw = static_cast<float>(alpha * y_slider + (1.0 - alpha) * y_slider_old);
		shader_slider.move_to(x_slider_draw, y_slider_draw);
	}

	//Float zwischen 0 (ganz links) und 1 (ganz rechts)
	float get_slider_position()
	{
		return ( (float)x_slider - (x_box-0.5f*shape_box.width)) / shape_box.width;
	}

	void unload()
	{
		if(loaded)
		{
			shader_box.unload();
			shader_slider.unload();
			loaded = false;
		}
	}

	~Slider()
	{
		unload();
	}

	bool keyboard_active{ true };

private:
	bool loaded{ false };

	//Physics
	double x_slider, y_slider, x_slider_old, y_slider_old, v_slider{0.0};
	const double v_slider_max{ 0.5 };

	//Shader
	float x_box, y_box, x_slider_draw, y_slider_draw;
	Scale_2D shape_box, shape_slider;
	Colour colour_box, colour_slider, border_colour_box, border_colour_slider;
	Shader_Square_with_Border shader_box{}, shader_slider{};
};

//Textbox
class TextBox
{
public:
	TextBox(const std::vector<std::string> &text_list, float text_size = 0.05f,
		const Colour &box_colour = { 0.0f,0.5f,0.5f }, const Colour &border_colour = { 0.0f,0.2f,0.2f }, const Colour &text_colour = Colour_List::black):
		txt_list{text_list}, n{text_list.size()}, box_shape{ 1.65f, 3.0f * txt_size * Game::window.ratio() }, txt_size{text_size},
		box_colour{box_colour}, border_colour{border_colour}, txt_colour{text_colour}
	{
		y_box = -1.0f*box_shape.height;
	}

	void load()
	{
		if (!loaded)
		{
			shader.load();
			shader.change_size(box_shape,{border_thickness,border_thickness});
			shader.change_colour(box_colour,border_colour);
			shader.move_to(x_box, y_box);

			txt_current.load();
			txt_current.change_text( txt_list[0] );
			txt_current.change_colour(txt_colour);
			txt_current.change_glyph_size({txt_size,2.0f*txt_size});
			txt_current.move_to( -0.5f*box_shape.width + border_thickness, 
				y_box + 0.5f*box_shape.height - (2.0f*txt_size + border_thickness)*Game::window.ratio());
			loaded = true;
		}
	}

	void change_text(const std::vector<std::string>& text_list)
	{
		n = text_list.size();
		txt_list.clear();
		for(int i=0; i<n; i++)
		{
			txt_list.push_back(text_list[i]);
		}
		txt_current.change_text(txt_list[0]);
		txt_position = 0;
	}

	void centre_text()
	{
		txt_current.centre_text_horizontally();
	}

	void uncentre_text()
	{
		txt_current.move_to(-0.5f * box_shape.width + border_thickness,
			y_box + 0.5f * box_shape.height - (2.0f * txt_size + border_thickness) * Game::window.ratio());
	}

	void move_on()
	{
		if (txt_position < n-1)
		{
			txt_position += 1;
			txt_current.change_text(txt_list[txt_position]);
		}
		else
			active = false;
	}

	void reset()
	{
		txt_position = 0;
		txt_current.change_text(txt_list[txt_position]);
		active = false;
	}

	void check_mouse()
	{
		bool x_col = (x_box - 0.5f * box_shape.width < Game::mouse.x) && (Game::mouse.x < x_box + 0.5f * box_shape.width);
		bool y_col = (y_box - 0.5f * box_shape.height < Game::mouse.y) && (Game::mouse.y < y_box + 0.5f * box_shape.height);

		if (x_col && y_col && Game::mouse.left_click && !Game::mouse.left_click_old)
		{
			move_on();
		}
	}

	void check_keyboard()
	{
		using namespace Game;
		if( keystate[key.KEY_CONFIRM] && !keystate_old[key.KEY_CONFIRM] )
		{
			move_on();
		}
	}

	void check_inputs()
	{
		check_mouse();
		check_keyboard();
	}

	void draw()
	{
		if(active)
		{
			shader.draw();
			txt_current.draw();
		}
	}

	void unload()
	{
		if (loaded)
		{
			shader.unload();
			loaded = false;
		}
	}

	~TextBox()
	{
		unload();
	}

	bool active{ false };
	int txt_position{ 0 };

private:
	bool loaded{ false };

	//Text
	Text_Bitmap txt_current{};
	std::size_t n;
	std::vector<std::string> txt_list;
	Colour txt_colour;
	float txt_size;

	//Textbox
	float x_box{ 0.0f }, y_box{ 0.0f };
	Scale_2D box_shape;
	const float border_thickness{0.01f};
	Colour box_colour, border_colour;
	Shader_Square_with_Border shader{};
};