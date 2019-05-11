
var cross = require("../lib/index");
var ctypes = cross.ctypes;
var filter = new cross.crossfilter();
var assert = require('assert');
var vows = require("vows");


var suite = vows.describe("crossfilter");

suite.addBatch({
    "crossfilter": {
        "adding data": {
            "add strings to filter": function() {
                var data = new cross.crossfilter();
                data.add("a",true);
                data.add("b",true);
                data.add("c",true);
                assert.deepEqual(data.all(),["a","b","c"]);
                assert.equal(data.size(),3);
            },
            "add number to filter": function() {
                var data = new cross.crossfilter();
                data.add(1);
                data.add(2);
                data.add(3);
                assert.equal(data.size(),3);
                assert.deepEqual(data.all(), [1,2,3]);
            },
            "add floating point number to filter": function() {
                var data = new cross.crossfilter();
                data.add(1.5);
                data.add(2.0);
                data.add(3.2345);
                assert.equal(data.size(),3);
                assert.deepEqual(data.all(), [1.5,2.0,3.2345]);
            },
            "add complex objext to filter": function() {
                var data = new cross.crossfilter();
                data.add({a:1, b:2});
                data.add({a:2, b:3});
                data.add({a:3, b:4});
                assert.equal(data.size(),3);
                assert.deepEqual(data.all(), [{a:1, b:2},{a:2, b:3},{a:3, b:4}]);
            },
            "add array of objects to filter": function() {
                var data = new cross.crossfilter();
                data.add([{a:1, b:2},
                          {a:2, b:3},
                          {a:3, b:4}]);
                assert.equal(data.size(),3);
                assert.deepEqual(data.all(), [{a:1, b:2},{a:2, b:3},{a:3, b:4}]);
            },
            "add array of ints to filter": function() {
                var data = new cross.crossfilter();
                data.add([1,2,3,4]);
                assert.equal(data.size(),4);
                assert.deepEqual(data.all(),[1,2,3,4]);
            },
            "push_back" : function() {
                var data = new cross.crossfilter();
                data.push_back(1);
                assert.equal(data.size(),1);
                assert.deepEqual(data.all(),[1]);
                data.push_back(2);
                assert.equal(data.size(),2);
                assert.deepEqual(data.all(),[1,2]);
            },
            "insert to begin to empty filter": function() {
                var data = new cross.crossfilter();
                data.insert(0,1);
                assert.equal(data.size(),1);
                assert.deepEqual(data.all(),[1]);
            },
            "insert to end to empty filter": function() {
                var data = new cross.crossfilter();
                data.insert(data.size(),1);
                assert.equal(data.size(),1);
                assert.deepEqual(data.all(),[1]);
            },
            "insert to begin to non empty": function() {
                var data = new cross.crossfilter();
                data.add([1,2,3,4]);
                data.insert(0,1);
                assert.equal(data.size(),5);
                assert.deepEqual(data.all(),[1,1,2,3,4]);
            },
            "insert to end to non empty": function() {
                var data = new cross.crossfilter();
                data.add([1,2,3,4]);
                data.insert(data.size(),1);
                assert.equal(data.size(),5);
                assert.deepEqual(data.all(),[1,2,3,4,1]);
            },
            "insert to middle to non empty": function() {
                var data = new cross.crossfilter();
                data.add([1,2,3,4]);
                data.insert(2,1);
                assert.equal(data.size(),5);
                assert.deepEqual(data.all(),[1,2,1,3,4]);
            }

        },
        "duplicates": {
            topic: function() {
                var id = 0;
                var data = new cross.crossfilter(function(v) { return id++;});
                data.add([
                    {a:1, b:2}
                ]);
                return data;
            },
            "add objects with hashing hash function": function(data){
                data.add({a:1, b:2}, true); // allow duplicates
                assert.equal(data.size(),2);
                data.add({a:1, b:2}, false); // disallow duplicates
                assert.equal(data.size(),2);
                data.add({a:1, b:2}, true); // allow duplicates
                assert.equal(data.size(),3);
                data.remove(function(v) { return v.a == 1;});
                data.add({a:1, b:2}, false); // disallow duplicates
                assert.equal(data.size(),1);
            }

        },
        "remove data": {
            "remove fundamental type with predicate": function() {
                var data = new cross.crossfilter();
                data.add([1,2,3,4]);
                data.remove(function(d) { return d % 2 == 0;});
                assert.deepEqual(data.all(), [1,3]);
                assert.equal(data.size(),2);
            },
            "remove complex type with predicate": function() {
                var data = new cross.crossfilter();
                data.add([{a:1, b:2},
                          {a:2, b:3},
                          {a:3, b:4}]);
                data.remove(function(d) { return d.a % 2 == 0;});
                assert.deepEqual(data.all(), [{a:1, b:2}, {a:3, b:4}]);
                assert.equal(data.size(),2);
            },
            "remove data with dimension filter": function() {
                var data = new cross.crossfilter();
                data.add([1,2,3,4,5,6,7,8,9,10,11,12]);
                var dim = data.dimension(ctypes.int64,function(d) { return d;});
                dim.filter(function(d) { return d % 2 == 0;});
                data.remove();
                dim.filter();
                assert.equal(data.size(),6);
                assert.deepEqual(data.all(), [1,3,5,7,9,11]);
            },
            "remove complex data with dimension filter": function() {
                var data = new cross.crossfilter();
                data.add( [
                    {a:1, b:2},
                    {a:2, b:3},
                    {a:3, b:4},
                    {a:4, b:5},
                    {a:6, b:7},
                    {a:8, b:9},
                    {a:10, b:11}
                ]);
                var dim = data.dimension(ctypes.int32,function(d) { return d.a;});
                dim.filter(function(d) { return d % 2 == 0;});
                data.remove();
                dim.filter();
                assert.equal(data.size(),2);
                assert.deepEqual(data.all(), [{ a: 1, b: 2 }, 
                                              { a: 3, b: 4 }]);
            },
            "erase one element": function() {
                var data = new cross.crossfilter();
                data.add([1,2,3,4]);
                data.erase(0);
                assert.deepEqual(data.all(),[2,3,4]);
                data.erase(2);
                assert.deepEqual(data.all(),[2,3]);
            },
            "erase range of elements": function() {
                var data = new cross.crossfilter();
                data.add([1,2,3,4,5,6,7,8]);
                data.erase(2,5);
                assert.deepEqual(data.all(),[1,2,6,7,8]);
            }
        },
        "accessors": {
            topic: function() {
                var data = new cross.crossfilter();
                data.add([1,2,3,4,5,6,7,8]);
                return data;
            },
            "front": function(data) {
                assert.equal(data.front(),1);
            },
            "back": function(data) {
                assert.equal(data.back(),8);
            },
            "at": function(data) {
                for(var i = 0; i < data.size(); i++) {
                    assert.equal(data.at(i),i+1);
                }
            },
        },
        "callback": {
            topic: function() {
                var data = new cross.crossfilter();
                data.add([1,2,3,4,5,6,7,8]);
                return data;
            },
            "callback on data add": function(data) {
                var b = false;
                data.on_change(function(event) { if(event == "dataAdded") b = true;});
                data.add(1);
                assert.isTrue(b);
            },
            "callback on data remove": function(data) {
                var b = false;
                data.on_change(function(event) { if(event == "dataRemoved") b = true;});
                data.erase(1);
                assert.isTrue(b);
            },
            "callback on data filtered": function(data) {
                var b = false;
                data.on_change(function(event) { if(event == "dataFiltered") b = true;});
                var dim = data.dimension(ctypes.int32,function(d) { return d;});
                assert.isFalse(b);
                dim.filter(1);
                assert.isTrue(b);
            }
        },
        "filters": {
            topic: function() {
                var data = new cross.crossfilter();
                data.add([
                    {a:1, b:2},
                    {a:2, b:3},
                    {a:3, b:4},
                    {a:4, b:5},
                    {a:6, b:7},
                    {a:8, b:9},
                    {a:10, b:11},
                    {a:11, b:12}
                ]);
                return data;
            },
            "all() does not affected by filters": function(data) {
                var dim1 = data.dimension(ctypes.int32, function(d) { return d.a;});
                dim1.filter(1,5);
                var all = data.all();
                assert.equal(all.length, 8);
                assert.deepEqual(all, [ {a:1, b:2},
                                        {a:2, b:3},
                                        {a:3, b:4},
                                        {a:4, b:5},
                                        {a:6, b:7},
                                        {a:8, b:9},
                                        {a:10, b:11},
                                        {a:11, b:12}
                                      ]);
                dim1.filter();
            },
            "all_filtered equals all without filters": function(data) {
                var dim1 = data.dimension(ctypes.int32, function(d) { return d.a;});
                var filtered = data.all_filtered();;
                var all = data.all();
                assert.deepEqual(all,filtered);
                dim1.filter();
            },
            "all_filtered affected by dimensions": function(data) {
                var dim1 = data.dimension(ctypes.int32, function(d) { return d.a;});
                dim1.filter(1,5);
                var all = data.all_filtered();
                assert.equal(all.length,4);
                dim1.filter();
            },
            "all_filtered affected by all dimension filters": function(data) {
                var dim1 = data.dimension(ctypes.int32, function(d) {return d.a;});
                var dim2 = data.dimension(ctypes.int32, function(d) {return d.b;});
                dim1.filter(function(d) {return d % 2 == 0;});
                dim2.filter(1,6);
                var all_filtered = data.all_filtered();
                assert.deepEqual(all_filtered, [
                    {a:2, b:3},
                    {a:4, b:5}
                ]);
                dim1.filter();
                dim2.filter();
            },
            "all_filtered affected by mask": function(data) {
                var dim1 = data.dimension(ctypes.int32, function(d) {return d.a;});
                var dim2 = data.dimension(ctypes.int32, function(d) {return d.b;});
                var dim3 = data.dimension(ctypes.int32, function(d) {return d.a;});
                dim1.filter(function(d) {return d % 2 == 0;});
                dim2.filter(1,6);
                var all_filtered = data.all_filtered(dim1);
                assert.deepEqual(all_filtered, [
                    {a:1, b:2},
                    {a:2, b:3},
                    {a:3, b:4},
                    {a:4, b:5}
                ]);
                var all2 = data.all_filtered([dim1,dim2]);
                assert.deepEqual(all2, data.all());
                dim1.filter();
                dim2.filter();

            },
            "is_element_filtered affected by mask": function(data) {
                var dim1 = data.dimension(ctypes.int32, function(d) {return d.a;});
                var dim2 = data.dimension(ctypes.int32, function(d) {return d.b;});
                var dim3 = data.dimension(ctypes.int32, function(d) {return d.a;});
                dim1.filter(function(d) {return d % 2 == 0;});
                dim2.filter(1,6);
                assert.equal(data.is_element_filtered(0,[dim1]), true);
                assert.equal(data.is_element_filtered(0,[dim2]), false);
                assert.equal(data.is_element_filtered(6,[dim2]), true);
                assert.equal(data.is_element_filtered(6,[dim1]), false);
                assert.equal(data.is_element_filtered(7,[dim1,dim2]), true);
                assert.equal(data.is_element_filtered(4), false);

            },
            "is_element_filtered affected by all dimension filters": function(data) {
                var dim1 = data.dimension(ctypes.int32, function(d) {return d.a;});
                var dim2 = data.dimension(ctypes.int32, function(d) {return d.b;});
                dim1.filter(function(d) {return d % 2 == 0;});
                dim2.filter(1,6);
                assert.equal(data.is_element_filtered(1), true);
                assert.equal(data.is_element_filtered(3), true);
                assert.equal(data.is_element_filtered(0), false);
                assert.equal(data.is_element_filtered(4), false);
            },
        },
    },
    "dimension": {
        topic: function() {
            var data = new cross.crossfilter();
            data.add([
                {a:1, b:2},
                {a:2, b:3},
                {a:3, b:4},
                {a:4, b:5},
                {a:6, b:7},
                {a:8, b:9},
                {a:10, b:11},
                {a:11, b:12}
            ]);
            return data;
        },
        "make int64 dimension": function(data) {
            var dim = data.dimension(ctypes.int64, function(d) { return d.a;});
            assert.deepEqual(dim.top(2), [{a:11, b:12}, {a:10, b:11} ]);
        },
        "make int32 dimension": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) { return d.b;});
            assert.deepEqual(dim.top(2), [{a:11, b:12}, {a:10, b:11} ]);
        },
        "make bool dimension": function(data) {
            var dim = data.dimension(ctypes.bool, function(d) { return d % 2 == 0;});
            assert.deepEqual(dim.top(2), [{a:11, b:12}, {a:10, b:11} ]);
        },
        "make double dimension": function(data) {
            var dim = data.dimension(ctypes.double, function(d) { return d.a / 0.1;});
            assert.deepEqual(dim.top(2), [{a:11, b:12}, {a:10, b:11} ]);
        },
        "make string dimension": function(data) {
            var dim = data.dimension(ctypes.string, function(d) { return d.a + "_";});
            assert.deepEqual(dim.top(2), [{a:8, b:9}, {a:6, b:7} ]);
        },
        
        "top": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) { return d.a;});
            assert.deepEqual(dim.top(2),[{a:11, b:12}, {a:10, b:11}]);
        },
        "top offset": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) { return d.a;});
            assert.deepEqual(dim.top(2,2),[{a:8, b:9}, {a:6, b:7}]);
        },
        "bottom": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) { return d.a;});
            assert.deepEqual(dim.bottom(2),[{a:1, b:2}, {a:2, b:3}]);
        },
        "bottom offset": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) { return d.a;});
            assert.deepEqual(dim.bottom(2,2),[{a:3, b:4}, {a:4, b:5}]);
        },
        "filter exact": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) { return d.a;});
            dim.filter(1);
            assert.deepEqual(dim.top(2),[{a:1, b:2}]);
            dim.filter();
        },
        "filter range": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) { return d.a;});
            dim.filter(3,8);
            assert.deepEqual(dim.bottom(3),[{a:3, b:4}, { a:4, b:5}, {a:6, b:7}]);
            dim.filter();
        },
        "filter function": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) { return d.a;});
            dim.filter(function(d) { return d % 2 == 0;});
            assert.deepEqual(dim.bottom(5),[{a:2, b:3}, { a:4, b:5}, {a:6, b:7}, {a:8, b:9}, {a:10, b:11}]);
            dim.filter();
        }
    },
    "iterable dimension": {
        topic: function() {
            var data = new cross.crossfilter();
            data.add([
                {a:1, b:[2,3]},
                {a:2, b:[3,4]},
                {a:3, b:[4,5,6]},
                {a:4, b:[6,7,8]},
                {a:6, b:[9,10]},
                {a:8, b:[11]},
                {a:10, b:[12,13]},
                {a:11, b:[]}
            ]);
            return data;
        },
        "create iterable_dimension int32": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) { return d.b;}, true);
            assert.deepEqual(dim.top(2), [ { a: 10, b: [ 12, 13 ] }, { a: 10, b: [ 12, 13 ] } ]);
        },
        "create iterable_dimension int64": function(data) {
            var dim = data.dimension(ctypes.int64, function(d) { return d.b;}, true);
            assert.deepEqual(dim.top(2), [ { a: 10, b: [ 12, 13 ] }, { a: 10, b: [ 12, 13 ] } ]);
        },
        "create iterable_dimension double": function(data) {
            var dim = data.dimension(ctypes.double, function(d) { return d.b;}, true);
            assert.deepEqual(dim.top(2), [ { a: 10, b: [ 12, 13 ] }, { a: 10, b: [ 12, 13 ] } ]);
        },
        "create iterable_dimension string": function() {
            var data = new cross.crossfilter();
            data.add([
                {a:1, b:["2","3"]},
                {a:2, b:["3","4"]},
                {a:3, b:["4","5","6"]},
                {a:4, b:["6","7","8"]},
                {a:6, b:["9","10"]},
                {a:8, b:["11"]},
                {a:10, b:["12","13"]},
                {a:11, b:[]}
            ]);
            var dim = data.dimension(ctypes.string, function(d) { return d.b;}, true);

            assert.deepEqual(dim.top(2),[ { a: 6, b: [ '9', '10' ] }, { a: 4, b: [ '6', '7', '8' ] } ]);
        }
    },
    "feature": {
        topic: function() {
            var data = new cross.crossfilter();
            data.add([
                {a:1, b:2},
                {a:2, b:3},
                {a:3, b:4},
                {a:4, b:5},
                {a:6, b:7},
                {a:8, b:9},
                {a:10, b:11},
                {a:11, b:12}
            ]);
            return data;
        },
        "filter feature_count": function(data) {
            var feature = data.feature_count();
            assert.equal(feature.size(),1);
            assert.equal(feature.value(), data.size());
            assert.deepEqual(feature.all(), [{key:0, value:8}]);
        },
        "filter feature_sum": function(data) {
            var feature = data.feature_sum(ctypes.int32, function(d) { return 1;});
            var feature2 = data.feature_sum(ctypes.int32, function(d) { return d.a;});
            assert.equal(feature.size(),1);
            assert.equal(feature.value(), data.size());
            assert.deepEqual(feature.all(), [{key:0, value:8}]);
            assert.equal(feature2.size(),1);
            assert.equal(feature2.value(), 45);
            assert.deepEqual(feature2.all(), [{key:0, value:45}]);
        },
        "dimension feture_count": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) {return d.a;});
            var f1 = dim.feature_count(ctypes.bool, function(d) { return d % 2 == 0;});
            assert.equal(f1.size(),2);
            //FIXME: this is strange behaviour, I expect that f1.all returns [{key:false, value:3}, {key:true, value:5}], but
            // the original JavaScript crossfilter do the same.
            assert.deepEqual(f1.all(), [{key:false, value:1}, {key:true, value:7}]);
        },
        "dimension feature_sum": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) {return d.a;});
            var f1 = dim.feature_sum(ctypes.bool, ctypes.int32, function(d) { return d % 2 == 0;}, function(d) {return 1;});
            assert.equal(f1.size(), 2);
            assert.deepEqual(f1.all(),[ { key: false, value: 1 }, { key: true, value: 7 } ]);
        },
        "dimension feature": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) {return d.a;});
            var f1 = dim.feature(ctypes.int32, ctypes.int32,function(d) { return d;},
                                 function(r, rec) { return r + rec.b;},
                                 function(r, rec) { return r - rec.b;},
                                 function() { return 0;});
            assert.equal(f1.size(), 8);
            assert.deepEqual(f1.all(),[ { key: 1, value: 2 },
                                        { key: 2, value: 3 },
                                        { key: 3, value: 4 },
                                        { key: 4, value: 5 },
                                        { key: 6, value: 7 },
                                        { key: 8, value: 9 },
                                        { key: 10, value: 11 },
                                        { key: 11, value: 12 } ] );
        },
        "dimension feature_all_count": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) {return d.a;});
            var f = dim.feature_all_count();
            assert.equal(f.size(),1);
            assert.equal(f.value(), data.size());
        },
        "dimension feature_all_sum": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) {return d.a;});
            var f = dim.feature_all_sum(ctypes.int32, function(d) { return d.a;});
            assert.equal(f.value(),45);
        },
        "dimension feature_all": function(data) {
            var dim = data.dimension(ctypes.int32, function(d) {return d.a;});
            var f = dim.feature_all(ctypes.int32, function(r, rec) { return r + 1;},
                                     function(r, rec) { return r - 1;},
                                     function() { return 0;});
            assert.equal(f.value(), data.size());
        }
    },
    "iterable feature": {
        topic: function() {
            var data = new cross.crossfilter();
            data.add([
                {a:1, b:[2,3]},
                {a:2, b:[3,4]},
                {a:3, b:[4,5,6]},
                {a:4, b:[6,7,8]},
                {a:6, b:[9,10]},
                {a:8, b:[11]},
                {a:10, b:[12,13]},
                {a:11, b:[]}
            ]);

            data.dim1 = data.dimension(ctypes.int64, function(d) { return d.b;}, true);
            data.dim2 = data.dimension(ctypes.int32, function(d) { return d.b;}, true);
            data.dim3 = data.dimension(ctypes.double, function(d) { return d.b;}, true);
            data.dim4 = data.dimension(ctypes.bool, function(d) { return d.b.map(function(v) { return v %2 == 0;});}, true);
            data.dim5 = data.dimension(ctypes.string, function(d) { return d.b.map(function(v) { return String(v);});}, true);
            return data;
        },
        "feature_count int32": function(data) {
            var f1 = data.dim2.feature_count(ctypes.int64, function(d) { return d;});
            var f2 = data.dim2.feature_count(ctypes.int32, function(d) { return d;});
            var f3 = data.dim2.feature_count(ctypes.double, function(d) { return d;});
            var f4 = data.dim2.feature_count(ctypes.bool, function(d) { return d % 2 == 0;});
            var f5 = data.dim2.feature_count(ctypes.string, function(d) { return String(d) ;});
            assert.equal(f1.size(),12);
            assert.equal(f2.size(),12);
            assert.equal(f3.size(),12);
            assert.equal(f4.size(),1);
            assert.equal(f5.size(),8);
        },
        "feature_count double": function(data) {
            var f1 = data.dim3.feature_count(ctypes.int64, function(d) { return d;});
            var f2 = data.dim3.feature_count(ctypes.int32, function(d) { return d;});
            var f3 = data.dim3.feature_count(ctypes.double, function(d) { return d;});
            var f4 = data.dim3.feature_count(ctypes.bool, function(d) { return d % 2 == 0;});
            var f5 = data.dim3.feature_count(ctypes.string, function(d) { return String(d) ;});
            assert.equal(f1.size(),12);
            assert.equal(f2.size(),12);
            assert.equal(f3.size(),12);
            assert.equal(f4.size(),1);
            assert.equal(f5.size(),8);
        },
        "feature_count bool": function(data) {
            var f1 = data.dim4.feature_count(ctypes.int64, function(d) { if(d) return 1; else return 0;});
            var f2 = data.dim4.feature_count(ctypes.int32, function(d) { if(d) return 1; else return 0;});
            var f3 = data.dim4.feature_count(ctypes.double, function(d) {if(d) return 1.0; else return 0.0;});
            var f4 = data.dim4.feature_count(ctypes.bool, function(d) { return d;});
            var f5 = data.dim4.feature_count(ctypes.string, function(d) { if(d) return "1"; else return "0"; ;});
            assert.equal(f1.size(),2);
            assert.equal(f2.size(),2);
            assert.equal(f3.size(),2);
            assert.equal(f4.size(),2);
            assert.equal(f5.size(),2);
        },
        "feature_count string": function(data) {
            var f1 = data.dim5.feature_count(ctypes.int64, function(d) { return parseInt(d,10);});
            var f2 = data.dim5.feature_count(ctypes.int32, function(d) { return parseInt(d,10);});
            var f3 = data.dim5.feature_count(ctypes.double, function(d) { return parseFloat(d);});
            var f4 = data.dim5.feature_count(ctypes.bool, function(d) { return parseInt(d) % 2 == 0;});
            var f5 = data.dim5.feature_count(ctypes.string, function(d) { return d ;});
            assert.equal(f1.size(),4);
            assert.equal(f2.size(),4);
            assert.equal(f3.size(),4);
            assert.equal(f4.size(),1);
            assert.equal(f5.size(),12);
        },

        "feature_count int64": function(data) {
            var f1 = data.dim1.feature_count(ctypes.int64, function(d) { return d;});
            var f2 = data.dim1.feature_count(ctypes.int32, function(d) { return d;});
            var f3 = data.dim1.feature_count(ctypes.double, function(d) { return d;});
            var f4 = data.dim1.feature_count(ctypes.bool, function(d) { return d % 2 == 0;});
            var f5 = data.dim1.feature_count(ctypes.string, function(d) { return String(d) ;});
            assert.deepEqual(f1.all(),[ { key: 2, value: 1 },
                                        { key: 3, value: 2 },
                                        { key: 4, value: 2 },
                                        { key: 5, value: 1 },
                                        { key: 6, value: 2 },
                                        { key: 7, value: 1 },
                                        { key: 8, value: 1 },
                                        { key: 9, value: 1 },
                                        { key: 10, value: 1 },
                                        { key: 11, value: 1 },
                                        { key: 12, value: 1 },
                                        { key: 13, value: 1 } ]);
            assert.deepEqual(f2.all(),[ { key: 2, value: 1 },
                                        { key: 3, value: 2 },
                                        { key: 4, value: 2 },
                                        { key: 5, value: 1 },
                                        { key: 6, value: 2 },
                                        { key: 7, value: 1 },
                                        { key: 8, value: 1 },
                                        { key: 9, value: 1 },
                                        { key: 10, value: 1 },
                                        { key: 11, value: 1 },
                                        { key: 12, value: 1 },
                                        { key: 13, value: 1 } ]);
            assert.deepEqual(f3.all(),[ { key: 2, value: 1 },
                                        { key: 3, value: 2 },
                                        { key: 4, value: 2 },
                                        { key: 5, value: 1 },
                                        { key: 6, value: 2 },
                                        { key: 7, value: 1 },
                                        { key: 8, value: 1 },
                                        { key: 9, value: 1 },
                                        { key: 10, value: 1 },
                                        { key: 11, value: 1 },
                                        { key: 12, value: 1 },
                                        { key: 13, value: 1 } ]);
            assert.deepEqual(f4.all(), [{key:true, value:15}]);
        },
        "feature_sum int64": function(data){
            var f1 = data.dim1.feature_sum(ctypes.int64, ctypes.int64, function(d) { return d;}, function(v) { return 1;});
            var f11 = data.dim1.feature_sum(ctypes.int64, ctypes.int32, function(d) { return d;}, function(v) { return 1;});
            var f12 = data.dim1.feature_sum(ctypes.int64, ctypes.double, function(d) { return d;}, function(v) { return 1;});
            var f2 = data.dim1.feature_sum(ctypes.int32, ctypes.int64, function(d) { return d;}, function(v) { return 1;});
            var f21 = data.dim1.feature_sum(ctypes.int32, ctypes.int32, function(d) { return d;}, function(v) { return 1;});
            var f22 = data.dim1.feature_sum(ctypes.int32, ctypes.double, function(d) { return d;}, function(v) { return 1;});
            var f3 = data.dim1.feature_sum(ctypes.double, ctypes.int64, function(d) { return d;}, function(v) { return 1;});
            var f31 = data.dim1.feature_sum(ctypes.double, ctypes.int32, function(d) { return d;}, function(v) { return 1;});
            var f32 = data.dim1.feature_sum(ctypes.double, ctypes.double, function(d) { return d;}, function(v) { return 1;});
            var f4 = data.dim1.feature_sum(ctypes.bool, ctypes.int64, function(d) { return d %2 == 0;}, function(v) { return 1;});
            var f41 = data.dim1.feature_sum(ctypes.bool, ctypes.int32, function(d) { return d % 2 == 0;}, function(v) { return 1;});
            var f42 = data.dim1.feature_sum(ctypes.bool, ctypes.double, function(d) { return d % 2 == 0;}, function(v) { return 1;});
            var f5 = data.dim1.feature_sum(ctypes.string, ctypes.int64, function(d) { return String(d);}, function(v) { return 1;});
            var f51 = data.dim1.feature_sum(ctypes.string, ctypes.int32, function(d) { return String(d);}, function(v) { return 1;});
            var f52 = data.dim1.feature_sum(ctypes.string, ctypes.double, function(d) { return String(d);}, function(v) { return 1;});
            assert.equal(f11.size(),12);
            assert.equal(f12.size(),12);
            assert.equal(f2.size(),12);
            assert.equal(f21.size(),12);
            assert.equal(f22.size(),12);
            assert.equal(f3.size(),12);
            assert.equal(f31.size(),12);
            assert.equal(f32.size(),12);
            assert.equal(f4.size(),1);
            assert.equal(f41.size(),1);
            assert.equal(f42.size(),1);
            assert.equal(f5.size(),8);
            assert.equal(f51.size(),8);
            assert.equal(f52.size(),8);
        },
        "feature_sum int32": function(data){
            var f1 = data.dim2.feature_sum(ctypes.int64, ctypes.int64, function(d) { return d;}, function(v) { return 1;});
            var f11 = data.dim2.feature_sum(ctypes.int64, ctypes.int32, function(d) { return d;}, function(v) { return 1;});
            var f12 = data.dim2.feature_sum(ctypes.int64, ctypes.double, function(d) { return d;}, function(v) { return 1;});
            var f2 = data.dim2.feature_sum(ctypes.int32, ctypes.int64, function(d) { return d;}, function(v) { return 1;});
            var f21 = data.dim2.feature_sum(ctypes.int32, ctypes.int32, function(d) { return d;}, function(v) { return 1;});
            var f22 = data.dim2.feature_sum(ctypes.int32, ctypes.double, function(d) { return d;}, function(v) { return 1;});
            var f3 = data.dim2.feature_sum(ctypes.double, ctypes.int64, function(d) { return d;}, function(v) { return 1;});
            var f31 = data.dim2.feature_sum(ctypes.double, ctypes.int32, function(d) { return d;}, function(v) { return 1;});
            var f32 = data.dim2.feature_sum(ctypes.double, ctypes.double, function(d) { return d;}, function(v) { return 1;});
            var f4 = data.dim2.feature_sum(ctypes.bool, ctypes.int64, function(d) { return d %2 == 0;}, function(v) { return 1;});
            var f41 = data.dim2.feature_sum(ctypes.bool, ctypes.int32, function(d) { return d % 2 == 0;}, function(v) { return 1;});
            var f42 = data.dim2.feature_sum(ctypes.bool, ctypes.double, function(d) { return d % 2 == 0;}, function(v) { return 1;});
            var f5 = data.dim2.feature_sum(ctypes.string, ctypes.int64, function(d) { return String(d);}, function(v) { return 1;});
            var f51 = data.dim2.feature_sum(ctypes.string, ctypes.int32, function(d) { return String(d);}, function(v) { return 1;});
            var f52 = data.dim2.feature_sum(ctypes.string, ctypes.double, function(d) { return String(d);}, function(v) { return 1;});
            assert.equal(f11.size(),12);
            assert.equal(f12.size(),12);
            assert.equal(f2.size(),12);
            assert.equal(f21.size(),12);
            assert.equal(f22.size(),12);
            assert.equal(f3.size(),12);
            assert.equal(f31.size(),12);
            assert.equal(f32.size(),12);
            assert.equal(f4.size(),1);
            assert.equal(f41.size(),1);
            assert.equal(f42.size(),1);
            assert.equal(f5.size(),8);
            assert.equal(f51.size(),8);
            assert.equal(f52.size(),8);
        },
        "feature_sum double": function(data){
            var f1 = data.dim3.feature_sum(ctypes.int64, ctypes.int64, function(d) { return d;}, function(v) { return 1;});
            var f11 = data.dim3.feature_sum(ctypes.int64, ctypes.int32, function(d) { return d;}, function(v) { return 1;});
            var f12 = data.dim3.feature_sum(ctypes.int64, ctypes.double, function(d) { return d;}, function(v) { return 1;});
            var f2 = data.dim3.feature_sum(ctypes.int32, ctypes.int64, function(d) { return d;}, function(v) { return 1;});
            var f21 = data.dim3.feature_sum(ctypes.int32, ctypes.int32, function(d) { return d;}, function(v) { return 1;});
            var f22 = data.dim3.feature_sum(ctypes.int32, ctypes.double, function(d) { return d;}, function(v) { return 1;});
            var f3 = data.dim3.feature_sum(ctypes.double, ctypes.int64, function(d) { return d;}, function(v) { return 1;});
            var f31 = data.dim3.feature_sum(ctypes.double, ctypes.int32, function(d) { return d;}, function(v) { return 1;});
            var f32 = data.dim3.feature_sum(ctypes.double, ctypes.double, function(d) { return d;}, function(v) { return 1;});
            var f4 = data.dim3.feature_sum(ctypes.bool, ctypes.int64, function(d) { return d %2 == 0;}, function(v) { return 1;});
            var f41 = data.dim3.feature_sum(ctypes.bool, ctypes.int32, function(d) { return d % 2 == 0;}, function(v) { return 1;});
            var f42 = data.dim3.feature_sum(ctypes.bool, ctypes.double, function(d) { return d % 2 == 0;}, function(v) { return 1;});
            var f5 = data.dim3.feature_sum(ctypes.string, ctypes.int64, function(d) { return String(d);}, function(v) { return 1;});
            var f51 = data.dim3.feature_sum(ctypes.string, ctypes.int32, function(d) { return String(d);}, function(v) { return 1;});
            var f52 = data.dim3.feature_sum(ctypes.string, ctypes.double, function(d) { return String(d);}, function(v) { return 1;});
            assert.equal(f11.size(),12);
            assert.equal(f12.size(),12);
            assert.equal(f2.size(),12);
            assert.equal(f21.size(),12);
            assert.equal(f22.size(),12);
            assert.equal(f3.size(),12);
            assert.equal(f31.size(),12);
            assert.equal(f32.size(),12);
            assert.equal(f4.size(),1);
            assert.equal(f41.size(),1);
            assert.equal(f42.size(),1);
            assert.equal(f5.size(),8);
            assert.equal(f51.size(),8);
            assert.equal(f52.size(),8);
        },
        "feature_sum bool": function(data){
            var f1 = data.dim4.feature_sum(ctypes.int64, ctypes.int64, function(d) { return d*1;}, function(v) { return 1;});
            var f11 = data.dim4.feature_sum(ctypes.int64, ctypes.int32, function(d) { return d*1;}, function(v) { return 1;});
            var f12 = data.dim4.feature_sum(ctypes.int64, ctypes.double, function(d) { return d*1;}, function(v) { return 1;});
            var f2 = data.dim4.feature_sum(ctypes.int32, ctypes.int64, function(d) { return d*1;}, function(v) { return 1;});
            var f21 = data.dim4.feature_sum(ctypes.int32, ctypes.int32, function(d) { return d*1;}, function(v) { return 1;});
            var f22 = data.dim4.feature_sum(ctypes.int32, ctypes.double, function(d) { return d*1;}, function(v) { return 1;});
            var f3 = data.dim4.feature_sum(ctypes.double, ctypes.int64, function(d) { return d*1;}, function(v) { return 1;});
            var f31 = data.dim4.feature_sum(ctypes.double, ctypes.int32, function(d) { return d*1;}, function(v) { return 1;});
            var f32 = data.dim4.feature_sum(ctypes.double, ctypes.double, function(d) { return d*1;}, function(v) { return 1;});
            var f4 = data.dim4.feature_sum(ctypes.bool, ctypes.int64, function(d) { return d;}, function(v) { return 1;});
            var f41 = data.dim4.feature_sum(ctypes.bool, ctypes.int32, function(d) { return d;}, function(v) { return 1;});
            var f42 = data.dim4.feature_sum(ctypes.bool, ctypes.double, function(d) { return d ;}, function(v) { return 1;});
            var f5 = data.dim4.feature_sum(ctypes.string, ctypes.int64, function(d) { return String(d);}, function(v) { return 1;});
            var f51 = data.dim4.feature_sum(ctypes.string, ctypes.int32, function(d) { return String(d);}, function(v) { return 1;});
            var f52 = data.dim4.feature_sum(ctypes.string, ctypes.double, function(d) { return String(d);}, function(v) { return 1;});
            assert.equal(f11.size(),2);
            assert.equal(f12.size(),2);
            assert.equal(f2.size(),2);
            assert.equal(f21.size(),2);
            assert.equal(f22.size(),2);
            assert.equal(f3.size(),2);
            assert.equal(f31.size(),2);
            assert.equal(f32.size(),2);
            assert.equal(f4.size(),2);
            assert.equal(f41.size(),2);
            assert.equal(f42.size(),2);
            assert.equal(f5.size(),2);
            assert.equal(f51.size(),2);
            assert.equal(f52.size(),2);
        },
        "feature_sum string": function(data){
            var f1 = data.dim5.feature_sum(ctypes.int64, ctypes.int64, function(d) { return parseInt(d);}, function(v) { return 1;});
            var f11 = data.dim5.feature_sum(ctypes.int64, ctypes.int32, function(d) { return parseInt(d);}, function(v) { return 1;});
            var f12 = data.dim5.feature_sum(ctypes.int64, ctypes.double, function(d) { return parseInt(d);}, function(v) { return 1;});
            var f2 = data.dim5.feature_sum(ctypes.int32, ctypes.int64, function(d) { return parseInt(d);}, function(v) { return 1;});
            var f21 = data.dim5.feature_sum(ctypes.int32, ctypes.int32, function(d) { return parseInt(d);}, function(v) { return 1;});
            var f22 = data.dim5.feature_sum(ctypes.int32, ctypes.double, function(d) { return parseInt(d);}, function(v) { return 1;});
            var f3 = data.dim5.feature_sum(ctypes.double, ctypes.int64, function(d) { return parseFloat(d);}, function(v) { return 1;});
            var f31 = data.dim5.feature_sum(ctypes.double, ctypes.int32, function(d) { return parseFloat(d);}, function(v) { return 1;});
            var f32 = data.dim5.feature_sum(ctypes.double, ctypes.double, function(d) { return parseFloat(d);}, function(v) { return 1;});
            var f4 = data.dim5.feature_sum(ctypes.bool, ctypes.int64, function(d) { return parseInt(d) %2 == 0;}, function(v) { return 1;});
            var f41 = data.dim5.feature_sum(ctypes.bool, ctypes.int32, function(d) { return parseInt(d) % 2 == 0;}, function(v) { return 1;});
            var f42 = data.dim5.feature_sum(ctypes.bool, ctypes.double, function(d) { return parseInt(d) % 2 == 0;}, function(v) { return 1;});
            var f5 = data.dim5.feature_sum(ctypes.string, ctypes.int64, function(d) { return d;}, function(v) { return 1;});
            var f51 = data.dim5.feature_sum(ctypes.string, ctypes.int32, function(d) { return d;}, function(v) { return 1;});
            var f52 = data.dim5.feature_sum(ctypes.string, ctypes.double, function(d) { return d;}, function(v) { return 1;});
            assert.equal(f11.size(),4);
            assert.equal(f12.size(),4);
            assert.equal(f2.size(),4);
            assert.equal(f21.size(),4);
            assert.equal(f22.size(),4);
            assert.equal(f3.size(),4);
            assert.equal(f31.size(),4);
            assert.equal(f32.size(),4);
            assert.equal(f4.size(),1);
            assert.equal(f41.size(),1);
            assert.equal(f42.size(),1);
            assert.equal(f5.size(),12);
            assert.equal(f51.size(),12);
            assert.equal(f52.size(),12);
        },


        "feature int64": function(data){
            var f1 = data.dim1.feature(ctypes.int64, ctypes.int64, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f11 = data.dim1.feature(ctypes.int64, ctypes.int32, function(d) { return d;},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f12 = data.dim1.feature(ctypes.int64, ctypes.double, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f2 = data.dim1.feature(ctypes.int32, ctypes.int64, function(d) { return d;},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f21 = data.dim1.feature(ctypes.int32, ctypes.int32, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f22 = data.dim1.feature(ctypes.int32, ctypes.double, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f3 = data.dim1.feature(ctypes.double, ctypes.int64, function(d) { return d;},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f31 = data.dim1.feature(ctypes.double, ctypes.int32, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f32 = data.dim1.feature(ctypes.double, ctypes.double, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f4 = data.dim1.feature(ctypes.bool, ctypes.int64, function(d) { return d %2 == 0;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f41 = data.dim1.feature(ctypes.bool, ctypes.int32, function(d) { return d % 2 == 0;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f42 = data.dim1.feature(ctypes.bool, ctypes.double, function(d) { return d % 2 == 0;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f5 = data.dim1.feature(ctypes.string, ctypes.int64, function(d) { return String(d);},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f51 = data.dim1.feature(ctypes.string, ctypes.int32, function(d) { return String(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f52 = data.dim1.feature(ctypes.string, ctypes.double, function(d) { return String(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            assert.equal(f11.size(),12);
            assert.equal(f12.size(),12);
            assert.equal(f2.size(),12);
            assert.equal(f21.size(),12);
            assert.equal(f22.size(),12);
            assert.equal(f3.size(),12);
            assert.equal(f31.size(),12);
            assert.equal(f32.size(),12);
            assert.equal(f4.size(),1);
            assert.equal(f41.size(),1);
            assert.equal(f42.size(),1);
            assert.equal(f5.size(),8);
            assert.equal(f51.size(),8);
            assert.equal(f52.size(),8);
        },
        "feature int32": function(data){
            var f1 = data.dim2.feature(ctypes.int64, ctypes.int64, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f11 = data.dim2.feature(ctypes.int64, ctypes.int32, function(d) { return d;},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f12 = data.dim2.feature(ctypes.int64, ctypes.double, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f2 = data.dim2.feature(ctypes.int32, ctypes.int64, function(d) { return d;},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f21 = data.dim2.feature(ctypes.int32, ctypes.int32, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f22 = data.dim2.feature(ctypes.int32, ctypes.double, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f3 = data.dim2.feature(ctypes.double, ctypes.int64, function(d) { return d;},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f31 = data.dim2.feature(ctypes.double, ctypes.int32, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f32 = data.dim2.feature(ctypes.double, ctypes.double, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f4 = data.dim2.feature(ctypes.bool, ctypes.int64, function(d) { return d %2 == 0;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f41 = data.dim2.feature(ctypes.bool, ctypes.int32, function(d) { return d % 2 == 0;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f42 = data.dim2.feature(ctypes.bool, ctypes.double, function(d) { return d % 2 == 0;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f5 = data.dim2.feature(ctypes.string, ctypes.int64, function(d) { return String(d);},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f51 = data.dim2.feature(ctypes.string, ctypes.int32, function(d) { return String(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f52 = data.dim2.feature(ctypes.string, ctypes.double, function(d) { return String(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            assert.equal(f11.size(),12);
            assert.equal(f12.size(),12);
            assert.equal(f2.size(),12);
            assert.equal(f21.size(),12);
            assert.equal(f22.size(),12);
            assert.equal(f3.size(),12);
            assert.equal(f31.size(),12);
            assert.equal(f32.size(),12);
            assert.equal(f4.size(),1);
            assert.equal(f41.size(),1);
            assert.equal(f42.size(),1);
            assert.equal(f5.size(),8);
            assert.equal(f51.size(),8);
            assert.equal(f52.size(),8);
        },
        "feature double": function(data){
            var f1 = data.dim3.feature(ctypes.int64, ctypes.int64, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f11 = data.dim3.feature(ctypes.int64, ctypes.int32, function(d) { return d;},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f12 = data.dim3.feature(ctypes.int64, ctypes.double, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f2 = data.dim3.feature(ctypes.int32, ctypes.int64, function(d) { return d;},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f21 = data.dim3.feature(ctypes.int32, ctypes.int32, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f22 = data.dim3.feature(ctypes.int32, ctypes.double, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f3 = data.dim3.feature(ctypes.double, ctypes.int64, function(d) { return d;},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f31 = data.dim3.feature(ctypes.double, ctypes.int32, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f32 = data.dim3.feature(ctypes.double, ctypes.double, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f4 = data.dim3.feature(ctypes.bool, ctypes.int64, function(d) { return d %2 == 0;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f41 = data.dim3.feature(ctypes.bool, ctypes.int32, function(d) { return d % 2 == 0;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f42 = data.dim3.feature(ctypes.bool, ctypes.double, function(d) { return d % 2 == 0;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f5 = data.dim3.feature(ctypes.string, ctypes.int64, function(d) { return String(d);},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f51 = data.dim3.feature(ctypes.string, ctypes.int32, function(d) { return String(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f52 = data.dim3.feature(ctypes.string, ctypes.double, function(d) { return String(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            assert.equal(f11.size(),12);
            assert.equal(f12.size(),12);
            assert.equal(f2.size(),12);
            assert.equal(f21.size(),12);
            assert.equal(f22.size(),12);
            assert.equal(f3.size(),12);
            assert.equal(f31.size(),12);
            assert.equal(f32.size(),12);
            assert.equal(f4.size(),1);
            assert.equal(f41.size(),1);
            assert.equal(f42.size(),1);
            assert.equal(f5.size(),8);
            assert.equal(f51.size(),8);
            assert.equal(f52.size(),8);
        },
        "feature bool": function(data){
            var f1 = data.dim4.feature(ctypes.int64, ctypes.int64, function(d) { return d*1;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f11 = data.dim4.feature(ctypes.int64, ctypes.int32, function(d) { return d*1;},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f12 = data.dim4.feature(ctypes.int64, ctypes.double, function(d) { return d*1;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f2 = data.dim4.feature(ctypes.int32, ctypes.int64, function(d) { return d*1;},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f21 = data.dim4.feature(ctypes.int32, ctypes.int32, function(d) { return d*1;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f22 = data.dim4.feature(ctypes.int32, ctypes.double, function(d) { return d*1;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f3 = data.dim4.feature(ctypes.double, ctypes.int64, function(d) { return d*1;},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f31 = data.dim4.feature(ctypes.double, ctypes.int32, function(d) { return d*1;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f32 = data.dim4.feature(ctypes.double, ctypes.double, function(d) { return d*1;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f4 = data.dim4.feature(ctypes.bool, ctypes.int64, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f41 = data.dim4.feature(ctypes.bool, ctypes.int32, function(d) { return d ;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f42 = data.dim4.feature(ctypes.bool, ctypes.double, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f5 = data.dim4.feature(ctypes.string, ctypes.int64, function(d) { return String(d);},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f51 = data.dim4.feature(ctypes.string, ctypes.int32, function(d) { return String(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f52 = data.dim4.feature(ctypes.string, ctypes.double, function(d) { return String(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            assert.equal(f11.size(),2);
            assert.equal(f12.size(),2);
            assert.equal(f2.size(),2);
            assert.equal(f21.size(),2);
            assert.equal(f22.size(),2);
            assert.equal(f3.size(),2);
            assert.equal(f31.size(),2);
            assert.equal(f32.size(),2);
            assert.equal(f4.size(),2);
            assert.equal(f41.size(),2);
            assert.equal(f42.size(),2);
            assert.equal(f5.size(),2);
            assert.equal(f51.size(),2);
            assert.equal(f52.size(),2);
        },
        "feature string": function(data){
            var f1 = data.dim5.feature(ctypes.int64, ctypes.int64, function(d) { return parseInt(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f11 = data.dim5.feature(ctypes.int64, ctypes.int32, function(d) { return parseInt(d);},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f12 = data.dim5.feature(ctypes.int64, ctypes.double, function(d) { return parseInt(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f2 = data.dim5.feature(ctypes.int32, ctypes.int64, function(d) { return parseInt(d);},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f21 = data.dim5.feature(ctypes.int32, ctypes.int32, function(d) { return parseInt(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f22 = data.dim5.feature(ctypes.int32, ctypes.double, function(d) { return parseInt(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f3 = data.dim5.feature(ctypes.double, ctypes.int64, function(d) { return parseFloat(d);},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f31 = data.dim5.feature(ctypes.double, ctypes.int32, function(d) { return parseFloat(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f32 = data.dim5.feature(ctypes.double, ctypes.double, function(d) { return parseFloat(d);}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f4 = data.dim5.feature(ctypes.bool, ctypes.int64, function(d) { return parseInt(d) %2 == 0;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f41 = data.dim5.feature(ctypes.bool, ctypes.int32, function(d) { return parseInt(d) % 2 == 0;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f42 = data.dim5.feature(ctypes.bool, ctypes.double, function(d) { return parseInt(d) % 2 == 0;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f5 = data.dim5.feature(ctypes.string, ctypes.int64, function(d) { return d;},  function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f51 = data.dim5.feature(ctypes.string, ctypes.int32, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f52 = data.dim5.feature(ctypes.string, ctypes.double, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            assert.equal(f11.size(),4);
            assert.equal(f12.size(),4);
            assert.equal(f2.size(),4);
            assert.equal(f21.size(),4);
            assert.equal(f22.size(),4);
            assert.equal(f3.size(),4);
            assert.equal(f31.size(),4);
            assert.equal(f32.size(),4);
            assert.equal(f4.size(),1);
            assert.equal(f41.size(),1);
            assert.equal(f42.size(),1);
            assert.equal(f5.size(),12);
            assert.equal(f51.size(),12);
            assert.equal(f52.size(),12);
        },
        "feature_all_count": function(data) {
            var f1 = data.dim1.feature_all_count();
            var f2 = data.dim2.feature_all_count();
            var f3 = data.dim3.feature_all_count();
            var f4 = data.dim4.feature_all_count();
            var f5 = data.dim5.feature_all_count();
            assert.equal(f1.size(),1);
            assert.deepEqual(f1.all(),[{key:0, value:15}]);
            assert.equal(f2.size(),1);
            assert.deepEqual(f2.all(),[{key:0, value:15}]);
            assert.equal(f3.size(),1);
            assert.deepEqual(f3.all(),[{key:0, value:15}]);
            assert.equal(f4.size(),1);
            assert.deepEqual(f4.all(),[{key:0, value:15}]);
            assert.equal(f5.size(),1);
            assert.deepEqual(f5.all(),[{key:0, value:15}]);
        },
        "feature_all_sum int64": function(data) {
            var f1 = data.dim1.feature_all_sum(ctypes.int64, function(r) { return r.a;});
            var f2 = data.dim1.feature_all_sum(ctypes.int32, function(r) { return r.a;});
            var f3 = data.dim1.feature_all_sum(ctypes.double, function(r) { return r.a;});
            assert.equal(f1.value(),67);
            assert.equal(f2.value(),67);
            assert.equal(f3.value(),67);
        },
        "feature_all_sum int32": function(data) {
            var f1 = data.dim2.feature_all_sum(ctypes.int64, function(r) { return r.a;});
            var f2 = data.dim2.feature_all_sum(ctypes.int32, function(r) { return r.a;});
            var f3 = data.dim2.feature_all_sum(ctypes.double, function(r) { return r.a;});
            assert.equal(f1.value(),67);
            assert.equal(f2.value(),67);
            assert.equal(f3.value(),67);
        },
        "feature_all_sum double": function(data) {
            var f1 = data.dim3.feature_all_sum(ctypes.int64, function(r) { return r.a;});
            var f2 = data.dim3.feature_all_sum(ctypes.int32, function(r) { return r.a;});
            var f3 = data.dim3.feature_all_sum(ctypes.double, function(r) { return r.a;});
            assert.equal(f1.value(),67);
            assert.equal(f2.value(),67);
            assert.equal(f3.value(),67);
        },
        "feature_all_sum bool": function(data) {
            var f1 = data.dim4.feature_all_sum(ctypes.int64, function(r) { return r.a;});
            var f2 = data.dim4.feature_all_sum(ctypes.int32, function(r) { return r.a;});
            var f3 = data.dim4.feature_all_sum(ctypes.double, function(r) { return r.a;});
            assert.equal(f1.value(),67);
            assert.equal(f2.value(),67);
            assert.equal(f3.value(),67);
        },
        "feature_all_sum string": function(data) {
            var f1 = data.dim5.feature_all_sum(ctypes.int64, function(r) { return r.a;});
            var f2 = data.dim5.feature_all_sum(ctypes.int32, function(r) { return r.a;});
            var f3 = data.dim5.feature_all_sum(ctypes.double, function(r) { return r.a;});
            assert.equal(f1.value(),67);
            assert.equal(f2.value(),67);
            assert.equal(f3.value(),67);
        },
        "feature_all int64": function(data) {
            var f1 = data.dim1.feature_all(ctypes.int64,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f2 = data.dim1.feature_all(ctypes.int32,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f3 = data.dim1.feature_all(ctypes.double,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            assert.equal(f1.value(),15);
            assert.equal(f2.value(),15);
            assert.equal(f3.value(),15);
        },
        "feature_all int32": function(data) {
            var f1 = data.dim2.feature_all(ctypes.int64,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f2 = data.dim2.feature_all(ctypes.int32,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f3 = data.dim2.feature_all(ctypes.double,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            assert.equal(f1.value(),15);
            assert.equal(f2.value(),15);
            assert.equal(f3.value(),15);
        },
        "feature_all double": function(data) {
            var f1 = data.dim3.feature_all(ctypes.int64,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f2 = data.dim3.feature_all(ctypes.int32,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f3 = data.dim3.feature_all(ctypes.double,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            assert.equal(f1.value(),15);
            assert.equal(f2.value(),15);
            assert.equal(f3.value(),15);
        },
        "feature_all bool": function(data) {
            var f1 = data.dim4.feature_all(ctypes.int64,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f2 = data.dim4.feature_all(ctypes.int32,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f3 = data.dim4.feature_all(ctypes.double,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            assert.equal(f1.value(),15);
            assert.equal(f2.value(),15);
            assert.equal(f3.value(),15);
        },

        "feature_all string": function(data) {
            var f1 = data.dim5.feature_all(ctypes.int64,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f2 = data.dim5.feature_all(ctypes.int32,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            var f3 = data.dim5.feature_all(ctypes.double,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
            assert.equal(f1.value(),15);
            assert.equal(f2.value(),15);
            assert.equal(f3.value(),15);
        },

    }

});

suite.export(module);
