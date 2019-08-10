#pragma once

TS_PACKAGE2(app, viewer)

class FileScanner
{
public:
	FileScanner(const std::wstring &directoryPath, const std::vector<std::wstring> &allowedExtensions);
	~FileScanner();

	std::vector<std::wstring> getFileList();

	static void scanTask(FileScanner *parent);

private:
	class ScannerBackgroundTask;
	ScopedPointer<ScannerBackgroundTask> backgroundTask;
};

TS_END_PACKAGE2()
