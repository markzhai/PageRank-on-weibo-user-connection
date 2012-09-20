var client = require('./db');

function User(user) {
  this.name = user.name;
  this.password = user.password;
};

function User(name, password) {
  this.name = name;
  this.password = password;
};

module.exports = User;

User.prototype.save = function save(callback) {
  var user = {
    name: this.name,
    password: this.password,
  };
  client.set("microblog.username:" + user.name, user.password, function (err, reply) {
    console.log("user registered " + reply);
    callback(err, user);
  });
};

User.get = function get(username, callback) {
  client.get("microblog.username:" + username, function(err, reply) {
    // reply is null when the key is missing
    if (reply) {
      console.log(reply);
      var user = new User(username, reply);
      callback(err, user);
    } else {
      callback(err, null);
    }
  });
};
