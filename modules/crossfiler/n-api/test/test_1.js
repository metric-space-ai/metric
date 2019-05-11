var cross = require("../lib/index");
var ctypes = cross.ctypes;
var filter = new cross.crossfilter();
var assert = require('assert');
console.log("hello");
var vows = require("vows");

var data = new cross.crossfilter();

data.add([1,2,3,4]);
data.erase(0);
assert.deepEqual(data.all(),[2,3,4]);
data.erase(data.size());
assert.deepEqual(data.all(),[2]);

