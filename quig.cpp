/*
	quig - quick lua game system
	(C) 2020-2022 B.M.Deeal <brenden.deeal@gmail.com>
	
	This program is free software: you can redistribute it and/or modify it under the terms of version 3 of the GNU General Public License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program.
    If not, see <https://www.gnu.org/licenses/>.

	---
	
	other, major TODO:
	* really should get this building on Windows with something other than MSYS2 so I don't need to distribute a gazillion DLLs
	  quig doesn't use webp or tiff or opus (actually, we probably should support opus audio), why do I need to include those
	  EDIT: we can build with Visual Studio now, although we don't build the dependencies ourselves
	* audio should be a compile-time option, I think there was some work done on that
	* we should support building SDL, SDL_Image, SDL_Mixer, and Lua ourselves instead of only supporting the package manager installed versions
	* there's got to be an automated way to do header files in C++ -- cproto works for C, but gets confused with C++ declarations
	* proper analog stick direction check -- it is way too easy to hit diagonal inputs on accident, we should check based on the angle rather than just an x/y check
	* the GIF saving needs to show progress or something (maybe? on my current desktop, it's entirely seamless, but I remember it pausing badly on my laptop and the Pi)
	
	per-version TODO:
	for 1.1-release:
	* document audio playback and file saving, provide proper examples (in progress!)
	
	for 1.2-alpha:
	* should refactor input handling, it's a mess and it doesn't need to be 
	* really should give a look over the API so we don't do wild, breaking changes once we get to a not-beta release
	  in particular, we should implement support for multiple controllers
	  might just modify the API to take an optional controller parameter
	* file reading/writing isn't particularly well tested at all
	
	for 1.3:
	* user configurable deadzone for analog stick
	* really need to check on squcol, need to make an example game with it that needs actually precise collision
	  I miiight have like an off-by-one error or something, dunno
	* there are gaps in non-integer-scale text, should really just draw a little bit beyond the character if there's another character after (at least, for the filled modes of course)
	  or honestly just draw the backgrounds separately
	* have quig go through a list of other supported formats for audio files
	  we currently just use WAV and MP3 since this is designed to be simple, rather than flexible (and various formats may/may not have support depending on platform)
	  but I really do want to have Opus support since Opus files can be really tiny while not sounding awful
	
	for 1.4:
	* spr_xys and squ_xys with separate x/y scale (VERY easy, just need to do it)
	* hiragana text support -- the characters are in the font, but you just can't really access them nicely
	* user-adjustable screen scale
	* selectable gif export length? or at least make it so that it can be ended early (eg, pressing select again)
	* frame blending option during display (we do this during gif export, so it should be reasonable as a runtime option)
	* better joystick support handling in general (it's all just a right mess and I really should just write a separate library that handles all that garbage and there are certainly going to be generic joysticks that should work but SDL doesn't have Xbox-style mappings for them)
	* sprbig, sprbig_xys -- draws 4 sprites at once (puts 'em in a 32x32 surface, then scales that), takes a table with which entries to draw? dunno about this really
	
	for 1.5 and beyond:
	* more examples (in-progress)
	* a better front-end, or at least one that works on Windows without lots of configuration (I am working on a project for Windows called ezLoadGUI, and I should probably provide a pre-configured version alongside quig)
	* config file (eg, default video settings, etc)
	* general cleanup, it's a bit messy, loads of init-related stuff should probably be refactored into their own functions
	
	other potential features:
	* add blending layer?
	* add perspective terrain drawing? think SNES Mode 7 or Saturn RBG0; this seems way out of scope for this project, but would really be pretty cool
	* even if the "mode 7" layer didn't rotate, it would be cool to do anyway for skies or side-scrolling ground
*/
;

#include <SDL.h>
#include <SDL_image.h>

#ifndef QUIG_NOSOUND
#include <SDL_mixer.h>
#endif

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include "gif.h"
#include "font8x8_basic.h"
#include "font8x8_hiragana.h"
#include "quig.h"

//constants
const char *QUIG_VERSION="1.1-beta2a"; //version string -- major.minor-status
const int QUIG_DEBUG = 1; //TODO: compile script flag for this
const int FPS_RATE = 60; //quig runs at a fixed 60fps, period
const int FPS_TICKS = (1000 / FPS_RATE);
//screen res
const int VIEW_WIDTH=240; //15 tiles, 30 characters
const int VIEW_HEIGHT=144; //9 tiles, 18 characters

//sound stuff
const int NUM_CHANNELS = 8;
const int AUDIO_MAX=31; //00-30
bool sound_enabled=false; //has audio been initialized
int sound_freq=48000;
int buffer_len=2048;

//filename of game
std::string arg_name;
//the above, without the extension
std::string base_name;

#ifndef QUIG_NOSOUND
//music
struct quig_Music {
	Mix_Music *mus=NULL; //loaded song
	bool loaded=false; //we only try to play if we loaded
	//load song into memory
	void load(const char *filename) {
		loaded=false;
		mus=Mix_LoadMUS(filename);
		if (mus) {
			loaded=true;
		}
	}
	//play the song once
	void play() {
		if (sound_enabled && loaded) {
			Mix_PlayMusic(mus, 1);
		}
	}
	void playloop() {
		if (sound_enabled && loaded) {
			Mix_PlayMusic(mus, -1);
		}
	}
	//stop all music (TODO: does this get used anywhere?)
	void stop() {
		//Mix_HaltMusic();
		Mix_PauseMusic();
		Mix_RewindMusic();
	}
};
quig_Music songs[AUDIO_MAX];

//samples
struct quig_Sound {
	Mix_Chunk *snd=NULL;
	bool loaded=false;
	//load the sound into memory
	void load(const char *filename) {
		loaded=false;
		snd=Mix_LoadWAV(filename);
		if (snd) {
			loaded=true;
		}
	}
	//play a sample on a given channel
	void play(int channel) {
		Mix_PlayChannel(channel, snd, 0);
	}
	//play looping
	void playloop(int channel) {
		Mix_PlayChannel(channel, snd, -1);
	}
};
quig_Sound samples[AUDIO_MAX];
#endif

