Prints thread usage for BO3. Will show the file and (if available) line number of each thread.

Steps to use:
	1. Enable `/developer 2`
	2. Do things in the game to increase the threads
	3. From the cmd prompt run `bo3dbg.exe > output_file.txt`
	4. Open `output_file.txt` to see the dump

If you want line information for a file it must have been compiled into a mod. 

This was a side-project so the code is awful, and there are a lot of improvements that could probably be made.
