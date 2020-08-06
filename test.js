const { listInterfaces, dcpIdentify } = require('.')


listInterfaces().then((list) => {
  const interfaces = list.filter((intf) => !intf.isLoopback && intf.status == 1)

  if (interfaces.length > 0) {
    console.log(`Found ${interfaces.length} network interfaces with link status "up"`)
    console.log('---------------------------------------------------------------------')
    console.log(interfaces)
    console.log('---------------------------------------------------------------------')
    
    console.log('Sending DCP identify requests ...')
  
    interfaces.forEach((intf) => {
      dcpIdentify(intf).then((hosts) => {
        if (hosts.length > 0) {
          console.log(`Hosts found on interface ${intf.name}`)
          console.log(hosts)
        } else {
          console.log(`No hosts found on interface ${intf.name}`)
        }
      }).catch((err) => {
        console.log(`Failed to indentify hosts on interface ${intf.name}`)
        console.log(err.message)
      })
    })  
  }  
})
