/*********************************************************************
/*********************************************************************
	compressor.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "compressor.h"
#include <thread>
#include <future>

using namespace std;

#pragma region generalLib
//-----------------------------------------------------------------------------
// Name: generalLib()
// Desc: generalLib class constructor
//-----------------------------------------------------------------------------
compressor::generalLib::generalLib()
{
	// init default values
	osPrint		= &wcout;
	verbosity	= 3;
}

//-----------------------------------------------------------------------------
// Name: ~generalLib()
// Desc: generalLib class destructor
//-----------------------------------------------------------------------------
compressor::generalLib::~generalLib()
{
}

//-----------------------------------------------------------------------------
// Name: print()
// Desc: 
//-----------------------------------------------------------------------------
bool compressor::generalLib::print(wstringstream &ss, int level)
{
	if (verbosity > level) {
		*osPrint << ss.str();
		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------------
// Name: compress()
// Desc: 
//-----------------------------------------------------------------------------
bool compressor::uncompressed::compress(void *compressedData, void *sourceData, unsigned int nBytesToCompress, unsigned int &nBytesCompressed)
{
	memcpy(compressedData, sourceData, nBytesToCompress);
	nBytesCompressed	= nBytesToCompress;
	return true;
}

//-----------------------------------------------------------------------------
// Name: decompress()
// Desc: 
//-----------------------------------------------------------------------------
bool compressor::uncompressed::decompress(void *destData, void *compressedData, unsigned int nBytesCompressed, unsigned int &nBytesDecompressed)
{
	memcpy(destData, compressedData, nBytesCompressed);
	nBytesDecompressed	= nBytesCompressed;
	return true;
}

//-----------------------------------------------------------------------------
// Name: estimateMaxSizeOfCompressedData()
// Desc: 
//-----------------------------------------------------------------------------
long long compressor::uncompressed::estimateMaxSizeOfCompressedData(long long amountUncompressedData)
{
	return amountUncompressedData;
}
#pragma endregion

#pragma region file
//-----------------------------------------------------------------------------
// Name: file()
// Desc: Constructor
//-----------------------------------------------------------------------------
compressor::file::file(generalLib & comp) 
	: comp{&comp}
{
	footer.usedLib = comp.getLibId();
}

//-----------------------------------------------------------------------------
// Name: ~file()
// Desc: Destructor
//-----------------------------------------------------------------------------
compressor::file::~file()
{
	for (auto& curTmpFile : tmpFiles) {
		delete curTmpFile;
	}
	tmpFiles.clear();
	if (fs.is_open()) fs.close();
}

//-----------------------------------------------------------------------------
// Name: open()
// Desc: Open the compressed file and reads in the footer information.
//-----------------------------------------------------------------------------
bool compressor::file::open(wstring const & filePath, bool onlyRead)
{
	if (!comp) return false;
	if (fs.is_open()) return false;

	// convert wstring to string
	std::filesystem::path filePathStr = filePath;

	// file does not exist
	if (!filesystem::exists(filePathStr)) {
		if (onlyRead) return false;

		// create directory if it does not exist
		if (filePathStr.has_parent_path() && !filesystem::exists(filePathStr.parent_path())) {
			try {
				if (!filesystem::create_directories(filePathStr.parent_path())) {
					return false;
				}
			} catch (const std::exception&) {
				return false;
			}
		}

		// open file
		fs.open(filePathStr, ios::in | ios::out | ios::binary | ios::trunc);

		if (!fs.good() || !fs.is_open()) return false;

	} else {

		// open file
		fs.open(filePathStr, onlyRead ? ios::in | ios::binary | ios::ate : ios::in | ios::out | ios::binary | ios::ate);

		// read footer
		footer.read(fs, comp->getLibId());
	}

	readOnlyMode = onlyRead;
	return true;
}

//-----------------------------------------------------------------------------
// Name: close()
// Desc: Rewrites the compressed file, by copying the data from the temporary files,
//		 deletes the temporary files and clears all data in memory.
//-----------------------------------------------------------------------------
bool compressor::file::close()
{
	if (!fs.good())		return false;
	if (!fs.is_open())	return false;
	flush();
	footer.clear();
	fs.close();
	return true;
}

//-----------------------------------------------------------------------------
// Name: isOpen()
// Desc: Returns true if the file is open.
//-----------------------------------------------------------------------------
bool compressor::file::isOpen()
{
	return fs.is_open();
}

//-----------------------------------------------------------------------------
// Name: getKeys()
// Desc: Returns a list of all keys in the file.
//-----------------------------------------------------------------------------
std::vector<std::wstring> compressor::file::getKeys()
{
	vector<wstring> keys;
	if (!fs.good()) return keys;
	if (!fs.is_open()) return keys;
	for (auto& curSection : footer.sections) {					// get all keys from the footer, which are already written to the compressed file
		keys.push_back(curSection.keyName);
	}
	for (auto& curTmpFile : tmpFiles) {							// get all keys from the temporary files
		keys.push_back(curTmpFile->getKeyName());
	}
	sort(keys.begin(), keys.end());								// sort keys
	keys.erase(unique(keys.begin(), keys.end()), keys.end());	// remove duplicates
	return keys;
}

//-----------------------------------------------------------------------------
// Name: doesKeyExist()
// Desc: Returns true if a key exists in the file.
//-----------------------------------------------------------------------------
bool compressor::file::doesKeyExist(wstring const & key)
{
	if (!fs.good()) return false;
	if (!fs.is_open()) return false;
	return footer.doesKeyExist(key) || tmpFile::doesExist(key);
}

//-----------------------------------------------------------------------------
// Name: read()
// Desc: Reads data from the temporary and compressed files for one section.
//-----------------------------------------------------------------------------
bool compressor::file::read(wstring const & key, long long position, long long numBytes, void * pBytes)
{
	// checks
	if (!numBytes)	return false;					// some bytes neds to be read
	if (!comp)		return false;					// a compressor must be available
	if (!pBytes)	return false;					// data pointer must not be nullptr
	if (!fs.good()) return false;					// a file must be open
	if (!fs.is_open()) return false;				// a file must be open

	// if there is already a temporary file for the section, then read from it
	if (tmpFile::doesExist(key)) {
		return getTmpFile(key).read(position, numBytes, pBytes);
	} else {
		if (!footer.doesKeyExist(key)) return false;	// key must exist
		return readFromCompressed(key, position, numBytes, pBytes);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: getTmpFile()
// Desc: Returns the temporary file for a section.
//-----------------------------------------------------------------------------
compressor::file::tmpFile& compressor::file::getTmpFile(wstring const & key)
{
	// check if the temporary file already exists
	for (auto& curTmpFile : tmpFiles) {
		if (curTmpFile->getKeyName() == key) {
			return *curTmpFile;
		}
	}
	// create a new temporary file
	tmpFiles.push_back(new tmpFile(key));
	return *tmpFiles.back();
}

//-----------------------------------------------------------------------------
// Name: readFromCompressed()
// Desc: Reads data from the compressed files for one section.
//-----------------------------------------------------------------------------
bool compressor::file::readFromCompressed(wstring const & key, long long position, long long numBytes, void * pBytes)
{
	if (!numBytes)	return false;			// some bytes neds to be read
	if (!comp)		return false;			// a compressor must be available
	if (!pBytes)	return false;			// data pointer must not be nullptr
	if (!fs.good()) return false;			// a file must be open
	if (!fs.is_open()) return false;		// a file must be open
	
	// locals
	if (!footer.doesKeyExist(key)) return false;
	return footer.getSection(key).readData(fs, footer, *comp, pBytes, numBytes, position);
}

//-----------------------------------------------------------------------------
// Name: write()
// Desc: Write data to the compressed file, which is compressed and stored in blocks.
//		 However, the data is not written to the file, but stored in temporary files.
//		 One file for each section, allowing to write data in parallel and without fragmentation.
//-----------------------------------------------------------------------------
bool compressor::file::write(wstring const& key, long long position, long long numBytes, const void* pBytes)
{
	// check preconditions
	if (readOnlyMode) return false;													// no appending when in read only mode
	if (key.length() > maxKeyLength) return false;									// limit key length
	if (!pBytes)	return false;													// data pointer must not be nullptr
	if (!numBytes)	return false;													// some bytes neds to be written
	if (!comp)		return false;													// a compressor must be available
	if (!fs.good()) return false;													// a file must be open
	if (!fs.is_open()) return false;												// a file must be open

	// create or open a temporary file for the section
	tmpFile& curTmpFile = getTmpFile(key);

	// When the section already exist in the compressed file, and the temporary file is new, 
	// then copy data to the temporary file, so the the data can be modified.
	if (curTmpFile.getSize() == 0 && footer.doesKeyExist(key)) {
		// read data from compressed file in chucks of footer.blockSizeInBytes, and write to the temporary file
		sectionInfo& 	curSection 				= footer.getSection(key);
		long long 		numBytesResting 		= curSection.uncompressedSize;
		long long 		offsetWithinSection 	= 0;
		vector<char> 	tmpData(footer.blockSizeInBytes);
		while (numBytesResting) {
			long long numBytesToRead = std::min((long long) footer.blockSizeInBytes, numBytesResting);
			if (!readFromCompressed(key, offsetWithinSection, numBytesToRead, &tmpData[0])) {
				return false;
			}
			curTmpFile.write(0, numBytesToRead, &tmpData[0]);
			numBytesResting -= numBytesToRead;
		}
	}

	// write user data to temporary file
	return curTmpFile.write(position, numBytes, pBytes);
}

//-----------------------------------------------------------------------------
// Name: flush()
// Desc: Copies the data from the temporary files to the compressed file.
//-----------------------------------------------------------------------------
bool compressor::file::flush()
{
	// check preconditions
	if (readOnlyMode) return false;				// no appending when in read only mode
	if (!comp)		return false;				// a compressor must be available
	if (!fs.good()) return false;				// a file must be open
	if (!fs.is_open()) return false;			// a file must be open
	if (tmpFiles.size() == 0) return false;		// no data to write

	// locals
	vector<filesystem::path> 	tmpFilePaths;

	// get all temporary file paths
	for (auto& curTmpFile : tmpFiles) {
		tmpFilePaths.push_back(curTmpFile->getFilePath());
	}

	// loop over all files in the temporary directory
	for (auto& curTmpFile : tmpFiles) {

		// get key from file name and numBytes from file size
		wstring 	key 		= curTmpFile->getKeyName();
		long long	numBytes 	= curTmpFile->getSize();
		sectionInfo curSection;

		// print info
		wstringstream ss;
		ss << L"Flushing section \"" << key << L"\" with " << numBytes << L" bytes." << endl;
		comp->print(ss, 2);

		// does the section already exist?
		if (footer.doesKeyExist(key)) {
			curSection = footer.getSection(key);
		}

		// set section properties
		curSection.offsetInFile		= footer.fileInfoOffsetInFile;
		curSection.uncompressedSize	= numBytes;
		curSection.compressedSize	= 0;
		curSection.numBlocks		= (unsigned int) numBytes / footer.blockSizeInBytes + 1;
		curSection.keyLengthInBytes	= key.length() * sizeof(wchar_t);
		curSection.keyName			= key;
		curSection.sectionId		= footer.sections.size();
		curSection.blocks.resize(curSection.numBlocks);
	
		// read data from temporary file in chucks of footer.blockSizeInBytes, compress and write to the compressed file
		if (!curSection.writeData(fs, footer, *comp, *curTmpFile)) {
			return false;
		}

		// update file info
		footer.dictionary[key]				= curSection.sectionId;
		footer.sections.push_back(curSection);

		// update footer info
		footer.numSections++;
		footer.fileInfoOffsetInFile += curSection.compressedSize + curSection.numBlocks * sizeof(sectionInfo::blockInfo);
		footer.footerOffsetInFile	+= curSection.compressedSize + curSection.numBlocks * sizeof(sectionInfo::blockInfo);
	}

	// write footer
	footer.write(fs);
	   
	// delete all temporary files
	for (auto& curTmpFile : tmpFiles) {
		delete curTmpFile;
	}
	tmpFiles.clear();

	return true;
}

//-----------------------------------------------------------------------------
// Name: setBlockSize()
// Desc: By default the block size is 1000 bytes. This function allows to change the block size, but only before flushing.
//-----------------------------------------------------------------------------
bool compressor::file::setBlockSize(unsigned int newSizeInBytes)
{
	if (footer.numSections || footer.sections.size()) return false;
	footer.blockSizeInBytes = newSizeInBytes;
	return true;
}

//-----------------------------------------------------------------------------
// Name: getSizeOfUncompressedSection()
// Desc: Returns the size of the uncompressed section, but only after flushing.
//-----------------------------------------------------------------------------
long long compressor::file::getSizeOfUncompressedSection(wstring const & key)
{
	if (tmpFile::doesExist(key)) {
		return getTmpFile(key).getSize();
	}
	if (!footer.doesKeyExist(key)) return 0;
	return footer.getSection(key).uncompressedSize;
}

//-----------------------------------------------------------------------------
// Name: getSizeOfCompressedSection()
// Desc: Returns the size of the compressed section, but only after flushing.
//-----------------------------------------------------------------------------
long long compressor::file::getSizeOfCompressedSection(wstring const & key)
{
	if (!footer.doesKeyExist(key)) return 0;
	return footer.getSection(key).compressedSize;
}
#pragma endregion

#pragma region footerStruct
//-----------------------------------------------------------------------------
// Name: footerStruct::clear()
// Desc: Clears the footer.
//-----------------------------------------------------------------------------
void compressor::file::footerStruct::clear()
{
	fileInfoOffsetInFile 	= 0;
	footerOffsetInFile		= 0;
	numSections				= 0;
	sectionsOffsetInFile	= 0;
	dictionary.clear();
	sections.clear();
}

//-----------------------------------------------------------------------------
// Name: footerStruct::getSection()
// Desc: Returns the section info for a given key. If the key does not exist, then a new section is created.
//-----------------------------------------------------------------------------
compressor::file::sectionInfo& compressor::file::footerStruct::getSection(wstring const & key)
{
	auto sectionId = dictionary.find(key);
	if (sectionId == dictionary.end()) {
		sections.push_back(sectionInfo());
		sections.back().keyName = key;
		sections.back().sectionId = sections.size();
		dictionary[key] = sections.size();
		return sections.back();
	} else {
		return sections[sectionId->second];
	}
}

//-----------------------------------------------------------------------------
// Name: footerStruct::write()
// Desc: Writes the footer to the file.
//-----------------------------------------------------------------------------
bool compressor::file::footerStruct::write(std::fstream & fs)
{
	// check preconditions
	if (!fs.good()) return false;						// a database file must be open
	if (!fs.is_open()) return false;					// a database file must be open
	if (numSections == 0) return false;					// must have at least one section
	if (fileInfoOffsetInFile == 0) return false;		// file info must be present
	if (sections.size() == 0) return false;				// must have at least one section
	if (versionId != 1) return false;					// check file version 

	// set file pointer, to write infos about the sections
	fs.seekg(fileInfoOffsetInFile, ios_base::beg);

	size_t sectionId = 0;
	for (auto& curSectionInfo : sections) {
		curSectionInfo.write(fs, *this);
		sectionId++;
	}

	// set file pointer, to write the footer
	footerOffsetInFile = fs.tellg();

	// write footer
	fs.write((char*) this, 
		sizeof(typeId					) + 
		sizeof(versionId				) + 
		sizeof(numSections				) + 
		sizeof(blockSizeInBytes			) + 
		sizeof(fileInfoOffsetInFile		) +
		sizeof(footerOffsetInFile		) + 
		sizeof(sectionsOffsetInFile		) +
		sizeof(usedLib					)
	);

	return true;
}

//-----------------------------------------------------------------------------
// Name: footerStruct::read()
// Desc: Reads the footer from the file.
//-----------------------------------------------------------------------------
bool compressor::file::footerStruct::read(std::fstream & fs, generalLib::libId libId)
{
	// check preconditions
	if (!fs.good()) return false;
	if (!fs.is_open()) return false;

	// read footer, which is at the end of the file
	size_t sizeOfFooter = 
		sizeof(typeId					) + 
		sizeof(versionId				) + 
		sizeof(numSections				) + 
		sizeof(blockSizeInBytes			) + 
		sizeof(fileInfoOffsetInFile		) +
		sizeof(footerOffsetInFile		) + 
		sizeof(sectionsOffsetInFile		) +
		sizeof(usedLib					);
	fs.seekg(-1 * (int) sizeOfFooter, ios_base::end);
	fs.read((char*) this, sizeOfFooter);

	if (numSections == 0) return false;				// must have at least one section
	if (fileInfoOffsetInFile == 0) return false;	// file info must be present
	if (typeId != 0x7d67) return false; 			// does typeId match?
	if (usedLib != libId) return false;				// does libId match?
	if (versionId != 1) return false;				// check file version 

	// set file pointer, to read infos about the sections
	fs.seekg(fileInfoOffsetInFile, ios_base::beg);

	// read sections
	sections.resize(numSections);
	for (auto& curSectionInfo : sections) {
		if (!curSectionInfo.read(fs, *this)) return false;
		dictionary[curSectionInfo.keyName] = curSectionInfo.sectionId;
	}
	return true;
}
#pragma endregion

#pragma region sectionInfo
//-----------------------------------------------------------------------------
// Name: sectionInfo::write()
// Desc: Writes the section info to the file, at the current file pointer position.
//-----------------------------------------------------------------------------
bool compressor::file::sectionInfo::write(std::fstream& fs, footerStruct& footer)
{
	// check preconditions
	if (!fs.good()) return false;
	if (!fs.is_open()) return false;
	if (numBlocks == 0) return false;

	// write section info
	unsigned int dummy_unit32 = 0;
	fs.write((char*) &offsetInFile,		sizeof(offsetInFile		));
	fs.write((char*) &uncompressedSize,	sizeof(uncompressedSize	));
	fs.write((char*) &compressedSize,	sizeof(compressedSize	));
	fs.write((char*) &numBlocks,		sizeof(numBlocks		));
	fs.write((char*) &dummy_unit32,		sizeof(dummy_unit32		));		// 4 bytes were formerly used for padding
	fs.write((char*) &sectionId,		sizeof(sectionId		));
	fs.write((char*) &dummy_unit32,		sizeof(dummy_unit32		));		// 4 bytes were formerly used for padding
	fs.write((char*) &keyLengthInBytes,	sizeof(keyLengthInBytes	));
	fs.write((char*) keyName.c_str(), keyLengthInBytes);

	// blocks are already stored in each section

	return true;
}

//-----------------------------------------------------------------------------
// Name: sectionInfo::read()
// Desc: Reads the section info from the file, from the current file pointer position.
//-----------------------------------------------------------------------------
bool compressor::file::sectionInfo::read(std::fstream & fs, footerStruct& footer)
{
	// check preconditions
	if (!fs.good()) return false;
	if (!fs.is_open()) return false;

	// locals
	vector<wchar_t> keyNameTmp(maxKeyLength, L'\0');
	unsigned int dummy_unit32;

	// read section info
	fs.read((char*) &offsetInFile,		sizeof(offsetInFile		));
	fs.read((char*) &uncompressedSize,	sizeof(uncompressedSize	));
	fs.read((char*) &compressedSize,	sizeof(compressedSize	));
	fs.read((char*) &numBlocks,			sizeof(numBlocks		));
	fs.read((char*) &dummy_unit32,		sizeof(dummy_unit32		));		// 4 bytes were formerly used for padding
	fs.read((char*) &sectionId,			sizeof(sectionId		));
	fs.read((char*) &dummy_unit32,		sizeof(dummy_unit32		));		// 4 bytes were formerly used for padding
	fs.read((char*) &keyLengthInBytes,	sizeof(keyLengthInBytes	));

	// keys
	if (keyLengthInBytes > maxKeyLength) {
		fs.close();
		return false;
	}
	fs.read((char*) &keyNameTmp[0], keyLengthInBytes);
	keyNameTmp[keyLengthInBytes] = L'\0';
	keyName.assign(maxKeyLength, L'\0');
	keyName.assign(&keyNameTmp[0]);
	return true;
}

//-----------------------------------------------------------------------------
// Name: sectionInfo::writeData()
// Desc: Writes the data from tmpFile to the compressed file. IMPORTANT: The whole section must be written at once.
//-----------------------------------------------------------------------------
bool compressor::file::sectionInfo::writeData(std::fstream& fs, footerStruct& footer, generalLib& comp, tmpFile& curTmpFile, bool forceSingleThreading)
{
	// set file pointer
	fs.seekp(offsetInFile, ios_base::beg);

	// BUG: Multithreading does not work
	forceSingleThreading = true;

	// single thread compression
	if (forceSingleThreading || curTmpFile.getSize() < 1000000) {

		vector<char> 	uncompressedData;
		vector<char>	compressedData;
		long long		numBytesResting		= curTmpFile.getSize();
		unsigned int	nBytesCompressed	= 0;
		unsigned int	curOffsetInSection	= 0;
		unsigned int 	curOffsetInTmpFile	= 0;
		size_t			blockId				= 0;
		
		compressedData.resize(comp.estimateMaxSizeOfCompressedData(footer.blockSizeInBytes));
		uncompressedData.resize(footer.blockSizeInBytes);

		while (numBytesResting) {
			
			// locals
			unsigned int numBytesToCompress = (unsigned int) std::min((long long) footer.blockSizeInBytes, numBytesResting);
			curTmpFile.read(curOffsetInTmpFile, numBytesToCompress, &uncompressedData[0]);

			// compress data
			if (!comp.compress(&compressedData[0], &uncompressedData[0], numBytesToCompress, nBytesCompressed)) return false;
			
			// write to file
			fs.write(&compressedData[0], nBytesCompressed);

			compressedSize					+= nBytesCompressed;
			numBytesResting					-= numBytesToCompress;
			blocks[blockId].compressedSize	= nBytesCompressed;
			blocks[blockId].offsetInSection	= curOffsetInSection;
			curOffsetInSection				+= nBytesCompressed;
			curOffsetInTmpFile				+= numBytesToCompress;
			blockId++;
		}

	// use multi-threading and a buffered file for compression
	} else {
		// Multi-threaded compression with buffered file
		const size_t numThreads = std::thread::hardware_concurrency();
		size_t totalBlocks = (size_t)((curTmpFile.getSize() + footer.blockSizeInBytes - 1) / footer.blockSizeInBytes);

		vector<char> 	uncompressedData;
		vector<char>	compressedData;

		compressedData.resize(comp.estimateMaxSizeOfCompressedData(footer.blockSizeInBytes));
		uncompressedData.resize(footer.blockSizeInBytes);

		std::vector<std::future<std::pair<std::vector<char>, unsigned int>>> futures;
		std::vector<std::vector<char>> compressedBlocks(totalBlocks);
		std::vector<unsigned int> compressedSizes(totalBlocks, 0);

		for (size_t blockId = 0; blockId < totalBlocks; ++blockId) {
			futures.push_back(std::async(std::launch::async, [&, blockId]() {
				std::vector<char> blockUncompressed(footer.blockSizeInBytes);
				std::vector<char> blockCompressed(comp.estimateMaxSizeOfCompressedData(footer.blockSizeInBytes));
				unsigned int nBytesCompressed = 0;
				long long offset = blockId * footer.blockSizeInBytes;
				unsigned int numBytesToCompress = (unsigned int)std::min((long long)footer.blockSizeInBytes, curTmpFile.getSize() - offset);
				curTmpFile.read(offset, numBytesToCompress, &blockUncompressed[0]);
				if (!comp.compress(&blockCompressed[0], &blockUncompressed[0], numBytesToCompress, nBytesCompressed)) {
					throw std::runtime_error("Compression failed in thread");
				}
				blockCompressed.resize(nBytesCompressed);
				return std::make_pair(std::move(blockCompressed), nBytesCompressed);
			}));
			// Limit number of concurrent threads
			if (futures.size() >= numThreads) {
				for (auto& fut : futures) {
					auto result = fut.get();
					size_t idx = &fut - &futures[0] + blockId - futures.size() + 1;
					compressedBlocks[idx] = std::move(result.first);
					compressedSizes[idx] = result.second;
				}
				futures.clear();
			}
		}

		// Collect remaining futures
		size_t startIdx = totalBlocks - futures.size();
		for (size_t i = 0; i < futures.size(); ++i) {
			auto result = futures[i].get();
			compressedBlocks[startIdx + i] = std::move(result.first);
			compressedSizes[startIdx + i] = result.second;
		}

		// Write all compressed blocks to file and update block info
		unsigned int curOffsetInSection = 0;
		compressedSize = 0;
		for (size_t blockId = 0; blockId < totalBlocks; ++blockId) {
			fs.write(compressedBlocks[blockId].data(), compressedSizes[blockId]);
			blocks[blockId].compressedSize = compressedSizes[blockId];
			blocks[blockId].offsetInSection = curOffsetInSection;
			curOffsetInSection += compressedSizes[blockId];
			compressedSize += compressedSizes[blockId];
		}
	}

	// write all block infos to file
	for (auto& curBlock : blocks) {
		fs.write((char*) &curBlock, sizeof(curBlock));
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: sectionInfo::readData()
// Desc: Reads the data from the file. This may happen as random access.
//-----------------------------------------------------------------------------
bool compressor::file::sectionInfo::readData(std::fstream& fs, footerStruct& footer, generalLib& comp, void* pBytes, long long numBytes, long long position)
{
	// check preconditions
	if (fs.good() == false) return false;
	if (fs.is_open() == false) return false;
	if (pBytes == nullptr) return false;
	if (numBytes <= 0) return false;
	if (position < 0) return false;
	if (position >= uncompressedSize) return false;
	if (position > numBlocks * footer.blockSizeInBytes) return false;

	// locals
	long long			blockId				= position / footer.blockSizeInBytes;			// block index
	long long			offsetInsideBlock	= position %  footer.blockSizeInBytes;			// offset inside block
	long long			numBytesResting		= numBytes;										// number of bytes still to read and to copy to pBytes
	char*				pBlock				= (char*) pBytes;								// moving pointer to the current position in pBytes during the copy process
	vector<char>		compressedData;														// buffer for compressed data
	vector<char>		uncompressedData;													// buffer for uncompressed data
	long long			numRestingCompBytes	= compressedSize;								// number of bytes still available in the compressed section
	unsigned int		nBytesDecompressed	= 0;											// number of bytes decompressed in the current block
	
	// is block id valid?
	if (blockId >= numBlocks) return false;

	// seek to beginning of section
	compressedData	.resize(comp.estimateMaxSizeOfCompressedData(footer.blockSizeInBytes));	
	uncompressedData.resize(footer.blockSizeInBytes);

	// if blockInfo not loaded yet than do it now
	if (blocks.size() == 0) {
		fs.seekg(offsetInFile + compressedSize, ios_base::beg);
		blocks.resize(numBlocks);
		for (auto& curBlock : blocks) {
			fs.read((char*) &curBlock, sizeof(curBlock));
		}
	}

	// seekg to relevant block
	fs.seekg(offsetInFile, ios_base::beg);
	fs.seekg(blocks[blockId].offsetInSection, ios_base::cur);
	numRestingCompBytes -= blocks[blockId].offsetInSection;									// update numRestingCompBytes, because we have seeked to the block

	// loop through all remaining blocks
	// only single threading here
	while (numBytesResting) {

		// is block id valid?
		if (blockId >= numBlocks) return false;

		// read current block from compresed file
		fs.read(&compressedData[0], blocks[blockId].compressedSize);
		
		// decompress data
		if (!comp.decompress(&uncompressedData[0], &compressedData[0], blocks[blockId].compressedSize, nBytesDecompressed)) return false;

		// copy data to passed pointer from caller
		long long numBytesToCopy = std::min({(long long) nBytesDecompressed, numBytesResting, (long long) footer.blockSizeInBytes - offsetInsideBlock});
		memcpy(pBlock, &uncompressedData[offsetInsideBlock], numBytesToCopy);

		// goto next block
		numBytesResting		-= numBytesToCopy;
		numRestingCompBytes -= std::min((long long) blocks[blockId].compressedSize, numRestingCompBytes);
		pBlock				+= numBytesToCopy;
		offsetInsideBlock	 = 0;
		blockId++;
	}

	return true;
}
#pragma endregion

#pragma region tmpFile
//-----------------------------------------------------------------------------
// Name: tmpFile()
// Desc: Constructor
//-----------------------------------------------------------------------------
compressor::file::tmpFile::tmpFile(wstring const & keyName) : 
	keyName{keyName}
{
	filePath = (filesystem::temp_directory_path() / L"compressor" / (keyName + L".dat")).c_str();
	std::filesystem::path filePathStr = filePath;
	std::filesystem::create_directories(filePathStr.parent_path());
	fsTmp.open(filePathStr, ios::in | ios::out | ios::binary | ios::trunc);
	if (!fsTmp.good() || !fsTmp.is_open()) {
		throw std::runtime_error("Could not create temporary file for section.");
	}
	fsTmp.close();
}

//-----------------------------------------------------------------------------
// Name: ~tmpFile()
// Desc: Destructor
//-----------------------------------------------------------------------------
compressor::file::tmpFile::~tmpFile()
{
	if (fsTmp.is_open()) fsTmp.close();
	filesystem::remove(filePath);
}

//-----------------------------------------------------------------------------
// Name: doesExist()
// Desc: Returns true if a temporary file exists for a section.
//-----------------------------------------------------------------------------
bool compressor::file::tmpFile::doesExist(wstring const & keyName)
{
	std::filesystem::path tmpFilePath = (std::filesystem::temp_directory_path() / L"compressor" / (keyName + L".dat")).c_str();
	return std::filesystem::exists(tmpFilePath);
}

//-----------------------------------------------------------------------------
// Name: openIfNotOpen()
// Desc: Opens the temporary file if not already open.
//-----------------------------------------------------------------------------
bool compressor::file::tmpFile::openIfNotOpen()
{
    if (!fsTmp.good() || !fsTmp.is_open()) {
		std::filesystem::path filePathStr = filePath;
		fsTmp.open(filePathStr.string(), ios::in | ios::out | ios::binary);
        if (!fsTmp.good() || !fsTmp.is_open()) {
			return false;
		}
    }
    return true;
}

//-----------------------------------------------------------------------------
// Name: read()
// Desc: Reads data from the temporary file.
//-----------------------------------------------------------------------------
bool compressor::file::tmpFile::read(long long position, long long numBytes, void* pBytes)
{
	if (!pBytes) return false;
	if (position < 0) return false;
	if (numBytes <= 0) return false;
	if (position + numBytes > getSize()) return false;
	if (!openIfNotOpen()) return false;
	fsTmp.seekg(position, ios_base::beg);
	fsTmp.read((char*) pBytes, numBytes);
	fsTmp.close();
	return true;
}

//-----------------------------------------------------------------------------
// Name: write()
// Desc: Writes data to the temporary file.
//-----------------------------------------------------------------------------
bool compressor::file::tmpFile::write(long long position, long long numBytes, const void* pBytes)
{
	if (!pBytes) return false;
	if (position < 0) return false;
	if (numBytes <= 0) return false;
	if (!openIfNotOpen()) return false;
	fsTmp.seekp(position, ios_base::beg);
	fsTmp.write((char*) pBytes, numBytes);
	fsTmp.close();
	return true;
}

//-----------------------------------------------------------------------------
// Name: getSize()
// Desc: Returns the size of the temporary file.
//-----------------------------------------------------------------------------
long long compressor::file::tmpFile::getSize()
{
	if (!openIfNotOpen()) return -1;
	fsTmp.seekg(0, ios_base::end);
	long long size = fsTmp.tellg();
	fsTmp.close();
	return size;
}

#pragma endregion