Error.prototype.toString = function () {
    return this.name + ": " + this.message + ", stacktrace:\n" + this.stack;
};
ReferenceError = function (message) {
    this.name = "ReferenceError";
    this.message = message || "";
    this.stack = console.trace();
};
ReferenceError.prototype = new Error();
sys.builtin(ReferenceError);
SyntaxError = function (message) {
    this.name = "SyntaxError";
    this.message = message || "";
    this.stack = console.trace();
};
SyntaxError.prototype = new Error();
sys.builtin(SyntaxError);
return;