{
  'targets': [
    {
      'target_name': 'computron-native',
      'sources': [ '<!@(find . -name \*.cpp)' ],
      'include_dirs': [
        "<!@(node -p \"require('node-addon-api').include\")",
        '/usr/include/libxml2'
      ],
      'dependencies': [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      'cflags!': [
        '-fno-exceptions',
        '-Wall',
        '-Werror'
      ],
      'cflags_cc!': [
        '-fno-exceptions',
        '-Wall',
        '-Werror'
      ],
      'libraries': [
        '-lxml2',
        '-lxslt',
        '-lexslt'
      ]
    }
  ]
}
