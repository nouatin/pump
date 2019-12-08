const express = require('express');
const app = express();
const bodyParser = require('body-parser');
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true}));
app.use(express.static('public'));
const mqtt = require('mqtt');
const client = mqtt.connect('http://mosquitto:1883');
//const client = mqtt.connect('http://localhost:1883');
const fs = require('fs');
const states = require('./modules/states');
const port = process.env.PORT | 80;
var r_msg;

app.get('/', function(req, res){
	res.sendFile(__dirname + "/public/" + "index.html");
});

app.get('/values', function(req, res){
	//onsole.log(r_msg);
	if(r_msg === undefined) return res.send({error:true});
	res.send(r_msg);
});

app.delete('/values', (req, res)=>{
	fs.unlink('public/data.txt', (err)=>{
		if(err) return res.send("File not found, Error occured");
		res.send("File successfully erased !");
	});
});

client.on('connect', () =>{
	client.subscribe('info/moteur');
	console.log('Client mqtt connected !');
});

client.on('message', (topic, message) =>{
	let _topic = topic.toString();
	//console.log(_topic);
	if(message != "undefined"){
	    //console.log('%s', message);
	    try{			
			let msg = JSON.parse(message.toString());	
			let d = "";
			d = new Date();
			let date = d.toDateString() + ", " + d.toLocaleTimeString();
			//date = date.toString();
			msg.date = date;	//.slice(0, 33);
			if((!isNaN(msg.temp)) || (!isNaN(msg.vib)) || (!isNaN(msg.smok) || (!isNaN(msg.flowRate)))){					
				fs.appendFile('public/data.txt', JSON.stringify(msg) + '\n', (err)=>{
					if(err) console.log(err.message);	
				});
			}			
			if(!isNaN(msg.temp)) msg.temp = [msg.temp + '℃', states.temperature(msg.temp)] ;
			if(!isNaN(msg.vib)) msg.vib = [msg.vib, states.vibration(msg.vib)];
			if(!isNaN(msg.smok)) msg.smok = [msg.smok + '/1024', states.smoke(msg.smok)];
			if(!isNaN(msg.flowRate)) msg.flowRate = msg.flowRate + "mL/s";
			msg.error = false;
			r_msg = msg;	
		}
		catch(err){
			console.log(err.message);
		} 
	}
});

client.on('disconnect', () =>{
	//client.subscribe('info/moteur');
	console.log('Client mqtt disconnect !');
});

app.listen(port, function() {
	console.log(`server is listening on port ${port} `);
});
