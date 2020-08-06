#ifndef __DCP_PDU_H
#define __DCP_PDU_H


#define IS_VLAN_TAGGED(frame) ((frame)[12] == 0x81 && (frame)[13] == 0x00)


#define DCP_MAX_NAME_LENGTH  240
#define DCP_MAX_LABEL_LENGTH 63
#define DCP_RESPONSE_SUCCESS 0x01

#define MSK_IP_ADDRESS_RESPONSE_DHCP 0x0002

static const uint8_t  OPTION_IP           = 1; /* IP Option */
static const uint8_t  OPTION_DEVPROP      = 2; /* Device properties */
static const uint8_t  OPTION_DHCP         = 3; /* DHCP options */
static const uint8_t  OPTION_LLDP         = 4; /* LLDP options */
static const uint8_t  OPTION_CONTROL      = 5; /* Control */

static const uint8_t  CONTROL_SUBOPTION_STARTTRANS = 1; /* Start transmission suboption */
static const uint8_t  CONTROL_SUBOPTION_ENDTRANS   = 2; /* End transmission suboption */
static const uint8_t  CONTROL_SUBOPTION_SIGNAL     = 3; /* Signal suboption */
static const uint8_t  CONTROL_SUBOPTION_RESPONSE   = 4; /* Response suboption (given as response on set request) */
static const uint8_t  CONTROL_SUBOPTION_FACTRESET  = 5; /* Factory reset */

static const uint8_t  IP_SUBOPTION_MAC             = 1; /* MAC address settings */
static const uint8_t  IP_SUBOPTION_IPPARAM         = 2; /* IP Parameter settings */

static const uint8_t  DHCP_SUBOPTION_DHCP_CLIENT_IDENTIFIER = 61;

static const uint8_t  DCP_SIGNAL_FLASH_ONCE        = 1; /* Flash once for signal request */

static const uint8_t  DEVPROP_DEVICEVENDOR         = 1; /* Type of Station */
static const uint8_t  DEVPROP_NAMEOFSTATION        = 2; /* Name of Station */
static const uint8_t  DEVPROP_DEVICEID             = 3; /* Device/Vendor ID */
static const uint8_t  DEVPROP_DEVICEROLE           = 4; /* Device role */
static const uint8_t  DEVPROP_DEVICEOPTIONS        = 5; /* Device options */
static const uint8_t  DEVPROP_ALIAS                = 6; /* Alias for NameOfStation */


#pragma pack(1)

struct ETHERNET_FRAME_HEADER {
  uint8_t  abDest[6];   //!< Destination MAC address
  uint8_t  abSource[6]; //!< Source MAC address
  uint16_t usFrameType; //!< Ethernet frame type
};


struct ETHERNET_VLAN_FRAME_HEADER {
  uint8_t  abDest[6];     //!< Destination MAC address
  uint8_t  abSource[6];   //!< Source MAC address
  uint16_t usVLANTPID;    //!< VLAN TPID
  uint16_t usVLANTCI;     //!< VLAN TCI
  uint16_t usFrameType;   //!< Ethernet frame type
};


struct DCP_HEADER {
  uint16_t usFrameIdent;    //!< Frame identifier
  uint8_t  bServiceId;      //!< Service identifier
  uint8_t  bServiceType;    //!< Service type
  uint32_t  ulXid;           //!< Transmission ID
};


struct DCP_RESPONSE_HEADER : public DCP_HEADER {
  uint16_t usReserved;      //!< unused/reserved
  uint16_t usDcpDataLength; //!< total data provided by frame
};


struct DCP_RESPONSE_BLOCK_HEADER {
  uint8_t  bOption;      //!< Service identifier
  uint8_t  bSubOption;    //!< Service type
  uint16_t usDcpBlockLength; //!< block data length (without the header)
};


#pragma pack()

#endif // __DCP_PDU_H
