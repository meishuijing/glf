#ifndef __PACKETPARSER_H_2009_0902__
#define __PACKETPARSER_H_2009_0902__

#define PKG_MARK    0xAA55

#pragma pack(push, 1)
typedef struct tagPacketHeader
{
    UINT16	wCheckSum;  //У����
    UINT16	wMark;      //��ͷ��ʾ
    UINT16	wDataLen;   //���ݰ�����
}SPacketHeader;
#pragma  pack(pop)

class FxPacketParser
{
public:
    FxPacketParser(void){};
    virtual ~FxPacketParser(void){};

    inline int ParsePacket(const char* pBuf, UINT32 dwLen)
    {
        if(dwLen < sizeof(SPacketHeader))
            return 0;

        SPacketHeader* pstHeader = (SPacketHeader *)pBuf;
        INT32 iPkgLen = __CheckPkgHeader(pstHeader);

        if(iPkgLen < 0)
        {
            return -1;
        }
        
        return iPkgLen;
    }

    inline int BuildPkgHeader(SPacketHeader* pstHeader, UINT16 wDataLen)
    {
        pstHeader->wMark = PKG_MARK;
        pstHeader->wDataLen = wDataLen;
        pstHeader->wCheckSum = (pstHeader->wDataLen ^ 0xBBCC) & 0x88AA;

        return sizeof(SPacketHeader);
    }

private:
    inline int __CheckPkgHeader(SPacketHeader* pstHeader)
    {
        if(pstHeader->wMark != PKG_MARK)
        {
            return -1;
        }

        UINT16 wCheckSum = (pstHeader->wDataLen ^ 0xBBCC) & 0x88AA;
        if(pstHeader->wCheckSum != wCheckSum)
        {
            return -1;
        }

        return (sizeof(SPacketHeader) + pstHeader->wDataLen);
    }
};


#endif  // __PACKETPARSER_H_2009_0902__
