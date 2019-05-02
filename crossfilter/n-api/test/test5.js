
var cross = require("../lib/index");
var ctypes = cross.ctypes;
var filter = new cross.crossfilter();
var assert = require('assert');
console.log("hello");
var vows = require("vows");


var suite = vows.describe("crossfilter");

suite.addBatch({
    "crossfilter": {
        "remove data": {
            "remove data with dimension filter": function() {
                var data = new cross.crossfilter();
                data.add([1,2,3,4,5,6,7,8,9,10,11,12]);
                var dim = data.dimension(ctypes.int64,function(d) { return d;});
                                dim.filter(function(d) { return d % 2 == 0;});
//                dim.filter(1,5);
                assert.deepEqual(dim.top(10), [12,10,8,6,4,2]);
                data.remove();
                dim.filter();
                assert.equal(data.size(),6);
                assert.deepEqual(data.all(), [1,3,5,7,9,11]);
            }


        }
    }
});

suite.export(module);
