Pebble.addEventListener('showConfiguration', function() {
   var url = 'http://jackv24.github.io/Importance-Pebble-Watchface/';
   
   Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) { 
   var configData = JSON.parse(decodeURIComponent(e.response));
   
   console.log('COnfiguration page returned: ' + JSON.stringify(configData));
   
   if(configData.batteryBackgroundColor) {
      Pebble.sendAppMessage({
         secondsToggle: configData.secondsToggle ? 1 : 0,
         
         batteryBackgroundColor: parseInt(configData.batteryBackgroundColor, 16),
         batteryForegroundColor: parseInt(configData.batteryForegroundColor, 16),
         
         timeColor: parseInt(configData.timeColor, 16),
         dateColor: parseInt(configData.dateColor, 16),
         backgroundColor: parseInt(configData.backgroundColor, 16)
      }, function() {
         console.log('Send successful!');
      }, function() {
         console.log('Send failed!');
         console.log("Message with transactionId="+e.data.transactionId+ " failed: "+JSON.stringify(e));
      });
   }
});