//TODO: log errors
void initAudio() {
	sound_enabled=false;
	#ifndef QUIG_NOSOUND
	std::cerr << "notice: initializing audio...\n";
	if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
		std::cerr << "error: couldn't initialize SDL audio!\n";
		return; //we failed
	}
	int flags=MIX_INIT_MP3;
	if (!(Mix_Init(flags) & flags)) {
		std::cerr << "error: couldn't initialize MP3 support, music will not work!\n";
	}
	if (Mix_OpenAudio(sound_freq, MIX_DEFAULT_FORMAT, 2, buffer_len)) {
		std::cerr << "error: couldn't open audio device, sound will not work!" << "frequency: " << sound_freq << ", buffer: " << buffer_len << "!\n";
		return;
	}
	if (QUIG_DEBUG) {
		std::cerr << "debug: sound is activated\n";
	}
	sound_enabled=true;
	//load music files
	for (int ii=0; ii<AUDIO_MAX; ii++) {
		std::stringstream songname;
		songname << base_name << ".song" << ii << ".mp3";
		std::string songstr=songname.str();
		if (QUIG_DEBUG) {
			std::cerr << "debug: [music] looking for '" << songstr << "'\n";
		}
		songs[ii].load(songstr.c_str());
		if (songs[ii].loaded) {
			std::cerr << "notice: loaded song file '" << songstr << "!'\n";
		}
	}
	//load samples
	for (int ii=0; ii<AUDIO_MAX; ii++) {
		std::stringstream soundname_wav;
		//generate filename
		soundname_wav << base_name << ".snd" << ii << ".wav";
		std::string soundstr_wav=soundname_wav.str();
		//load file
		if (QUIG_DEBUG) {
			std::cerr << "debug: [samples] looking for '" << soundstr_wav << "'...\n";
		}
		samples[ii].load(soundstr_wav.c_str());
		if (samples[ii].loaded) {
			std::cerr << "notice: loaded sound file '" << soundstr_wav << "!'\n";
		}
	}
	#endif
}

//these were constants, now they aren't
//these get resized, but just in case something goes wrong, we initialize them to 1x scale
int window_scale=1;
int window_width=VIEW_WIDTH;
int window_height=VIEW_HEIGHT;

//set the window size
//this doesn't do anything after the window has been created
void setWindowScale(int s) {
	if (QUIG_DEBUG) {
		std::cerr << "debug: setting window scale to " << s << "\n";
	}
	window_scale=s;
	window_width=(VIEW_WIDTH*window_scale);
	window_height=(VIEW_HEIGHT*window_scale);
}

//quig isn't hardware accelerated (at all, even though it probably could/should be)
//but we do at least stretch the final screen size in hardware if asked to
//and doing that is required for vsync
enum class DisplayMode {
	soft, hard_novsync, hard_vsync
};
DisplayMode display_mode=DisplayMode::hard_novsync; //TODO: make this a compile-time option for what is default?
bool fullscreen=false; //TODO: fullscreen ignores aspect ratio, need to fix that


//TODO: allow the user to specify the screen scale rather than it being entirely automatic
//TODO: main should bail if this function returns !=0
//TODO: add a --help and -? option
int handleArgs(int argc, char **argv) {
	int size=-1; //automatically set the screen scale based on screen width and screen height (minus 64)
	std::string current="";
	for (int ii=0; ii<argc; ii++) {
		current=argv[ii];
		if (QUIG_DEBUG) {
			std::cerr << "debug: arg #" << ii << ": '" << current << "'\n";
		}
		//empty argument
		if (current.size()<1) {
			continue;
		}
		//handle arguments
		if (current[0]=='-') {
			if (current=="--soft") {
				display_mode=DisplayMode::soft;
			}
			else if (current=="--hard") {
				display_mode=DisplayMode::hard_novsync;
			}
			else if (current=="--hard-vsync") {
				display_mode=DisplayMode::hard_vsync;
			}
			else if (current=="--fullscreen") {
				fullscreen=true;
			}
			else if (current=="--window") {
				fullscreen=false;
			}
			//TODO: show help on stdout here
			else {
				std::cerr << "fatal error: invalid arguments!" << std::endl;
				return 1;
			}
			
		}
		//anything that isn't an argument is a game name, and we run the last one specified
		//NOTE: should we bail if multiple game files are specified? that seems like it might be more reasonable behavior
		//although, I can easily see some default script that passes a game and that gets overridden, so maybe not
		else {
			arg_name=current;
		}
	}
	//limit the display size to 3x in software mode because software scaling gets slow, fast
	int xscale=1,yscale=1;
	if (size==-1) {
		SDL_DisplayMode mode;
		//get a window size that'll fit entirely on the screen, including things like window borders, taskbars, etc
		//TODO: look into how things are dealt with when working with multiple monitors, we just get the first display here
		//really would be nice to know what monitor a window will be on before we put it there
		if (!SDL_GetCurrentDisplayMode(0, &mode)) {
			xscale=(mode.w-16)/VIEW_WIDTH;
			yscale=(mode.h-64)/VIEW_HEIGHT; //yeah, this is a bit high, but a 720px window doesn't actually fit on a 768px display because of title/borders/taskbar
			if (QUIG_DEBUG) {
				std::cerr << "debug: screen scale values are " << xscale << "," << yscale << "\n";
			}
		}
		//this shouldn't happen, but maybe quig's being run on something weird?
		else {
			std::cerr << "error: could not get display information! Window will be unscaled." <<std::endl;
		}
		setWindowScale(min2(xscale,yscale));
		//limit the size of software scaling because it's slow and if you're running in software, you probably aren't (or shouldn't be) driving a high-res screen
		if (display_mode==DisplayMode::soft && window_scale > 3) {
			setWindowScale(3);
		}
	}
	return 0;
}

//graphics stuff
SDL_Window *window = NULL;
SDL_Surface *window_surface = NULL; //this is only used in software blit mode
SDL_Surface *program_surface = NULL; //game graphics all get drawn here, and then we scale it to the screen size
SDL_Surface *sprites = NULL; //the loaded spritesheet
SDL_Surface *font[4] = {NULL,NULL,NULL,NULL}; //generated fonts
SDL_Renderer *renderer = NULL; //only used in hardware blit mode -- I could, and even should unify hardware and software final blitting to use the SDL2 renderer API, but really, this was bolted on after-the-fact

//display recording to files
//TODO: some way to cancel video recording early
const int VIDEO_TIME=(60*15);
SDL_Surface *video_record[VIDEO_TIME];
int frames_recorded=0; //if this is less than 0, recording is disabled
bool recording=false;

//setup recording
//if this returns !=0, recording is disabled
int initRecording() {
	frames_recorded=0;
	for (int ii=0; ii<VIDEO_TIME; ii++) {
		video_record[ii]=NULL;
		video_record[ii]=SDL_CreateRGBSurface(0, VIEW_WIDTH, VIEW_HEIGHT, 32, 0, 0, 0, 0);
		//bail early if it fails
		if (video_record[ii]==NULL) {
			frames_recorded=-1;
			std::cerr<<"error: could not create recording surfaces in memory, recording will not work\n";
			return 1;
		}
	}
	return 0;
}

