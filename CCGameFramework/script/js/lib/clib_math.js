Math = {};
sys.builtin(Math);
Math.max = function(a, b) {
    return a > b ? a : b;
};
Math.min = function(a, b) {
    return a < b ? a : b;
};
return;