#ifndef _LOCK_
#define _LOCK_

#include <Windows.h>


class CCriticalSection
{
public:
	CCriticalSection()
	{
		::InitializeCriticalSection( &m_cs );
	}

	~CCriticalSection()
	{
		::LeaveCriticalSection( &m_cs );
	}

	void Enter()
	{
		::EnterCriticalSection( &m_cs );
	}

	void Leave()
	{
		::LeaveCriticalSection( &m_cs );
	}

protected:
	CRITICAL_SECTION m_cs;
};

#endif