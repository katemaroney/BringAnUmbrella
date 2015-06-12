var Timeline = require('pebble-api');

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function(){
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

var key = '04f327092bc0cbc9b2e36e5509626ae8';

function locationSuccess(pos){
  //requests the weather here
  var url = 'https://api.forecast.io/forecast/' + key + '/' + pos.coords.latitude + ',' + pos.coords.longitude;
  xhrRequest(url, 'GET', function(responseText){
    var json = JSON.parse(responseText);
    var data = json.hourly.data;
    
    var hour;
    for (hour in data){
      console.log(data[hour]['temperature']);
      console.log(data[hour]['summary']);
      var date = new Date(data[hour]['time'] * 1000);
      console.log(date.getHours());
      console.log(date.getMinutes());
    }
    var dictionary = {
      'KEY_TEMP' : data[0].temperature,
      'KEY_CONDITONS': data[0].conditions
    };
    var timeline = new Timeline();
    var pin = new Timeline.Pin({
        id: 'test-pin-'+hour,
        time: date,
        layout: new Timeline.Pin.Layout({
            type: "weatherPin",
            title: data[0].conditions,
            subtitle: data[0].temperature,
            tinyIcon: "system://images/TIMELINE_SUN",
            largeIcon: "system://images/TIMELINE_SUN",
            lastUpdated: data[0]['time']
        })
    });
    var userToken;
    Pebble.getTimelineToken(
        function(token) {
            console.log('Got token!' + token);
            userToken = token;
         },
         function(error) {
            console.log('Error getting token: ' + error);
         }
    );
    timeline.sendUserPin(usertoken, pin, function(err) {
        if (err) {
            return console.error(err);
         }

         console.log('Pin is successful!');
    });

    Pebble.sendAppMessage(dictionary, function(e){
      console.log('Weather info sent successfully');
    }, function(e) {
      console.log('Error sending weather info');
    });
    
  });
}

function locationError(err){
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
      locationSuccess,
      locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

Pebble.addEventListener('ready', function(e){
  console.log('WeatherApp is ready');
  getWeather();
});

Pebble.addEventListener('appmessage', function(e){
  console.log('AppMessage recieved');
  getWeather();
});

