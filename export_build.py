import os
import shutil
import msvcrt
import sys

project_name = "Ivie"
export_dir_name = "export"
export_build_configuration = "FinalRelease"

base_path = os.path.dirname(os.path.realpath(__file__))
export_path = os.path.join(base_path, export_dir_name, project_name)
workdir_path = os.path.join(base_path, "workdir")

builds_dir = os.path.join(base_path, "builds")
builds_path = os.path.join(builds_dir, export_build_configuration)

files_to_copy = {
	base_path : [
		"usage_and_keybinds.txt"
	],
	builds_path : [
		"Ivie.exe"
	],
	workdir_path : [
		"shader/area_clip.frag",
		"shader/background_gradient.frag",
		"shader/convert_freeimage.frag",
		"shader/convert_webm.frag",
		"SourceHanSans-Medium.ttc",
		"selawk.ttf",
		"ivie_logo_32.png",
	]
}

# ------------------------------------------------------------------------

print("\n")
print( "Exporting a new distributable build.\n")

print(f"  Project              : {project_name}")
print(f"  Export configuration : {export_build_configuration}\n")
print(f"  Export path          : {export_path}")
print(f"  Builds path          : {builds_path}")
print(f"  Workdir path         : {workdir_path}")
print("")

def exists_not_empty(path):
	return os.path.exists(path) and len(os.listdir(path)) > 0

def mkdir_p(path):
	try:
		os.makedirs(path)
	except OSError as exc:
		import errno
		if exc.errno == errno.EEXIST and os.path.isdir(path):
			pass
		else:
			raise

def prompt_choice(question, valid_choices, default = None):
	if isinstance(valid_choices, str):
		valid_choices = list(valid_choices)
	
	if not isinstance(valid_choices, list):
		raise ValueError("'valid_choices' should be a list or a string of 2 characters or more (1 character per choice)")
	else:
		if len(valid_choices) != len(set(valid_choices)):
			raise ValueError("'valid_choices' has duplicate choices")
		
		for c in valid_choices:
			if len(c) != 1:
				raise ValueError("'valid_choices' should be a list or a string of 2 characters or more (1 character per choice)")
	
	if default != None:
		if len(default) != 1:
			raise ValueError("'default' should be exactly 1 character long")
		if not default in valid_choices:
			raise ValueError("'default' is not in list of choices")
	
	choices = '/'.join([x.upper() if x == default else x for x in valid_choices])
	print(f"{question} [{choices}]  ", end='', flush=True)
	
	retries = 0
	while True:
		choice = msvcrt.getch()
		
		if choice == b'\r' and default != None:
			return default
		
		if choice == b'\x03' or choice == b'\x1b':
			print("Cancelled")
			return None
		
		try:
			decoded_choice = choice.decode('utf-8').lower()
			if decoded_choice in valid_choices:
				print(decoded_choice)
				return decoded_choice
			
		except UnicodeDecodeError:
			pass
		
		retries += 1
		if retries == 5:
			print("Press Escape or Ctrl-C to cancel choice. ", end='', flush=True)
		
if exists_not_empty(export_path):
	print("Export target folder already exists. Everything inside will be deleted.")
	overwrite_choice = prompt_choice("Do you wish to overwrite?", 'yn', 'y')
	if overwrite_choice == 'n':
		print("Export aborted: Overwrite not allowed.\n")
		exit()
	
	shutil.rmtree(export_path)
	print("Export target folder has been deleted.\n")
	
mkdir_p(export_path)

has_errors = False

for root, files in files_to_copy.items():
	for file in files:
		target_dir_path = os.path.join(export_path, os.path.dirname(file))
		mkdir_p(target_dir_path)
		
		src_path = os.path.normpath(os.path.join(root, file))
		dst_path = os.path.normpath(os.path.join(export_path, file))
		print(f"  Copying {src_path.ljust(55)} -> {dst_path.ljust(60)} ... ", end='', flush=True)
		if os.path.exists(src_path):
			try:
				shutil.copy(src_path, dst_path)
				print("OK")
			except IOError as e:
				print(e)
				has_errors = True
		else:
			print("File does not exist!")
			has_errors = True
			
print()

if not has_errors:
	print("Export complete!\n")
else:
	print("Export had errors and it may be partial or completely failed. See errors above.\n")

