/*********************************************************************\
	compressor.h												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef COMPRESSOR_H
#define COMPRESSOR_H

// standard library & win32 api
#ifndef NOMINMAX
	#define NOMINMAX 
#endif

#include <windows.h>
#include <sstream>
#include <iostream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <list>
#include <filesystem>
#include <algorithm>
#include <map>
#include <vector>

namespace compressor
{
	// base class for all compression libraries, providing a common interface
	class generalLib
	{
	public:
		enum class libId { undefined, uncompressed, winCompApi, bzip2 };					// list of all available compression libraries

	protected:
		// Variables
		std::wostream						*osPrint;										// stream for output. default is cout
		int									verbosity;										// output detail level. default is 2
		std::wstring						name;											// name of the compression library
		libId								id;												// id of the compression library

											// Constructor / destructor
											generalLib();
											~generalLib();

	public:
		bool								print							(std::wstringstream &ss, int level);
		bool								setVerbosity					(int newVerbosity) { verbosity = newVerbosity; return true; };
		std::wstring const&					getName							() { return name; };
		libId const&						getLibId						() { return id; };
		virtual bool						compress						(void *compressedData, void *sourceData, unsigned int nBytesToCompress, unsigned int &nBytesCompressed) { return false; };
		virtual bool						decompress						(void *destData, void *compressedData, unsigned int nBytesCompressed, unsigned int &nBytesDecompressed) { return false; };
		virtual long long					estimateMaxSizeOfCompressedData	(long long amountUncompressedData) { return 0; };
	};

	// class for uncompressed data
	class uncompressed : public generalLib
	{
	public:
											uncompressed					() { name = L"uncompressed"; id = libId::uncompressed; };
		bool								compress						(void *compressedData, void *sourceData, unsigned int nBytesToCompress, unsigned int &nBytesCompressed);
		bool								decompress						(void *destData, void *compressedData, unsigned int nBytesCompressed, unsigned int &nBytesDecompressed);
		long long							estimateMaxSizeOfCompressedData	(long long amountUncompressedData);
	};

	// Class providing read/write functions for a compressed file.
	// It can handle very large files and is able to randomly read and write single bytes of the file.
	// Any C++ compression library can be used.
	// The access to the file is done in named sections, addressed by a string key.
	// The actual file is written in the destructor during the flush() function. 
	// Before, the sections are written to temporary files.
	class file
	{
	public:

											file							(generalLib& comp);
											~file							();

		bool								open							(std::wstring const& filePath, bool onlyRead);
		bool								close							();
		bool								isOpen							();
		bool								read							(std::wstring const& key, long long position, long long numBytes,       void* pBytes);
		bool								write							(std::wstring const& key, long long position, long long numBytes, const void* pBytes);
		bool								flush							();
		bool								setBlockSize					(unsigned int newSizeInBytes);
		long long							getSizeOfUncompressedSection	(std::wstring const& key);
		long long							getSizeOfCompressedSection		(std::wstring const& key);
		std::vector<std::wstring>			getKeys							();
		bool 								doesKeyExist					(std::wstring const& key);
		
		// Each section is stored in blocks of a fixed size. This enables random read access.
		// Instead of a header a footer is used here.
		// File Format
		// position									bytes			description
		// sections[0].blocks[0].offsetInSection	0				block[0]
		// 											compressedSize		data
		// sections[0].blocks[1].offsetInSection	0				block[1]
		// 											...				...
		// sections[0].offsetInFile					0 				blockInfo
		// 											0   	  			sections[0].blocks[0]
		// 											4						offsetInSection
		// 											4						compressedSize
		// 											0					sections[0].blocks[1]
		// 											...						...
		// 											0				footer
		// fileInfoOffsetInFile						0					sections[0]
		// 											8						offsetInFile
		// 											8						uncompressedSize
		// 											8						compressedSize
		// 											4						numBlocks
		// 											8						sectionId
		// 											8						keyLengthInBytes
		// 											keyLengthInBytes			keyName[keyLengthInBytes]
		// 											0					sections[1]
		// 											...						...
		// footerOffsetInFile						2					typeId
		// 											2					versionId
		// 											4					numSections
		// 											8 					sectionsOffsetInFile
		// 											8					footerOffsetInFile
		// 											8					fileInfoOffsetInFile
		// 											4					blockSizeInBytes
		// 											4					usedLib

		class footerStruct;
		class tmpFile;

		// section of the file, containing the data
		struct sectionInfo	
		{	
			struct blockInfo
			{
				// data copied directly to file
				unsigned int				offsetInSection					= 0;								// offset in bytes within the section
				unsigned int				compressedSize					= 0;								// size of the compressed block in bytes
			};

			// data copied directly to file	
			long long						offsetInFile					= 0;								// offset in bytes within the file
			long long						uncompressedSize				= 0;								// size of the uncompressed section in bytes
			long long						compressedSize					= 0;								// size of the compressed section in bytes
			unsigned int					numBlocks						= 0;								// number of blocks in the section
			unsigned int					sectionId						= 0;								// index of the section in the file
			unsigned int					keyLengthInBytes				= 0;								// length of the key in bytes

			// abstract data (cannot be written directly to file)
			std::wstring					keyName;															// key of the section, by which the section is addressed in the file
			std::vector<blockInfo>			blocks;																// data

			// functions
			bool							write							(std::fstream& fs, footerStruct& footer);
			bool							read							(std::fstream& fs, footerStruct& footer);
			bool							writeData						(std::fstream& fs, footerStruct& footer, generalLib& comp, tmpFile& tmpFile, bool forceSingleThreading = false);
			bool							readData						(std::fstream& fs, footerStruct& footer, generalLib& comp, void* pBytes, long long numBytes, long long position);
		};

		// footer of the file, containing infos about the sections
		struct footerStruct
		{
			// data copied directly to file
			short							typeId							= 0x7d67;							// unique id of the file format
			short							versionId						= 1;								// version of the file format
			unsigned int					numSections						= 0;								// number of sections in the file
			long long						sectionsOffsetInFile			= 0;								// offset in file where the sections start
			long long						footerOffsetInFile				= 0;								// offset in file where the footer starts
			long long						fileInfoOffsetInFile			= 0;								// offset in file where the file info starts
			unsigned int					blockSizeInBytes				= 1000;								// each section is separated in blocks to enable random read access
			generalLib::libId				usedLib							= generalLib::libId::undefined;		// id of the used compression algorithmn/library

			// abstract data (cannot be written directly to file)	
			std::map<std::wstring,size_t>	dictionary;															// mapping from keys to section indizes
			std::vector<sectionInfo>		sections;															// data

			// functions
			bool 							doesKeyExist					(std::wstring const& key) { return dictionary.find(key) != dictionary.end(); };
			bool							write							(std::fstream& fs);
			bool							read							(std::fstream& fs, generalLib::libId libId);
			void							clear							();
			sectionInfo&					getSection						(std::wstring const& key);
		};

		// uncompressed temporary file for writing the sections, used before it is written to the actual compressed file
		// this allows fast reading and writing of the sections, even if the file is very large
		class tmpFile
		{
		public:
											tmpFile							(std::wstring const& keyName);
											~tmpFile						();

			std::wstring const&				getKeyName						() { return keyName; };
			std::wstring const&				getFilePath						() { return filePath; };
			long long 						getSize							();
			bool							write							(long long position, long long numBytes, const void* pBytes);
			bool							read							(long long position, long long numBytes, void* pBytes);

			static bool  					doesExist						(std::wstring const& keyName);

		private:
			std::wstring					keyName;															// key of the section	
			std::wstring					filePath;															// path to the temporary file
			std::fstream					fsTmp;																// file stream for reading/writing the temporary file

			bool 							openIfNotOpen					();
		};

	private:
		static const size_t 				maxKeyLength 					= 240;								// each section in the file is identified by a string key. this is the maximum length of the key.

		footerStruct						footer;																// footer of the file, containing infos about the sections
		std::fstream						fs;																	// file stream for readiong/writing the actual file on the disk
		generalLib*							comp							= nullptr;							// pointer to the compression library
		bool								readOnlyMode					= false;							// if true, no writing is allowed
		std::vector<tmpFile*>				tmpFiles;															// temporary files for writing the sections

		tmpFile&							getTmpFile						(std::wstring const& key);				
		bool 								readFromCompressed				(std::wstring const& key, long long position, long long numBytes, void* pBytes);
	};

} // namespace compressor

// compressors libs
// #include "compLib_bzip2.h"
// #include "compLib_easyzlib.h"
// #include "compLib_lzma.h"
// #include "compLib_lzo.h"
// #include "compLib_minilzo.h"
// #include "compLib_miniz.h"
// #include "compLib_snappy.h"
// #include "compLib_ucl.h"
#include "compLib_winCompApi.h"

#endif // COMPRESSOR_H
