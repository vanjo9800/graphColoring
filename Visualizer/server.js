var fs = require('fs');
var http = require('http');
var io = require('socket.io');
var express = require('express');
var path = require('path');
var child_process = require('child_process');
var stream = require('stream');
var app = express();

app.get('/vis.min.css', function(req, res){
	res.set('Content-Type','text/css');
	var file = fs.readFileSync('vis.min.css');
	res.send(file);
});

app.get('/vis.min.js', function (req,res){
	res.set('Content-Type','text/javascript');
	var file = fs.readFileSync('vis.min.js');
	res.send(file);
});

app.get('/socket.io/socket.io.js', function(req, res){
	res.set('Content-Type','text/javascript');
	var file = fs.readFileSync('socket.io/socket.io.js');
	res.send(file);
});

app.get('/bootstrap/bootstrap.min.css', function(req, res){
	res.set('Content-Type','text/css');
	var file = fs.readFileSync('bootstrap/bootstrap.min.css');
	res.send(file);
});

app.get('/bootstrap/jquery.min.js', function(req, res){
	res.set('Content-Type','text/javascript');
	var file = fs.readFileSync('bootstrap/jquery.min.js');
	res.send(file);
});

app.get('/bootstrap/bootstrap.min.js', function(req, res){
	res.set('Content-Type','text/javascript');
	var file = fs.readFileSync('bootstrap/bootstrap.min.js');
	res.send(file);
});

app.get('/render.js', function(req, res){
	res.set('Content-Type','text/javascript');
	var file = fs.readFileSync('render.js');
	res.send(file);
});

app.get('/colors.js', function(req, res){
	res.set('Content-Type','text/javascript');
	var file = fs.readFileSync('colors.js');
	res.send(file);
});

app.get('/loading.gif', function(req, res){
	res.set('Content-Type','image/gif');
	var file = fs.readFileSync('loading.gif');
	res.send(file);
});

app.get('/executing.gif', function(req, res){
	res.set('Content-Type','image/gif');
	var file = fs.readFileSync('executing.gif');
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

		if(data.device===1){ //Parallella
			data=data.graph;
			fs.writeFile("../antColony/task",data,function(error){
				child_process.exec('cd ../antColony; time bin/master task answer', function(error,stdout, stderr){
					var coloring=fs.readFileSync('../antColony/answer').toString();
				
					socket.emit('solution', coloring);
					
					stdout={device: 1,log:stdout};
					socket.emit('log',stdout);
				});
			});
		}
		if(data.device===2){ //ANTCOL i3
			data=data.graph;
			fs.writeFile("../antColony/task",data,function(error){
				child_process.exec('scp ../antColony/task graph@192.168.0.244:~; ssh graph@192.168.0.244 "time mpirun -n 4 bin/antcol task answer.txt > log.txt"; scp graph@192.168.0.244:~/answer.txt ../antColony/answer; scp graph@192.168.0.244:~/log.txt ../antColony/log', function(error,stdout, stderr){
					var coloring=fs.readFileSync('../antColony/answer').toString();
					var log=fs.readFileSync('../antColony/log').toString();

					socket.emit('solution', coloring);
					log={device:2, log:log};
					socket.emit('log',log);
				});
			});
		}
		if(data.device===3){ //ANTCOL i7
			data=data.graph;
			fs.writeFile("../antColony/task",data,function(error){
				child_process.exec('scp -P 1515 ../antColony/task innofair@93.123.116.190:~; ssh -p 1515 innofair@93.123.116.190 "time mpirun -n 8 bin/antcol task answer.txt > log.txt"; scp -P 1515 innofair@93.123.116.190:~/answer.txt ../antColony/answer; scp -P 1515 innofair@93.123.116.190:~/log.txt ../antColony/log', function(error,stdout, stderr){
					var coloring=fs.readFileSync('../antColony/answer').toString();
					var log=fs.readFileSync('../antColony/log').toString();

					socket.emit('solution', coloring);
					log={device:3, log:log};
					socket.emit('log',log);
				});
			});
		}
		if(data.device===4){ //Genetic i7
			data=data.graph;
			fs.writeFile("../antColony/task",data,function(error){
				child_process.exec('scp -P 1515 ../antColony/task innofair@93.123.116.190:~; ssh -p 1515 innofair@93.123.116.190 "time mpirun -n 8 bin/genetic task answer.txt > log.txt"; scp -P 1515 innofair@93.123.116.190:~/answer.txt ../antColony/answer; scp -P 1515 innofair@93.123.116.190:~/log.txt ../antColony/log', function(error,stdout, stderr){
					var coloring=fs.readFileSync('../antColony/answer').toString();
					var log=fs.readFileSync('../antColony/log').toString();

					socket.emit('solution', coloring);
					log={device:4, log:log};
					socket.emit('log',log);
				});
			});
		}
	});
});
