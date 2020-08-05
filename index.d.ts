export interface NetworkInterface {
  name: string;
  description: string;
  adapterName: string;
  hardwareAddress: number[];
  IP?: string;
  IPv6?: string;
  status: number;
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
  Role?: number;
  DHCP?: boolean;
  IPAddress?: string;
  Netmask?: string;
  Gateway?: string;
  SupportedOptions: DcpOption[];
}

export function listInterfaces(): NetworkInterface[];
export function dcpIdentify(intf: NetworkInterface): Promise<DcpHost[]>;
export function dcpGet(intf: NetworkInterface): any;
export function dcpSet(intf: NetworkInterface): any;
