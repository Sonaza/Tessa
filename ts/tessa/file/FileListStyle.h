#pragma once

TS_PACKAGE1(file)

namespace priv 
{

enum FileListStyleBits
{
	ListStyleBits_Files       = (1 << 0),
	ListStyleBits_Directories = (1 << 1),
	ListStyleBits_Recursive   = (1 << 2),
};

}

enum FileListStyle : uint8
{
	FileListStyle_Directories           = priv::ListStyleBits_Directories,
	FileListStyle_Directories_Recursive = FileListStyle_Directories | priv::ListStyleBits_Recursive,

	FileListStyle_Files                 = priv::ListStyleBits_Files,
	FileListStyle_Files_Recursive       = FileListStyle_Files | priv::ListStyleBits_Recursive,

	FileListStyle_All                   = FileListStyle_Files | FileListStyle_Directories,
	FileListStyle_All_Recursive         = FileListStyle_All | priv::ListStyleBits_Recursive,

};

TS_END_PACKAGE1()

