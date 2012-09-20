var crypto = require('crypto');
var User = require('../models/user.js');
var Post = require('../models/post.js');

exports.index = function(req, res){
  res.render('index', {
    title: '欢迎来到MicroBlog',
  });
};

exports.hello = function(req, res){
  res.send('The time is ' + new Date().toString());
};

exports.user = function(req, res) {
};

exports.post = function(req, res) {
};

exports.reg = function(req, res) {
  res.render('reg', {
    title: '用户注册',
  });
};

exports.doReg = function(req, res) {
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
};

exports.login = function(req, res) {
  res.render('login', {
    title: '用户登录',
  });
};

exports.doLogin = function(req, res) {
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
};

exports.logout = function(req, res) {
  req.session.user = null;
  req.flash('success', '登出成功');
  res.redirect('/');
};

function checkLogin(req, res, next) {
  if (!req.session.user) {
    req.flash('error', '未登录');
    return res.direct('/login');
  }
  next();
}

function checkNotLogin(req, res, next) {
  if (req.session.user) {
    req.flash('error', '已登录');
    return res.direct('/');
  }
  next();
}
