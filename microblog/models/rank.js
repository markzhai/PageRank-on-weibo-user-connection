var client = require('./db');

function Rank(username, rank, rankValue) {
  this.user = username;
  this.rank = rank;
  this.rankValue = rankValue;
};
module.exports = Rank;

Rank.get = function get(page, count, callback) {
  client.select(0);
  var start = (page - 1) * count;
  var end = page * count - 1;
  var rank = [];
  client.zrevrange("pagerank", start, end, "WITHSCORES", 
  function (err, reply) {
    for (var i = 0; i < reply.length; i = i + 2) {
      var rankItem = new Rank(reply[i], i / 2 + 1, reply[i+1]);
      rank.push(rankItem);
      console.dir(rankItem);
    }
    client.select(15);
    callback(err, rank);
  });
};
