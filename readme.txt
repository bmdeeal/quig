quig - quick lua game system
(C) 2020 B.M.Deeal

===
Legal:

quig is distributed under the GPLv3, see quig-license.txt for details.
In addition, see the files under other-licenses/ for the licensing details of other libraries that quig uses, such as SDL and Lua.

Sources for many libraries are provided in libraries-w32/sources.7z.

Several of the example files are under the GPLv3, several are under CC0 (effectively no restrictions). Check the license if you want to copy the code into your own game or build off of the examples!

===
About:

quig is a very simple program designed to let you quickly and easily make small games with Lua. The various limitations found in quig are largely designed to keep the process simple and easy -- low resolution to reduce the focus on graphics, few buttons to encourage pick-up-and-play titles, and a small command set that contains the essentials of what is needed to make a game.

A simple tutorial showing how to create a simple game is provided. See get-the-dot.txt for details.

At the moment, it's not really a fantasy console or anything.
In addition, it's maybe a bit too simple, and the lack of integrated editors (in particular, for graphics) really does make it less than obvious to get started with, even if it's not too hard. Draw some graphics, write some code, play. 

quig doesn't bother limiting colors to a fixed palette or anything. As of this writing, quig doesn't support sound, although support is planned in the future.
Games are drawn to a true-color 240x144 canvas that gets scaled up.
quig updates at 60hz (ideally -- in software mode or without vsync enabled, quig has been observed to update as quickly as 62hz on some systems).

Games for quig are comprised of two files, a graphics file (just a plain 128x128 PNG image containing 16x16 tiles) and a Lua source file.
For example, my-game.quig would contain the Lua source and my-game.png would contain the graphics.

===
System requirements:

quig has been tested under Raspbian Linux and Windows 10 (via MSYS2 and a VS2019 build).
quig requires SDL2, SDL_image, SDL_mixer, and Lua 5.3. In addition, the GUI launcher requires YAD and bash (so it currently is not known to run on Windows).
quig will run on a wide range of hardware -- quig has even been run on a Raspberry Pi Zero (running at 1x resolution), with minor frame drops in the example game astro-burst.
quig was initially developed on a Raspberry Pi 2, and runs acceptably in software mode at 3x resolution there. quig runs very well on a Raspberry Pi 4, supporting hardware mode and vsync there.

===
Usage:

