var crypto = require('crypto');
var User = require('../models/user.js');
var Post = require('../models/post.js');
var Rank = require('../models/rank.js');

module.exports = function(app) {
  app.get('/', function(req, res) {
    Post.getAll(function(err, posts) {
      if (err) {
        posts = [];
      }
      res.render('index', {
        title: '首页',
        posts: posts,
      });
    });
  });

  app.get('/hello', function(req, res){
    res.send('The time is ' + new Date().toString());
  });

  app.get('/u/:user', function(req, res) {
    User.get(req.params.user, function(err, user) {
      if (!user) {
        req.flash('error', '用戶不存在');
        return res.redirect('/');
      }
      Post.getByUser(user.name, function(err, posts) {
        if (err) {
          req.flash('error', err);
          return res.redirect('/');
        }
        res.render('user', {
          title: user.name,
          posts: posts,
        });
      });
    });
  });
  
  app.post('/post', checkLogin);
  app.post('/post', function(req, res) {
    var currentUser = req.session.user;
    var post = new Post(currentUser.name, req.body.post);
    post.save(function(err) {
      if (err) {
        req.flash('error', err);
        return res.redirect('/');
      }
      req.flash('success', '发布成功');
      res.redirect('/u/' + currentUser.name);
    });
  });

  app.get('/rank', function(req, res) {
    var rank;
    Rank.get(1, 10, function(err, rankingChart) {
      if (err) {
        req.flash('error', err);
        return res.redirect('/');
      }
      res.render('rank', {
        title: '影响力排行',
        rank: rankingChart
      });
    });
  });
  
  app.get('/reg', checkNotLogin);
  app.get('/reg', function(req, res) {
    res.render('reg', {
      title: '用户注册',
    });
  });

  app.post('/reg', checkNotLogin);
  app.post('/reg', function(req, res) {
    // 检查密码
    if (req.body['password-repeat'] != req.body['password']) {
      req.flash('error', '两次输入的密码不一致');
      return res.redirect('/reg');
    }
    // 生成密码的MD5
    var md5 = crypto.createHash('md5');
    var password = md5.update(req.body.password).digest('base64');
    
    var newUser = new User(req.body.username, password);
    
    // 检查用户名是否存在
    User.get(newUser.name, function(err, user) {
      if (user)
        err = 'Username already exists.';
      if (err) {
        req.flash('error', err);
        return res.redirect('/reg');
      }
      // 如果不存在则新增用戶
      newUser.save(function(err) {
        if (err) {
          req.flash('error', err);
          return res.redirect('/reg');
        }
        req.session.user = newUser;
        req.flash('success', '注册成功');
        res.redirect('/');
      });
    });
  });

  app.get('/login', checkNotLogin);
  app.get('/login', function(req, res) {
    res.render('login', {
      title: '用户登录',
    });
  });

  app.post('/login', checkNotLogin);
  app.post('/login', function(req, res) {
    var md5 = crypto.createHash('md5');
    var password = md5.update(req.body.password).digest('base64');
    
    User.get(req.body.username, function(err, user) {
      if (!user) {
        req.flash('error', '用户不存在');
        return res.redirect('/login');
      }
      if (user.password != password) {
        req.flash('error', '用户密码错误');
        return res.redirect('/login');
      }
      req.session.user = user;
      req.flash('success', '登入成功');
      res.redirect('/');
    });
  });

  app.get('/logout', checkLogin);
  app.get('/logout', function(req, res) {
    req.session.user = null;
    req.flash('success', '登出成功');
    res.redirect('/');
  });
  
  app.get('/:undefined', function(req, res) {
    res.render('error', {
      title: '错误',
    });
  });
};

function checkLogin(req, res, next) {
  if (!req.session.user) {
    req.flash('error', '未登录');
    return res.redirect('/login');
  }
  next();
}

function checkNotLogin(req, res, next) {
  if (req.session.user) {
    req.flash('error', '已登录');
    return res.redirect('/');
  }
  next();
}
