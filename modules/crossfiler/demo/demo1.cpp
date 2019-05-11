#include "../index.hpp"
#include <vector>
#include <iostream>
#include <ThorSerialize/Traits.h>

struct Record
{
    std::string date;
    int quantity;
    int total;
    int tip;
    std::string type;
    std::vector<std::string> productIDS;
};

int main()
{

    std::vector<Record> data = {
        {"2011-11-14T16:17:54Z", 2, 190, 100, "tab", {"001", "002"}},
        {"2011-11-14T16:20:19Z", 2, 190, 100, "tab", {"001", "005"}},
        {"2011-11-14T16:28:54Z", 1, 300, 200, "visa", {"004", "005"}},
        {"2011-11-14T16:30:43Z", 2, 90, 1, "tab", {"001", "002"}},
        {"2011-11-14T16:48:46Z", 2, 90, 2, "tab", {"005"}},               //
        {"2011-11-14T16:53:41Z", 2, 90, 3, "tab", {"001", "004", "005"}}, //
        {"2011-11-14T16:54:06Z", 1, 100, 4, "cash", {"001", "002", "003", "004", "005"}},
        {"2011-11-14T16:58:03Z", 2, 90, 5, "tab", {"001"}},         //
        {"2011-11-14T17:07:21Z", 2, 90, 6, "visa", {"004", "005"}}, //
        {"2011-11-14T17:22:59Z", 2, 90, 7, "tab", {"001", "002", "004", "005"}}};
    cross::filter<Record> payments(data);

    

    auto totals = payments.dimension([](auto r) { return r.total; });
    totals.filter(90); // by value
    auto filtered_results1 = payments.all_filtered();
    // "2011-11-14T16:30:43Z", 2, 90, 1, "tab", {"001", "002"}
    // "2011-11-14T16:48:46Z", 2, 90, 2, "tab", {"005"}
    // "2011-11-14T16:53:41Z", 2, 90, 3, "tab", {"001", "004", "005"}
    // "2011-11-14T16:58:03Z", 2, 90, 5, "tab", {"001"}
    // "2011-11-14T17:07:21Z", 2, 90, 6, "visa", {"004", "005"}
    // "2011-11-14T17:22:59Z", 2, 90, 7, "tab", {"001", "002", "004", "005"}

    auto tips = payments.dimension([](auto r) { return r.tip; });
    tips.filter(2, 7); // by range (2 ...... 6.99999999999)
    filtered_results1 = payments.all_filtered();
    // "2011-11-14T16:48:46Z", 2, 90, 2, "tab", {"005"}
    // "2011-11-14T16:53:41Z", 2, 90, 3, "tab", {"001", "004", "005"}
    // "2011-11-14T16:58:03Z", 2, 90, 5, "tab", {"001"}
    // "2011-11-14T17:07:21Z", 2, 90, 6, "visa", {"004", "005"}

    auto products = payments.dimension([](auto r) { return r.productIDS.size(); });
    products.filter([](auto d) { return d >= 2; }); // by custom function
    filtered_results1 = payments.all_filtered();
    // "2011-11-14T16:53:41Z", 2, 90, 3, "tab", {"001", "004", "005"}
    // "2011-11-14T17:07:21Z", 2, 90, 6, "visa", {"004", "005"}

    auto no_tabs = payments.dimension([](auto r) { return r.type != std::string("tab") ? 1 : 0; });
    no_tabs.filter(1);
    filtered_results1 = payments.all_filtered();
    // "2011-11-14T17:07:21Z", 2, 90, 6, "visa", {"004", "005"}

    payments.add({"2011-11-14T17:20:20Z", 4, 90, 2, "cash", {"001", "002"}});
    filtered_results1 = payments.all_filtered();
    // "2011-11-14T17:07:21Z", 2, 90, 6, "visa", {"004", "005"}
    // "2011-11-14T17:20:20Z", 4, 90, 2, "cash", {"001", "002"}

    no_tabs.filter();
    totals.filter();

    filtered_results1 = payments.all_filtered();

    // "2011-11-14T17:20:20Z", 4, 90, 2, "cash", {"001", "002"}
    // "2011-11-14T17:07:21Z", 2, 90, 6, "visa", {"004", "005"}
    // "2011-11-14T16:54:06Z", 1, 100, 4, "cash", {"001", "002", "003", "004", "005"}
    // "2011-11-14T16:53:41Z", 2, 90, 3, "tab", {"001", "004", "005"}


    for (int i = 0; i < filtered_results1.size(); ++i)
    {
        std::cout << "\"" << filtered_results1[i].date << "\"" << " " << filtered_results1[i].quantity << " " << filtered_results1[i].total << " " << filtered_results1[i].tip << " " << filtered_results1[i].type << " "
                  << "{\"";
        for (int j = 0; j < filtered_results1[i].productIDS.size() - 1; ++j)
        {
            std::cout << filtered_results1[i].productIDS[j] << "\", \"";
        }
        std::cout << filtered_results1[i].productIDS[filtered_results1[i].productIDS.size() - 1] << "\"}" << std::endl;
    }

    return 0;
}