quig provides a simple GUI for running games. The bash script run-quig.sh will allow you to graphically select a quig game and configure any options.
When quig is exited, the GUI will reappear. Either select a game again or press cancel to exit.
This interface has only been tested under Raspbian Linux so far, and requires YAD to be installed.
On Raspbian, ($ is the prompt, don't type it!)
	$ sudo apt install yad
will install YAD if needed.

If you do not have YAD and bash installed or cannot install it (for example, on Windows), quig is still primarily a command-line program.
From the command line, games can be run as follows:
	$ quig mygame.quig
if mygame.quig and mygame.png are together in the same folder.

If mygame.quig has no errors and mygame.png can be loaded, the game will start.

quig currently supports three graphics configurations:
	--soft: full software drawing. As of this writing, this is the default option. This is the slowest option, but will run acceptably on the majority of systems, including my Raspberry Pi 2. The window maxes out at 3x internal resolution when using this mode.
	--hard: hardware window drawing. Use this option if your monitor doesn't run at 60hz and you want a larger screen size.
	--hard-vsync: hardware window drawing, timed to vertical sync. Generally the best option. quig is designed to run at 60fps, so using this on a display that runs faster or slower will cause games to run at entirely the wrong speed.

In addition, to run quig in fullscreen, use the --fullscreen option. This does not change the screen resolution, this merely stretches the window.
As of this writing, fullscreen mode is somewhat broken and does not preserve aspect ratio -- most screens are 4:3 or 16:9, and quig runs at a 5:3 (15:9) resolution.
	
For example,
	$ quig examples/astro-burst.quig --hard-vsync --fullscreen
will run astro-burst in vsync hardware mode and in fullscreen.

When in windowed mode, quig will automatically resize the window to the largest integer scale it can, minus a little space to account for window borders and taskbars and things like that.

===
Controls:

quig pretends that it's hooked up to a controller with the following layout:
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
Future versions of quig may support custom controller mappings.

The F6 key on the keyboard allows you to take an unscaled screenshot in the current directory as quig-sshot.png. Take note that if the file already exists, it will be overwritten.
The F8 key on the keyboard allows you to record a few seconds of gameplay as quig-vid.gif. Again, if the file already exists, it will be overwritten. The Back or Select key on a controller will also begin recording. Take note that the game will be unresponsive after the recording is finished for a few moments as it saves the recording to disk.

The Esc key immediately quits quig. 

===
Compile:

On Linux or via MSYS2, quig requires Lua 5.3, SDL2, SDL_mixer, and SDL_image for SDL2. quig is written in C++ and has been built with g++.
On Debian and Ubuntu based systems, ./deps-debian.sh will install the required dependencies for you.
quig has been compiled on Windows with MSYS2, and ./deps-msys2.sh will install the required dependencies if you wish to build quig yourself.

Run ./build.sh to compile quig. build.sh uses pkg-config to provide the correct compiler flags.

On Windows with VS2019, the quig-for-windows.sln project is pre-configured to be ready to compile 32-bit x86 builds. 
You will still need the .dll files for each of the libraries (available in dll-files.7z).

[TODO: proper install instructions]

===
Development:

All quig games have two mandatory functions: init() and step(). init() is called at the start of the game, and step() is called every frame.
Both MUST be present, even if init is left empty.
In addition, all quig games MUST have an associated .png holding the game graphics.

Included with quig are various example files that range from simple tutorials on how to get some graphics on screen and take user input to whole games with high speed scaling "3D" graphics and a complex object management system.

An empty.quig and empty.png file are in the examples folder, demonstrating the minimum requirements to create a quig program.
empty.quig just has the two required functions, and empty.png is entirely filled with the transparent color (#FF00FF, 255,0,255).

===
Command reference (it's quite short!):
TODO: document the sound functions once I stabilize the details of that API
TODO: same with file saving

* cls(red, green, blue)
	Clear the screen to a given color.
	The colors red, green, and blue are from 0-255.
	If you don't call this, remanants of the last frame will be left behind (which might be desirable! One effect that can be done is to clear the background every other frame, giving a slight "motion blur").
	example: cls(0,24,255)

* spr(x, y, scale, sheet_x, sheet_y)
	Draw a sprite to the screen. All sprites are 16x16 and drawn centered.
	Sprites can be scaled up and down to arbitrary floating point sizes, 1.0 is normal size.
	sheet_x/sheet_y are which 16x16 portion of the sprite sheet to display.
	0,0 is the top-left tile in the sheet, 7,7 the bottom-right. If the sheet is larger than 128x128, you still won't be able to access the outside area.
	example: spr(view_width/2,view_height/2,2,0,0)

* rect(x, y, width, height, red, green, blue)
	Draw a rectangle on the screen in a given color.
	x and y are the top-left corner of the rectangle.
	example: rect(32,32,8,48,255,255,255)

* squ(x, y, scale, red, green, blue)
	Draw a 16x16 square in a given color. This square can be scaled like a sprite.
	x and y are the center of the square.
	example(64,64,4,255,0,0)

* key(n)
	Checks if a key is pressed.
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
	example: key(key_b)

* text(str,x,y,scale,mode)
	Draw a string at the given position. The font is 8x8, but it can be scaled freely.
	Newlines are supported. The ~ character is currently reserved as a control character, do not use it.
	mode is from 0-3:
		0 - black on white
		1 - white on black
		2 - black on transparent
		3 - white on transparent
	note: fractional scaling doesn't quite work with the non-transparent modes as of this writing, gaps appear in the text due to floating point errors. This will be fixed in a future release of quig.
	example: str("Hello, world!",8,8,2,3)

* squcol(x1,y1,scale1,x2,y2,scale2)
	Check for collisions.
	Designed for sprite use, so it takes the position and scale of two 16x16 boxes.
	note: this function doesn't have the testing that it should, so there are probably a few issues with it.
	example squcol(32,32,2,38,38,1)

* getfps()
	Get the game's average FPS at a 1 second resolution.
	When quig starts up, this will return 0.
	quig may run somewhat fast on some systems -- it reports a speed of 61-62hz on my Pi 2, for example. Ideally, vsync should be enabled on systems that can support hardware drawing.
	example: getfps()

===
Constants reference:

* screen size
	view_width - 240
	view_height - 144
*key codes
	key_up
	key_down
	key_left
	key_right
	key_b
	key_a
	key_start

===
Future release to-do list:

* hiragana text support (the font is loaded, but there isn't a good way to access it, maybe add control characters? I might make the control character ~, so avoid using it)
* maybe add paletted versions of various commands? sprites still have no palette restriction, but you should be able to define a palette and not constantly have to do things like palette[2].r,palette[2].g,palette[2].b or directly inputting color codes over and over
* although quig is a command-line program, a good front-end is a must, and the one that quig currently ships with isn't terribly good
* add option to force-run a game without a graphics file (likely with the position of the intended sprite overlaid?)
* add a combined .quigpak format that puts the graphics and the game in the same file and provide a utility for packing/unpacking games

===
Additional notes:

Be very careful: programs for quig are programs like any other on your system. As such, they may be able to access or modify files and other data on your machine. Do not run quig games from a source you do not trust. If you wouldn't download and install a normal program from that source, you shouldn't trust a quig game from it either.

As of this writing, quig loads ALL of the Lua standard library. This will almost certainly change in the future -- Lua's file I/O and various OS interaction functions will be removed. This doesn't guarantee security by any means, it just makes it so that a developer using quig has to go out of their way to be malicious.

If a quig game lacks a graphics file, one workaround to run it is to simply rename another. examples/works-all.png is provided for this purpose. It's quite ugly, but it's made to be visible.
In the future, there might be a quig option that generates a simple texture for this purpose.

===
Some musings:

The first game I wrote with quig was astro-burst, and a lot of the feature set in quig was built around being able to create "pseudo-3D" sprite scaling games, similar to Sega's classic mid-80s/early-90s output. quig can't match them for resolution or detail due to its other goals, but it can deliver a sizzlingly fast experience with tons of graphics thrown on screen.

astro-burst and pop.drop'n were developed as showcases for quig. With astro-burst, I wanted something immediately impressive, and a pseudo-3D (well, the graphics calcuations are in 3D, but the camera doesn't rotate at all) driving game in space seemed like the trick. 

get-the-dot! is a game I want to expand on, but I have to contend with the fact that it's meant to be a tutorial piece on using quig. Unlike the two showcase games, which are meant as proper, monolithic releases (and are licensed under the GPLv3 as a result), get-the-dot! was designed as a particularly simple game to introduce how to use quig.

It's still not too huge -- 174 heavily commented and somewhat spaced out lines as of this writing, but I really keep wanting to expand on it. This really veers hard from the idea of a small game for tutorial use, where the reader, when finished, would have their own version of get-the-dot!. Yeah, I should stop with all of the lowercased and punctuated proper nouns, but alas.

get-the-dot! is provided under CC0, so you can do anything whatsoever to it. My name doesn't even show up when it's running. That's not to say I'm not proud of it, and I spent a bit of time trying to tweak it to make it a more fun to play game out of the box -- the boost system makes it more exciting by far, but ultimately, the project is for those who want to get started with quig, and adding a bazillion bells and whistles and things like particles and what-have-you would really get in the way of delivering a good introductory tutorial, and leaving some gaps for you, the reader, to fill in is important.

Like, I guess I could write another tutorial on how to add that stuff, but I'd much rather work on a different aspect of the project.

Some of the roughness is because it was meant to be very simple, and it grew a little bigger than the absolute beginning game it was supposed to be, gaining things like a pause screen and title screen (which, because I didn't actually want to bog the thing down with my usual framework of how to write games, are both the same screen almost). It's in a weird halfway state between an entirely unarchitected design and the sort of thing I ended up doing for astro-burst and pop.drop'n. Alas.

I hope you enjoy using quig as much as I do. It's simple, with just enough limitation to enforce a little bit of its own character on the results without stifling them. As I developed pop.drop'n, I found myself increasingly glad that I didn't enforce any palette limitations. The graphics largely use the rather nice Sweetie-16 palette, but ultimately, if something needed to be different, it could be. Also, particles. Full-color RGB particles, everywhere.

During this whole 2020 "stuck at home" deal, developing quig provided a nice way for me to write games for my Raspberry Pi 2. There are other small game development systems and fantasy consoles and things like that, but quig is its own thing, focusing on letting the user sling a bunch of scaled sprites around the screen like there's no tomorrow, easily.

It was a good learning experience too. Embedding Lua was a surprisingly easy task, and it let me brush up on using SDL2, which was something I had done before, but not in any particularly major project.

I have somewhat recently moved to a Pi 4. One major design decision made in quig was to focus on pure software drawing, which was probably a mistake, but given the poor desktop hardware acceleration support on earlier Pi models, it's a mistake I can live with. One artifact of this is that by default, quig starts up in software mode.

You can probably port quig games to something like TIC-80 semi-easily. Maybe. The quig command set is quite small, and only file saving wouldn't really mesh too well. spr could be easily implemented with two textri calls.

Sound is implemented, but a: hasn't had much testing, and b: seems to have a few issues. SDL_Mixer seems to have issues with resuming stopped music. If I load the music files as ordinary sounds, I have no such issues, but quig now takes longer to start up and loads all of that into memory at once.

