# node-profinetdcp

NodeJS Addon for sending PROFINET DCP requests.

## Prerequisites

Libpcap library should be already installed prior to the installation of the npm package.

### For Windows

Install NPcap from https://nmap.org/npcap/#download

### For Linux

Install libpcap-dev e.g.

```bash
sudo apt-get install -y libpcap-dev
```

### for Mac OSX

Libpcap is already installed

## Installation

```bash
npm install profinetdcp
```

## Example

```javascript
const { listInterfaces, dcpIdentify, LinkStatus } = require('profinetdcp')

listInterfaces().then((list) => {
  const interfaces = list.filter((intf) => !intf.isLoopback && intf.status == LinkStatus.Up)

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
  } else {
    console.log(`No network interfaces found with link status "up"`)
  }  
})
```

## TODO

- Only DCP Identify request is implemented at the moment. DCP-Get and DCP-Set to follow ...

- List of hosts is returned after the 5 sec. scan timeout. No partial progress at the moment.

- Add documentation and examples

