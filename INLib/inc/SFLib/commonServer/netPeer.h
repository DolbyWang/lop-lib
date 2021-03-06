//////////////////////////////////////////////////////////////////////
//  created:    2011/11/01
//  filename:   SFLib/commonServer/netPeer.h
//  author:     League of Perfect
/// @brief
///
//////////////////////////////////////////////////////////////////////
#ifndef __SFLIB_COMMONSERVER_NETPEER_H__
#define __SFLIB_COMMONSERVER_NETPEER_H__

#include <BCLib/utility/pointer.h>
#include <SFLib/commonDefine/baseDef.h>
#include <SFLib/message/message.h>
#include <BCLib/utility/spointer.h>

namespace SFLib
{
namespace CommonServer
{
enum EPeerState
{
    PEER_NULL,
    PEER_INIT,
    PEER_OK,
    PEER_LEAVE,
};

class CNetPeer : public BCLib::Utility::CRefCnt
{
public:
    CNetPeer();
    CNetPeer(PeerID netPeerID);
    virtual ~CNetPeer();

    GroupID getGroupID() const;

    PeerID getPeerID() const
    {
        return m_netPeerID;
    }
    EPeerState getState()
    {
        return m_peerState;
    }

	void setEntityID(EntityID entityID)
	{
		m_entityID = entityID;
	}
	EntityID getEntityID() const
	{
		return m_entityID;
	}

    virtual void init();
    virtual bool reqLeave();
    virtual void ackLeave();
    virtual void terminate();

    virtual bool serializeTo(BCLib::Utility::CStream& stream) const;
    virtual bool serializeFrom(BCLib::Utility::CStream& stream);

public:
    virtual bool sendMsgByType(ServerType serverType, const SFLib::Message::CNetMessage* msg);
    virtual bool sendMsgByType(ServerType serverType, const SFLib::Message::SNetMessage* msg, const BCLib::uint32 msgSize);

private:
    PeerID m_netPeerID;
    EPeerState m_peerState;
	EntityID m_entityID;
};

typedef BCLib::Utility::CSPointer<CNetPeer> CNetPeerPtr;
}//CommonServer
}//SFLib

#endif//__SFLIB_COMMONSERVER_NETPEER_H__
