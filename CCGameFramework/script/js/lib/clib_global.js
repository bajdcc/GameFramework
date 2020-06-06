isNaN = function(value) {
    var n = Number(value);
    return n !== n;
};
sys.builtin(isNaN);