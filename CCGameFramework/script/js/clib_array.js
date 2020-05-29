Array.prototype.push = function (...args) {
    var len = this.length || 0;
    for (var i in args) {
        if (args.hasOwnProperty(i))
            this[len++] = args[i];
    }
    this.length = len;
    return this.length;
};
Array.prototype.slice = function (n) {
    n = n || 0;
    var len = this.length || 0;
    if (len <= n)
        return Array();
    var arr = Array();
    var j = 0;
    for (var i = n; i < len; i++) {
        if (this.hasOwnProperty(i))
            arr[j++] = this[i];
    }
    arr.length = j;
    return arr;
};
Array.prototype.concat = function (...args) {
    var _this = this;
    var unbox = (arr, o) => {
        if (o === null || typeof o === "undefined")
            return;
        switch (typeof o) {
            case "number":
                return arr.push(Number(o));
            case "string":
                return arr.push(String(o));
            case "boolean":
                return arr.push(Boolean(o));
            case "function":
                return arr.push(Function(o));
            case "object":
                if (o instanceof Array)
                    return arr.push(...o);
                return arr.push(o);
            default:
                break;
        }
    };
    var unbox2 = (arr, o) => {
        if (o instanceof Array)
            return arr.push(...o);
        return arr.push(o);
    };
    var arr = Array();
    unbox(arr, _this);
    for (var i in args) {
        if (args.hasOwnProperty(i))
            unbox2(arr, args[i]);
    }
    return arr;
};
Array.prototype.map = function (f) {
    var arr = Array(this.length);
    var _this = this instanceof Array ? this : [...this];
    for (var i in _this) {
        arr[i] = f(_this[i]);
    }
    return arr;
};
Array.prototype.filter = function (f) {
    var arr = Array();
    var _this = this instanceof Array ? this : [...this];
    for (var i in _this) {
        if (f(_this[i]))
            arr.push(_this[i]);
    }
    return arr;
};
Array.prototype.reduce = function (f, init) {
    var _this = this instanceof Array ? this : [...this];
    if (!_this.length)
        return init;
    if (_this.length === 1)
        return this[0];
    var acc;
    if (typeof init === 'undefined') {
        var first = true;
        for (var i in _this) {
            if (first) {
                first = false;
                acc = _this[i];
                continue;
            }
            acc = f(acc, _this[i]);
        }
        return acc;
    } else {
        acc = init;
        for (var i in _this) {
            acc = f(acc, _this[i]);
        }
        return acc;
    }
};
Array.prototype.fill = function (init) {
    var _this = this instanceof Array ? this : [...this];
    if (!_this.length)
        return _this;
    var len = _this.length || 0;
    for (var i = 0; i < len; i++) {
        _this[i] = init;
    }
    return _this;
};
Array.prototype.join = function (s) {
    var str = this.reduce((a, b) => a + s + b);
    return typeof str !== "undefined" ? ("" + str) : "";
};
Array.prototype.toString = function (hint) {
    if (!hint)
        return "" + this.join(",");
    return "[" + this.map(x => typeof x === "object" ? x.toString(hint) : x).join(", ") + "]";
};
return;