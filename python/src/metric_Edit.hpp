/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#pragma once

namespace metric_Edit {

class NotUsed {};
class metricsEdit {
    metric::Edit<NotUsed> metricEdit;

public:
    int call(const python_wrap_stl& A, const python_wrap_stl& B) {
            return metricEdit(A,B);
    }
};

};
