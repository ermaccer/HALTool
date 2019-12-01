// HALTool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <filesystem>


#include "filef.h"
#include "hal.h"

enum eMode {
	MODE_EXTRACT = 1,
	MODE_CREATE,
	PARAM_MORTYR
};



int main(int argc, char* argv[])
{
	if (argc == 1) {
		std::cout << "Usage: haltool <params> <file/folder>\n"
			<< "    -c             Creates archive from a folder\n"
			<< "    -e             Extracts archive\n"
			<< "    -o <input>     Specifies a folder for extraction/output filename\n"
			<< "    -m             Allows to work with Mortyr/WH40k AoD\n"
			<< "    -l <file>      Saves list file\n";

		return 1;
	}

	int mode = 0;
	int param = 0;
	std::string o_param;
	std::string l_param;

	// params
	for (int i = 1; i < argc - 1; i++)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
			return 1;
		}
		switch (argv[i][1])
		{
		case 'e': mode = MODE_EXTRACT;
			break;
		case 'c': mode = MODE_CREATE;
			break;
		case 'm': param = PARAM_MORTYR;
			break;
		case 'o':
			i++;
			o_param = argv[i];
			break;
		case 'l':
			i++;
			l_param = argv[i];
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			break;
		}
	}
	if (mode == MODE_EXTRACT)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}
		if (pFile)
		{
			hal_header hal;
			pFile.read((char*)&hal, sizeof(hal_header));

			if (hal.header != 'KUPA')
			{
				std::cout << "ERROR: " << argv[argc - 1] << "is not a valid HAL archive!" << std::endl;
				return 1;
			}

			std::unique_ptr<hal_entry_large[]> hal_ent_large = std::make_unique<hal_entry_large[]>(hal.files);
			std::unique_ptr<hal_entry_small[]> hal_ent_small = std::make_unique<hal_entry_small[]>(hal.files);

			for (int i = 0; i < hal.files; i++)
			{
				if (param == PARAM_MORTYR)
					pFile.read((char*)&hal_ent_small[i], sizeof(hal_entry_small));
				else
				pFile.read((char*)&hal_ent_large[i], sizeof(hal_entry_large));
			}



			if (!l_param.empty())
			{
				std::ofstream list(l_param, std::ofstream::binary);
				for (int i = 0; i < hal.files; i++)
				{
					std::string str;
					std::string name;
					if (param == PARAM_MORTYR) name = hal_ent_small[i].filename;
					else name = hal_ent_large[i].filename;
					if (!o_param.empty())
					{
						str = o_param;
						str += "\\";
					}
					str += name;
					list << str << std::endl;
				}
			}


			if (!o_param.empty())
			{
				std::experimental::filesystem::create_directory(o_param);
				std::experimental::filesystem::current_path(
					std::experimental::filesystem::system_complete(std::experimental::filesystem::path(o_param)));
			}

			for (int i = 0; i < hal.files; i++)
			{
				std::string output = hal_ent_large[i].filename;
			

				unsigned int dataSize = hal_ent_large[i].size;
				unsigned int dataOffset = hal_ent_large[i].offset;

				if (param == PARAM_MORTYR)
				{
					dataSize = hal_ent_small[i].size;
					dataOffset = hal_ent_small[i].offset;
					output = hal_ent_small[i].filename;
				}

				if (output.length() < 1) {
					std::cout << "ERROR: Invalid string length, try with -m switch." << std::endl;
					return 1;
				}

				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);

				std::cout << "Processing: " << output << std::endl;

				pFile.seekg(dataOffset, pFile.beg);
				pFile.read(dataBuff.get(), dataSize);

				std::ofstream oFile(output, std::ofstream::binary);
				oFile.write(dataBuff.get(), dataSize);
			}
		}
		std::cout << "Finished." << std::endl;
	}
	if (mode == MODE_CREATE)
	{

		if (l_param.empty())
		{
			std::cout << "ERROR: List file was not specified!" << std::endl;
			return 1;
		}

		std::experimental::filesystem::path folder(argv[argc - 1]);
		if (!std::experimental::filesystem::exists(folder))
		{
			std::cout << "ERROR: Could not open directory: " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (std::experimental::filesystem::exists(folder))
		{

			int filesFound = 0;
			int foldersFound = 0;
			// get files number
			for (const auto & file : std::experimental::filesystem::recursive_directory_iterator(folder))
			{
				filesFound++;
				if (std::experimental::filesystem::is_directory(file)) foldersFound++;

			}
			filesFound -= foldersFound;


			// get files info
			int files = 0;
			std::unique_ptr<unsigned int[]> size = std::make_unique<unsigned int[]>(filesFound);
			std::unique_ptr<std::string[]> path = std::make_unique<std::string[]>(filesFound);
			std::unique_ptr<std::string[]> name = std::make_unique<std::string[]>(filesFound);
			std::string line;


			if (!l_param.empty())
			{
				std::ifstream pList(l_param);
				if (!pList)
				{
					std::cout << "ERROR: Could not open list file: " << l_param << "!" << std::endl;
					return 1;
				}
		
				while (getline(pList, line))
				{
					path[files] = line;
					name[files] = splitString(path[files], true);
					std::ifstream pFile(path[files]);
					if (pFile)
						size[files] = (unsigned int)getSizeToEnd(pFile);
					files++;
				}
			}

			hal_header hal;
			hal.files = filesFound;
			hal.header = 'KUPA';

			std::string output = "new.hal";
			if (!o_param.empty())
				output = o_param;

			std::ofstream oFile(output, std::ofstream::binary);

			std::cout << "Writing to: " << output << std::endl;
			oFile.write((char*)&hal, sizeof(hal_header));

			int baseOffset = hal.files * sizeof(hal_entry_large) + sizeof(hal_header);
			if (param == PARAM_MORTYR) baseOffset = hal.files * sizeof(hal_entry_small) + sizeof(hal_header);

			for (int i = 0; i < hal.files; i++)
			{
				if (param == PARAM_MORTYR)
				{
					hal_entry_small ent;
					if (name[i].length() > 16) {
						std::cout << "ERROR: Too long filename! (16 max) (" << name[i] << ")";
						return 1;
					}
					sprintf(ent.filename, "%s", name[i].c_str());
					ent.offset = baseOffset;
					ent.size = size[i];
					baseOffset += size[i];
					oFile.write((char*)&ent, sizeof(hal_entry_small));
				}
				else
				{
					hal_entry_large ent;
					if (name[i].length() > 48) {
						std::cout << "ERROR: Too long filename! (48 max) (" << name[i] << ")";
						return 1;
					}
					sprintf(ent.filename, "%s", name[i].c_str());
					ent.offset = baseOffset;
					ent.size = size[i];
					baseOffset += size[i];
					oFile.write((char*)&ent, sizeof(hal_entry_large));
				}

			}

	
			for (int i = 0; i < hal.files; i++)
			{
				std::ifstream pFile(path[i], std::ifstream::in);
				
				if (!pFile)
				{
					std::cout << "ERROR: Could not open " << path[i] << "!"<< std::endl;
					return 1;
				}

				unsigned int dataSize = (unsigned int)getSizeToEnd(pFile);
				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
				std::cout << "Processing: " << name[i] << std::endl;
				pFile.read(dataBuff.get(), dataSize);
				oFile.write(dataBuff.get(),dataSize);
			}
		}
		std::cout << "Finished." << std::endl;
	}
    return 0;
}

