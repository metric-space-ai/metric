var cross = require("../lib/index");
var ctypes = cross.ctypes;

//var crossfilter = new cross.crossfilter();
var d3 = require("d3");

var then,
    then2;

// Various generators for our synthetic dataset.
var firstSize = 9e4,
    secondSize = 1e4,
    totalSize = firstSize + secondSize,
    randomDayOfWeek = randomIndex([0, .6, .7, .75, .8, .76, 0]),
    randomHourOfDay = randomIndex([0, 0, 0, 0, 0, 0, 0, .2, .5, .7, .85, .9, .8, .69, .72, .8, .78, .7, .3, 0, 0, 0, 0, 0]),
    randomDate = randomRecentDate(randomDayOfWeek, randomHourOfDay, 13),
    randomAmount = randomLogNormal(2.5, .5),
    randomPayment = function() { return {date: randomDate(), amount: randomAmount()}; };

// Various formatters to show our synthetic distributions.
var x = d3.scale.linear().rangeRound([0, 20]),
    dayNames = ["S", "M", "T", "W", "T", "F", "S"],
    formatNumber = d3.format(",.02r"),
    formatInteger = d3.format("8d"),
    formatDate = d3.time.format("%x"),
    formatDay = function(i) { return dayNames[i]; };

// Create the synthetic records.
then = Date.now();
var paymentRecords = d3.range(totalSize).map(randomPayment);
console.log("Synthesizing " + formatNumber(totalSize) + " records: " + formatNumber(Date.now() - then) + "ms.");

// Slice the records into batches so we can measure incremental add.
var firstBatch = paymentRecords.slice(0, firstSize),
    secondBatch = paymentRecords.slice(firstSize);

// Create the crossfilter and relevant dimensions and groups.
then = then2 = Date.now();
var payments = new cross.crossfilter();
payments.add(firstBatch);
payments.add(secondBatch);
console.log(payments.size());

var all = payments.feature_count();
var amount = payments.dimension(ctypes.double, function(d) { return d.amount; });
then = Date.now();

var k = 0;
var sum = 0;
for( var i = 0; i < 1000; i++) {
    var t = amount.top(10);
    sum += t.length;
    k++;
}
console.log("get top: " + formatNumber((Date.now() - then) / k) + "ms/op.");
console.log(sum);

// Simulates updating the display whenever the filters change.
function updateDisplay() {
 //  dates.all(); // update the date chart
 //  days.all(); // update the day-of-week chart
 //  hours.all(); // update the hour-of-day chart
 //  amounts.all(); // update the amount histogram
 //  all.value(); // update the summary totals
 // date.top(40); // update the payment list
//    console.log("t=",t.length);
  ++k; // count frame rate
}

// Returns a function that returns random index in [0, distribution.length -
// 1], based on the relative values in the specified distribution. Internally,
// the distribution is converted to a normalized cumulative distribution in
// [0, 1], and then a uniform random value is used with bisection.
function randomIndex(distribution) {
  var k = 1 / d3.sum(distribution), s = 0;
  for (var i = 0, n = distribution.length; i < n; ++i) {
    s = (distribution[i] = distribution[i] * k + s);
  }
  return function() {
    return d3.bisectLeft(distribution, Math.random());
  };
}

// Returns a function that returns random values with a log-normal
// distribution, with the specified mean and deviation.
function randomLogNormal(µ, σ) {
  var random = d3.random.normal();
  return function() {
    return Math.exp(µ + σ * random());
  };
}

// Returns a function that returns random dates, built on top of the specified
// random day-of-week and hour-of-day generators. The minutes, seconds, and
// milliseconds of the return dates are uniform random; as is the week of the
// returned date, which is between now and some *weeks* ago.
function randomRecentDate(randomDayOfWeek, randomHourOfDay, weeks) {
  var now = Date.now();
  return function() {
    var d = d3.time.week.offset(new Date(), -Math.floor(Math.random() * weeks));
    d.setDate(d.getDate() + randomDayOfWeek() - d.getDay());
    d.setHours(randomHourOfDay(), Math.random() * 60, Math.random() * 60, Math.random() * 1000);
    return d;
  };
}
