# node-profinetdcp

NodeJS Addon for sending PROFINET DCP requests.

## Prerequisites

Libpcap tool should be present prior to the installation of the npm package.

### For Windows

Install NPcap from https://nmap.org/npcap/#download

### For Linux

Install libpcap-dev e.g.
`sudo apt-get install -y libpcap-dev`

## Installation

`npm i profinetdcp`

## TODO

- Working only on Windows (Linux support is to follow ...)

- Only DCP Identify request is implemented at the moment. DCP-Get and DCP-Set to follow ...

- List of hosts is returned after the 5 sec. scan timeout. No partial progress at the moment.

- Add typings

- Add examples
