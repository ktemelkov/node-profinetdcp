#ifndef __DCP_PDU_H
#define __DCP_PDU_H


#define IS_VLAN_TAGGED(frame) ((frame)[12] == 0x81 && (frame)[13] == 0x00)


#define DCP_MAX_NAME_LENGTH  240
#define DCP_MAX_LABEL_LENGTH 63
#define DCP_RESPONSE_SUCCESS 0x01


#pragma pack(1)

struct ETHERNET_FRAME_HEADER
{
  unsigned char  abDest[6];   //!< Destination MAC address
  unsigned char  abSource[6]; //!< Source MAC address
  unsigned short usFrameType; //!< Ethernet frame type
};


struct ETHERNET_VLAN_FRAME_HEADER
{
  unsigned char  abDest[6];     //!< Destination MAC address
  unsigned char  abSource[6];   //!< Source MAC address
  unsigned short usVLANTPID;    //!< VLAN TPID
  unsigned short usVLANTCI;     //!< VLAN TCI
  unsigned short usFrameType;   //!< Ethernet frame type
};


struct DCP_HEADER
{
  unsigned short usFrameIdent;    //!< Frame identifier
  unsigned char  bServiceId;      //!< Service identifier
  unsigned char  bServiceType;    //!< Service type
  unsigned long  ulXid;           //!< Transmission ID
};


struct DCP_RESPONSE_HEADER : public DCP_HEADER
{
    unsigned short usReserved;      //!< unused/reserved
    unsigned short usDcpDataLength; //!< total data provided by frame
};

#pragma pack()

#endif // __DCP_PDU_H