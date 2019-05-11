// clang++ tests/test_0.cpp -std=c++17 -o run.o -Ofast -l boost_unit_test_framework
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE MetricClassificationTest
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>



#include "../metric_classification.hpp"

#include "assets/helpers.cpp" // csv reader


template <class ContainerType>
struct TestDataset
{
    typedef typename ContainerType::value_type Record;
    TestDataset(ContainerType dataset)
    {
        data = dataset;

        if (data.size() < 1)
            return;

        for (int i=0; i<(int)dataset[0].size() - 1; ++i) {
            features.push_back(
                [=](auto r) { return r[i]; }
            );
        }

        response = [](Record r) {
            if(r[r.size() - 1] >= 0.5)
                return true;
            else
                return false;
        };

    }
    ContainerType data;
    std::vector<std::function<double(Record)> > features;
    std::function<bool(Record)> response;
};


template <class ContainerType>
struct AllDatasets
{
    typedef typename ContainerType::value_type Record;
    AllDatasets<ContainerType>()
    {
        ContainerType table = {
            {0,3,5,0},
            {1,4,5,0},
            {2,5,2,1},
            {3,6,2,1}
        };
        TestDataset<ContainerType> ds = TestDataset(table);
        dataset.push_back(ds);

        table = {
            {3,6,2,1}
        };
        ds = TestDataset(table);
        dataset.push_back(ds);


        table = {
            {0,0},
            {1,0},
            {2,1},
            {3,1}
        };
        ds = TestDataset(table);
        dataset.push_back(ds);

        //  no features at all
        table = {
            {0},
            {0},
            {1},
            {1}
        };
        ds = TestDataset(table);
        dataset.push_back(ds);

        table = {
            {},
        };
        ds = TestDataset(table);
        dataset.push_back(ds);

        table = {
        };
        ds = TestDataset(table);
        ds.features = {};
        ds.response = [](Record r) {
            throw "response accessor called on empty dataset";
            return false; // this is to fit the signature
        };
        dataset.push_back(ds);

    }

    std::vector<TestDataset<ContainerType>> dataset;
};

template <class ContainerType>
struct AllExamples
{
    typedef typename ContainerType::value_type Record;
    AllExamples<ContainerType>()
    {
        ContainerType table = {
            {0,3,5,0},
            {2,5,2,1}
        };
        TestDataset<ContainerType> ds = TestDataset(table);
        dataset.push_back(ds);

        table = {
            {0,0,0,0}
        };
        ds = TestDataset(table);
        dataset.push_back(ds);

        table = {
            {0, 0}
        };
        ds = TestDataset(table);
        dataset.push_back(ds);

        table = {
            {0},
            {0}
        };
        ds = TestDataset(table);
        dataset.push_back(ds);

        table = {
            {0}
        };
        ds = TestDataset(table);
        dataset.push_back(ds);

        table = {
            {}
        };
        ds = TestDataset(table);
        dataset.push_back(ds);

        table = {
        };
        ds = TestDataset(table);
        ds.features = {};
        ds.response = [](Record r) {
            throw "response accessor must not be used in example for prediction";
            return false; // this is to fit the signature
        };
        dataset.push_back(ds);


        table = {
            {3,6,2},
            {10, 10, 10}
        };

        ds = TestDataset(table);

        for (int i=0; i<(int)ds.data[0].size(); ++i) { // no response column
            ds.features.push_back(
                [=](auto r) { return r[i]; }
            );
        }

        ds.response = [](Record r) {
            throw "response accessor must nut be used in example for prediction";
            return false; // this is to fit the signature
        };

        dataset.push_back(ds);


        table = {
            {10, 10, 10}
        };

        ds = TestDataset(table);

        for (int i=0; i<(int)ds.data[0].size(); ++i) { // no response column
            ds.features.push_back(
                [=](auto r) { return r[i]; }
            );
        }

        ds.response = [](Record r) {
            throw "response accessor must nut be used in example for prediction";
            return false; // this is to fit the signature
        };

        dataset.push_back(ds);

        // TODO add special datasets
    }

    std::vector<TestDataset<ContainerType>> dataset;
};





