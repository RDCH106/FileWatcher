#pragma once

#include "stdafx.h"
#include "NotifyDirCheck.h"
#include <StringFuncs.h>

#include <iostream>

class FileWatcher // Implements Singleton pattern
{
public:
	static FileWatcher* Instance();

	bool runFileWatcher(void);
	void stopFileWatcher(void);
	CString getDirectory(void);
	void setDirectory(CString dir);
	std::string getFilter(void);
	void setFilter(std::string filter);
	int getThreadFrequency(void);
	void setThreadFrequency(int tfreq);
	void setActionCallback(NOTIFICATION_CALLBACK_PTR pCallback);

protected:

	FileWatcher(void);
	~FileWatcher(void);
	FileWatcher(const FileWatcher & ) ;
	FileWatcher &operator= (const FileWatcher & ) ;	

private:

	static FileWatcher* pinstance;
	CNotifyDirCheck m_ndc;
	std::string m_filter;
	int m_tfreq;

};

//Callback Function Example
//
//UINT DirCallback(...) {...}
//...
//FileWatcher *fw = FileWatcher::Instance();
//fw->setActionCallback(DirCallback);

//Full Example of Callback (Default)
/*
UINT DirCallback( CFileInformation fiObject, EFileAction faAction, LPVOID lpData )
	{
		CString           csBuffer;
		CString           csFile = fiObject.GetFilePath();

		char * cextensions = reinterpret_cast<char *>(lpData);
		std::string extensions (cextensions);
		std::vector<std::string> vExtensions = RlibC_String::StringFuncs::split(cextensions, ";");
		for(int i=0; i<vExtensions.size(); i++){
			vExtensions[i] = RlibC_String::StringFuncs::toLowerCase(vExtensions[i]); 
		}

		USES_CONVERSION;

		std::vector<std::string> v = RlibC_String::StringFuncs::split(OLE2A(csFile), ".");
		std::string fileExtension = v.at(v.size()-1);
		fileExtension = RlibC_String::StringFuncs::toLowerCase(fileExtension);

		bool extFound = false;
		for(int i=0; i< vExtensions.size(); i++){

			if(vExtensions.at(i).compare(fileExtension) == 0){
				
				extFound = TRUE;
				break;
			}
		}

		if(extFound){

			if( IS_CREATE_FILE( faAction ) )
			{
				csBuffer.Format( L"Created %s", csFile );
			}
			else if( IS_DELETE_FILE( faAction ) )
			{
				csBuffer.Format( L"Deleted %s", csFile );
			}
			else if( IS_CHANGE_FILE( faAction ) )
			{
				csBuffer.Format( L"Changed %s", csFile );
			}

		}		
		else
		{
			csBuffer.Format( L"Nothing to do %s", csFile );
			//return 1; //error, stop thread
		}

		std::cout << OLE2A(csBuffer) << std::endl;

		return 0; //success
	}
*/

