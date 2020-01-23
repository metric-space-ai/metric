/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

/*
#pragma once
class metrics {
    using MGC_Euclidian_Euclidian = metric::MGC<NotUsed,metric::Euclidian<double>,NotUsed,metric::Euclidian<double>>;
    using MGC_Euclidian_Manhatten = metric::MGC<NotUsed,metric::Euclidian<double>,NotUsed,metric::Manhatten<double>>;
    using MGC_Manhatten_Manhatten = metric::MGC<NotUsed,metric::Manhatten<double>,NotUsed,metric::Manhatten<double>>;
    using MGC_Manhatten_Euclidian = metric::MGC<NotUsed,metric::Manhatten<double>,NotUsed,metric::Euclidian<double>>;

    using mgc_variant = boost::variant<
            MGC_Euclidian_Euclidian,
            MGC_Euclidian_Manhatten,
            MGC_Manhatten_Manhatten,
            MGC_Manhatten_Euclidian
        >;
    static const char constexpr* euclidian = "euclidian";
    static const char constexpr* manhatten = "manhatten";
public:
    std::shared_ptr<mgc_variant> mgc;
    metrics(const std::string& Metric1, const std::string& Metric2)
    {
        if (Metric1 == euclidian && Metric2 == euclidian) mgc = std::make_shared<mgc_variant>(MGC_Euclidian_Euclidian());
        else if (Metric1 == euclidian && Metric2 == manhatten) mgc = std::make_shared<mgc_variant>(MGC_Euclidian_Manhatten());

        else if (Metric1 == manhatten && Metric2 == manhatten) mgc = std::make_shared<mgc_variant>(MGC_Manhatten_Manhatten());
        else if (Metric1 == manhatten && Metric2 == euclidian) mgc = std::make_shared<mgc_variant>(MGC_Manhatten_Euclidian());

        else throw std::runtime_error("No such metrics");
    }
};
*/
