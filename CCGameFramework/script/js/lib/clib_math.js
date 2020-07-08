Math = {};
sys.builtin(Math);
Math.max = function(a, b) {
    return sys.math(201, a, b);
};
Math.min = function(a, b) {
    return sys.math(202, a, b);
};
Math.floor = function(a, b) {
    return sys.math(101, a, b);
};
Math.ceil = function(a, b) {
    return sys.math(102, a, b);
};
Math.round = function(a, b) {
    return sys.math(103, a, b);
};
return;