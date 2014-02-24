// Copyright (C) 2011 Seth A. Robinson
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
#include "PlatformPrecomp.h"
#include "CProtonReader.h"
#include "FileSystem/FileSystemZip.h"

#include "os.h"



#ifdef __IRR_COMPILE_WITH_PROTON_ARCHIVE_LOADER_

#include "CFileList.h"
#include "CReadFile.h"
#include "coreutil.h"

#include "IrrCompileConfig.h"
#include "Irrlicht/IrrlichtManager.h"

namespace irr
{
namespace io
{


// -----------------------------------------------------------------------------
// Proton loader
// -----------------------------------------------------------------------------

//! Constructor
CArchiveLoaderProton::CArchiveLoaderProton(io::IFileSystem* fs)
: FileSystem(fs)
{
	#ifdef _DEBUG
	setDebugName("CArchiveLoaderProton");
	#endif
}

//! returns true if the file maybe is able to be loaded by this class
bool CArchiveLoaderProton::isALoadableFileFormat(const io::path& filename) const
{
	return true;
}

//! Check to see if the loader can create archives of this type.
bool CArchiveLoaderProton::isALoadableFileFormat(E_FILE_ARCHIVE_TYPE fileType) const
{
	return fileType == EFAT_PROTON;
}


//! Creates an archive from the filename
/** \param file File handle to check.
\return Pointer to newly created archive, or 0 upon error. */
IFileArchive* CArchiveLoaderProton::createArchive(const io::path& filename, bool ignoreCase, bool ignorePaths) const
{
	assert("!Huh?");
	return 0;

	/*
	IFileArchive *archive = 0;
	io::IReadFile* file = FileSystem->createAndOpenFile(filename);

	if (file)
	{
		archive = createArchive(file, ignoreCase, ignorePaths);
		file->drop();
	}

	return archive;
	*/
}

//! creates/loads an archive from the file.
//! \return Pointer to the created archive. Returns 0 if loading failed.
IFileArchive* CArchiveLoaderProton::createArchive(io::IReadFile* file, bool ignoreCase, bool ignorePaths) const
{
	IFileArchive *archive = 0;
	archive = new CProtonReader(file, ignoreCase, ignorePaths);
	return archive;
}

//! Check if the file might be loaded by this class
/** Check might look into the file.
\param file File handle to check.
\return True if file seems to be loadable. */
bool CArchiveLoaderProton::isALoadableFileFormat(io::IReadFile* file) const
{
	return false;
}

// -----------------------------------------------------------------------------
// Proton archive
// -----------------------------------------------------------------------------

CProtonReader::CProtonReader(IReadFile* file, bool ignoreCase, bool ignorePaths)
 : CFileList((file ? file->getFileName() : io::path("")), ignoreCase, ignorePaths), File(file)
{
	#ifdef _DEBUG
	setDebugName("CProtonReader");
	#endif

}

CProtonReader::~CProtonReader()
{
	
}


//! get the archive type
E_FILE_ARCHIVE_TYPE CProtonReader::getType() const
{
	return EFAT_PROTON;
}

const IFileList* CProtonReader::getFileList() const
{
	return this;
}

//! opens a file by file name
IReadFile* CProtonReader::createAndOpenFile(const io::path& filename)
{
	int				size,apk_size;
	int				pos_slash, pos_dot;
	unsigned int	decompressedSize;
	//std::string	workingDir;
	std::string		newFileName;
	std::string		load_zip;
	std::string		load_data;
	FileSystemZip*	pfilesystem;
	byte*			pDecompressedData;
	byte*			apk_buffer;
	
#ifdef _DEBUG
    
  //  LogMsg("CProtonReader: going open %s", filename.c_str());
#endif
    
	byte* pBytes = FileManager::GetFileManager()->Get( filename.c_str(), &size, false);

	if (!pBytes)
	{
		/*workingDir = IrrlichtManager::GetIrrlichtManager()->GetDevice()->getFileSystem()->getWorkingDirectoryChange().c_str();
		if (!workingDir.empty())
		{
            //try again with full path
            newFileName = workingDir+"/"+string(filename.c_str());
			
			//LogMsg("CProtonReader: Trying with WD %s", newFileName.c_str());
            pBytes = FileManager::GetFileManager()->Get( newFileName, &size, true);
		}*/

		newFileName	= std::string(filename.c_str());
	
		pos_dot		= newFileName.rfind(".");
		if( pos_dot > 0 )
			load_zip	= std::string(newFileName.begin(),newFileName.begin()+pos_dot)+".zip";
		
		pos_slash	= newFileName.rfind("/");
		if( pos_slash > 0 )
			load_data	= std::string(newFileName.begin()+pos_slash+1,newFileName.end());

		if(  pos_dot > 0 && pos_slash > 0 )
		{
#ifdef ANDROID_NDK
			apk_buffer	= FileManager::GetFileManager()->Get(load_zip.c_str(), &apk_size, false, false);
			pfilesystem	= new FileSystemZip();
			pfilesystem->Init_unzMemory(apk_buffer, apk_size);
			pBytes = pfilesystem->Get_unz(load_data, &size);
#else
			pfilesystem = new FileSystemZip();
			pfilesystem->Init_unz(load_zip.c_str());
			pBytes = pfilesystem->Get_unz(load_data.c_str(), &size);
#endif
			delete pfilesystem;
		}
	}
	
	if (pBytes)
	{
		if (IsAPackedFile(pBytes)) //applicable to rttex files
		{
			//let's decompress it to memory before passing it back
			pDecompressedData	= DecompressRTPackToMemory(pBytes, &decompressedSize);
			size				= decompressedSize;
			delete pBytes; //done with the original
			
			pBytes = pDecompressedData;
		}
		
		return io::createMemoryReadFile(pBytes, size, filename, true);
	} 
	else
	{
#ifdef _DEBUG
		//LogMsg("Proton Irrlicht Filesystem: Was unable to locate the file %s", filename.c_str());
#endif
	}

	return 0;
}

irr::s32 CProtonReader::findFile( const io::path& filename, bool isFolder ) const
{
	
#ifdef _DEBUG
    
   // LogMsg("CProtonReader::findFile: going to search for %s", filename.c_str());
#endif
    
	if (FileManager::GetFileManager()->FileExists(filename.c_str(), false)) return 0; //dummy ID to signal we found it

	//try another way too
	std::string workingDir = IrrlichtManager::GetIrrlichtManager()->GetDevice()->getFileSystem()->getWorkingDirectoryChange().c_str();
	//try again with full path
	string newFileName = workingDir+"/"+string(filename.c_str());
	if (FileManager::GetFileManager()->FileExists(newFileName, false)) return 0; //dummy ID to signal we found it


	return -1; //can't find it
}
IReadFile* io::CProtonReader::createAndOpenFile( u32 index )
{
	assert(!"this isn't really used, right?");
	return 0;
}

} // end namespace io
} // end namespace irr

#endif // __IRR_COMPILE_WITH_PROTON_ARCHIVE_LOADER_
