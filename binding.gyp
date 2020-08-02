{
  "targets": [
    {
      "target_name": "profinetdcp",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "src/module.cc" ],
      "include_dirs": [ "<!@(node -p \"require('node-addon-api').include\")" ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
      'conditions': [
        [ 
          'OS=="win"', {
            'include_dirs': [ 'winpcap/Include' ],
            'defines': [ 'WPCAP' ],
            'conditions': [
              [ 
                'target_arch=="ia32"', 
                { 'link_settings': { 'libraries': [ 'ws2_32.lib', '<(PRODUCT_DIR)/../../winpcap/Lib/wpcap.lib' ]}},
                { 'link_settings': { 'libraries': [ 'ws2_32.lib', '<(PRODUCT_DIR)/../../winpcap/Lib/x64/wpcap.lib' ]}}]
              ]
          }, {
            'link_settings': { 'libraries': [ '-lpcap' ] }
          }
        ]
      ]
    }
  ]
}