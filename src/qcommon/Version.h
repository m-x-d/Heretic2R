//
// Version.h -- Based on https://www.zachburlingame.com/2011/02/versioning-a-native-cc-binary-with-visual-studio
//
// Copyright 2026 mxd
//

#pragma once

#define STR2(s)	#s
#define STR(s)	STR2(s)

#define VERSION_MAJOR				1
#define VERSION_MINOR				0
#define VERSION_REVISION			4
#define VERSION_BUILD				0

#define VER_FILE_DESCRIPTION_STR	"Heretic2R reverse-engineered source port"
#define VER_FILE_VERSION			VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD
#define VER_FILE_VERSION_STR		STR(VERSION_MAJOR) "." STR(VERSION_MINOR) "." STR(VERSION_REVISION) "." STR(VERSION_BUILD)

#define VER_PRODUCTNAME_STR			"Heretic2R"
#define VER_PRODUCT_VERSION			VER_FILE_VERSION
#define VER_PRODUCT_VERSION_STR		"R" STR(VERSION_REVISION)
#define VER_ORIGINAL_FILENAME_STR	VER_PRODUCTNAME_STR ".exe"
#define VER_INTERNAL_NAME_STR		VER_ORIGINAL_FILENAME_STR
#define VER_COPYRIGHT_STR			"Copyright (C) 1998 Raven Software, 2024-2026 MaxED"
