var client = require('./db');
var dateFormat = require('dateformat');

function Post(username, post, time) {
  this.user = username;
  this.post = post;
  if (time) {
    this.time = time;
  } else {
    this.time = new Date();
  }
};
module.exports = Post;

Post.prototype.save = function save(callback) {
  var post = {
    user: this.user,
    post: this.post,
    time: this.time,
  };
  client.incr("next.post.id", function (err, reply) {
    client.lpush('posts.list', reply);
    client.lpush('posts.by.user:' + post.user, reply);
    client.HMSET("post.id:" + reply, post);
    callback(err, post);
  });
};

Post.getAll = function getAll(callback) {
  client.get("next.post.id", function (err, reply) {
    console.log("Reply count: " + reply);
    reply = parseInt(reply);
    var posts = [];
    var count = reply;
    for (var i = reply; i >= 1; i--) {
      client.hgetall("post.id:" + i, function (err, obj) {
        //dateFormat(obj.time, "isoDateTime");
        posts.push(obj);
        //console.dir(obj);
        count--;
        if (count === 0) {
          callback(null, posts);
        };
      });
    }
  });
};

Post.getByUser = function getByUser(username, callback) {
  client.lrange('posts.by.user:' + username, '0', '1',
  function (err, reply) {
    var posts = [];
    var count = reply.length;
    //console.log(reply);
    reply.forEach(function(pid) {
      client.hgetall("post.id:" + pid, function (err, obj) {
        posts.push(obj);
        //console.dir(obj);
        count--;
        if (count === 0) {
          callback(null, posts);
        };
      });
    });
  });
};