//write a GIF
//this used to do awful png frame dumping so it could be converted with ffmpeg later, but now we just use gif.h
//we blend frames together to a: limit the amount and b: make the resulting output look nicer rather than just drop the frames entirely
//TODO: add an option to merge frames or not and by how many (eg, support for 15fps GIF files would REALLY speed up the final save)
//TODO: add an option to change what frames we record
//TODO: doesn't Windows complain badly if we spend too long spinning on a task without updating the window? We need to look into that...
int saveRecording() {
	static int savenum; //TODO: name
	//this really should only show up if I messed up somewhere
	if (frames_recorded<=-1) {
		std::cerr << "error: this message SHOULD NOT APPEAR; attempting to save recorded frames that don't exist!\n";
		return 1;
	}
	//SDL will convert from display format to AGBR format for gif.h
	//colors were wrong without this
	//TODO: GIF scale option, it outputs at 1x right now
	SDL_Surface *target_surf=SDL_CreateRGBSurface(0, VIEW_WIDTH, VIEW_HEIGHT, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
	//also, if you can't write to the current directory, this doesn't work
	int gifdelay=3; //TODO: make configurable
	//TODO: each invocation gets an increasing number for filename so you can save multiple clips
	//generate filename
	//std::stringstream outbuild;
	//outbuild << "quig-record-" << std::setfill('0') << std::setw(4) << record_num << ".gif";
	//std::string outname=outbuild.str();
	GifWriter g;
	GifBegin(&g, "quig-vid.gif", VIEW_WIDTH, VIEW_HEIGHT, gifdelay);
	//TODO: have this go based on recorded frames (easy, but still requires testing at all)
	for (int ii=0; ii<VIDEO_TIME; ii+=2) {
		//we output the result at 30hz, with blended frames
		SDL_SetSurfaceBlendMode(video_record[ii+1], SDL_BLENDMODE_BLEND);
		SDL_SetSurfaceAlphaMod(video_record[ii+1],128);
		//blend the odd frames onto the even ones
		SDL_BlitSurface(video_record[ii+1], NULL, video_record[ii], NULL);
		SDL_BlitSurface(video_record[ii], NULL, target_surf, NULL);
		//write each frame
		GifWriteFrame(&g, (uint8_t*)target_surf->pixels, VIEW_WIDTH, VIEW_HEIGHT, gifdelay);
	}
	//save and return to game
	GifEnd(&g);
	SDL_FreeSurface(target_surf);
	return 0;
}

//record frames, gets called every frame
//possible todo: maybe we can actually always record, so pressing the record key would just write the circular buffer to disk?
int doRecording() {
	//bail if not recording anything
	if (!recording) { 
		return 0;
	}
	//if recording can't work, error out
	//really, I can't think of when this would show up and everything still works, but hey
	if (frames_recorded<=-1) {
		recording=false;
		std::cerr<<"error: recording cannot happen!\n";
		return 1;
	}
	//copy frames
	if (frames_recorded<VIDEO_TIME) {
		//make sure we're overwriting instead of blending
		SDL_SetSurfaceBlendMode(video_record[frames_recorded], SDL_BLENDMODE_NONE);
		//copy frame to buffer
		SDL_BlitSurface(program_surface, NULL, video_record[frames_recorded], NULL);
	}
	//increase the frame count, and write to disk if we've filled the buffer
	frames_recorded++;
	if (frames_recorded >= VIDEO_TIME) {
		saveRecording();
		frames_recorded=0;
		recording=false;
	}
	return 0;
}

//maxN, minN -- compare numbers
int max2(int a, int b) {
	if (a>b) { return a; }
	return b;
}
int max3(int a, int b, int c) {
	return max2(a, max2(b, c));
}
int min2(int a, int b) {
	if (a<b) { return a; }
	return b;
}

//controller handling
SDL_GameController *controller = NULL;
//massive duplication of work because SDL handles controller input wildly differently than keyboard input
//TODO: cleanup
struct ControllerState {
	enum {
		//TODO: rename the contents of this enum so it's more consistent with the Inputs struct
		a=0,b,x,y,u,d,l,r,s,sel,end
	};
	int stick_up=0;
	int stick_down=0;
	int stick_left=0;
	int stick_right=0;
	int buttons[end];
	int	button_quig_b=0; //buttons X and B on the Xbox-style layout
	int button_quig_a=0; //buttons Y and A on the Xbox-style layout
	//initialize buttons
	ControllerState() {
		for(int ii=0; ii<end; ii++) {
			buttons[ii]=0;
		}
	}
	//update the controller's state, we the mix this together with the keyboard input's state
	void readState() {
		//analog stick
		int deadzone=13000; //TODO: move this
		int jsx, jsy;
		jsx=SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
		jsy=SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
		//TODO: this needs to be based on the stick angle and not just x/y position
		//stick to right
		if (jsx>deadzone) { 
			stick_right++;
			if (stick_right>2) {
				stick_right=2;
			}
		}
		else {
			stick_right=0;
		}
		//stick to left
		if (jsx<-deadzone) {
			stick_left++;
			if (stick_left>2) {
				stick_left=2;
			}
		}
		else {
			stick_left=0;
		}
		//stick down
		if (jsy>deadzone) {
			stick_down++;
			if (stick_down>2) {
				stick_down=2;
			}
		}
		else {
			stick_down=0;
		}
		//stick up
		if (jsy<-deadzone) {
			stick_up++;
			if (stick_up>2) {
				stick_up=2;
			}
		}
		else {
			stick_up=0;
		}
		//a
		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) {
			buttons[a]++;
		}
		else {
			buttons[a]=0;
		}
		if (buttons[a] > 2) { buttons[a]=2; }
		//b
		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B)) {
			buttons[b]++;
		}
		else {
			buttons[b]=0;
		}
		if (buttons[b] > 2) { buttons[b]=2; }
		//x
		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X)) {
			buttons[x]++;
		}
		else {
			buttons[x]=0;
		}
		if (buttons[x] > 2) { buttons[x]=2; }
		//y
		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y)) {
			buttons[y]++;
		}
		else {
			buttons[y]=0;
		}
		if (buttons[y] > 2) { buttons[y]=2; }
		//up
		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP)) {
			buttons[u]++;
		}
		else {
			buttons[u]=0;
		}
		if (buttons[u] > 2) { buttons[u]=2; }
		//down
		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
			buttons[d]++;
		}
		else {
			buttons[d]=0;
		}
		if (buttons[d] > 2) { buttons[d]=2; }
		//left
		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
			buttons[l]++;
		}
		else {
			buttons[l]=0;
		}
		if (buttons[l] > 2) { buttons[l]=2; }
		//right
		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
			buttons[r]++;
		}
		else {
			buttons[r]=0;
		}
		if (buttons[r] > 2) { buttons[r]=2; }
		//start
		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START)) {
			buttons[s]++;
		}
		else {
			buttons[s]=0;
		}
		if (buttons[s] > 2) { buttons[s]=2; }
		//select (well, back I guess)
		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK)) {
			buttons[sel]++;
		}
		else {
			buttons[sel]=0;
		}
		if (buttons[sel] > 2) { buttons[sel]=2; }
		/*
		//very verbose debug
		for (int ii=0; ii<end; ii++) {
			std::cerr<<buttons[ii];
		}
		std::cerr<<"b\n";
		std::cerr<<"js: "<<jsx<<", " <<jsy <<"\n";
		*/
		//handle the dual mapping for A/B
		button_quig_b=max2(buttons[b], buttons[x]);
		button_quig_a=max2(buttons[a], buttons[y]);
	}
};
ControllerState controllerState;

