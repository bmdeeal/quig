quig - quick Lua-based game system
(C) 2020-2022 B.M.Deeal <brenden.deeal@gmail.com>
<https://github.com/bmdeeal/quig>

===
Legal:

quig is distributed under the GPLv3, see quig-license.txt for details.
In addition, see the files under other-licenses/ for the licensing details of other libraries that quig uses, such as SDL and Lua.

Sources for many libraries are provided in libraries-w32/sources.7z.

Several of the example files are under the GPLv3, several are under CC0 (effectively no restrictions, either public domain or as close as possible in your jurisdiction). Check the license if you want to copy the code into your own game or build off of the examples!

===
About:

quig is a simple program designed to let you quickly and easily make small games with Lua. The various limitations found in quig are largely designed to keep the process simple and easy -- low resolution to reduce the focus on graphics, few buttons to encourage pick-up-and-play titles, and a small command set that contains the essentials of what is needed to make a game.

A simple tutorial showing the process of creating a simple game from start to finish is provided. See get-the-dot.txt for details.

At the moment, quig isn't really a fantasy console or anything (like TIC-80 or PICO-8), all editing is done with separate text editors and graphics editors.
In addition, it's maybe a bit too simple, and the lack of integrated editors (in particular, for graphics) really does make it less than obvious to get started with vs other Lua-based game making systems, but it's still not too hard. Draw some graphics, write some code, play. 

