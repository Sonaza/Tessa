#pragma once

#include "ts/ivie/viewer/ViewerImageFile.h"

TS_PACKAGE2(app, util)

extern bool naturalSort(const String &lhs, const String &rhs);

extern bool naturalSortFile(const viewer::ViewerImageFile &lhs, const viewer::ViewerImageFile &rhs);
extern bool naturalSortFileByType(const viewer::ViewerImageFile &lhs, const viewer::ViewerImageFile &rhs);
extern bool naturalSortFileByLastModified(const viewer::ViewerImageFile &lhs, const viewer::ViewerImageFile &rhs);

extern bool naturalSortFileByExtension(const viewer::ViewerImageFile &lhs, const viewer::ViewerImageFile &rhs);

TS_END_PACKAGE2()
