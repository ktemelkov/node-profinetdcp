module.exports = require('bindings')('profinetdcp')

module.exports.LinkStatus = Object.freeze({
  Up: 1,
  Down: 2
})

module.exports.DeviceRole = Object.freeze({
  IODevice: 1,
  Controller: 2
})
