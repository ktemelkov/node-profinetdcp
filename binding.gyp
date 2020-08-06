{
  "targets": [
    {
      "target_name": "profinetdcp",
      "sources": [ "src/module.cc", "src/interfaces.cc", "src/dcp.cc" ],
      "include_dirs": [ "<!@(node -p \"require('node-addon-api').include\")" ],
      "cflags_cc": [ "-fno-exceptions", "-Wno-missing-field-initializers" ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
      'conditions': [
        [
          'OS=="win"', {
            'include_dirs': [ 'winpcap/Include' ],
            'defines': [ 'WPCAP' ],
            'sources': [ 'src/platform_windows.cc' ],
            'conditions': [
              [ 
                'target_arch=="ia32"', 
                { 'link_settings': { 'libraries': [ 'ws2_32.lib', 'IPHLPAPI.lib', '<(PRODUCT_DIR)/../../winpcap/Lib/wpcap.lib' ]}},
                { 'link_settings': { 'libraries': [ 'ws2_32.lib', 'IPHLPAPI.lib', '<(PRODUCT_DIR)/../../winpcap/Lib/x64/wpcap.lib' ]}}]
              ]
          }, {
            'sources': [ 'src/platform_linux.cc' ],
            'link_settings': { 'libraries': [ '-lpcap' ] }
          }
        ]
      ],
      'target_conditions': [
        ['_win_delay_load_hook=="true"', {
          # If the addon specifies `'win_delay_load_hook': 'true'` in its
          # binding.gyp, link a delay-load hook into the DLL. This hook ensures
          # that the addon will work regardless of whether the node/iojs binary
          # is named node.exe, iojs.exe, or something else.
          'conditions': [
            [ 'OS=="win"', {
              'sources': [
                '<(node_gyp_dir)/src/win_delay_load_hook.cc',
              ],
              'msvs_settings': {
                'VCLinkerTool': {
                  'DelayLoadDLLs': [ 'wpcap.dll' ],
                  'AdditionalOptions': [ '/ignore:4199' ]
                }
              }
            }]
          ]}
        ]
      ]
    }
  ]
}