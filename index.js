const profinetrpc = require('bindings')('profinetdcp')

const interfaces = profinetrpc.listInterfaces()

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
