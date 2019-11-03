const mqtt = require('mqtt');
const client = mqtt.connect('http://localhost:1883');
const fs = require('fs');
const states = require('./states');

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
			let date = new Date();
			date = date.toString();
			msg.date = date.slice(0, 33);
			if((!isNaN(msg.temp)) || (!isNaN(msg.vib)) || (!isNaN(msg.smok))){
				//console.log(r_msg);
				// https://www.tutorialkart.com/nodejs/node-js-append-data-to-file/				
				fs.appendFile('public/data.txt', JSON.stringify(msg) + '\n', (err)=>{
					if(err) console.log(err.message);	
					//console.log("Write");
				});
			}			
			if(!isNaN(msg.temp)) msg.temp = [msg.temp + '*C', states.temperature(msg.temp)] ;
			if(!isNaN(msg.vib)) msg.vib = [msg.vib + '/100', states.vibration(msg.vib)];
			if(!isNaN(msg.smok)) msg.smok = [msg.smok + '/1024', states.smoke(msg.smok)];
			/*Write msg to database .......*/
			// msg;
			// to batabase
		}
		catch(err){
			console.log(err.message);
		} 
	}
});

client.on('disconnect', () =>{
	client.subscribe('info/moteur');
	console.log('Client mqtt disconnect !');
});

module.exports = client;
