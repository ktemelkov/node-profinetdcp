
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

export interface DcpOption {
  Option: number;
  SubOption: number;
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
export function dcpIdentify(intf: NetworkInterface): Promise<DcpHost[]>;
export function dcpGet(intf: NetworkInterface): any;
export function dcpSet(intf: NetworkInterface): any;
