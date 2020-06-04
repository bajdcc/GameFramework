Object.prototype.keys = function(o) {
    var keys = [];
    if (typeof o === 'string') {
        for (var k in o) {
            keys.push(k);
        }
    } else {
        for (var k in o) {
            if (o.hasOwnProperty(k))
                keys.push(k);
        }
    }
    return keys;
};
Object.prototype.values = function(o) {
    var values = [];
    if (typeof o === 'string') {
        for (var k in o) {
            values.push(o[k]);
        }
    } else {
        for (var k in o) {
            if (o.hasOwnProperty(k))
                values.push(o[k]);
        }
    }
    return values;
};
return;