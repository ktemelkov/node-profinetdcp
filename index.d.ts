
export enum LinkStatus {
  Up = 1,
  Down = 2
}


export enum DeviceRole {
  IODevice = 1,
  Controller = 2
}


export interface NetworkInterface {
  name: string;
  adapterName: string;
  hardwareAddress: number[];
  IP?: string;
  IPv6?: string;
  status: LinkStatus;
  isLoopback: boolean;
}


export enum DcpOptionType {
  IP = 1,
  DeviceProp = 2,
  DHCP = 3,
  LLDP = 4,
  Control = 5,
}


export enum DcpIpSubOptionType {
  MAC = 1,
  IP = 2
}


export enum DcpControlSubOptionType {
  StartTransmission = 1,
  EndTransmission = 2,
  Signal = 3,
  Response = 4,
  FactoryReset = 5 
}


export enum DcpDevicePropSubOptionType {
  Vendor = 1,         /* Type of Station */
  NameOfStation = 2,  /* Name of Station */
  DeviceId = 3,       /* Device/Vendor ID */
  Role = 4,           /* Device role */
  Options = 5,        /* Device options */
  Alias = 6,          /* Alias for NameOfStation */  
}


export enum DcpDhcpSubOptionType {
  ClientIdentifier = 61
}


export enum DcpResult {
  NoError = 0,
  OptionNotSupported = 1,
  SubOptionNotSupported = 2,
  SubOptionNotSet = 3,
  ResourceError = 4,
  SetNotPossible = 5  
}


export interface DcpOption {
  Option: DcpOptionType;
  SubOption: DcpIpSubOptionType | DcpControlSubOptionType | DcpDevicePropSubOptionType | DcpDhcpSubOptionType;
}


export interface DcpHost {
  MAC: number[];
  NameOfStation?: string;
  Alias?: string;
  Vendor?: string;
  VendorId?: number;
  DeviceId?: number;
  Role?: DeviceRole;
  DHCP?: boolean;
  IPAddress?: string;
  Netmask?: string;
  Gateway?: string;
  SupportedOptions: DcpOption[];
}


export function listInterfaces(): Promise<NetworkInterface[]>;
export function dcpIdentify(intf: NetworkInterface, partialResultCallback?: (host: DcpHost) => void): Promise<DcpHost[]>;
export function dcpGet(intf: NetworkInterface): any;
export function dcpSet(intf: NetworkInterface): any;
