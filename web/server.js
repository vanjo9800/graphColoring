var fs = require('fs');
var http = require('http');
var io = require('socket.io');
var express = require('express');
var path = require('path');
var child_process = require('child_process');
var stream = require('stream');
var app = express();

app.get('/css/vis.min.css', function(req, res){
	res.set('Content-Type','text/css');
	var file = fs.readFileSync('css/vis.min.css');
	res.send(file);
});

app.get('/js/vis.min.js', function (req,res){
	res.set('Content-Type','text/javascript');
	var file = fs.readFileSync('js/vis.min.js');
	res.send(file);
});

app.get('/socket.io/socket.io.js', function(req, res){
	res.set('Content-Type','text/javascript');
	var file = fs.readFileSync('socket.io/socket.io.js');
	res.send(file);
});

app.get('/css/bootstrap.min.css', function(req, res){
	res.set('Content-Type','text/css');
	var file = fs.readFileSync('css/bootstrap.min.css');
	res.send(file);
});

app.get('/js/jquery.min.js', function(req, res){
	res.set('Content-Type','text/javascript');
	var file = fs.readFileSync('js/jquery.min.js');
	res.send(file);
});

app.get('/js/bootstrap.min.js', function(req, res){
	res.set('Content-Type','text/javascript');
	var file = fs.readFileSync('js/bootstrap.min.js');
	res.send(file);
});

app.get('/js/render.js', function(req, res){
	res.set('Content-Type','text/javascript');
	var file = fs.readFileSync('js/render.js');
	res.send(file);
});

app.get('/js/colors.js', function(req, res){
	res.set('Content-Type','text/javascript');
	var file = fs.readFileSync('js/colors.js');
	res.send(file);
});

app.get('/images/loading.gif', function(req, res){
	res.set('Content-Type','image/gif');
	var file = fs.readFileSync('images/loading.gif');
	res.send(file);
});

app.get('/images/executing.gif', function(req, res){
	res.set('Content-Type','image/gif');
	var file = fs.readFileSync('images/executing.gif');
	res.send(file);
});

app.get('/', function(req,res){
	res.set('Content-Type','text/html');
	var file = fs.readFileSync('index.html');
	res.send(file);
});

var server = app.listen(8080);

var socketServer = io.listen(server);

socketServer.sockets.on('connection', function(socket){
	socket.on('compute', function(data){

		if(data.algorithm===1){ //Genetic algorithm
			data=data.graph;
			fs.writeFile("temp/task",data,function(error){
				child_process.exec('time mpirun -n 4 ../bin/genetic temp/task temp/answer', function(error,stdout, stderr){
					var coloring=fs.readFileSync('temp/answer').toString();

					socket.emit('solution', coloring);
					stdout={algorithm:1, log:stdout};
					socket.emit('log',stdout);
				});
			});
		}
		if(data.algorithm===2){ //Ant colony optimization
			data=data.graph;
			fs.writeFile("temp/task",data,function(error){
				child_process.exec('time mpirun -n 4 ../bin/antcol temp/task temp/answer', function(error,stdout, stderr){
					var coloring=fs.readFileSync('temp/answer').toString();

					socket.emit('solution', coloring);
					stdout={algorithm:2, log:stdout};
					socket.emit('log',stdout);

				});
			});
		}
		if(data.algorithm===3){ //Simulated annealing
			data=data.graph;
			fs.writeFile("temp/task",data,function(error){
				child_process.exec('time mpirun -n 4 ../bin/simann temp/task', function(error,stdout, stderr){
					var coloring=fs.readFileSync('temp/answer').toString();

					socket.emit('solution', coloring);
					stdout={algorithm:3, log:stdout};
					socket.emit('log',stdout);
				});
			});
		}
		if(data.algorithm===4){ //Tabu search
			data=data.graph;
			fs.writeFile("temp/task",data,function(error){
				child_process.exec('time mpirun -n 4 ../bin/tabus temp/task', function(error,stdout, stderr){
					var coloring=fs.readFileSync('temp/answer').toString();

					socket.emit('solution', coloring);
					stdout={algorithm:4, log:stdout};
					socket.emit('log',stdout);
				});
			});
		}
	});
});
