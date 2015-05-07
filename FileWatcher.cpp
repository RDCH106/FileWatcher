#include "FileWatcher.h"

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


FileWatcher* FileWatcher::pinstance = 0;// Initialize pointer
FileWatcher* FileWatcher::Instance (){

  if (pinstance == 0)  // Detect if it's the first call
  {
    pinstance = new FileWatcher(); // Create instance
  }
  return pinstance; // Return the instance address
}

FileWatcher::FileWatcher(void)
{
	m_ndc.SetActionCallback( DirCallback );
}

FileWatcher::~FileWatcher(void)
{
	delete pinstance;
}

bool FileWatcher::runFileWatcher(){
	return m_ndc.Run();
}

void FileWatcher::stopFileWatcher(){
	m_ndc.Stop();
}

CString FileWatcher::getDirectory(){
	return m_ndc.GetDirectory();
}

void FileWatcher::setDirectory(CString dir){
	m_ndc.SetDirectory(dir);
}

std::string FileWatcher::getFilter(){
	return m_filter;
}

void FileWatcher::setFilter(std::string filter){
	m_filter = filter;
	m_ndc.SetData( (LPVOID)m_filter.c_str() );	
}

int FileWatcher::getThreadFrequency(){
	return m_tfreq;
}

void FileWatcher::setThreadFrequency(int tfreq){
	m_tfreq = tfreq;
	m_ndc.SetThreadFrequency(m_tfreq);
}

void FileWatcher::setActionCallback(NOTIFICATION_CALLBACK_PTR pCallback){
	m_ndc.SetActionCallback(pCallback);
}

//Example of CNotifyDirCkeck usage
int main( int argc, const char* argv[] )
{

	bool singleton = true;

	if(singleton){

		FileWatcher *fw = FileWatcher::Instance();
		fw->setDirectory("FolderSync");
		fw->setFilter("ply;txt");
		fw->setThreadFrequency(10);
		fw->setActionCallback(DirCallback);
		FileWatcher *fw2 = FileWatcher::Instance();
		fw2->runFileWatcher();
		while(true){}

		return 0;

	}else{

		CNotifyDirCheck m_ndc;

		m_ndc.SetDirectory( "FolderSync" );//root directory
		m_ndc.SetData( "ply;txt" );//user’s data
		m_ndc.SetActionCallback( DirCallback );//user’s callback
		m_ndc.SetThreadFrequency(10);//user’s thread frequecy (millisecods)

		m_ndc.Run();//start workthread
		//m_ndc.Stop();//stop workthread

		while(true){}

	}

}