Games are drawn to a true-color 240x144 canvas that gets scaled up. Sprites are loaded from a truecolor .PNG file and have no color restrictions (except for the color #FF00FF, which is used as transparency and will not be drawn).
quig updates at a fixed 60hz. However, in software mode or without vsync enabled, quig has been observed to update as quickly as 62hz on some systems.

Games for quig are comprised of two files, a graphics file (just a plain 128x128 PNG image containing 16x16 tiles) and a Lua source file.
For example, my-game.quig would contain the Lua source and my-game.png would contain the graphics. Both files are required.

===
System requirements:

quig has been tested under Raspberry Pi OS 10 and Windows 10 (via MSYS2 and a VS2019 build).
quig requires SDL2, SDL_image, SDL_mixer, and Lua 5.3 to be installed on Linux; the Windows build provides its own copies of them.
On Windows systems, the scripts run-quig.bat and run-quig.ps1 are provided to make it easier to run games with quig. These scripts require PowerShell to be installed to work.
On Linux systems, the script run-quig.sh is similar, if more featureful. It requires Bash and YAD to be installed.
quig will run on a wide range of hardware -- quig has even been run on a Raspberry Pi Zero (running at 1x resolution), although many games will not run at full speed on such low-powered hardware.
quig was originally developed on a Raspberry Pi 2, and runs acceptably in software mode at 3x resolution on it. quig runs very well on a Raspberry Pi 4, supporting hardware mode and vsync there.

The VS2019 build requires the x86 Microsoft Visual C++ Redistributable for Visual Studio 2019 to be installed to run.
As of this writing, it can be downloaded here:
	https://aka.ms/vs/16/release/vc_redist.x86.exe
If that link does not work, this one might:
	https://support.microsoft.com/en-us/topic/the-latest-supported-visual-c-downloads-2647da03-1eea-4433-9aff-95f26a218cc0
Under the heading "Visual Studio 2015, 2017 and 2019", download the file from the link marked "x86: vc_redist.x86.exe".

In addition, the main interface, quig-ui, requires .NET Core 3.1 or another compatible version of .NET to be installed. If it is not installed, you will be prompted to download it.
If that does not work, you can download it from here, under '.NET Desktop Runtime':
	https://dotnet.microsoft.com/en-us/download/dotnet/3.1

quig for Windows is currently only distributed as a 32-bit binary.
Currently, quig has not been compiled under a 64-bit environment.

===
Installation on Windows:
These instructions are specifically for Windows 10, although the process is similar on other versions.

quig doesn't need any particularly special installation.
Simply run quig-ui.exe. After that, go to Configure editor settings to change your preferred graphics and code editor, as well as configure quig-ui to be the default editor for .quig games.

quig.exe is the main game engine, and is a command-line based program. Double-clicking on it will simply bring up a message that there isn't a game to load. Dragging a .quig file onto it will run it, although you cannot configure any other options.

===
Installation on Linux:
quig requires SDL 2, SDL_Mixer 2.0, SDL_Image 2.0, and Lua 5.3 to be installed. On a Debian-based distro, running deps-debian.sh should install the needed dependencies.
The package build-essential should also be installed.

Run build.sh to compile quig. You can then move the quig binary to somewhere in your $PATH, such as /usr/local/bin.

If your desktop environment supports it, you can associate .quig files with quig to run them by double-clicking them.

The main way to load quig games on Linux is with run-quig.sh, which requires YAD to be installed. Alternatively, quig can be run directly from the command line.

===
Usage:

On Windows, quig provides an easy-to-use interface known as quig-ui to create and edit games. Simply run quig-ui.exe.
quig-ui requires .NET Core 3.1 and will prompt you to download it if it is not installed on the system.
To load and play a game, go to 'Open existing game', select the .quig file you would like to play, and then click on 'Play game'. You can edit a game with 'Edit game code' and 'Edit game graphics', which will (by default) open up the .quig file in Notepad and the .png file in Paint respectively.
If you would like to change the editors used, click on 'Configure editor settings' and select other programs with the 'Browse' buttons before selecting 'Save new settings'.
quig-ui can be configured to be the default editor for .quig games by going to 'Configure editor settings' > 'Associate .quig files with quig-ui'.

On Linux, the Bash script run-quig.sh will allow you to graphically select a quig game and configure any options.
When the game is exited, the GUI will reappear. Either select a game again or press cancel to exit.
This interface has only been tested under Raspberry Pi OS so far, and requires YAD to be installed.
On Raspberry Pi OS, ($ is the prompt, don't type it!)
	$ sudo apt install yad
will install YAD if needed.
It may run on other systems that provide YAD and bash. If you do not have YAD and bash installed or cannot install it (for example, on Windows), quig can be used as a command-line program.

If you cannot use quig-ui or run-quig.sh, you can still run games from the command line as follows:
	$ quig mygame.quig
where myname is the name of the game, and mygame.quig and mygame.png are together in the same folder.
If mygame.quig has no errors and mygame.png can be loaded, the game will start.

The following command line arguments are supported:
	--help, -?: get a list of supported arguments.
	--soft: full software drawing. This is the slowest option, but will run acceptably on the majority of systems, including my Raspberry Pi 2. The window maxes out at 3x internal resolution when using this mode.
	--hard: hardware window drawing (default). Use this option if your monitor doesn't run at 60hz and you want a larger screen size.
	--hard-vsync: hardware window drawing, timed to vertical sync. Generally the best option. Take note that quig is designed to run at 60fps, so using this on a display that runs faster or slower will cause games to run at entirely the wrong speed (such as on a 144hz display, running games more than twice as fast as intended).
	--fullscreen: runs the game in windowed fullscreen mode.
	--window: run the game in a window (default).
	--auto-scale: automatically set the window size (default).
	--scale n: set the window size to a given scale factor. For example, --scale 1 will run quig in a tiny 240x144 window. --scale 4 will run quig in a 960x576 window. Currently, only integer values are handled.
	
For example,
	$ quig examples/astro-burst.quig --hard-vsync --fullscreen
will run astro-burst in vsync hardware mode and in fullscreen.

When in windowed mode, quig will automatically resize the window to the largest integer scale it can, minus a little space to account for window borders and taskbars and things like that. Future versions may add an option for non-integer scaling without needing to be in fullscreen mode.

===
Controls:

The quig environment expects a controller with the following layout:
[D-pad  start  A B]

On a keyboard, this maps to:
	Arrow keys: D-Pad
	Z/S/Q/2: A-button
	X/A/W/1: B-button
	Enter: start button

This layout was designed to be usable on a lot of keyboards, where many combinations of four keys (both buttons+diagonal input) may not work.

In addition, if your controller is supported by SDL's game controller API, the mappings are as follows:
	Analog stick, D-Pad: D-Pad
	Xinput A/Y (bottom and top buttons): A-button
	Xinput B/A (left side and right side buttons): B-button
	Start (generally the center or center-right button): Start button

If your controller isn't recognized, make sure that it is the only controller plugged into the computer, as quig may be attempting to use a different controller. In addition, Xinput controllers are far more likely to work than other controller types as of this writing (as one set of mappings allows all to work).
Future versions of quig may support custom controller or keyboard mappings.

The F6 key on the keyboard allows you to take an unscaled screenshot in the current directory as quig-sshot.png. Take note that if the file already exists, it will be overwritten.
The F8 key on the keyboard allows you to record a few seconds of gameplay as quig-vid.gif. Again, if the file already exists, it will be overwritten. The Back or Select key on a controller will also begin recording. Take note that the game will be unresponsive for a few moments after the recording is finished as it saves the recording to disk.

The Esc key immediately quits quig. 

===
Game development:

All quig games have two mandatory functions: init() and step(). init() is called at the start of the game, and step() is called every frame.
Both MUST be present, even if init is left empty.
In addition, all quig games MUST have an associated .png holding the game graphics.

Included with quig are various example files that range from simple tutorials on how to get some graphics on screen and take user input to whole games with high speed scaling "3D" graphics and a complex object management system.

An empty.quig and empty.png file are in the examples folder, demonstrating the minimum requirements to create a quig program.
empty.quig just has the two required functions, and empty.png is entirely filled with the transparent color (#FF00FF, rgb(255,0,255)).

quig-ui can create a new, empty game for you: with no file loaded, click 'Create new game', choose a location to save to, and then click 'Create new quig game'.

===
Command reference (it's quite short!):

* cls(red, green, blue)
	Clear the screen to a given color.
	The colors red, green, and blue are from 0-255.
	If you don't call this, remnants of the last frame will be left behind (which might be desirable! One effect that can be done is to clear the background every other frame, giving a slight "motion blur").
	example: cls(0,24,255)

* spr(x, y, scale, sheet_x, sheet_y)
	Draw a sprite to the screen. All sprites are 16x16 and drawn centered.
	Sprites can be scaled up and down to arbitrary floating point sizes, 1.0 is normal size.
	sheet_x/sheet_y are which 16x16 portion of the sprite sheet to display.
	0,0 is the top-left tile in the sheet, 7,7 the bottom-right. If the sheet is larger than 128x128, you still won't be able to access the outside area.
	example: spr(view_width/2,view_height/2,2,0,0) --draw a 2x scaled sprite at the center of the screen

* rect(x, y, width, height, red, green, blue)
	Draw a rectangle on the screen in a given color.
	x and y are the top-left corner of the rectangle.
	example: rect(32,32,8,48,255,255,255) -- draw a short and wide white rectangle near the top-left of the screen

* squ(x, y, scale, red, green, blue)
	Draw a 16x16 square in a given color. This square can be scaled like a sprite.
	x and y are the center of the square.
	example: squ(64,64,4,255,0,0) --draw a large red square near the top left of the screen

* key(n)
	Checks if a key on the controller is pressed.
	Returns 2 or higher if the key is held, 1 if it's just pressed, and 0 if it's not pressed. As of this writing, quig stops at 2, but future versions will count how many frames a key has been pressed for, up to an arbitrary, but high limit (60*60*60 -- one hour)
	Usually, you want to check if it's not 0 or if it's equal to 1, depending on if you want something to happen when the button is held or when you press it just once.
	n is the key to check for, one of 7 pre-defined key codes:
		key_up
		key_down
		key_left
		key_right
		key_b
		key_a
		key_start
	example: key(key_b) --check if the B button on the controller is being pressed

* text(str,x,y,scale,mode)
	Draw a string at the given position. The font is 8x8, but it can be scaled freely.
	Newlines are supported. The ~ character is currently reserved as a control character, do not use it.
	mode is from 0-3:
		0 - black on white
		1 - white on black
		2 - black on transparent
		3 - white on transparent
	note: fractional scaling doesn't quite work with the non-transparent modes as of this writing, gaps appear in the text due to floating point errors. This will be fixed in a future release of quig. Several of the example games simply draw a rectangle behind the text instead.
	example: text("Hello, world!",8,8,2,3) --draw the text "Hello, world!" in double size in white text near the top left of the screen

* squcol(x1,y1,scale1,x2,y2,scale2)
	Check for collisions.
	Designed for sprite use, so it takes the position and scale of two 16x16 boxes.
	example: squcol(30,30,2,38,38,1) --check if a double-sized square at 30,30 and an unscaled square at 38,38 are colliding (yes)

* getfps()
	Get the game's average FPS at a 1 second resolution.
	When quig starts up, this will return 0.
	quig may run somewhat fast on some systems -- it reports a speed of 61-62hz on my Pi 2, for example. Ideally, vsync should be enabled on systems that can support hardware drawing.
	example: text(getfps(),0,0,1,0) --display the game's FPS at the top-left corner of the screen
	
provisional/deprecated commands:
None of these commands should currently be used at all.
If these commands remain in newer versions of quig, they may have entirely different parameters!

* playsong
* loopsong
* stopsong
* readfile
* writefile
* playsound
* loopsound
* stopsound

reserved commands/names:
These commands don't exist yet, but may be used at some point, so make sure your own functions aren't named the same!
Don't use these in this version of quig!

* spr_xys()
	sprite drawing with independent x/y scaling
* squ_xys()
	"square" drawing with independent x/y scaling
* squcol_xys()
	"square-square" collision with independent x/y scaling
* rectcol()
	rectangle-rectangle collision
* scrolltext()
	scrolling text
* audio()
	generate audio?
* keymulti()
	get input from more than the first controller (up to 4)
* half_width
	just view_width/2
* half_height
	just view_height/2
* getms()
	get how long it took for the last frame to run, in milliseconds (under normal circumstances would report around 16.67)

In addition, qu_* and quig.* are entirely reserved (eg, quig.spr() or qu_spr()).
A future release of quig may prefix all of the commands so that there is no chance of a new quig version accidentally naming a function the same as one existing in your program.

===
Constants reference:

* screen size
	view_width (240)
	view_height (144)
*key codes
	key_up
	key_down
	key_left
	key_right
	key_b
	key_a
	key_start

===
Compiling quig:

quig's source code is available at <https://github.com/bmdeeal/quig>.
quig-ui's source code is available at <https://github.com/bmdeeal/quig-ui>.

On Linux or via MSYS2, quig requires Lua 5.3, SDL2, SDL_mixer, and SDL_image for SDL2. quig is written in C++ and has been built with g++.
On Debian and Ubuntu based systems, ./deps-debian.sh will install the required dependencies for you.
quig has been compiled on Windows with MSYS2, and ./deps-msys2.sh will install the required dependencies if you wish to build quig yourself.

Run ./build.sh to compile quig. build.sh uses pkg-config to provide the correct compiler flags.

On Windows with VS2019, the quig-for-windows.sln project is pre-configured to be ready to compile 32-bit x86 builds. 
You will still need the .dll files for each of the libraries (available in dll-files.7z, or compilable from source) to run quig.

quig-ui is built using VS2019. After downloading the quig-ui sources, simply open quig-ui.sln and compile.

===
Future release to-do list:

* quickstart.txt, a nicer set of HTML pages for documentation -- this file is getting big and it explains everything, but it is absolutely just a truly gigantic wall of text
* hiragana text support (the font is loaded, but there isn't a good way to access it, maybe add control characters? I might make the control character ~, so avoid using it)
* maybe add paletted versions of various commands? sprites still have no palette restriction, but you should be able to define a palette and not constantly have to do things like palette[2].r,palette[2].g,palette[2].b or directly inputting color codes over and over
* quig-ui needs Linux support/testing (with minor modifications, it compiles and runs under Mono, we just need to do them in a nice, toggleable way)
* add option to force-run a game without a graphics file (likely with the position of the intended sprite overlaid?)
* add a combined .quigpak format that puts the graphics and the game in the same file and provide a utility for packing/unpacking games

===
Additional notes:

Be very careful with downloaded quig games! Programs for quig are programs like any other on your system.
As such, they may be able to access or modify files and other data on your machine. Do not run quig games from a source you do not trust.
If you wouldn't download and install a normal program from that source (eg, a .exe), you shouldn't trust a quig game from it either.

As of this writing, quig loads ALL of the Lua standard library. This will almost certainly change in the future -- Lua's file I/O and various OS interaction functions will be removed.
This doesn't guarantee security by any means, it just makes it so that a developer using quig has to go out of their way to be malicious.

If a quig game lacks a graphics file, one workaround to run it is to simply rename another. examples/works-all.png is provided for this purpose. It's quite ugly, but it's made to be visible.
In the future, there might be a quig option that generates a simple texture for this purpose.
As of this writing, quig-ui will not load a .quig file that doesn't have an associated .png file. 

===
Some musings:

The first game I wrote with quig was astro-burst, and a lot of the feature set in quig was built around being able to create "pseudo-3D" sprite scaling games, similar to Sega's classic mid-80s/early-90s output.
quig can't match them for resolution or detail due to its other goals, but it can deliver a sizzlingly fast experience with tons of graphics thrown on screen, even on a machine like the Raspberry Pi 2.
To keep things simple, there's no tilemap or anything. Everything on the screen is either text, a colored rectangle, or a sprite.

astro-burst and pop.drop'n were developed as showcases for quig.
With astro-burst, I wanted something immediately impressive, and a pseudo-3D driving game in space seemed like the trick.
The graphics calculations are in 3D, but the camera doesn't rotate at all.
pop-drop'n is a basic side-scroller where everything is bouncy. I really should make a more traditional platformer example to give people a better starting point, as many aspects of the game design stem from not caring to implement various things.

get-the-dot! is a game I wanted to expand on, but it was meant to be a tutorial piece on using quig. Unlike the other two showcase games, which are meant as proper, monolithic releases (and are licensed under the GPLv3 as a result), get-the-dot! was designed as a particularly simple game to ease users into making games for quig.
It's still not too huge -- 174 heavily commented and somewhat spaced out lines as of this writing, but I really keep wanting to expand on it. This really veers hard from the idea of a small game for tutorial use, where the reader, when finished, would have their own version of get-the-dot!.

...I should stop with all of the lowercased and punctuated proper nouns, but alas.

get-the-dot! is provided under CC0, so you can do anything whatsoever to it (such as porting it to a different environment entirely, entirely unencumbered by the fact that quig itself is under GPLv3). My name doesn't even show up when it's running.
That's not to say I'm not proud of it, and I spent a bit of time trying to tweak it to make it a more fun to play game out of the box -- the boost system makes it more exciting by far, but ultimately, the project is for those who want to get started with quig.
Adding a bazillion bells and whistles and things like particles and what-have-you would really get in the way of delivering a good introductory tutorial. Making it a better, more stylish, more fun game is thus left as an exercise to the reader.

Like, I guess I could write another tutorial on how to add that stuff, but I'd much rather work on a different aspect of the project.

Some of the roughness of the sample game is because it was meant to be very simple, and it grew a little bigger than the absolute beginning game it was supposed to be, gaining things like a pause screen and title screen (which, because I didn't actually want to bog the thing down with my usual framework of how to write games, are both the same screen almost). It's in a weird halfway state between an entirely unarchitected design and the design I ended up doing for astro-burst and pop.drop'n, which focuses more on very well delineated states. Alas.

I've moved to a Pi 4 from my old Pi 2 since starting quig. One major design decision made in quig was to focus on pure software drawing, which was probably a mistake, but given the poor desktop hardware acceleration support on earlier Pi models, it's a mistake I can live with. One artifact of this is that quig doesn't use hardware acceleration at all when drawing sprites to the screen. Hardware acceleration is only used to scale the tiny quig drawing area up after everything else has been drawn in software. The tiny drawing area was largely driven by quig initially having been developed on a Pi 2 in software mode, too, since drawing less was faster.

Sound is implemented, but a: hasn't had much testing, and b: seems to have a few issues. SDL_Mixer seems to have issues with resuming stopped music. If I load the music files as ordinary sounds, I have no such issues, but quig then takes way longer to start up and loads all of that into memory at once.

I hope you enjoy using quig as much as I do. It's simple, with just enough limitation to enforce a little bit of its own character on the results without stifling them. As I developed pop.drop'n, I found myself increasingly glad that I didn't enforce any palette limitations. The graphics I've drawn for my quig games largely use the rather nice Sweetie-16 palette by GrafxKid (see <https://lospec.com/palette-list/sweetie-16> for details), but ultimately, if something needed to be different from those colors, it could be. Also, particles. Full-color RGB particles, everywhere.

quig might be redundant as a platform -- you can probably port quig games to something like TIC-80 semi-easily, and there are a lot of other similar systems due to how easy it is to embed Lua. quig's spr command could be easily implemented with two textri calls.

Still, I had an idea of the look and style of games I wanted to be able to design with quig, and the various "fantasy console" platforms like TIC-80 and PICO-8 had certain restrictions or design elements that I didn't like that much. quig is its own thing, focusing on letting the user sling a bunch of scaled sprites around the screen like there's no tomorrow, easily. It has restrictions that you may not like, but it is also intended to be somewhat more free-form than either TIC-80 or PICO-8.

Also, I really wanted something I could run at the desktop. I might have been wrong, but at the time I started designing quig, TIC-80 required to be in full screen, since it used the hardware acceleration on the Pi 2 to draw, which didn't work under X11.

During the whole 2020 "stuck at home" deal, developing quig provided a nice way for me to write games that I could play on my Raspberry Pi 2. 

It was a good learning experience too. Embedding Lua was a surprisingly easy task, and it let me brush up on using SDL2, which was something I had done before, but not in any particularly major project. In addition, writing the front-end utilities to make it nicer to load .quig games has let me stay a bit sharper regarding GUI based development, since quig itself is command-line based.

It's been quite fun working on this, and I hope you have fun using it too.

--B.M.Deeal
