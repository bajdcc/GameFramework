isNaN = function(value) {
    var n = Number(value);
    return n !== n;
};
sys.builtin(isNaN);
Number.isNaN = function(value) {
    return typeof value === "number" && isNaN(value);
};
isFinite = function(value) {
    if (value !== value || value === Infinity || value === -Infinity) {
        return false;
    }
    return true;
};
sys.builtin(isFinite);
Number.isFinite = function(value) {
    return typeof value === "number" && isFinite(value);
};
eval = sys.eval;
sys.builtin(eval);