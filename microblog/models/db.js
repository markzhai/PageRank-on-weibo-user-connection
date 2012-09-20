var redis = require("redis"),
		settings = require('../settings');
    //client = redis.createClient();

var client = redis.createClient(settings.port, settings.host);

client.on("error", function (err) {
  console.log("Error " + err);
});

client.select(15);

module.exports = client;
