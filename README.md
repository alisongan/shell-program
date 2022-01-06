# shell-program
This is a rudimentary shell program I wrote for CMSC 15400 at UChicago. It supports basic commands pwd, cd, and exit as well as ls with args (actually any program can be run but I've only tested ls). The shell automatically filters out extra spaces or tabs.
If you want to run multiple commands in one line the format is [command] ; [ command] ; .... 
It also supports redirection (>). There is also a feature, advanced reditction (>+) which has the same format as >. If you >+ to a non existent file, it creates that file, 
otherwise it writes whatever you intended to the beginning of the specified file. >+ feature is buggy though. 

You can run the shell in interactive mode (make, ./myshell) where you type into the command line or you can run it in batch mode (make, ./myshell [filename] ). The shell will read 
each line of the file and execute the commands in the file, and then print the output with the commands all at once. 
