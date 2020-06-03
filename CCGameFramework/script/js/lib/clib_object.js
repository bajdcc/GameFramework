Object.prototype.keys = function(o) {
    var keys = [];
    for (var k in o) {
        if (o.hasOwnProperty(k))
            keys.push(k);
    }
    return keys;
};
Object.prototype.values = function(o) {
    var values = [];
    for (var k in o) {
        if (o.hasOwnProperty(k))
            values.push(this[k]);
    }
    return values;
};
return;