//holds the average fps, we calculate it once a second
double avg_fps=0.0;

//lua interpreter
lua_State *L;

//setPixel -- plot a single pixel to a surface
//no idea why this isn't a SDL built-in in some way
void setPixel(SDL_Surface *target, int x, int y, Uint32 color) {
	Uint32 *px = (Uint32 *)target->pixels;
	px[y*target->w+x]=color;
}

//generateFont -- convert the font data
//we call this four times to generate each variation
//this is kinda ugly but whatever
//TODO: we probably don't need to fill the background, having it as part of the character causes ugly scaling artifacts
//maybe we'll just draw the rectangle as ceil(scale)
SDL_Surface* generateFont(int mode) {
	//font holds up to 256 chracters, but large portions are empty
	if (mode >= 4 || mode < 0) {
		return NULL;
	}
	//make a tall surface holding everything
	font[mode] = SDL_CreateRGBSurface(0, 8, 8*256, 32, 0, 0, 0, 0);
	if (!font[mode]) { return NULL; }
	//colors
	Uint32 bg, fg;
	//black text
	if (mode == 0 || mode == 2) {
		bg=SDL_MapRGB(font[mode]->format, 255,255,255);
		fg=SDL_MapRGB(font[mode]->format, 0,0,0);
	}
	//white text (modes 1 and 3)
	else {
		bg=SDL_MapRGB(font[mode]->format, 0,0,0);
		fg=SDL_MapRGB(font[mode]->format, 255,255,255);	
	}
	//transparent background
	if (mode == 2 || mode == 3) {
		SDL_SetColorKey(font[mode], SDL_TRUE, bg);
	}
	SDL_FillRect(font[mode], NULL, bg);
	//load roman characters
	for (int cc=0; cc<128; cc++) {
		for (int jj=0; jj<8; jj++) {
			for (int ii=0; ii<8; ii++) {
				if (font8x8_basic[cc][jj] & 1<<ii) {
					setPixel(font[mode],ii,jj+cc*8,fg);
				}
			}
		}
	}
	//load hiragana
	//TODO: you can't really use them yet, need to provide a way to access them
	for (int cc=0; cc<96; cc++) {
		for (int jj=0; jj<8; jj++) {
			for (int ii=0; ii<8; ii++) {
				if (font8x8_hiragana[cc][jj] & 1<<ii) {
					setPixel(font[mode],ii,128*8+jj+cc*8,fg);
				}
			}
		}
	}
	return font[mode];
}


//cleanup -- registered with atexit(), clean up everything at the end
//we don't actually cleanup much right now, should really look into that, although none of the platforms we target right now have anything get left behind if we don't
void cleanup() {
	SDL_Quit();
}

//handle input -- 0 is not pressed, 1 is just pressed, 2 is held.
//usually, you just want to check for >0 or ==1
//this used to be the only thing holding input until controller handling was implemented, look at how small and clean it is compared to the controller code
struct Inputs {
	static const int UP = 0;
	static const int DOWN = 1;
	static const int LEFT = 2;
	static const int RIGHT = 3;
	static const int B = 4;
	static const int A = 5;
	static const int START = 6;
	int keys[7];
	int keys_held[7];
	Inputs() {
		for (int ii=0; ii<7; ii++) {
			keys[ii]=0;
			keys_held[ii]=0;
		}
	}
	//check for held keys
	void update() {
		for (int ii=0; ii<7; ii++) {
			if (keys[ii]) {
				if (keys_held[ii]) { keys[ii]=2; }
				else {keys_held[ii]=1; }
			}
		}
	}
};
Inputs inputs;
Inputs inputs_final;


//put a string into a table at the top of the stack
void strToTable(lua_State *LL, int index, const char *data) {
	lua_pushnumber(LL, index);
	lua_pushstring(LL, data);
	lua_settable(LL, -3);
}

//get a string from a table
std::string strFromTable(lua_State *LL, int index) {
	std::string result;
	lua_pushnumber(LL, index);
	lua_gettable(LL, -2);
	result=lua_tostring(LL, -1);
	lua_pop(LL, 1);
	return result;
}


//c_readfile -- this should read each line of a file and put it into a table of strings
//TODO: handle any errors properly instead of blithely progressing
//TODO: this probably should return another value indicating whether the read was fully successful or not
//TODO: these should probably go somewhere other than the current directory? I think SDL has a function for files like this...
//TODO: there should possibly be a function that allows the user to select a file -- quig handles what file is loaded, so the game doesn't just muck about with accessing random files, but that would require a way to open a file dialog across platforms
int c_readfile(lua_State *LL) {
	std::ifstream infile;
	std::string line;
	std::string fname=base_name+".quigsav";
	infile.open(fname.c_str());
	//populate the table
	lua_newtable(LL);
	for (int ii=1; std::getline(infile, line); ii++) {
		if (QUIG_DEBUG) {
			std::cerr << "debug: reading line " << ii << ": '" << line << "'\n";
		}
		strToTable(LL, ii, line.c_str());
	}
	infile.close();
	return 1;
}

//this should take a table and sequentially write each line to a file
//TODO: handle any errors properly instead of blithely progressing
//TODO: this should return true if the write was successful
int c_writefile(lua_State *LL) {
	bool result=false;
	if (lua_istable(LL, 1)) {
		std::ofstream outfile;
		std::string fname=base_name+".quigsav";
		std::string line;
		outfile.open(fname.c_str());
		for (int ii=1; ii<=luaL_len(LL, -1); ii++) {
			line=strFromTable(LL,ii);
			outfile << line << "\n";
			if (QUIG_DEBUG) {
				std::cerr << "debug: writiing line " << ii << ": '" << line << "'\n";
			}
		}
		outfile.close();
	}
	lua_pushboolean(LL, result);
	return 1;
}


//do_key -- check if a given key number is being pressed
int do_key(int key) {
	if (key < 0 || key >= 7) {
		return 0;
	}
	return inputs_final.keys[key];
}
//c_key -- run do_key from lua code
int c_key(lua_State *LL) {
	int key = (int)lua_tonumber(LL, 1);
	lua_pushnumber(LL, do_key(key));
	return 1;
}

//handle timing
struct FrameTimer {
	Uint32 now = 0;
	//get how long it's been since the timer has been started
	Uint32 getTime() {
		return SDL_GetTicks() - now;
	}
	//start the timer
	void setTime() {
		now = SDL_GetTicks();
	}
};
//handle game timing (in non-vsync modes)
FrameTimer timer;

//optimize a surface for fast drawing to the window
//should really muck about with this again, just in case it turns out that it's actually an issue on some platforms
/*
SDL_Surface* optimizeSurface(SDL_Surface *target) {
	SDL_Surface *temp_surface = SDL_ConvertSurface(target, program_surface->format, 0);
	return temp_surface;
}
*/

