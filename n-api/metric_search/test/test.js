/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Dmitry Vinokurov (c) 2018.
*/

var mtree = require("../lib/index");
var metrics = mtree.metrics;
var assert = require('assert');
var vows = require("vows");


var suite = vows.describe("metric_search");

suite.addBatch({
    "metric_search": {
        "adding data": {
            "add scalar to tree": function() {
                var data = new mtree.metric_search(metrics.euclidian);
		            data.insert(1);
		            data.insert(0);
		            assert.equal(data.get(0),1);
		            assert.equal(data.get(1),0);
                assert.equal(data.size(),2);
            },
            "add vector to tree": function() {
        	      var data = new mtree.metric_search(metrics.euclidian);
        	      data.insert([0,1]);
                data.insert([1,1]);
		            assert.deepEqual(data.get(0),[0,1]);
                assert.deepEqual(data.get(1),[1,1]);
                assert.equal(data.size(),2);
            },
            "add matrix to tree": function() {
        	      var data = new mtree.metric_search(metrics.ssim);
        	      var v1 = [[0,1,2,3,4,5,6,7,8,9,10],
        		              [1,0,0,0,0,0,0,0,0,0,0],
        		              [2,0,0,0,0,0,0,0,0,0,0],
        		              [3,0,0,0,0,0,0,0,0,0,0],
        		              [4,0,0,0,0,0,0,0,0,0,0],
        		              [5,0,0,0,0,0,0,0,0,0,0],
        		              [6,0,0,0,0,0,0,0,0,0,0],
        		              [7,0,0,0,0,0,0,0,0,0,0],
        		              [8,0,0,0,0,0,0,0,0,0,0],
        		              [9,0,0,0,0,0,0,0,0,0,0],
        		              [10,0,0,0,0,0,0,0,0,0,0],
        		              [11,0,0,0,0,0,0,0,0,0,0]
        		             ];
        	      var v2 = [[0,1,2,3,4,5,6,7,8,9,10],
        		              [1,0,0,0,0,0,0,0,0,0,1],
        		              [2,0,0,0,0,0,0,0,0,0,2],
        		              [3,0,0,0,0,0,0,0,0,0,3],
        		              [4,0,0,0,0,0,0,0,0,0,4],
        		              [5,0,0,0,0,0,0,0,0,0,5],
        		              [6,0,0,0,0,0,0,0,0,0,6],
        		              [7,0,0,0,0,0,0,0,0,0,7],
        		              [8,0,0,0,0,0,0,0,0,0,8],
        		              [9,0,0,0,0,0,0,0,0,0,9],
        		              [10,0,0,0,0,0,0,0,0,0,10],
        		              [11,0,0,0,0,0,0,0,0,0,11]
        		             ];

        	      data.insert(v1);
        	      data.insert(v2);
		            assert.deepEqual(data.get(0),v1);
		            assert.deepEqual(data.get(1),v2);
                assert.equal(data.size(),2);
            },
            "erase scalar from tree" : function() {
                var data = new mtree.metric_search(metrics.euclidian);
		            data.insert(1);
                data.insert(2);
                data.insert(3);
                assert.equal(data.size(),3);
                data.erase(2);
                assert.equal(data.size(),2);
                assert.equal(data.get(0),1);
                assert.equal(data.get(2),3);
            },
            "erase vector from tree" : function() {
                var data = new mtree.metric_search(metrics.euclidian);
		            data.insert([0,0,1]);
                data.insert([0,1,0]);
                data.insert([1,0,0]);
                assert.equal(data.size(),3);
                data.erase([0,0,1]);
                assert.equal(data.size(),2);
                assert.deepEqual(data.get(1),[0,1,0]);
            },
            "erase matrix from tree": function() {
        	      var data = new mtree.metric_search(7);
        	      var v1 = [[0,1,2,3,4,5,6,7,8,9,10],
        		              [1,0,0,0,0,0,0,0,0,0,0],
        		              [2,0,0,0,0,0,0,0,0,0,0],
        		              [3,0,0,0,0,0,0,0,0,0,0],
        		              [4,0,0,0,0,0,0,0,0,0,0],
        		              [5,0,0,0,0,0,0,0,0,0,0],
        		              [6,0,0,0,0,0,0,0,0,0,0],
        		              [7,0,0,0,0,0,0,0,0,0,0],
        		              [8,0,0,0,0,0,0,0,0,0,0],
        		              [9,0,0,0,0,0,0,0,0,0,0],
        		              [10,0,0,0,0,0,0,0,0,0,0],
        		              [11,0,0,0,0,0,0,0,0,0,0]
        		             ];
        	      var v2 = [[0,1,2,3,4,5,6,7,8,9,10],
        		              [1,0,0,0,0,0,0,0,0,0,1],
        		              [2,0,0,0,0,0,0,0,0,0,2],
        		              [3,0,0,0,0,0,0,0,0,0,3],
        		              [4,0,0,0,0,0,0,0,0,0,4],
        		              [5,0,0,0,0,0,0,0,0,0,5],
        		              [6,0,0,0,0,0,0,0,0,0,6],
        		              [7,0,0,0,0,0,0,0,0,0,7],
        		              [8,0,0,0,0,0,0,0,0,0,8],
        		              [9,0,0,0,0,0,0,0,0,0,9],
        		              [10,0,0,0,0,0,0,0,0,0,10],
        		              [11,0,0,0,0,0,0,0,0,0,11]
        		             ];

        	      data.insert(v1);
        	      data.insert(v2);
                assert.equal(data.size(),2);
                data.erase(v2);
                assert.equal(data.size(),1);
                assert.deepEqual(data.get(0),v1);
            },
            "insert_if scalar": function() {
                var data = new mtree.metric_search(metrics.euclidian);
                data.insert(1);
                var rc = data.insert_if(10,5);
                assert.equal(rc,1);
                rc = data.insert_if(5,50);
                assert.equal(rc,0);
            },
            "insert_if vector": function() {
                var v1 = [0,0,0];
                var v2 = [0,0,1];
                var v3 = [0,1,0];
                var data = new mtree.metric_search(metrics.euclidian);
                data.insert(v1);
                assert.equal(data.size(),1);
                var rc = data.insert_if(v2,1);
                assert.equal(rc,0);
                assert.equal(data.size(),1);
                rc = data.insert_if(v3,0.5);
                assert.equal(rc,1);
                assert.equal(data.size(),2);
            },
        },
        "nearest neibourghs": {
            topic: function() {
                var data = new mtree.metric_search(metrics.euclidian);
                data.insert(7);
                data.insert(8);
                data.insert(9);
                data.insert(10);
                data.insert(11);
                data.insert(12);
                data.insert(13);
                return data;
            },
            "nn": function(data) {
                var nn = data.nn(8.6);
                assert.equal(nn.data,9);
                nn = data.nn(8.2);
                assert.equal(nn.data,8);
                nn = data.nn(150);
                assert.equal(nn.data,13);
            },
            "knn": function(data) {
                var knn = data.knn(8.6, 4);
                assert.equal(knn.length,4);
                var last_distance = 0.0;
                for(var i = 0; i < 4; i++) {
                    assert.isTrue(knn[i].distance > last_distance);
                    last_distance = knn[i].distance;
                }
            },
            "rnn": function(data) {
                var rnn = data.rnn(9.5, 2);
                assert.equal(rnn.length,4);
                for(var i = 0; i < 4; i++) {
                    assert.isTrue(rnn[i].distance < 2);
                }
            }
        },
        "dev tools": {
            topic: function() {
                var data = new mtree.metric_search(metrics.euclidian);
                data.insert(7);
                data.insert(8);
                data.insert(9);
                data.insert(10);
                data.insert(11);
                data.insert(12);
                data.insert(13);
                return data;
            },
            "size": function(data) {
                assert.equal(data.size(),7);
            },
            "traverse": function(data) {
                var nodes = 0;
                data.traverse(function(n) { nodes++;});
                assert.equal(data.size(), nodes);
            },
            "level_size": function(data) {
                var ls = data.level_size();
                assert.equal(ls,2);
            },
            "print": function(data) {
                console.log(data.print());
                data.print_levels();
            },
            "empty": function() {
                var data = new mtree.metric_search(metrics.euclidian);
                assert.isTrue(data.empty());
                data.insert(1);
                assert.isFalse(data.empty());
            },
            "check_covering": function(data) {
                assert.isTrue(data.check_covering());
            },
            "to_vector": function(data) {
                var v = data.to_vector();
                assert.deepEqual(v, [7,8,9,10,11,12,13]);
            },
            "to_json": function(data) {
                var v = data.to_json();
                console.log(v);
            }
        },
        "clustering": {
            "check_clustering_interface": function() {
                var input = [7,8,9,10,11,12,13];
                var tree = new mtree.metric_search(metrics.euclidian);
                for(var i = 0; i < input.length; i++) {
                    tree.insert(i);
                }
                var distribution = [0.1, 0.2, 0.3, 0.5];
                var IDS = [1,2,3];
                var clusters = tree.clustering(distribution, IDS, input);
                console.log(clusters);
            }
        }
    }
});

suite.export(module);
