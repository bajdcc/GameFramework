Array.prototype.push = function(...args) {
    var len = this.length || 0;
    for (var i in args) {
        if (args.hasOwnProperty(i))
            this[len++] = args[i];
    }
    this.length = len;
    return this.length;
};
Array.prototype.slice = function(begin, end) {
    end = (typeof end !== 'undefined') ? end : (this.length || 0);
    var i, cloned = [],
        size, len = this.length;
    var start = begin || 0;
    start = (start >= 0) ? start : Math.max(0, len + start);
    var upTo = (typeof end == 'number') ? Math.min(end, len) : len;
    if (end < 0) {
        upTo = len + end;
    }
    size = upTo - start;
    if (size > 0) {
        cloned = new Array(size);
        for (i = 0; i < size; i++) {
            if (this.hasOwnProperty(start + i))
                cloned[i] = this[start + i];
        }
    }
    return cloned;
};
Array.prototype.concat = function(...args) {
    var _this = this;
    var unbox = (arr, o) => {
        if (o === null || o === undefined)
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
Array.prototype.map = function(f) {
    var arr = Array(this.length || 0);
    var _this = this instanceof Array ? this : [...this];
    for (var i in _this) {
        arr[i] = f(_this[i], i);
    }
    return arr;
};
Array.prototype.filter = function(f) {
    var arr = Array();
    var _this = this instanceof Array ? this : [...this];
    for (var i in _this) {
        if (f(_this[i]))
            arr.push(_this[i]);
    }
    return arr;
};
Array.prototype.reduce = function(f, init) {
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
Array.prototype.fill = function(init) {
    var _this = this instanceof Array ? this : [...this];
    if (!_this.length)
        return _this;
    var len = _this.length || 0;
    for (var i = 0; i < len; i++) {
        _this[i] = init;
    }
    return _this;
};
Array.prototype.join = function(s) {
    var str = this.reduce((a, b) => a + s + b);
    return typeof str !== "undefined" ? ("" + str) : "";
};
Array.prototype.forEach = function(f) {
    var arr = Array(this.length);
    var _this = this instanceof Array ? this : [...this];
    for (var i in _this) {
        f(_this[i], i);
    }
};
Array.isArray = function(arr) {
    return Object.prototype.toString.call(arr) == "[object Array]";
};
(function() {
    Array.prototype.sort = mergeSort;

    function mergeSort(compare) {

        var length = this.length,
            middle = Math.floor(length / 2);

        if (!compare) {
            compare = function(left, right) {
                if (left < right)
                    return -1;
                if (left == right)
                    return 0;
                else
                    return 1;
            };
        }

        if (length < 2)
            return this;

        return merge(
            this.slice(0, middle).mergeSort(compare),
            this.slice(middle, length).mergeSort(compare),
            compare
        );
    }

    function merge(left, right, compare) {

        var result = [];

        while (left.length > 0 || right.length > 0) {
            if (left.length > 0 && right.length > 0) {
                if (compare(left[0], right[0]) <= 0) {
                    result.push(left[0]);
                    left = left.slice(1);
                } else {
                    result.push(right[0]);
                    right = right.slice(1);
                }
            } else if (left.length > 0) {
                result.push(left[0]);
                left = left.slice(1);
            } else if (right.length > 0) {
                result.push(right[0]);
                right = right.slice(1);
            }
        }
        return result;
    }
})();
(function() {
    function quote(string) {
        return "\"" + string + "\"";
    }

    function str(key, holder) {
        var value = holder[key].valueOf();
        switch (typeof value) {
            case "string":
                return quote(value);
            case "number":
            case "boolean":
            case "null":
                return String(value);
            case "object":
                if (!value) {
                    return "null";
                }
                if (Array.isArray(value)) {
                    return "[" + value.map((_, i) => str(i, value) || "null").join(",") + "]";
                }
                return "{...}";
        }
    }
    Array.prototype.toString = function(hint) {
        return !this.length ? '' : str("", { "": this });
    };
}());
return;