//init_fn() -- run the lua init() function, which gets called at the start of the game
int init_fn() {
	lua_getglobal(L, "init");
	return lua_pcall(L, 0,0,0);
}

//step_fn -- run the lua step() function, which gets called every frame
int step_fn() {
	lua_getglobal(L, "step");
	return lua_pcall(L, 0,0,0);
}

//do_squ -- draw a 16x16 colored square, centered at a point, which can be scaled
void do_squ(int x, int y, double scale, int r, int g, int b) {
	SDL_Rect target_size;
	target_size.w = 16*scale;
	target_size.h = 16*scale;
	target_size.x = x-(target_size.w/2);
	target_size.y = y-(target_size.h/2);
	SDL_FillRect(program_surface, &target_size, SDL_MapRGB(program_surface->format, r, g, b));
}

//c_squ -- run do_squ from Lua code
int c_squ(lua_State *LL) {
	int x = (int)lua_tonumber(LL,1);
	int y = (int)lua_tonumber(LL,2);
	double scale = lua_tonumber(LL,3);
	int r = (int)lua_tonumber(LL,4);
	int g = (int)lua_tonumber(LL,5);
	int b = (int)lua_tonumber(LL,6);
	do_squ(x,y,scale,r,g,b);
	return 0;
}

//do_rect -- draw a colored rectangle, isn't centered
void do_rect(int x, int y, int w, int h, int r, int g, int b) {
	SDL_Rect target_size;
	target_size.w = w;
	target_size.h = h;
	target_size.x = x;
	target_size.y = y;
	SDL_FillRect(program_surface, &target_size, SDL_MapRGB(program_surface->format, r, g, b));
}

//c_rect -- run do_rect from lua code
int c_rect(lua_State *LL) {
	int x=(int)lua_tonumber(LL,1);
	int y=(int)lua_tonumber(LL,2);
	int w=(int)lua_tonumber(LL,3);
	int h=(int)lua_tonumber(LL,4);
	int r=(int)lua_tonumber(LL,5);
	int g=(int)lua_tonumber(LL,6);
	int b=(int)lua_tonumber(LL,7);
	do_rect(x,y,w,h,r,g,b);
	return 0;
}

//do_spr -- draw a scaled sprite, centered at a point
void do_spr(int x, int y, double scale, int sx, int sy) {
	SDL_Rect source_size, target_size;
	//bounds check -- the sprite sheet is only 128x128
	if (sx >= 8 || sy >= 8 || sx < 0 || sy < 0) {
		return;
	}
	//select from sprite sheet
	source_size.w=16;
	source_size.h=16;
	source_size.x=sx*16;
	source_size.y=sy*16;
	//area to draw (sprites are drawn from their middle)
	target_size.w = 16*scale;
	target_size.h = 16*scale;
	target_size.x = x-(target_size.w/2);
	target_size.y = y-(target_size.h/2);
	SDL_BlitScaled(sprites, &source_size, program_surface, &target_size);
}
//c_spr -- run do_spr from Lua code
int c_spr(lua_State *LL) {
	int x = (int)lua_tonumber(LL,1);
	int y = (int)lua_tonumber(LL,2);
	double scale = lua_tonumber(LL,3);
	int sx = (int)lua_tonumber(LL,4);
	int sy = (int)lua_tonumber(LL,5);
	do_spr(x,y,scale,sx,sy);
	return 0;
}


//do_text -- show some text
//TODO: make sure the background doesn't have gaps on modes with a background color
//TODO: some kind of function for displaying hiragana, or at least a function that generates a string that you can use here
void do_text(const char *str, int x, int y, double scale, int mode) {
	int x_offset=0, y_offset=0;
	if (mode < 0 || mode >= 4) { return; } //don't draw anything with invalid modes
	//draw the text, character by character
	for (int ii=0; str[ii]!='\0'; ii++) {
		SDL_Rect font_rect, target_rect;
		//reset position on newlines
		if (str[ii]=='\n') {
			x_offset=0;
			y_offset++;
			continue;
		}
		//which character
		font_rect.x=0;
		font_rect.y=8*str[ii];
		font_rect.w=8;
		font_rect.h=8;
		//where on screen and at what size
		target_rect.x=x+x_offset*8*scale;
		target_rect.y=y+y_offset*8*scale;
		target_rect.w=8*scale;
		target_rect.h=8*scale;
		x_offset++;
		/*
		TODO: (don't feel like actually testing this right now, so this is just pseudocode)
		if (str[ii+1]!='\0' && str[ii+1]!='\n') {
			//make a rect to fill in the gap
			//TODO: write a test program so I can determine how much needs to be filled
			if (mode==0) {
				//draw a rectangle
			}
			else if (mode==1)) {
				//same thing, but with a black background
			}
		}
		*/
		//draw
		SDL_BlitScaled(font[mode], &font_rect, program_surface, &target_rect);
	}
}
//c_text -- run do_text from lua code
int c_text(lua_State *LL) {
	const char *str = lua_tostring(LL,1);
	int x=(int)lua_tonumber(LL,2);
	int y=(int)lua_tonumber(LL,3);
	double scale = lua_tonumber(LL,4);
	int mode = (int)lua_tonumber(LL,5);
	do_text(str,x,y,scale,mode);
	return 0;
}

//do_squcol -- check for two sprites/squares colliding 
//TODO: proper testing for corner cases (eg, fractional scale+position) -- pop.drop'n doesn't really need actually precise collision, but a more ordinary platform game probably would
//TODO: this could be optimized to a point in rectangle check, since both objects are squares
bool do_squcol(double x1, double y1, double s1, double x2, double y2, double s2) {
	double left1, right1, top1, bottom1, offset1;
	offset1 = 8*s1;
	left1=x1-offset1;
	right1=x1+offset1;
	top1=y1-offset1;
	bottom1=y1+offset1;
	double left2, right2, top2, bottom2, offset2;
	offset2 = 8*s2;
	left2=x2-offset2;
	right2=x2+offset2;
	top2=y2-offset2;
	bottom2=y2+offset2;
	return (left1 < right2 && right1 > left2 && top1 < bottom2 && bottom1 > top2);
}

//c_squcol -- run do_squcol from lua code
int c_squcol(lua_State *LL) {
	double x1=lua_tonumber(LL,1);
	double y1=lua_tonumber(LL,2);
	double s1=lua_tonumber(LL,3);
	double x2=lua_tonumber(LL,4);
	double y2=lua_tonumber(LL,5);
	double s2=lua_tonumber(LL,6);
	lua_pushboolean(LL, do_squcol(x1,y1,s1,x2,y2,s2));
	return 1;
}

//do_cls -- clear the screen
void do_cls(int r, int g, int b) {
	SDL_FillRect(program_surface, NULL, SDL_MapRGB(program_surface->format, r, g, b));
}

