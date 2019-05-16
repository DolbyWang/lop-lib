//////////////////////////////////////////////////////////////////////
//  created:    2011/11/01
//  filename:   BCLib/security/main.cpp
//  author:     League of Perfect
/// @brief
///
//////////////////////////////////////////////////////////////////////
#include <BCLib/security/security.h>
#include <stdio.h>

void PrintBinary(const char* buff, BCLib::uint32 buflen)
{
    for(BCLib::uint32 i = 0; i < buflen; i++)
    {
        printf("%02X ", (unsigned char)(buff[i]));
    }
}

int main()
{
    char srcbuf[] = "Microsoft Visual Studio Team System 2008";
    BCLib::uint32 srclen = strlen(srcbuf);
    char dstbuf[200];
    BCLib::uint32 dstlen = 200;
    char finalbuf[200];
    BCLib::uint32 finallen = 200;

    char key[] = "hello world!";
    BCLib::uint32 keylen = sizeof(key);

    dstlen = 200;
    finallen = 200;
    memset(dstbuf, 0, 200);
    memset(finalbuf, 0, 200);
    printf("MD5�㷨:\n");
    BCLib::Security::CSecurity* ps1 = createSecurity(BCLib::Security::EST_MD5);
    printf("����: %s\n", srcbuf);
    printf("���ĳ���: %d\n", srclen);
    ps1->encode(srcbuf, srclen, dstbuf, dstlen);
    printf("����: ");
    PrintBinary(dstbuf, dstlen);
    printf("\n");
    printf("���ĳ���: %d\n", dstlen);
    printf("MD5��һ�������㷨���޷�����\n");

    printf("\n\n");

    dstlen = 200;
    finallen = 200;
    memset(dstbuf, 0, 200);
    memset(finalbuf, 0, 200);
    printf("SHA1�㷨:\n");
    BCLib::Security::CSecurity* ps4 = createSecurity(BCLib::Security::EST_SHA1);
    printf("����: %s\n", srcbuf);
    printf("���ĳ���: %d\n", srclen);
    ps4->encode(srcbuf, srclen, dstbuf, dstlen);
    printf("����: ");
    PrintBinary(dstbuf, dstlen);
    printf("\n");
    printf("���ĳ���: %d\n", dstlen);
    printf("SHA1��һ�������㷨���޷�����\n");

    printf("\n\n");

    dstlen = 200;
    finallen = 200;
    memset(dstbuf, 0, 200);
    memset(finalbuf, 0, 200);
    printf("RC4�㷨:\n");
    BCLib::Security::CSecurity* ps2 = createSecurity(BCLib::Security::EST_RC4);
    ps2->setKey(key, keylen);
    printf("����: %s\n", srcbuf);
    printf("���ĳ���: %d\n", srclen);
    ps2->encode(srcbuf, srclen, dstbuf, dstlen);
    printf("����: ");
    PrintBinary(dstbuf, dstlen);
    printf("\n");
    printf("���ĳ���: %d\n", dstlen);
    ps2->decode(dstbuf, dstlen, finalbuf, finallen);
    printf("����: %s\n", finalbuf);
    printf("���ܺ󳤶�: %d\n", finallen);

    printf("\n\n");

    dstlen = 200;
    finallen = 200;
    memset(dstbuf, 0, 200);
    memset(finalbuf, 0, 200);
    printf("AES�㷨:\n");
    BCLib::Security::CSecurity* ps3 = createSecurity(BCLib::Security::EST_AES);
    ps3->setKey(key, keylen);
    printf("����: %s\n", srcbuf);
    printf("���ĳ���: %d\n", srclen);
    ps3->encode(srcbuf, srclen, dstbuf, dstlen);
    printf("����: ");
    PrintBinary(dstbuf, dstlen);
    printf("\n");
    printf("���ĳ���: %d\n", dstlen);
    ps3->decode(dstbuf, dstlen, finalbuf, finallen);
    printf("����: %s\n", finalbuf);
    printf("���ܺ󳤶�: %d\n", finallen);

    printf("\n\n");

    DWORD starttime;

    printf("����MD5����100000��: ");
    starttime = GetTickCount();
    for(int i = 0; i < 100000; i++)
    {
        dstlen = 200;
        ps1->encode(srcbuf, srclen, dstbuf, dstlen);
    }
    printf("��ʱ %d ms\n", GetTickCount() - starttime);

    printf("����SHA1����100000��: ");
    starttime = GetTickCount();
    for(int i = 0; i < 100000; i++)
    {
        dstlen = 200;
        ps4->encode(srcbuf, srclen, dstbuf, dstlen);
    }
    printf("��ʱ %d ms\n", GetTickCount() - starttime);

    printf("����RC4����100000��: ");
    starttime = GetTickCount();
    for(int i = 0; i < 100000; i++)
    {
        dstlen = 200;
        ps2->encode(srcbuf, srclen, dstbuf, dstlen);
    }
    printf("��ʱ %d ms\n", GetTickCount() - starttime);

    printf("����AES����100000��: ");
    starttime = GetTickCount();
    for(int i = 0; i < 100000; i++)
    {
        dstlen = 200;
        ps3->encode(srcbuf, srclen, dstbuf, dstlen);
    }
    printf("��ʱ %d ms\n", GetTickCount() - starttime);

    destroySecurity(ps1);
    destroySecurity(ps2);
    destroySecurity(ps3);
    destroySecurity(ps4);

    system("pause");

    return 0;
}