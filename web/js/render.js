var socket = io.connect();

function startNetwork(nodesArray, edgesArray) {

	nodes = new vis.DataSet(nodesArray);
	edges = new vis.DataSet(edgesArray);
	var container = document.getElementById("graph");
	var data = {
		nodes: nodes,
		edges: edges
	};

	var options = {
		"edges": {
			"smooth": {
				"forceDirection": "none",
				"roundness": 0
			}
		},
		"physics": {
			"enabled": true,
			/*"hierarchicalRepulsion": {
				"springLength": 100,
				"centralGravity" : 0,
				"nodeDistance": 335,
				"damping" : 1
			},
			"minVelocity": 0.75,*/
			"solver": "forceAtlas2Based",
			"timestep": 0.5,
			"adaptiveTimestep": true
		}
	}

	$("#notChosen").css("display","none");
	$("#loading").css("display","block");
	network = new vis.Network(container, data, options);
	
	network.on("stabilizationProgress", function(params) {
		var progress = Math.round((params.iterations/params.total)*100.0);
		$("#loadingPercentage").text("Loading "+progress+"%");
	});
	network.once("stabilizationIterationsDone", function() {
		$("#loading").css("display","none");
		$("#graph").css("display","block");
		$("#colornumber").css("display","block");
	});
}

var graphFile;
function loadGraph() {
	var file = $("#graphfile")[0].files[0]; 
	if (file) {
		$("#notChosen").css("display","block");
		$("#graph").css("display","none");
		$("#colornumber").css("display","none");
		$("#loading").css("display","none");
		var reader = new FileReader();
		reader.onload = function(content) {
			var contents = content.target.result;
			graphFile=contents;

			var nodesArray = [];
			var edgesArray = [];
			var vertexNumber;

			var inputArray = contents.split('\n');
			for(var i=0; i<inputArray.length; i++)
			{
				var newArr = inputArray[i].split(" ");
				for(var j =0; j< newArr.length; j++)
				{
					if(newArr[0] == 'p'){
						vertexNumber=parseInt(newArr[2]);
					}
					if(newArr[0] == 'e'){
						edgesArray.push({from: newArr[j+1]-1,to: newArr[j+2]-1});
						continue;
					}
				}

			}

			for(var i=0; i<vertexNumber; i++) 
			{
				nodesArray.push({id: i, label: "Node "+(i+1),font: {
					color: "white"
				} , color: "black"});
			}

			startNetwork(nodesArray,edgesArray);
		}
		reader.readAsText(file);
	} else { 
		alert("Failed to load file");
	}
}

function getContrastYIQ(hex){
	hex = hex.replace('#','');
	r = parseInt(hex.substring(0,2), 16);
	g = parseInt(hex.substring(2,4), 16);
	b = parseInt(hex.substring(4,6), 16);
	var yiq = ((r*299)+(g*587)+(b*114))/1000;
	return (yiq >= 128) ? 'black' : 'white';
}
function colorGraph(colored){
	var contents=graphFile;

	var nodesArray = [];
	var edgesArray = [];
	var vertexNumber;

	var inputArray = contents.split('\n');
	for(var i=0; i<inputArray.length; i++)
	{
		var newArr = inputArray[i].split(" ");
		for(var j =0; j< newArr.length; j++)
		{
			if(newArr[0] == 'p'){
				vertexNumber=parseInt(newArr[2]);
			}
			if(newArr[0] == 'e'){
				edgesArray.push({from: newArr[j+1]-1,to: newArr[j+2]-1});
				continue;
			}
		}

	}

	for(var i=0; i<vertexNumber; i++) 
	{
		var fontcolor = getContrastYIQ(colors[colored[i*2]]);
		nodesArray.push({id: i, label: "Node "+(i+1),font: {
			color: fontcolor
		} , color: colors[colored[i*2]]});
	}
	$("#numberColors").text(colored[vertexNumber*2]);

	startNetwork(nodesArray,edgesArray);
	$("#executing").css("display","none");
}

function computeAnswer(){
	$("#executing").css("display","block");
	socket.emit('compute',graphFile);
}

socket.on('solution',function(coloring){
	colorGraph(coloring);
});

socket.on('log',function(log){
	$("#logHeader").text((new Date()).toString());
	log=log.toString();
	$("#logCode").text(log);
});

$(document).ready(function(){
	$('#loadgraph').on("click",function(){
		return function(){loadGraph();};
	}());
	$('#compute').on("click",function(){
		return function(){ computeAnswer();};
	}());
	$("#graph").css("width",$("#graph").parent().width());
	$("#graph").css("height",$("#graph").parent().height());
	$("#graph").css("border","1px solid black");
});
