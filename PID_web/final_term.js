
//-----------------------------MQTT connection-------------------------//
var host = "broker.emqx.io";
var port = 8083;
var topic1 = "data/motordata";
var topic2 = "data/PIDdata";

// Khoi tao ket noi Paho client
var client = new Paho.MQTT.Client(host, Number(port),"TTP"); // sua lai host; port
client.qos = 0;
client.retained = 1;
client.destinationName = topic1;

// Cac function nay se duoc goi khi co gi do xay ra(mat ket noi, nhan duoc du lieu)
client.onConnectionLost = onConnectionLost;
client.onMessageArrived = onMessageArrived;

// tien hanh ket noi voi client
client.connect({onSuccess:onConnect});




// called when the client connects
function onConnect() {
  // Once a connection has been made, make a subscription and send a message.
  console.log("onConnect to "+topic1);
  client.subscribe(topic1);  

}

// goi ham nay toi khi client duoc ket noi
function Connect() {  
    message = new Paho.MQTT.Message("Hello"); // gui tin nhan da ket noi duoc toi publisher
    message.destinationName = topic1;
    message.qos = 1;
    console.log(message);
    client.send(message);
  }

// ham duoc goi khi tin nhan gui duoc nhan
function onMessageArrived(message) {
    console.log("onMessageArrived topic1:"+message.payloadString);
  }

  // goi ham nay khi mat ket noi voi client
function onConnectionLost(responseObject) {
    if (responseObject.errorCode !== 0) {
      console.log("onConnectionLost:"+responseObject.errorMessage);
    }

  }
// yeu cau nhan du lieu tu broker 
function onMessageArrived(message) { 
  console.log("onMessageArrived topic1:"+message.payloadString);
  var data=message.payloadString;
  console.log(data); // coi data lay ve la dang gi ( array or structure)
  const convert=JSON.parse(data);
  document.getElementById('output_speed').value=convert.Vel;
  document.getElementById('PWM').value=convert.PWMoutput;
  document.getElementById('output_set_point').value=convert.Setpoint;
  document.getElementById("Kp").innerHTML=convert.Kp;
  document.getElementById("Ki").innerHTML=convert.Ki;
  document.getElementById("Kd").innerHTML=convert.Kd;
  
  $(document).ready(function(){
    //new
    $.post("addData_motor.php",$('#form2').serialize(),function(){/*alert("sent") */});
    });


}

// Create mqtt client2
var client2 = new Paho.MQTT.Client(host, Number(port), "ttp2");

client2.destinationName = topic2;
client2.qos = 0;
client2.retained = true;

// set callback handlers
client2.onConnectionLost = onConnectionLost_2;
client2.onMessageArrived = onMessageArrived_2;

// connect the client
client2.connect({onSuccess:onConnect2});


// called when the client connects
function onConnect2() {
  // Once a connection has been made, make a subscription and send a message.
  console.log("onConnect to "+topic2);

}
function publish_client_start()
{
    var P_data = document.getElementById("P_set").value;
    var I_data = document.getElementById("I_set").value;
    var D_data = document.getElementById("D_set").value;
    var Speed_data = document.getElementById("speed_id").value;

    if(P_data=="" || I_data=="" || D_data==""  || Speed_data=="")
   {
    alert("Please enter your input");   
   }
   else{
    
    document.getElementById("output_set_point").innerHTML=Speed_data;
    message = new Paho.MQTT.Message(P_data +"/"+ I_data +"/"+ D_data + "/"+ Speed_data);
    message.destinationName = topic2;
    message.qos = 1;
    console.log(message);
    client2.send(message);
   }
   
}
function publish_client_reset(){
  document.getElementById("selectValue").value=0;
  document.getElementById("P_set").value ="";
  document.getElementById("I_set").value ="";
  document.getElementById("D_set").value ="";
  document.getElementById("speed_id").value=0;
  
}

function publish_client_stop(){
  var P = document.getElementById("Kp").innerHTML
  var I = document.getElementById("Ki").innerHTML
  var D = document.getElementById("Kd").innerHTML
  message = new Paho.MQTT.Message(P +"/"+ I +"/"+ D + "/0");
  message.destinationName = topic2;
  message.qos = 1;
  console.log(message);
  client2.send(message);
}

// called when the client loses its connection
function onConnectionLost_2(responseObject) {
  if (responseObject.errorCode !== 0) {
    console.log("onConnectionLost:"+responseObject.errorMessage);
  }
  
}
// yeu cau nhan du lieu tu broker 
function onMessageArrived_2(message) { 
  console.log("onMessageArrived topic2:"+message.payloadString);
  

}

 