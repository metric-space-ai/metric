/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Dmitry Vinokurov (c) 2018.
*/
module.exports = require("./bindings")
module.exports.metrics = Object.freeze({"euclidian":0, "manhatten":1,"euclidian_thresholded":2,  "cosine":3, "p_norm":4, "emd":5, "twed":6, "ssim":7});

