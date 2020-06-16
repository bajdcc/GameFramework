Buffer.prototype.toJSON = function() {
    console.log(this.length, this[1])
    var arr = this.slice(0, 100);
    if (this.length > 100)
        arr.push("...");
    return {
        type: 'Buffer',
        data: arr
    };
};
atob = function(a) {
    return Buffer.from(a, 'base64').toString();
};
sys.builtin(atob);
btoa = function(a) {
    return Buffer.from(a).toString('base64');
};
sys.builtin(btoa);
return;