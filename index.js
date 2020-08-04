const profinetdcp = require('bindings')('profinetdcp')

const interfaces = profinetdcp.listInterfaces()

console.log(interfaces)

profinetdcp.dcpIdentify(interfaces[1]).then((hosts) => {
  console.log(hosts)
}).catch((err) => {
  console.log(err.message)
});

/*
for (var i=0; i < interfaces.length; i++) {
  const interf = interfaces[i]
  console.log(interf.name)
  console.log('---------------------------------------------------------------------')
  console.log(`description: ${interf.description}`)
  console.log('addresses:')

  for (var j=0; j < interf.addresses.length; j++) {
    console.log(interf.addresses[j])
  }

  console.log()
}
*/