{
  "targets": [
    {
      "target_name": "<(module_name)",
        "sources": [ "bindings.cpp",
    		     "metric_search_js.cpp",
    		     "utils.cpp"
                   ],
      "include_dirs": [ "../", "./", "./metric_distance"],
      "cflags_cc": ['-O3', '-std=c++17'],
      'cflags_cc!': [ '-fno-exceptions'],
      'conditions': [
                    ['OS=="mac"', {
                                  'xcode_settings': {
                                  'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                                  'OTHER_CFLAGS': [
                                                  "-std=c++17",
                                                  "-stdlib=libc++",
                                                  "-mmacosx-version-min=10.14"
                                                  ],
                                  }
                                  }]
                  ],
      "defines": ['CROSS_FILTER_MULTI_THREAD', 'CROSS_FILTER_USE_THREAD_POOL'],
      'product_dir': '<(module_path)'
    }
  ]
}

