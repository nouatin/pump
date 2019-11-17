const rules = {
	states: ['normal', 'warning', 'critical'],
	temperature : {
		min: 21,
		max: 51
	},
	smoke: {
		min: 700,
		max: 900
	},
	vibration:{
		min: 200,
		max: 1000
	}
};
 
const stateTemperature = (val) => {
	if(val < rules.temperature.min) return rules.states[1];
	else if(val > rules.temperature.max) return rules.states[2];
	else return rules.states[0];
}

const stateVibration = (val) => {
	if(val < rules.vibration.min) return rules.states[0];
	else if(val > rules.vibration.max) return rules.states[2];
	else return rules.states[1];
}
const stateSmoke = (val) => {
	if(val > rules.smoke.max) return rules.states[2];
	else if(val < rules.smoke.min) return rules.states[0]
	else return rules.states[1];
}

module.exports.temperature = stateTemperature;
module.exports.vibration = stateVibration;
module.exports.smoke = stateSmoke;
