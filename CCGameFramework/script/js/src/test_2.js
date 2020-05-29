function A(a, b) {
    this.a = a;
    this.b = b;
    this.c = function () {
        console.log(console.trace());
        return this.a + ' ' + this.b;
    };
}

var d = new A('123', 12.3);
console.log(d.c());

var obj = {0: 'a', 1: 'b', length: 2};
console.log([].slice.call(obj, 0).slice(1));

console.log([1].concat(1, [2], 3));
console.log([1, 2, 3, 4].map(x => x + 1).filter(x => x % 2 === 0));
console.log([1, 2, 3, 4].reduce((a, b) => a + b));
console.log([1, 2, 3, 4].reduce((a, b) => a + b, 1));
console.log([...[1, 2], ...[3, 4]].fill(5));
console.log.bind(null, 1, 2, 3)();
console.log(Array.prototype.concat.bind(0, 1, 2, 3)());
for (var i in {a: 1, ...{b: 2}}) console.log(i);
for (var i in [1,2]) console.log(i);