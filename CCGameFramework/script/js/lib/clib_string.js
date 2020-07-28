String.prototype.charAt = function(n) {
    return this[n] || "";
};
String.prototype.charCodeAt = function(n) {
    if (!this[n])
        return NaN;
    return this[n].charCode;
};
String.prototype.slice = function(begin, end) {
    end = (typeof end !== 'undefined') ? end : this.length;
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
            cloned[i] = this[start + i];
        }
    }
    return cloned.join('');
};
String.prototype.split = function(re) {
    var t = [],
        res = [],
        idx = 0;
    this.replace(re, (a, b) => {
        t.push([b, b + a.length]);
        return '';
    });
    for (var i in t) {
        var k = t[i];
        if (idx != k[0]) {
            res.push(this.substring(idx, k[0]));
        }
        idx = k[1];
    }
    if (idx != this.length) {
        res.push(this.substring(idx));
    }
    return res;
};
return;