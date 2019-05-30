import os
import sys

def rename(folder_path, search_string, replacement):
	print("Searching folder path '{}' for files...".format(folder_path))
	
	num_renamed = 0
	for file_name in os.listdir(folder_path):
		new_file_name = file_name.replace(search_string, replacement)
		if (file_name != new_file_name):
			print("  Renamed '{}' -> '{}'".format(file_name, new_file_name))
			os.rename(os.path.join(folder_path, file_name), os.path.join(folder_path, new_file_name))
			num_renamed = num_renamed + 1
	
	if (num_renamed > 0):
		print("Done. {} files renamed.".format(num_renamed))
	else:
		print("Done. No matching files found.")
			

if len(sys.argv) < 4:
	print("--- Rename Utility ---\n")
	print("Replaces specified part of filenames of all matching files in given folder.")
	print("Usage: rename.py <folder path> <search string> <replacement>")
	exit

folder_path = sys.argv[1]
search_string = sys.argv[2]
replacement = sys.argv[3]

if (os.path.exists(folder_path)):
	rename(folder_path, search_string, replacement)
else:
	print("")
	print("Folder path '{}' does not exist. Aborting.".format(folder_path))