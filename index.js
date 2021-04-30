module.exports = require('bindings')('profinetdcp')


module.exports.LinkStatus = Object.freeze({
  Up: 1,
  Down: 2
})


module.exports.DeviceRole = Object.freeze({
  IODevice: 1,
  Controller: 2
})


module.exports.DcpResult = Object.freeze({
  NoError: 0,
  OptionNotSupported: 1,
  SubOptionNotSupported: 2,
  SubOptionNotSet: 3,
  ResourceError: 4,
  SetNotPossible: 5  
})


module.exports.DcpOptionType = Object.freeze({
  IP: 1,
  DeviceProp: 2,
  DHCP: 3,
  LLDP: 4,
  Control: 5,
})


module.exports.DcpIpSubOptionType = Object.freeze({
  MAC: 1,
  IP: 2
})


module.exports.DcpControlSubOptionType = Object.freeze({
  StartTransmission: 1,
  EndTransmission: 2,
  Signal: 3,
  Response: 4,
  FactoryReset: 5 
})


module.exports.DcpDevicePropSubOptionType = Object.freeze({
  Vendor: 1,         /* Type of Station */
  NameOfStation: 2,  /* Name of Station */
  DeviceId: 3,       /* Device/Vendor ID */
  Role: 4,           /* Device role */
  Options: 5,        /* Device options */
  Alias: 6,          /* Alias for NameOfStation */  
})


module.exports.DcpDhcpSubOptionType = Object.freeze({
  ClientIdentifier: 61
})
