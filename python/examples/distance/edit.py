import time
import numpy
from metric.distance import Edit

str1 = "1011001100110011001111111"
str2 = "1000011001100110011011100"
str3 = "Absolutely different string"

distance = Edit()

start_time = time.time()
res = distance(str1, str2)
end_time = time.time()
print("result for close strings: %d (Time = %06fms)" % (res, (end_time - start_time)*1000))

start_time = time.time()
res = distance(str1, str3)
end_time = time.time()
print("result for different strings: %d (Time = %06fms)" % (res, (end_time - start_time)*1000))

A3 = numpy.array([-0.991021875880222, -0.768275252129114, -0.526359355330172,
                  -0.318170691552235, -0.0468979315641954, 0.227289495956795, 0.317841938040349,
                  0.463820792484335, 0.660623198153296, 0.812210713528849])

B3 = numpy.array([2.36088477872717 , 1.23271042331569, 0.219758852423591,
                  0.0129766138306992, 0.00923506810444738, 1.49393468371558e-07, 0.00619896971968280,
                  0.212322021636953, 0.257245700714104, 1.59223791395715])

start_time = time.time()
res = distance(A3, B3)
end_time = time.time()
print("result for array: %d (Time = %06fms)" % (res, (end_time - start_time)*1000) )
