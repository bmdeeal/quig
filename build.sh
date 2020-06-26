#!/bin/sh
# 2020 B.M.Deeal
# Build script for quig.
# At the moment, this has been tested under Raspbian (on a Pi 2 and a Pi 4) and Windows 10 (with MSYS2).
#TODO: look for a local copy of Lua or SDL in the quig folder, for systems without pkg-config
#TODO: does SDL need a similar set of names to check like Lua does?
#TODO: add a nice way to build without SDL_mixer, it's not like sound is THAT important

echo "notice: quig build script started!"
echo "notice: identifying Lua..."
#check for lua, it's registered differently on different systems
if pkg-config lua-5.3
then
	luaname="lua-5.3"
elif pkg-config lua5.3
then
	luaname="lua5.3"
elif pkg-config lua53
then
	luaname="lua53"
elif pkg-config lua
then
	echo "warning: this system does not designate which version of Lua is to be used; quig is designed for Lua 5.3!"
	luaname="lua"
else
	#todo: option to build wiith a local source version of lua
	echo "fatal error: either Lua (5.3) is not installed or it isn't registered with pkg-config!"
	exit 1
fi
echo "notice: Lua detected as '$luaname'!"

#set up compiler flags
quig_outputname="quig"
quig_libs=$(pkg-config --libs --cflags sdl2 SDL2_image SDL2_mixer "$luaname")

#build quig
echo "notice: building quig..."
if g++ quig.cpp -O2 -Wall -funsigned-char $quig_libs -o $quig_outputname
then
	echo "notice: quig built!"
	exit 0
else
	echo "fatal error: could not compile!"
	exit 1
fi
