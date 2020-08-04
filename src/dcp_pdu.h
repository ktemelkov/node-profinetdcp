#ifndef __DCP_PDU_H
#define __DCP_PDU_H


#define IS_VLAN_TAGGED(frame) ((frame)[12] == 0x81 && (frame)[13] == 0x00)


#define DCP_MAX_NAME_LENGTH  240
#define DCP_MAX_LABEL_LENGTH 63
#define DCP_RESPONSE_SUCCESS 0x01

#define MSK_IP_ADDRESS_RESPONSE_DHCP 0x0002

static const unsigned char  OPTION_IP           = 1; /* IP Option */
static const unsigned char  OPTION_DEVPROP      = 2; /* Device properties */
static const unsigned char  OPTION_DHCP         = 3; /* DHCP options */
static const unsigned char  OPTION_LLDP         = 4; /* LLDP options */
static const unsigned char  OPTION_CONTROL      = 5; /* Control */

static const unsigned char  CONTROL_SUBOPTION_STARTTRANS = 1; /* Start transmission suboption */
static const unsigned char  CONTROL_SUBOPTION_ENDTRANS   = 2; /* End transmission suboption */
static const unsigned char  CONTROL_SUBOPTION_SIGNAL     = 3; /* Signal suboption */
static const unsigned char  CONTROL_SUBOPTION_RESPONSE   = 4; /* Response suboption (given as response on set request) */
static const unsigned char  CONTROL_SUBOPTION_FACTRESET  = 5; /* Factory reset */

static const unsigned char  IP_SUBOPTION_MAC             = 1; /* MAC address settings */
static const unsigned char  IP_SUBOPTION_IPPARAM         = 2; /* IP Parameter settings */

static const unsigned char  DHCP_SUBOPTION_DHCP_CLIENT_IDENTIFIER = 61;

static const unsigned char  DCP_SIGNAL_FLASH_ONCE        = 1; /* Flash once for signal request */

static const unsigned char  DEVPROP_DEVICEVENDOR         = 1; /* Type of Station */
static const unsigned char  DEVPROP_NAMEOFSTATION        = 2; /* Name of Station */
static const unsigned char  DEVPROP_DEVICEID             = 3; /* Device/Vendor ID */
static const unsigned char  DEVPROP_DEVICEROLE           = 4; /* Device role */
static const unsigned char  DEVPROP_DEVICEOPTIONS        = 5; /* Device options */
static const unsigned char  DEVPROP_ALIAS                = 6; /* Alias for NameOfStation */


#pragma pack(1)

struct ETHERNET_FRAME_HEADER {
  unsigned char  abDest[6];   //!< Destination MAC address
  unsigned char  abSource[6]; //!< Source MAC address
  unsigned short usFrameType; //!< Ethernet frame type
};


struct ETHERNET_VLAN_FRAME_HEADER {
  unsigned char  abDest[6];     //!< Destination MAC address
  unsigned char  abSource[6];   //!< Source MAC address
  unsigned short usVLANTPID;    //!< VLAN TPID
  unsigned short usVLANTCI;     //!< VLAN TCI
  unsigned short usFrameType;   //!< Ethernet frame type
};


struct DCP_HEADER {
  unsigned short usFrameIdent;    //!< Frame identifier
  unsigned char  bServiceId;      //!< Service identifier
  unsigned char  bServiceType;    //!< Service type
  unsigned long  ulXid;           //!< Transmission ID
};


struct DCP_RESPONSE_HEADER : public DCP_HEADER {
    unsigned short usReserved;      //!< unused/reserved
    unsigned short usDcpDataLength; //!< total data provided by frame
};


struct DCP_RESPONSE_BLOCK_HEADER {
  unsigned char  bOption;      //!< Service identifier
  unsigned char  bSubOption;    //!< Service type
  unsigned short usDcpBlockLength; //!< block data length (without the header)
};


#pragma pack()

#endif // __DCP_PDU_H