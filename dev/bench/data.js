window.BENCHMARK_DATA = {
  "lastUpdate": 1652717034077,
  "repoUrl": "https://github.com/panda-official/metric",
  "entries": {
    "Convolution": [
      {
        "commit": {
          "author": {
            "email": "44258878+maxfilippov@users.noreply.github.com",
            "name": "maxfilippov",
            "username": "maxfilippov"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "9c5bd8df9f14910b309242f7c7b13dbbb0cff451",
          "message": "PAS-17 add export & import to PCFA, add minimal PCFA test (#324)\n\n* add expotr & import to PCFA, add minimal PCFA test\r\n\r\n* format PCFA code, add comments, minor update of PCFA test",
          "timestamp": "2022-04-13T21:16:24+03:00",
          "tree_id": "867a293866181d6a7d45b5c3b32c8fd3cffe0238",
          "url": "https://github.com/panda-official/metric/commit/9c5bd8df9f14910b309242f7c7b13dbbb0cff451"
        },
        "date": 1649874114578,
        "tool": "catch2",
        "benches": [
          {
            "name": "Convolution2d() [320x240 3x3]",
            "value": 11.3928,
            "range": "± 425.775",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [320x240 3x3]",
            "value": 860.718,
            "range": "± 51.1424",
            "unit": "us",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [320x240 3x3]",
            "value": 1.17591,
            "range": "± 92.8697",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [320x240 5x5]",
            "value": 47.5537,
            "range": "± 918.364",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [320x240 5x5]",
            "value": 2.68248,
            "range": "± 92.5727",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [320x240 5x5]",
            "value": 2.95669,
            "range": "± 302.565",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [320x240 7x7]",
            "value": 81.7015,
            "range": "± 793.766",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [320x240 7x7]",
            "value": 5.73863,
            "range": "± 75.3878",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [320x240 7x7]",
            "value": 5.22494,
            "range": "± 365.495",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [640x480 3x3]",
            "value": 71.2836,
            "range": "± 402.454",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [640x480 3x3]",
            "value": 4.63322,
            "range": "± 202.425",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [640x480 3x3]",
            "value": 6.57651,
            "range": "± 583.149",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [640x480 5x5]",
            "value": 175.202,
            "range": "± 983.588",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [640x480 5x5]",
            "value": 13.2172,
            "range": "± 216.949",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [640x480 5x5]",
            "value": 10.5391,
            "range": "± 873.533",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [640x480 7x7]",
            "value": 325.306,
            "range": "± 1.32322",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [640x480 7x7]",
            "value": 25.0146,
            "range": "± 322.836",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [640x480 7x7]",
            "value": 17.8615,
            "range": "± 740.019",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [1920x1080 3x3]",
            "value": 511.601,
            "range": "± 3.52886",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [1920x1080 3x3]",
            "value": 34.8417,
            "range": "± 182.807",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [1920x1080 3x3]",
            "value": 51.2218,
            "range": "± 1.78369",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [1920x1080 5x5]",
            "value": 1.19015,
            "range": "± 4.22723",
            "unit": "s",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [1920x1080 5x5]",
            "value": 89.1531,
            "range": "± 592.827",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [1920x1080 5x5]",
            "value": 81.9164,
            "range": "± 5.98932",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [1920x1080 7x7]",
            "value": 2.21355,
            "range": "± 5.09246",
            "unit": "s",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [1920x1080 7x7]",
            "value": 169.588,
            "range": "± 609.364",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [1920x1080 7x7]",
            "value": 125.013,
            "range": "± 1.41676",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "kataev.victor.1234@gmail.com",
            "name": "Victor",
            "username": "victor1234"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "1dac8eb9ad50dcf5c014c29bc1c15d6b56d9c731",
          "message": "Remove REQUIRED and add if block to lapack (#325)\n\n* Remove REQUIRED and add if block to link\r\n\r\n* Add cereal link\r\n\r\n* Fix taget link\r\n\r\n* Change cereal version to 1.3.2\r\n\r\n* Update changelog",
          "timestamp": "2022-05-16T18:57:42+03:00",
          "tree_id": "b5c715186be7b29fa5d858e782da0634e17f59ed",
          "url": "https://github.com/panda-official/metric/commit/1dac8eb9ad50dcf5c014c29bc1c15d6b56d9c731"
        },
        "date": 1652717031935,
        "tool": "catch2",
        "benches": [
          {
            "name": "Convolution2d() [320x240 3x3]",
            "value": 9.87718,
            "range": "± 948.884",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [320x240 3x3]",
            "value": 736.073,
            "range": "± 55.491",
            "unit": "us",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [320x240 3x3]",
            "value": 1.24257,
            "range": "± 195.49",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [320x240 5x5]",
            "value": 50.2355,
            "range": "± 2.06622",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [320x240 5x5]",
            "value": 2.26763,
            "range": "± 537.319",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [320x240 5x5]",
            "value": 2.27836,
            "range": "± 320.893",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [320x240 7x7]",
            "value": 97.6773,
            "range": "± 825.242",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [320x240 7x7]",
            "value": 9.48678,
            "range": "± 235.375",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [320x240 7x7]",
            "value": 4.98025,
            "range": "± 280.037",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [640x480 3x3]",
            "value": 81.4501,
            "range": "± 1.56068",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [640x480 3x3]",
            "value": 7.5719,
            "range": "± 2.18784",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [640x480 3x3]",
            "value": 5.9256,
            "range": "± 291.76",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [640x480 5x5]",
            "value": 206.904,
            "range": "± 4.00392",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [640x480 5x5]",
            "value": 20.1455,
            "range": "± 337.53",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [640x480 5x5]",
            "value": 11.288,
            "range": "± 671.677",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [640x480 7x7]",
            "value": 390.418,
            "range": "± 5.23572",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [640x480 7x7]",
            "value": 39.6991,
            "range": "± 1.17583",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [640x480 7x7]",
            "value": 19.4246,
            "range": "± 645.087",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [1920x1080 3x3]",
            "value": 596.668,
            "range": "± 7.75676",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [1920x1080 3x3]",
            "value": 52.8037,
            "range": "± 737.988",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [1920x1080 3x3]",
            "value": 55.3864,
            "range": "± 2.20335",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [1920x1080 5x5]",
            "value": 1.41991,
            "range": "± 14.3735",
            "unit": "s",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [1920x1080 5x5]",
            "value": 137.366,
            "range": "± 2.9116",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [1920x1080 5x5]",
            "value": 89.9246,
            "range": "± 3.1092",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "Convolution2d() [1920x1080 7x7]",
            "value": 2.64853,
            "range": "± 24.4458",
            "unit": "s",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "setKernel() [1920x1080 7x7]",
            "value": 267.703,
            "range": "± 4.94104",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          },
          {
            "name": "operator() [1920x1080 7x7]",
            "value": 150.614,
            "range": "± 18.0102",
            "unit": "ms",
            "extra": "10 samples\n1 iterations"
          }
        ]
      }
    ]
  }
}