//c_cls -- call do_cls from lua code
int c_cls(lua_State *LL) {
	int r=(int)lua_tonumber(LL,1);
	int g=(int)lua_tonumber(LL,2);
	int b=(int)lua_tonumber(LL,3);
	do_cls(r,g,b);
	return 0;
}

//c_getfps -- get the value of avg_fps from lua code
int c_getfps(lua_State *LL) {
	lua_pushnumber(LL, avg_fps);
	return 1;
}



int c_playsound(lua_State *LL) {
	int sound=(int)lua_tonumber(LL,1);
	int channel=(int)lua_tonumber(LL,2);
	#ifndef QUIG_NOSOUND
	if (sound<AUDIO_MAX && sound>=0 && channel<NUM_CHANNELS) {
		samples[sound].play(channel);
	}
	#endif
	return 0;
}
int c_loopsound(lua_State *LL) {
	int sound=(int)lua_tonumber(LL,1);
	int channel=(int)lua_tonumber(LL,2);
	#ifndef QUIG_NOSOUND
	if (sound<AUDIO_MAX && sound>=0 && channel<NUM_CHANNELS) {
		samples[sound].playloop(channel);
	}
	#endif
	return 0;
}

int c_stopsound(lua_State *LL) {
	int channel=(int)lua_tonumber(LL,1);
	#ifndef QUIG_NOSOUND
	Mix_HaltChannel(channel);
	#endif
	return 0;
}

//c_playsong -- play a music file from lua code
int c_playsong(lua_State *LL) {
	int song=(int)lua_tonumber(LL,1);
	#ifndef QUIG_NOSOUND
	if (song<AUDIO_MAX && song>=0) {
		songs[song].play();
	}
	#endif
	return 0;
}

//c_loopsong -- loop a music file from lua code
int c_loopsong(lua_State *LL) {
	int song=(int)lua_tonumber(LL,1);
	#ifndef QUIG_NOSOUND
	if (song<AUDIO_MAX && song>=0) {
		songs[song].playloop();
	}
	#endif
	return 0;
}

//c_stopsong -- stop music from playing from lua code
//TODO: there's a weird thing where SDL_Mixer will stutter when replaying a song after this -- really would like to know how to fix it
int c_stopsong(lua_State *LL) {
	#ifndef QUIG_NOSOUND
	//Mix_RewindMusic();
	Mix_HaltMusic();
	#endif
	return 0;
}

