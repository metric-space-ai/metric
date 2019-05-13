
var cross = require("../lib/index");
var ctypes = cross.ctypes;
var filter = new cross.crossfilter();
var assert = require('assert');
console.log("hello");
var vows = require("vows");


var suite = vows.describe("crossfilter");

suite.addBatch({
    "crossfilter": {
        "adding data": {
        "add array of int to filter": function() {
                var data = new cross.crossfilter();
                data.add([1, 2,
                          2, 3,
                          3, 4]);
                assert.equal(data.size(),6);
//                assert.deepEqual(data.all(), [{a:1, b:2},{a:2, b:3},{a:3, b:4}]);
            },
        

        },
    }
});

suite.export(module);
