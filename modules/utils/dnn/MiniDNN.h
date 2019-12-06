#ifndef MINIDNN_H_
#define MINIDNN_H_

#include "../../../3rdparty/blaze/Math.h"

#include "RNG.h"

#include "Layer.h"
#include "Layer/FullyConnected.h"
#include "Layer/Conv2d.h"
#include "Layer/Conv2d-transpose.h"
#include "Layer/MaxPooling.h"

#include "Activation/ReLU.h"
#include "Activation/Identity.h"
#include "Activation/Sigmoid.h"
//#include "Activation/Softmax.h"

#include "Output.h"
#include "Output/RegressionMSE.h"
//#include "Output/BinaryClassEntropy.h"
//#include "Output/MultiClassEntropy.h"

#include "Optimizer.h"
//#include "Optimizer/SGD.h"
//#include "Optimizer/AdaGrad.h"
#include "Optimizer/RMSProp.h"

#include "Callback.h"
#include "Callback/VerboseCallback.h"

#include "Network.h"


#endif /* MINIDNN_H_ */
