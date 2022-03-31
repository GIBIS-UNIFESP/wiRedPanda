// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"

#ifdef DEBUG
int Comment::verbosity = DEBUG;
#else
int Comment::verbosity = -1;
#endif