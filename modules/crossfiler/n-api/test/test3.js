var cross = require("../lib/index");
var ctypes = cross.ctypes;
var data = new cross.crossfilter();
data.add("a",true);
data.add("b",true);
data.add("c",true);

console.log(data.all());