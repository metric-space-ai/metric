{
  "targets": [
    {
      "target_name": "<(module_name)",
        "sources": [ "feature_cast.cpp","jsfilter.cpp", "bindings.cpp",
                     "jsdimension_feature_sum.cpp",

                     "ext/jsdimension_feature_sum_i64.cpp",
                     "ext/jsdimension_feature_sum_i32.cpp",
                     "ext/jsdimension_feature_sum_ui64.cpp",
                     "ext/jsdimension_feature_sum_bool.cpp",
                     "ext/jsdimension_feature_sum_double.cpp",
                     "ext/jsdimension_feature_sum_string.cpp",

                     "jsfeature_order_natural.cpp",
                     "jsfeature.cpp",
                     "jsfeature_all.cpp",
                     "jsfeature_size.cpp",
                     "jsfeature_value.cpp",
                     "jsfeature_order.cpp",
                     "jsfeature_top.cpp",

                     "jsdimension_all_sum.cpp",
                     "jsdimension_all_count.cpp",
                     "jsdimension_all.cpp",
                     "jsdimension_feature.cpp",
                     "jsdimension_feature_iterable.cpp",
                     "jsdimension_feature_count.cpp",
                     "jsdimension.cpp",
                     "jsdimension_top_bottom.cpp",
                     "jsdimension_filter.cpp",

                     "utils.cpp",

                     "ext/jsfeature_order_i64.cpp",
                     "ext/jsfeature_order_i32.cpp",
                     "ext/jsfeature_order_ui64.cpp",
                     "ext/jsfeature_order_bool.cpp",
                     "ext/jsfeature_order_double.cpp",
                     "ext/jsfeature_order_string.cpp",
                     "ext/jsfeature_order_natural_i64.cpp",
                     "ext/jsfeature_order_natural_i32.cpp",
                     "ext/jsfeature_order_natural_ui64.cpp",
                     "ext/jsfeature_order_natural_bool.cpp",
                     "ext/jsfeature_order_natural_double.cpp",
                     "ext/jsfeature_order_natural_string.cpp",

                     "ext/jsfeature_top_i64.cpp",
                     "ext/jsfeature_top_i32.cpp",
                     "ext/jsfeature_top_ui64.cpp",
                     "ext/jsfeature_top_bool.cpp",
                     "ext/jsfeature_top_double.cpp",
                     "ext/jsfeature_top_string.cpp",
                     "ext/jsdimension_feature_i64.cpp",
                     "ext/jsdimension_feature_i32.cpp",
                     "ext/jsdimension_feature_ui64.cpp",
                     "ext/jsdimension_feature_bool.cpp",
                     "ext/jsdimension_feature_double.cpp",
                     "ext/jsdimension_feature_string.cpp",


                   ],
      "include_dirs": [ "../../crossfilter/include", "./"],
      "cflags": ['-O3', '-std=c++14'],
      'cflags_cc!': [ '-fno-exceptions' ],
      'conditions': [
                    ['OS=="mac"', {
                                  'xcode_settings': {
                                  'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                                  'OTHER_CFLAGS': [
                                                  "-std=c++14",
                                                  "-stdlib=libc++",
                                                  "-mmacosx-version-min=10.12"
                                                  ],
                                  }
                                  }]
                  ],
      "defines": ['CROSS_FILTER_MULTI_THREAD', 'CROSS_FILTER_USE_THREAD_POOL'],
      'product_dir': '<(module_path)'
    }
  ]
}