//updateScreen -- 
void updateScreen() {
	//just blit the surface to the window in software modes
	if (display_mode==DisplayMode::soft) {
		SDL_BlitScaled(program_surface, NULL, window_surface, NULL);
		SDL_UpdateWindowSurface(window);
	}
	//filthy hack that works, generate a texture every frame from the surface
	else {
		SDL_Texture *scr = SDL_CreateTextureFromSurface(renderer, program_surface);
		SDL_RenderCopy(renderer, scr, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(scr);
	}
}

//register lua functions and some useful globals
void registerLuaFn() {
	//available functions
	lua_register(L, "cls", c_cls);
	lua_register(L, "squ", c_squ);
	lua_register(L, "rect", c_rect);
	lua_register(L, "spr", c_spr);
	lua_register(L, "text", c_text);
	lua_register(L, "key", c_key);
	lua_register(L, "squcol", c_squcol);
	lua_register(L, "getfps", c_getfps);
	lua_register(L, "readfile", c_readfile);
	lua_register(L, "writefile", c_writefile);
	lua_register(L, "playsong", c_playsong);
	lua_register(L, "loopsong", c_loopsong);
	lua_register(L, "stopsong", c_stopsong);
	lua_register(L, "playsound", c_playsound);
	lua_register(L, "loopsound", c_loopsound);
	lua_register(L, "stopsound", c_stopsound);
	//size of the display
	lua_pushnumber(L, VIEW_WIDTH);
	lua_setglobal(L, "view_width");
	lua_pushnumber(L, VIEW_HEIGHT);
	lua_setglobal(L, "view_height");
	//key codes
	lua_pushnumber(L,0);
	lua_setglobal(L, "key_up");
	lua_pushnumber(L,1);
	lua_setglobal(L, "key_down");
	lua_pushnumber(L,2);
	lua_setglobal(L, "key_left");
	lua_pushnumber(L,3);
	lua_setglobal(L, "key_right");
	lua_pushnumber(L,4);
	lua_setglobal(L, "key_b");
	lua_pushnumber(L,5);
	lua_setglobal(L, "key_a");
	lua_pushnumber(L,6);
	lua_setglobal(L, "key_start");
}

//initialization, main loop
int main(int argc, char* argv[]) {
	atexit(cleanup);
	std::cerr << "notice: quig version " << QUIG_VERSION << " now init...\n";
	
	//attempt to initialize Lua
	L=luaL_newstate();
	if (L==NULL) {
		std::cerr << "fatal error: could not initialize Lua!" << std::endl;
		return 1;
	}
	//lua standard library
	//TODO: should probably only open a few of the libraries -- we don't use Lua's file I/O, for starters
	luaL_openlibs(L);
	
	//attempt to initialize SDL:
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "fatal error: could not initialize SDL! " << SDL_GetError() << std::endl;
		return 1;
	}
	
	//TODO: muck about with, what a total mess
	//attempt to initialize joysticks
	if (!SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
		std::cerr << "notice: detected " << SDL_NumJoysticks() << " joysticks!" << std::endl;
		if (SDL_NumJoysticks() > 1) {
			std::cerr << "warning: quig only uses the first controller found; for best results, unplug other controllers!\n";
		}
		if (SDL_IsGameController(0)) {
			controller = SDL_GameControllerOpen(0);
			if (controller) {
				std::cerr << "notice: controller found!\n";
				const char *gcname = SDL_GameControllerName(controller);
				if (gcname) {
					std::cerr << "notice: controller name is '" << gcname << "'\n";
				}
				else {
					std::cerr << "notice: controller name could not be found...\n";
				}
			}
			else {
			    std::cerr << "warning: could not open controller! " << SDL_GetError() << "\n";
			}
		}
		else {
			//TODO: move this around
			std::cerr << "warning: no mappings for controller! This controller will not work with quig!\n";
		}
	}
	else {
		std::cerr << "warning: could not initialize joystick subsystem!\n";
	}
	
	//attempt to initialize SDL_image:
	int img_flags = IMG_INIT_PNG;
	if ((IMG_Init(img_flags) & img_flags) != img_flags) {
		std::cerr << "fatal error: could not initialize SDL_image! " << IMG_GetError() << std::endl;
		return 1;
	}
	
	//initialize the game screen
	program_surface = SDL_CreateRGBSurface(0, VIEW_WIDTH, VIEW_HEIGHT, 32, 0, 0, 0, 0);
	
	//handle filename argument
	if (QUIG_DEBUG) {
		std::cerr << "debug: argument handling...\n";
	}
	//there needs to be at least one argument
	//TODO: this check is old, we should check if arg_name has something
	//TODO: like, everything about this is a bit of a mess
	//might display some help here on the terminal, really
	if (argc < 2) {
		std::cerr << "fatal error: quig requires a game to run!" << std::endl;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "quig fatal error!", "Fatal error:\nNo game to run!", window);
		return 1;
	}
	//read options, filename
	if (handleArgs(argc, argv)) {
		std::cerr << "fatal error: could not handle arguments!" << std::endl;
		return 1;
	}
	//check for ".quig" as the end
	//we bail if the filename is too short
	if (arg_name.size() < 6) {
		std::cerr << "fatal error: not a .quig file!" << std::endl;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "quig fatal error!", "Fatal error:\nNot a .quig file!", window);
		return 1;
	}
	//pull off the last 5 characters
	std::string arg_extension=arg_name.substr(arg_name.size()-5,arg_name.size());
	if (QUIG_DEBUG) {
		std::cerr << "debug: extension is '" << arg_extension << "'\n";
	}
	//TODO: we should probably also accept .lua as an extension for a quig game maybe
	if (arg_extension!=".quig") {
		std::cerr << "fatal error: not a .quig file!" << std::endl;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "quig fatal error!", "Fatal error:\nNot a .quig file!", window);
		return 1;
	}
	//strip the extension
	base_name = arg_name.substr(0,arg_name.size()-5);
	std::string gfx_name=base_name;
	//get the .png filename
	gfx_name += ".png";
	if (QUIG_DEBUG) {
		std::cerr << "debug: graphics filename is '" << gfx_name << "'\n";
	}
	
	//attempt to create the window:
	Uint32 full=0;
	if (fullscreen) {
		full=SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	window = SDL_CreateWindow("quig simple game system", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_SHOWN | full);
	if (window == NULL) {
		std::cerr << "fatal error: could not create window! " << SDL_GetError() << std::endl;
		return 1;
	}
	
	//software final blits
	//TODO: really, these should be unified and all use the renderer API
	//in fact, all of quig should, but eh
	if (display_mode==DisplayMode::soft) {
		std::cerr << "notice: using software driven window\n";
		window_surface = SDL_GetWindowSurface(window);
		SDL_FillRect(window_surface, NULL, SDL_MapRGB(window_surface->format, 0xFF, 0xFF, 0xFF));
	}
	//hardware accelerated final blits
	else {
		std::cerr << "notice: using hardware drawn window\n";
		Uint32 vsync_on=0;
		if (display_mode==DisplayMode::hard_vsync) {
			std::cerr<<"notice: vsync enabled\n";
			vsync_on = SDL_RENDERER_PRESENTVSYNC;
		}
		else {
			std::cerr<<"notice: vsync disabled\n";
		}
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | vsync_on);
		//TODO: should we just go and attempt to try software mode?
		if (renderer == NULL) {
			std::cerr << "fatal error: could not create renderer!" << std::endl;
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "quig fatal error!", "Fatal error:\nCould not create renderer!", window);
			return 1;
		}
	}
	
	//generate the four font styles
	if (QUIG_DEBUG) {
		std::cerr << "debug: generating fonts...\n";
	}
	for (int ff=0; ff<4; ff++) {
		generateFont(ff);
		if (font[ff]==NULL) {
			//this really shouldn't happen outside of OOM
			std::cerr << "fatal error: could not generate fonts!";
			return 1;
		}
	}
	//register lua functions
	registerLuaFn();
	
	//load the Lua code
	if (QUIG_DEBUG) {
		std::cerr << "debug: loading Lua code...\n";
	}
	//error with the lua code (usually, just a syntax error, but maybe you passed something that wasn't lua code at all or the file doesn't exist)
	if (luaL_dofile(L, arg_name.c_str())) {
		std::cerr << "fatal error: could not load Lua code! " << lua_tostring(L,-1) << std::endl;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "quig fatal error!", lua_tostring(L,-1), window);
		lua_pop(L,1);
		return 1;
	}
	
	//load user graphics
	//TODO: this should maybe not be a fatal error? maybe?
	sprites=IMG_Load(gfx_name.c_str());
	if (sprites==NULL) {
		std::cerr << "fatal error: could not load graphics!" << std::endl;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "quig fatal error!", "Fatal error:\nCould not load graphics!", window);
		return 1;
	}
	//magic pink (#FF00FF) is transparent
	SDL_SetColorKey(sprites, SDL_TRUE, SDL_MapRGB(sprites->format, 0xFF, 0x00, 0xFF));
	
	//used to calculate fps
	FrameTimer fps_timer;
	fps_timer.setTime();
	
	//setup recording
	initRecording();
	
	//setup audio, which also looks for audio files
	do_cls(0,0,0);
	updateScreen();
	initAudio();
	
	//main loop
	bool running = true;
	SDL_Event e;
	std::cerr << "notice: quig init complete, entering main loop...\n";
	//error in user lua code
	if (init_fn()) {
		std::cerr << "fatal error: lua error during init()! " << lua_tostring(L,-1) << std::endl;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "quig fatal error!", lua_tostring(L,-1), window);
		lua_pop(L,1);
		return 1;
	}
	//the main loop itself
	int second_count=0;
	while (running) {
		timer.setTime();
		bool sshot=false;
		//handle events
		while (SDL_PollEvent(&e)) {
			//user closes the window normally
			if (e.type == SDL_QUIT) {
				running = false;
			}
			//key inputs
			//TODO: joypad
			else if (e.type == SDL_KEYDOWN && e.key.repeat==0) {
				switch (e.key.keysym.sym) {
					//quit
					//TODO: change this, or at least make quig ask to quit
					//TODO: quig also needs a way to display its own info+take input, separate from the game
					//half tempted to make this an input that the game can handle, but it just seems like pointless extra work on both our and game makers parts
					case (SDLK_ESCAPE):
						running = false;
					break;
					//screenshot
					case (SDLK_F6):
						sshot=true;
					break;
					//record frames
					//TODO: specify length
					case (SDLK_F8):
						recording=true;
					break;
					//d-pad
					//TODO: ijkl? really, this should all just end up being remappable though
					case (SDLK_UP):
						if (inputs.keys[inputs.UP] == 0) {
							inputs.keys[inputs.UP]=1;
							if (QUIG_DEBUG) {
								std::cerr << "debug: pressed up!\n";
							}
						}				
					break;
					case (SDLK_DOWN):
						if (inputs.keys[inputs.DOWN] == 0) {
							inputs.keys[inputs.DOWN]=1;
							if (QUIG_DEBUG) {
								std::cerr << "debug: pressed down!\n";
							}
						}
					break;
					case (SDLK_LEFT):
						if (inputs.keys[inputs.LEFT] == 0) {
							inputs.keys[inputs.LEFT]=1;
							if (QUIG_DEBUG) {
								std::cerr << "debug: pressed left!\n";
							}
						}
					break;
					case (SDLK_RIGHT):
						if (inputs.keys[inputs.RIGHT] == 0) {
							inputs.keys[inputs.RIGHT]=1;
							if (QUIG_DEBUG) {
								std::cerr << "debug: pressed right!\n";
							}
						}
					break;
					//start button
					case (SDLK_RETURN):
					case (SDLK_RETURN2):
						if (inputs.keys[inputs.START] == 0) {
							inputs.keys[inputs.START]=1;
							if (QUIG_DEBUG) {
								std::cerr << "debug: pressed start-button!\n";
							}
						}
					break;
					//A button (ostensibly, the one to the (bottom) left)
					case (SDLK_z):
					case (SDLK_s):
					case (SDLK_q):
					case (SDLK_2):
						if (inputs.keys[inputs.A] == 0) {
							inputs.keys[inputs.A]=1;
							if (QUIG_DEBUG) {
								std::cerr << "debug: pressed A-button!\n";
							}
						}
					break;
					//B button (ostensibly, the one to the (far) right)
					case (SDLK_x):
					case (SDLK_a):
					case (SDLK_w):
					case (SDLK_1):
						if (inputs.keys[inputs.B] == 0) {
							inputs.keys[inputs.B]=1;
						}
						if (QUIG_DEBUG) {
							std::cerr << "debug: pressed B-button!\n";
						}
					break;
				}
			}
			else if (e.type == SDL_KEYUP && e.key.repeat==0) {
				switch (e.key.keysym.sym) {
					//d-pad
					//TODO: ijkl?
					case (SDLK_UP):
						if (inputs.keys[inputs.UP] != 0) {
							inputs.keys[inputs.UP]=0;
							inputs.keys_held[inputs.UP]=0;
							if (QUIG_DEBUG) {
								std::cerr << "debug: released up!\n";
							}
						}
					break;
					case (SDLK_DOWN):
						if (inputs.keys[inputs.DOWN] != 0) {
							inputs.keys[inputs.DOWN]=0;
							inputs.keys_held[inputs.DOWN]=0;
							if (QUIG_DEBUG) {
								std::cerr << "debug: released down!\n";
							}
						}
					break;
					case (SDLK_LEFT):
						if (inputs.keys[inputs.LEFT] != 0) {
							inputs.keys[inputs.LEFT]=0;
							inputs.keys_held[inputs.LEFT]=0;
							if (QUIG_DEBUG) {
								std::cerr << "debug: released left!\n";
							}
						}
					break;
					case (SDLK_RIGHT):
						if (inputs.keys[inputs.RIGHT] != 0) {
							inputs.keys[inputs.RIGHT]=0;
							inputs.keys_held[inputs.RIGHT]=0;
						}
						if (QUIG_DEBUG) {
							std::cerr << "debug: released right!\n";
						}
					break;
					//start button
					case (SDLK_RETURN):
					case (SDLK_RETURN2):
						if (inputs.keys[inputs.START] != 0) {
							inputs.keys[inputs.START]=0;
							inputs.keys_held[inputs.START]=0;
							if (QUIG_DEBUG) {
								std::cerr << "debug: released start-button!\n";
							}
						}
					break;
					//A button (the one on the bottom/left of the B-button)
					case (SDLK_z):
					case (SDLK_s):
					case (SDLK_q):
					case (SDLK_2):
						if (inputs.keys[inputs.A] != 0) {
							inputs.keys[inputs.A]=0;
							inputs.keys_held[inputs.A]=0;
							if (QUIG_DEBUG) {
								std::cerr << "debug: released A-button!\n";
							}
						}
					break;
					//B button (the one to the right of the A-button)
					case (SDLK_x):
					case (SDLK_a):
					case (SDLK_w):
					case (SDLK_1):
						if (inputs.keys[inputs.B] != 0) {
							inputs.keys[inputs.B]=0;
							inputs.keys_held[inputs.B]=0;
							if (QUIG_DEBUG) {
								std::cerr << "debug: released B-button!\n";
							}
						}
					break;
				}
			}
		}
		//handle held-down keys:
		inputs.update();
		//read and merge controller inputs with keyboard inputs
		//I've only tested an off-brand Xbone controller under linux so far, so this might be broken with other configs
		//yeah, this whole pile is kinda awful and I hate it
		if (controller) {
			controllerState.readState();
		}
		
		inputs_final.keys[inputs.UP]=max3(
			inputs.keys[inputs.UP],
			controllerState.buttons[controllerState.u],
			controllerState.stick_up
		);
		
		inputs_final.keys[inputs.DOWN]=max3(
			inputs.keys[inputs.DOWN],
			controllerState.buttons[controllerState.d],
			controllerState.stick_down
		);
		
		inputs_final.keys[inputs.LEFT]=max3(
			inputs.keys[inputs.LEFT],
			controllerState.buttons[controllerState.l],
			controllerState.stick_left
		);
		
		inputs_final.keys[inputs.RIGHT]=max3(
			inputs.keys[inputs.RIGHT],
			controllerState.buttons[controllerState.r],
			controllerState.stick_right
		);
		inputs_final.keys[inputs.START]=max2(inputs.keys[inputs.START], controllerState.buttons[controllerState.s]);
		inputs_final.keys[inputs.B]=max2(inputs.keys[inputs.B], controllerState.button_quig_b);
		inputs_final.keys[inputs.A]=max2(inputs.keys[inputs.A], controllerState.button_quig_a);
		
		//update game, give the user an error if something goes wrong (usually just a syntax error)
		if (step_fn()) {
			std::cerr << "fatal error: lua error during step()! " << lua_tostring(L,-1) << std::endl;
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "quig fatal error!", lua_tostring(L,-1), window);
			lua_pop(L,1);
			return 1;
		}
		
		//alternate button to record:
		if (controllerState.buttons[controllerState.sel]==1) {
			recording=true;
		}
		
		//save a screenshot if requested
		//TODO: numbering, so the user can take multiple shots and decide the best one
		//or maybe just timestamps, which saves a lot of effort
		if (sshot) {
			IMG_SavePNG(program_surface,"quig-sshot.png");
		}
		//handle recording
		doRecording();
		//draw everything
		updateScreen();
		
		//calculate FPS
		second_count++;
		if (second_count > FPS_RATE) {
			avg_fps=second_count / (fps_timer.getTime()/1000.0);
			//if (QUIG_DEBUG) {
			//	std::cerr << "debug: fps: " << avg_fps << "\n";
			//}
			second_count = 0;
			fps_timer.setTime();
		}
		
		//cap FPS when vsync is off
		if (display_mode != DisplayMode::hard_vsync) {
			int frame_time = timer.getTime();
			if (frame_time < FPS_TICKS) {
				SDL_Delay(FPS_TICKS - frame_time);
			}
		}
	}	
	return 0;
}
