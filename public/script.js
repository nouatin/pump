
stateStyle = {
	normal:"#90ee90",
	warning: "orange",
	critical: "#ff0000"
};
loadData("/values", "GET", updateFront);
setInterval(()=>{
	loadData("/values", "GET", updateFront);
}, 2000);

function loadData(url, method, cFunction) {

  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      cFunction(this);
    }
  };
  xhttp.open(method, url, true);
  xhttp.send();
}

function updateFront(xhttp) {
	let data = JSON.parse(xhttp.responseText);
	if(!data.error){
		for(key in data){
			let tag = document.getElementById(key);
			if(tag){
				if(key === "date") {
					tag.innerHTML = data[key];
					tag.style.backgroundColor = "";
				}
				else if(key === "flowRate") tag.innerHTML = data[key];
				else {
					tag.innerHTML = data[key][0];
					tag.style.backgroundColor = stateStyle[data[key][1]];
				}
			}
		}
	}
	else {
		let tag = document.getElementById("date");
		tag.innerHTML = "Device not connected !";
		tag.style.backgroundColor = "red";
		
	}
	console.log(data);
} 

function eraseDateReq(){
	if(confirm("Do you want to delete data file ?"))
		loadData("/values", "DELETE", notifyer);
}
function notifyer(xhttp){
	//let data = JSON.parse(xhttp.responseText);
	alert(xhttp.responseText);
}
