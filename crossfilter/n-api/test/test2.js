var addon=require('../lib/index')
var assert = require("assert")
var vows = require("vows")
var suite = vows.describe("crossfilter")


var cross = new addon.crossfilter();
var ctypes = Object.freeze({"int64":0, "double":1,"int32":2,  "string":3, "bool":4});
console.log(cross);

cross.add({a:1, b:20},true)
cross.add({a:2, b:10},true)

cross.remove(function(d) { return d.a == 2})
console.log(cross.all())
cross.add({a:3, b:30}, true);
var dim = cross.dimension(ctypes.int64, function(v) { return v.b})
cross.add({a:4, b:5},true)
console.log(cross.all())
//console.log(dim.top(2))

//console.log(dim.bottom(2))
dim.filter(function(d) { return d % 2 == 0;});
console.log("-------------")
//console.log(dim.top(20))
var g = dim.feature_count(ctypes.int32, function(d) {return d;});

cross.add({a:4, b:6},true)
console.log("dim.top()=",dim.top(20))
console.log("-------------")
console.log("g.all()=",g.all())
console.log("-------------")
var g1 = dim.feature_sum(ctypes.double, ctypes.int32, function(d) { return d+1;}, function(d) { return d.a + d.b;});
cross.add({a:4, b:6},true)
console.log(g1.all())

var g2 = dim.feature(ctypes.int32, ctypes.int32,function(d) { return d;},
						function(r, rec) { return r + rec.a;}, 
						function(r, rec) { return r - rec.a;}, 
						function() { return 0;})
console.log("g2=",g2.top(10))
		     
console.log(g2.all()) 
console.log(g2.value())
console.log(g2.size())
var g3 = dim.feature_all(ctypes.int32, function(r, rec) { return r + 1}, 
						function(r, rec) { return r - 1;}, 
						function() { return 0;});
console.log(cross.all())
console.log(g3.all())

var g4 = dim.feature_all_count()
console.log(g4.all())
var g5 = dim.feature_all_sum(ctypes.int32, function(d) { return 2; })
console.log(g5.all())

var g6 = cross.feature_count();
console.log(g6.all())
//var o1 = {a:2, b:10};
//var o2 = o1
//console.log(o1 === o2)
//cross.add(20,true);
