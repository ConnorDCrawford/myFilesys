NAME
	myFilesys

SYNOPSIS
	[command] [arguments]
	Where [command] is the name of an internal command, and where arguments are the arguments for that program or internal command. Each argument must be separated by whitespace. If the argument is a path, separate subdirectories with a ‘/‘, as you would in Linux, starting with the root folder. To enter the current command line, press ‘Enter’.

DESCRIPTION
	myFilesys is a simple file system based on the ext2 Unix File System and the HFS+ file system. 

ARGUMENTS
	The first argument is assumed to be a I/O command. Any following arguments are assumed to be arguments for the command. To enter a path as an argument, enter it as you would in Linux, starting with the root directory. Ex. “/first/second/third.txt”.

BUILT IN COMMANDS
	The following commands are defined internally. 

	create [name] [size]	Creates a new volume with the designated volume name. This will overwrite any existing volume in the same directory with the same name. The size argument is in megabytes.

	mount [volume name]	Mounts the volume with the designated named. The volume must exist.

	unmount			Unmounts the currently mounted volume (only one volume can be open at  time).

	touch [path/filename]	Creates a file with the specified name in the specified directory.

	mkdir [path/directory]	Creates a directory with the specified name in the specified directory.

	o [path/filename]	Opens the file at the specified path, if it exists. Creates a file in the current working directory containing the file’s data.

	c [path/filename]	Closes the file at the specified path, deleting the file created in the current working directory created when the file was opened.

	w [source file] [destination file]	Writes the data in the source file (this must be in your file system, not within myFilesys) into the destination file (a file within myFilesys). Overwrites any data in the destination file. Updates the open file in the current working directory. Creates the destination file if it does not exist.

	r [file]		Outputs the contents of the designated file to the terminal (WARNING: if the file is a binary file, the data will not be formatted and not be readable. Only use for text files).

	rm [file or directory]	Removes the specified file or directory. If removing a directory, it will also remove any files or directories contained in the directory. WARNING: this is not reversible.

	ls [directory]		Lists the contents of the specified directory (file/directory names).
	
	quit			Unmounts any open volume, closing all open files, and quits myShell.