struct WeightVector
{
    WeightVector(int sz, std::vector<double> values)
    {
        size_t v_idx = 0;
        for (size_t i=0; i<sz; i++)
        {
            w.push_back(values[v_idx]);
            v_idx++; // carousel
            if (v_idx>=values.size())
                v_idx = 0;
        } // no normalization
    }

    std::vector<double> w;
};



struct AllWeightVectors
{
    AllWeightVectors(int sz)
    {
        std::vector<double> values = {0.1, 0.3};
        WeightVector v = WeightVector(sz, values);
        w.push_back(v);

        values = {0, 0.5, 1};
        v = WeightVector(sz, values);
        w.push_back(v);

        values = {0}; // senseless
        v = WeightVector(sz, values);
        w.push_back(v);

        values = {0, -10, 10}; // not feasable
        v = WeightVector(sz, values);
        w.push_back(v);
    }

    std::vector<WeightVector> w;
};



typedef typename std::vector<int> TypeV;
typedef typename std::vector<TypeV> ContainerTypeV; // vector
typedef typename std::vector<int> TypeD;
typedef typename std::deque<TypeD> ContainerTypeD; // deque

typedef typename metric::classification::edmClassifier<TypeV, CC45> C45_V;
typedef typename metric::classification::edmClassifier<TypeV, CSVM> SVM_V;

typedef typename metric::classification::edmClassifier<TypeD, CC45> C45_D;
typedef typename metric::classification::edmClassifier<TypeD, CSVM> SVM_D;



typedef boost::mpl::list<C45_V, SVM_V> weak_types; // all needed types to loop through

//*
// weak learners against all datasets ans all test samples defined
// with datasets based on vector
BOOST_AUTO_TEST_CASE_TEMPLATE( test_weak_V, Learner, weak_types )
{
    AllDatasets<ContainerTypeV> DS;
    AllExamples<ContainerTypeV> EX;
    for (auto ds : DS.dataset)
    {
        Learner l = Learner();
        BOOST_CHECK_NO_THROW( l.train(ds.data, ds.features, ds.response); );
//        BOOST_CHECK_NO_THROW( throw "test"; );
        for (auto ex : EX.dataset)
        {
            std::vector<bool> prediction;
            BOOST_CHECK_NO_THROW( l.predict(ex.data, ex.features, prediction); );
        }
    }
}
//*/


//*
// boosting with both weak learners against all datasets ans all test samples defined
// with datasets based on vector
BOOST_AUTO_TEST_CASE_TEMPLATE( test_boosting_V, Learner, weak_types )
{
    AllDatasets<ContainerTypeV> DS;
    AllExamples<ContainerTypeV> EX;
    for (auto ds : DS.dataset)
    {
        Learner wl = Learner(); // weak
        int ens_len [6] = {-2, -1, 0, 1, 2, 10}; // ensemble length
        for (int idx1 = 0; idx1<6; idx1++)
        {
            double share [10] = {-0.5, 0, 0.0001, 0.1, 0.5, 0.75, 0.9, 0.9999, 1.0, 0.5};// {-0.5, 0, 0.0001, 0.1, 0.5, 0.75, 0.9, 0.9999, 1.0, 0.5};
            for (int idx2 = 0; idx2<10; idx2++)
            {
                double share_minor [10] = {-0.5, 0, 0.0001, 0.1, 0.5, 0.75, 0.9, 0.9999, 1.0, 0.5};// {-0.5, 0, 0.0001, 0.1, 0.5, 0.75, 0.9, 0.9999, 1.0, 0.5};
                for (int idx3 = 0; idx3<10; idx3++)
                {
                    // boosting with weak learner of Learner type
                    auto boosting = metric::classification::Boosting<TypeV, Learner, metric::classification::SubsampleRUS<TypeV> >(ens_len[idx1], share[idx2], share_minor[idx3], wl);
                    // test traing
                    BOOST_CHECK_NO_THROW( boosting.train(ds.data, ds.features, ds.response); );
                    // l.train(ds.data, ds.features, ds.response);
                    for (auto ex : EX.dataset)
                    {
                        std::vector<bool> prediction;
                        BOOST_CHECK_NO_THROW( boosting.predict(ex.data, ex.features, prediction); );
                    }
                    // additional parameter loop for Bagging only
                    double share_learner_type [10] = {-0.5, 0, 0.0001, 0.1, 0.5, 0.75, 0.9, 0.9999, 1.0, 0.5};// {-0.5, 0, 0.0001, 0.1, 0.5, 0.75, 0.9, 0.9999, 1.0, 0.5};
                    for (int idx4 = 0; idx4<10; idx4++)
                    {
                        // bagging  of 2 weak learners: types are edmC45 and Learner
                        using WeakLrnVariant = std::variant<metric::classification::edmC45<TypeV>, Learner>;
                        std::vector<WeakLrnVariant> clSet2 = {};
                        WeakLrnVariant weak1 = metric::classification::edmC45<TypeV>(2, 1e-3, 0.25, true);
                        WeakLrnVariant weak2 = Learner();
                        clSet2.push_back(weak1);
                        clSet2.push_back(weak2);
                        auto bagging = metric::classification::Bagging<TypeV, WeakLrnVariant, metric::classification::SubsampleRUS<TypeV> >(ens_len[idx1], share[idx2], share_minor[idx3], {share_learner_type[idx3], 1.0-share_learner_type[idx3]}, clSet2);
                        // test traing
                        BOOST_CHECK_NO_THROW( bagging.train(ds.data, ds.features, ds.response); );
                        // l.train(ds.data, ds.features, ds.response);
                        for (auto ex : EX.dataset)
                        {
                            std::vector<bool> prediction;
                            BOOST_CHECK_NO_THROW( bagging.predict(ex.data, ex.features, prediction); );
                        }
                    }
                }
            }
        }
    }
}
//*/




