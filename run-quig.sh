#!/bin/bash
# run-quig.sh -- simple GUI front-end for quig
# run-quig.sh is (C) 2020 B.M.Deeal, and is licensed under the GPLv3

# This file expects word-wrapping and a tab width of 4.

#	This program is free software: you can redistribute it and/or modify it under the terms of version 3 of the GNU General Public License as published by the Free Software Foundation.
#
#    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#    See the GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along with this program.
#    If not, see <https://www.gnu.org/licenses/>.


#notes: this is hilariously fragile and breaks severely if anything changes place, but it will do for now (and maybe even later if nothing changes place)
#TODO: check for quig in $PATH before trying ./quig
#TODO: make simple version that works with Zenity, which is more likely to be installed on more machines

#run in a loop, eg, if the user wants to play another game
#TODO: the previous settings should remain selected, rather than resetting!
echo "Now starting the quig launcher..."
echo "(C) 2020 B.M.Deeal."
echo ""
while true
do
	#main GUI
	echo "Displaying interface..."
	res=$(yad --form \
		--title="quig game launcher" \
		--width=400 \
		--field="<b>Welcome to quig!\nSelect options below.</b>":LBL "" \
		--field="":LBL "" \
		--field="Game to run:":FL "" \
		--field="Display mode:":CB "Software"!"^Hardware"!"Hardware (vsync)" \
		--field="Run in fullscreen?":CHK "false"
	)
	
	#handle cancel option
	if [ $? -ne 0 ]
	then
		echo "Exiting quig launcher..."
		exit 0
	fi
	
	#read data, show on terminal for debug (this is a GUI program, so we just spew this out always since most people shouldn't see it)
	IFS='|' read -r -a lines <<< "$res"
	echo "The following data will be decoded:"
	for index in "${!lines[@]}"
	do
		echo "$index" "${lines[index]}"
	done
	
	#parse data:
	#selected game file
	#it would be really nice if YAD had file extension filtering
	game="${lines[2]}"
	
	#decode video mode
	#it would be really nice if yad had a separate display option vs what it actually emitted, which would really simplify things a lot
	videosel="${lines[3]}"
	if [ "$videosel" == "Software" ]
	then
		videomode="--soft"
	elif [ "$videosel" == "Hardware" ]
	then
		videomode="--hard"
	elif [ "$videosel" == "Hardware (vsync)" ]
	then
		videomode="--hard-vsync"
	fi
	
	#handle fullscreen option
	fullscreen=""
	if [ "${lines[4]}" == "TRUE" ]
	then
		fullscreen="--fullscreen"
	fi
	
	#run quig, preferring a binary in the current folder if available
	quigloc="quig"
	if [ -f "./quig" ]
	then
		quigloc="./quig"
	elif hash "quig" &> /dev/null
	then
		quigloc="quig"
	else
		yad --form \
		--title="fatal error!" \
		--text-align="left" \
		--image="dialog-warning" \
		--text="<b>Fatal error:</b>\nCannot find quig!" \
		--button="gtk-ok"
		exit 1
	fi
	echo "Running command: '$quigloc $game $videomode $fullscreen'..."
	echo ""
	"$quigloc" "$game" "$videomode" "$fullscreen"
	
	
	
		
	echo ""
	echo "Exited quig..."
done
