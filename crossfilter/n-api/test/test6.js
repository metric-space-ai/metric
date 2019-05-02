
var cross = require("../lib/index");
var ctypes = cross.ctypes;
var filter = new cross.crossfilter();
var assert = require('assert');
console.log("hello");
var vows = require("vows");


var suite = vows.describe("crossfilter");

suite.addBatch({
    "crossfilter": {
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
            "all_filtered affected by dimensions": function() {
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

                var dim1 = data.dimension(ctypes.int32, function(d) { return d.a;});
                dim1.filter(1,5);
                var all = data.all_filtered();
                console.log(all);
                assert.equal(all.length,4);
            },
        },
    }
});

suite.export(module);
