//////////////////////////////////////////////////////////////////////
//  created:    2011/11/01
//  filename:   BCLib/luaScript/toluapp/recorder.h
//  author:     League of Perfect
/// @brief      Ϊ����ʾ��tolua++ת����c++��д��һ���򵥵�¼������
///             record() ��¼string
///             play()   ����string
///
//////////////////////////////////////////////////////////////////////
#ifndef __BCLIB_SAMPLE_LUASCRIPT_TOLUAPP_RECORDER_H__
#define __BCLIB_SAMPLE_LUASCRIPT_TOLUAPP_RECORDER_H__

#include <string>

class CRecorder
{
public:
    CRecorder()
    {
    }
    ~CRecorder()
    {
    }

    void play();
    void record(const char* content);

private:
    std::string m_content;
};


#endif//__BCLIB_SAMPLE_LUASCRIPT_TOLUAPP_RECORDER_H__
