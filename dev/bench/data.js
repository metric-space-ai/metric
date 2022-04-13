window.BENCHMARK_DATA = {
  "lastUpdate": 1649874115415,
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
      }
    ]
  }
}