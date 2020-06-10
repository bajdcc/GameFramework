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
return;