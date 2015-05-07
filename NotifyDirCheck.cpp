// NotifyDirCheck.cpp: implementation of the CNotifyDirCheck class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NotifyDirCheck.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Simple Notification Callback
//////////////////////////////////////////////////////////////////////

UINT DefaultNotificationCallback( CFileInformation fiObject, EFileAction faAction, LPVOID lpData )
{
	CString csBuffer;
	CString csFile = fiObject.GetFilePath();

	if( IS_CREATE_FILE( faAction ) )
	{
		csBuffer.Format( L"Created %s", csFile );
		AfxMessageBox( csBuffer );
	}
	else if( IS_DELETE_FILE( faAction ) )
	{
		csBuffer.Format( L"Deleted %s", csFile );
		AfxMessageBox( csBuffer );
	}
	else if( IS_CHANGE_FILE( faAction ) )
	{
		csBuffer.Format( L"Changed %s", csFile );
		AfxMessageBox( csBuffer );
	}
	else
		return 1; //error, stop thread

	return 0; //success
}

//////////////////////////////////////////////////////////////////////
// Show Error Message Box
//////////////////////////////////////////////////////////////////////

static void ErrorMessage( CString failedSource )
{
	LPVOID  lpMsgBuf;
	CString csError;
	DWORD   dwLastError = GetLastError();//error number

	//make error comments
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER| 
				   FORMAT_MESSAGE_FROM_SYSTEM| 
				   FORMAT_MESSAGE_IGNORE_INSERTS,
				   NULL,
				   dwLastError,
				   MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),// Default language
				   (LPTSTR)&lpMsgBuf,
				   0,
				   NULL );

	csError.Format( L"%s Error N%d\n%s", failedSource, dwLastError, (LPCTSTR)lpMsgBuf );
	AfxMessageBox( csError, MB_OK|MB_ICONSTOP );

	// Free the buffer.
	LocalFree( lpMsgBuf );
}
	
//////////////////////////////////////////////////////////////////////
// Work Thread 
//////////////////////////////////////////////////////////////////////

UINT NotifyDirThread( LPVOID pParam )
{
	BOOL             bStop = FALSE;
	HANDLE           hDir  = NULL; 
	CNotifyDirCheck* pNDC  = (CNotifyDirCheck*)pParam;
	FI_List          newFIL,
		             oldFIL;
	EFileAction      faAction;
	CFileInformation fi;

	if( pNDC == NULL )
		return 0;

	hDir = FindFirstChangeNotification( pNDC->GetDirectory(),
										TRUE,
										FILE_NOTIFY_CHANGE_FILE_NAME  |
										FILE_NOTIFY_CHANGE_DIR_NAME   |
										FILE_NOTIFY_CHANGE_SIZE       |
										FILE_NOTIFY_CHANGE_LAST_WRITE |
										FILE_NOTIFY_CHANGE_ATTRIBUTES );

	if( hDir == INVALID_HANDLE_VALUE )
	{
		ErrorMessage( _T("FindFirstChangeNotification") );
		return 0;
	}
	
 	while( pNDC->IsRun() )
	{
		CFileInformation::RemoveFiles( &oldFIL );
		CFileInformation::EnumFiles( pNDC->GetDirectory(), &oldFIL );

		bStop = FALSE;

		while( WaitForSingleObject( hDir, WAIT_TIMEOUT ) != WAIT_OBJECT_0 )
		{	if( !pNDC->IsRun() )
			{
				bStop = TRUE;//to end
				break;
			}
		}
		if( bStop )
			break;//to end

		CFileInformation::RemoveFiles( &newFIL );
		CFileInformation::EnumFiles( pNDC->GetDirectory(), &newFIL );
		
		Sleep( pNDC->GetThreadFrequency() );

		faAction = CFileInformation::CompareFiles( &oldFIL, &newFIL, fi );

		if( !IS_NOTACT_FILE( faAction ) )
		{
			NOTIFICATION_CALLBACK_PTR ncpAction = pNDC->GetActionCallback();

			if( ncpAction )	//call user's callback
				bStop = ( ncpAction( fi, faAction, pNDC->GetData() ) > 0 );
			else			//call user's virtual function
				bStop = ( pNDC->Action( fi, faAction ) > 0 );

			if( bStop )
				break;//to end
		}
		
		if( FindNextChangeNotification( hDir ) == 0 )
		{
			ErrorMessage( _T("FindNextChangeNotification") );
			return 0;
		}
	}

	//end point of notification thread
	CFileInformation::RemoveFiles( &newFIL );
	CFileInformation::RemoveFiles( &oldFIL );
	
	return FindCloseChangeNotification( hDir );
}

//////////////////////////////////////////////////////////////////////
// Class 
//////////////////////////////////////////////////////////////////////

CNotifyDirCheck::CNotifyDirCheck()
{
	SetDirectory( "" );
	SetActionCallback( NULL );
	SetData( NULL );
	SetStop();
	m_pThread = NULL;
	m_dwThreadFrequency = WAIT_TIMEOUT;
}

CNotifyDirCheck::CNotifyDirCheck(  CString csDir, NOTIFICATION_CALLBACK_PTR ncpAction, LPVOID lpData, DWORD m_dwThreadFrequency )
{
	SetDirectory( csDir );
	SetActionCallback( ncpAction );
	SetData( lpData );
	SetStop();
	m_pThread = NULL;
	SetThreadFrequency( m_dwThreadFrequency );
}

CNotifyDirCheck::~CNotifyDirCheck()
{
	Stop();
}

BOOL CNotifyDirCheck::Run()
{
	if( IsRun() || m_pThread != NULL || m_csDir.IsEmpty() )
		return FALSE;

	SetRun();
	m_pThread = AfxBeginThread( NotifyDirThread, this );
	
	if( m_pThread == NULL )
		SetStop();

	return IsRun();
}

void CNotifyDirCheck::Stop()
{
	if( !IsRun() || m_pThread == NULL )
		return;
	
	SetStop();

	WaitForSingleObject( m_pThread->m_hThread, 2 * NOTIFICATION_TIMEOUT );
	m_pThread = NULL;
}

UINT CNotifyDirCheck::Action( CFileInformation fiObject, EFileAction faAction )
{
	CString csBuffer;
	CString csFile = fiObject.GetFilePath();

	if( IS_CREATE_FILE( faAction ) )
	{
		csBuffer.Format( L"Created %s", csFile );
		AfxMessageBox( csBuffer );
	}
	else if( IS_DELETE_FILE( faAction ) )
	{
		csBuffer.Format( L"Deleted %s", csFile );
		AfxMessageBox( csBuffer );
	}
	else if( IS_CHANGE_FILE( faAction ) )
	{
		csBuffer.Format( L"Changed %s", csFile );
		AfxMessageBox( csBuffer );
	}
	else
		return 1; //error, stop thread

	return 0; //success
}
