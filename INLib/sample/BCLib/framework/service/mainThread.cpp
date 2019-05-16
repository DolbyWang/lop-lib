//////////////////////////////////////////////////////////////////////
//  created:    2012/08/01
//  filename:   BCLib/framework/netfw/server/mainThread.cpp
//  author:     aaron
/// @brief
///
//////////////////////////////////////////////////////////////////////
#include <BCLib/utility/logFile.h>
#include "mainThread.h"

BCLIB_SINGLETON_DEFINE(CMainThread);

CMainThread::CMainThread()
{
    ;
}

CMainThread::~CMainThread()
{
    ;
}

bool CMainThread::_init()
{
    if(!m_netServer.init())
    {
        BCLIB_LOG_ERROR(BCLib::ELOGMODULE_DEFAULT, "����NetServerʧ��");
        return false;
    }
    m_netServer.setStubPool(BCLib::Network::NETT_TCP, 1024, 4, 128);
    if(!m_netServer.addNetAcceptor(BCLib::Network::NETT_TCP, "192.168.1.118", 9500))
    {
        BCLIB_LOG_ERROR(BCLib::ELOGMODULE_DEFAULT, "���ü����˿�ʧ��");
        return false;
    }
    if(!BCLib::Framework::CMainThread::_init())
    {
        BCLIB_LOG_ERROR(BCLib::ELOGMODULE_DEFAULT, "��ʼ�� ʧ��");
        return false;
    }
    

    

    

    BCLIB_LOG_INFOR(BCLib::ELOGMODULE_DEFAULT, "����NetServer�ɹ�");

    return true;
}

int CMainThread::_final()
{
    m_netServer.final();

    return BCLib::Framework::CMainThread::_final();
}