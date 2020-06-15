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
return;