typedef boost::mpl::list<ContainerTypeV, ContainerTypeD> container_types;

//*
// testing both subsamlers againt satasets, weight vectors and container types
BOOST_AUTO_TEST_CASE_TEMPLATE( test_subsamplers, ContainerType, container_types )
{
    AllDatasets<ContainerType> DS;

    for (auto ds : DS.dataset)
    {
        ContainerType subset = {};
        metric::classification::Subsample<TypeD> sSimple;
        metric::classification::SubsampleRUS<TypeD> sRUS;

        int ens_len [6] = {0, 1, 2, 10}; // ensemble length
        for (int idx1 = 0; idx1<6; idx1++)
        {
            auto all_w = AllWeightVectors(ds.data.size());
            for (auto w : all_w.w)
            {
//                std::vector<double> D_t;           // sample weights
//                double init_val = 1.0 / ds.data.size(); // initial weights
//                for (int i = 0; i < ds.data.size(); i++)
//                {
//                    D_t.push_back(init_val);
//                } // TODO use structure w instead

                double share [10] = {-0.5, 0, 0.0001, 0.1, 0.5, 0.75, 0.9, 0.9999, 1.0, 0.5};// {-0.5, 0, 0.0001, 0.1, 0.5, 0.75, 0.9, 0.9999, 1.0, 0.5};
                for (int idx2 = 0; idx2<10; idx2++)
                {
                    double share_minor [10] = {-0.5, 0, 0.0001, 0.1, 0.5, 0.75, 0.9, 0.9999, 1.0, 0.5};// {-0.5, 0, 0.0001, 0.1, 0.5, 0.75, 0.9, 0.9999, 1.0, 0.5};
                    for (int idx3 = 0; idx3<10; idx3++)
                    {
                        bool replacement [2] = {true, false};
                        for (int idx4 = 0; idx4<2; idx4++)
                        {
                            BOOST_CHECK_NO_THROW (
                                sSimple(ds.data, ds.features, ds.response, w.w, share[idx2], 1 - share_minor[idx3], subset, replacement[idx4]);
                            );
                            BOOST_CHECK_NO_THROW (
                                sRUS(ds.data, ds.features, ds.response, w.w, share[idx2], 1 - share_minor[idx3], subset, replacement[idx4]);
                            );
                        }
                    }
                }
//                double share_overall = 0.75;
//                double share_minor = 0.5;
//                bool replacement = false;
//                sSimple(ds.data, ds.features, ds.response, w.w, share_overall, 1 - share_minor, subset, replacement);
            }
        }

    }
}
//*/
