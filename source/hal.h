#pragma once

struct hal_header {
	int               header; // KUPA
	int               files;
	char              pad[24] = {};
};

// Tajemnicza Wyspa, Tytus, Wilczy Szaniec
struct hal_entry_large {
	unsigned int           size;
	unsigned int           offset;
	char                   pad[8] = {};
	char                   filename[48] = {};
};

// Mortyr, WH40K AoD
struct hal_entry_small {
	unsigned int           size;
	unsigned int           offset;
	char                   pad[8] = {};
	char                   filename[16] = {};
};