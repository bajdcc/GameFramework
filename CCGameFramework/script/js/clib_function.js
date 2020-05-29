Function.prototype.bind = function (context, ...args) {
    var _this = this;
    return function () {
        return _this.apply(context, args);
    };
};
return;